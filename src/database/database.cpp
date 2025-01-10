#include "database.h"
#include <sstream>
#include <iostream>
#include <cstring>

Database::Database(const std::string& host, const std::string& user,
                  const std::string& password, const std::string& database)
    : host_(host), user_(user), password_(password), database_(database), conn_(nullptr) {
}

Database::~Database() {
    if (conn_) {
        mysql_close(conn_);
    }
}

bool Database::connect() {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        std::cerr << "mysql_init() failed" << std::endl;
        return false;
    }

    if (!mysql_real_connect(conn_, host_.c_str(), user_.c_str(),
                          password_.c_str(), database_.c_str(), 0, nullptr, 0)) {
        std::cerr << "mysql_real_connect() failed: " 
                  << mysql_error(conn_) << std::endl;
        return false;
    }

    return true;
}

bool Database::initTables() {
    const char* create_devices_table = 
        "CREATE TABLE IF NOT EXISTS devices ("
        "device_id VARCHAR(50) PRIMARY KEY,"
        "location_id VARCHAR(50) NOT NULL,"
        "device_type VARCHAR(50) NOT NULL,"
        "status INT NOT NULL DEFAULT 0,"
        "register_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "last_heartbeat TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "data_interval INT NOT NULL DEFAULT 60,"
        "heartbeat_interval INT NOT NULL DEFAULT 30,"
        "alert_temp_min DOUBLE NOT NULL DEFAULT 18.0,"
        "alert_temp_max DOUBLE NOT NULL DEFAULT 26.0,"
        "alert_hum_min DOUBLE NOT NULL DEFAULT 40.0,"
        "alert_hum_max DOUBLE NOT NULL DEFAULT 70.0,"
        "alert_co2_max DOUBLE NOT NULL DEFAULT 1000.0,"
        "alert_pm25_max DOUBLE NOT NULL DEFAULT 75.0"
        ")";

    const char* create_sensor_data_table = 
        "CREATE TABLE IF NOT EXISTS sensor_data ("
        "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
        "device_id VARCHAR(50) NOT NULL,"
        "temperature DOUBLE NOT NULL,"
        "humidity DOUBLE NOT NULL,"
        "co2 DOUBLE NOT NULL,"
        "pm25 DOUBLE NOT NULL,"
        "score DOUBLE NOT NULL,"
        "timestamp TIMESTAMP NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "INDEX idx_device_time (device_id, timestamp)"
        ")";

    return (mysql_query(conn_, create_devices_table) == 0 &&
            mysql_query(conn_, create_sensor_data_table) == 0);
}

bool Database::saveSensorData(const SensorData& data) {
    char query[1024];
    snprintf(query, sizeof(query),
        "INSERT INTO sensor_data (device_id, temperature, humidity, co2, pm25, score, timestamp) "
        "VALUES ('%s', %.2f, %.2f, %.2f, %.2f, %.2f, FROM_UNIXTIME(%ld))",
        data.device_id.c_str(), data.temperature, data.humidity,
        data.co2, data.pm25, data.score, data.timestamp);
    
    return mysql_query(conn_, query) == 0;
}

std::vector<SensorData> Database::getHistoryData(const std::string& device_id,
                                               time_t start_time,
                                               time_t end_time) {
    std::vector<SensorData> result;
    
    char query[512];
    snprintf(query, sizeof(query),
        "SELECT device_id, temperature, humidity, co2, pm25, score, "
        "UNIX_TIMESTAMP(timestamp) as ts "
        "FROM sensor_data "
        "WHERE device_id = '%s' AND timestamp BETWEEN FROM_UNIXTIME(%ld) AND FROM_UNIXTIME(%ld) "
        "ORDER BY timestamp",
        device_id.c_str(), start_time, end_time);
    
    if (mysql_query(conn_, query) == 0) {
        MYSQL_RES* res = mysql_store_result(conn_);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                SensorData data;
                data.device_id = row[0];
                data.temperature = std::stod(row[1]);
                data.humidity = std::stod(row[2]);
                data.co2 = std::stod(row[3]);
                data.pm25 = std::stod(row[4]);
                data.score = std::stod(row[5]);
                data.timestamp = std::stol(row[6]);
                result.push_back(data);
            }
            mysql_free_result(res);
        }
    }
    
    return result;
}

bool Database::saveDevice(const std::string& device_id,
                        const std::string& location_id,
                        const std::string& device_type) {
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO devices (device_id, location_id, device_type) "
        "VALUES ('%s', '%s', '%s')",
        device_id.c_str(), location_id.c_str(), device_type.c_str());
    
    return mysql_query(conn_, query) == 0;
}

bool Database::updateDeviceConfig(const std::string& device_id,
                                const DeviceInfo::Config& config) {
    char query[1024];
    snprintf(query, sizeof(query),
        "UPDATE devices SET "
        "data_interval = %d, "
        "heartbeat_interval = %d, "
        "alert_temp_min = %.2f, "
        "alert_temp_max = %.2f, "
        "alert_hum_min = %.2f, "
        "alert_hum_max = %.2f, "
        "alert_co2_max = %.2f, "
        "alert_pm25_max = %.2f "
        "WHERE device_id = '%s'",
        config.data_interval,
        config.heartbeat_interval,
        config.alert_temp_min,
        config.alert_temp_max,
        config.alert_hum_min,
        config.alert_hum_max,
        config.alert_co2_max,
        config.alert_pm25_max,
        device_id.c_str());
    
    return mysql_query(conn_, query) == 0;
}

std::vector<SensorData> Database::getSensorData(const std::string& device_id, 
                                              time_t start_time, 
                                              time_t end_time) {
    std::vector<SensorData> result;
    
    std::string query = "SELECT timestamp, temperature, humidity, co2, pm25 "
                       "FROM sensor_data "
                       "WHERE device_id = ? AND timestamp BETWEEN ? AND ? "
                       "ORDER BY timestamp ASC";
                       
    MYSQL_STMT* stmt = mysql_stmt_init(conn_);
    if (!stmt) {
        return result;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        mysql_stmt_close(stmt);
        return result;
    }
    
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)device_id.c_str();
    bind[0].buffer_length = device_id.length();
    
    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = (void*)&start_time;
    
    bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[2].buffer = (void*)&end_time;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        mysql_stmt_close(stmt);
        return result;
    }
    
    if (mysql_stmt_execute(stmt)) {
        mysql_stmt_close(stmt);
        return result;
    }
    
    MYSQL_BIND result_bind[5];
    memset(result_bind, 0, sizeof(result_bind));
    
    SensorData data;
    result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result_bind[0].buffer = &data.timestamp;
    
    result_bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[1].buffer = &data.temperature;
    
    result_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[2].buffer = &data.humidity;
    
    result_bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[3].buffer = &data.co2;
    
    result_bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[4].buffer = &data.pm25;
    
    if (mysql_stmt_bind_result(stmt, result_bind)) {
        mysql_stmt_close(stmt);
        return result;
    }
    
    while (!mysql_stmt_fetch(stmt)) {
        result.push_back(data);
    }
    
    mysql_stmt_close(stmt);
    return result;
}

bool Database::insertSensorData(const std::string& device_id, const SensorData& data) {
    char query[1024];
    snprintf(query, sizeof(query),
        "INSERT INTO sensor_data (device_id, temperature, humidity, co2, pm25, timestamp) "
        "VALUES ('%s', %.2f, %.2f, %.2f, %.2f, FROM_UNIXTIME(%ld))",
        device_id.c_str(), data.temperature, data.humidity,
        data.co2, data.pm25, data.timestamp);
    
    return mysql_query(conn_, query) == 0;
} 