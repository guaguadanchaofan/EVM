#include "database.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <ctime>
#include <algorithm>

Database& Database::getInstance() {
    static Database instance("localhost", "monitor", "123456", "evm_db");
    return instance;
}

Database::Database(const std::string& host, const std::string& user,
                  const std::string& password, const std::string& database)
    : host_(host)
    , user_(user)
    , password_(password)
    , database_(database)
    , conn_(nullptr) {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        throw std::runtime_error("mysql_init() failed");
    }

    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(),
                          password.c_str(), database.c_str(), 0, nullptr, 0)) {
        throw std::runtime_error(mysql_error(conn_));
    }

    initTables();
}

Database::~Database() {
    if (conn_) {
        mysql_close(conn_);
    }
}

bool Database::initTables() {
    // 实时数据表
    const char* create_realtime_table = R"(
        CREATE TABLE IF NOT EXISTS sensor_data_realtime (
            id BIGINT AUTO_INCREMENT PRIMARY KEY,
            device_id VARCHAR(50) NOT NULL,
            timestamp TIMESTAMP NOT NULL,
            temperature DOUBLE NOT NULL,
            humidity DOUBLE NOT NULL,
            co2 DOUBLE NOT NULL,
            pm25 DOUBLE NOT NULL,
            noise DOUBLE NOT NULL,
            light DOUBLE NOT NULL,
            area VARCHAR(50) NOT NULL,
            area_type INT NOT NULL,
            INDEX idx_device_time (device_id, timestamp)
        )
    )";

    // 小时聚合数据表
    const char* create_hourly_table = R"(
        CREATE TABLE IF NOT EXISTS sensor_data_hourly (
            id BIGINT AUTO_INCREMENT PRIMARY KEY,
            device_id VARCHAR(50) NOT NULL,
            hour_timestamp TIMESTAMP NOT NULL,
            avg_temperature DOUBLE NOT NULL,
            avg_humidity DOUBLE NOT NULL,
            avg_co2 DOUBLE NOT NULL,
            avg_pm25 DOUBLE NOT NULL,
            avg_noise DOUBLE NOT NULL,
            avg_light DOUBLE NOT NULL,
            max_temperature DOUBLE NOT NULL,
            min_temperature DOUBLE NOT NULL,
            samples_count INT NOT NULL,
            area VARCHAR(50) NOT NULL,
            area_type INT NOT NULL,
            INDEX idx_device_hour (device_id, hour_timestamp)
        )
    )";

    // 每日聚合数据表
    const char* create_daily_table = R"(
        CREATE TABLE IF NOT EXISTS sensor_data_daily (
            id BIGINT AUTO_INCREMENT PRIMARY KEY,
            device_id VARCHAR(50) NOT NULL,
            date_timestamp TIMESTAMP NOT NULL,
            avg_temperature DOUBLE NOT NULL,
            avg_humidity DOUBLE NOT NULL,
            avg_co2 DOUBLE NOT NULL,
            avg_pm25 DOUBLE NOT NULL,
            avg_noise DOUBLE NOT NULL,
            avg_light DOUBLE NOT NULL,
            max_temperature DOUBLE NOT NULL,
            min_temperature DOUBLE NOT NULL,
            samples_count INT NOT NULL,
            area VARCHAR(50) NOT NULL,
            area_type INT NOT NULL,
            INDEX idx_device_date (device_id, date_timestamp)
        )
    )";

    return mysql_query(conn_, create_realtime_table) == 0 &&
           mysql_query(conn_, create_hourly_table) == 0 &&
           mysql_query(conn_, create_daily_table) == 0;
}

bool Database::insertSensorData(const SensorData& data) {
    std::stringstream sql;
    sql << "INSERT INTO sensor_data_realtime "
        << "(device_id, timestamp, temperature, humidity, co2, pm25, noise, light, area, area_type) "
        << "VALUES ('" << data.device_id << "', "
        << "FROM_UNIXTIME(" << data.timestamp << "), "
        << data.temperature << ", "
        << data.humidity << ", "
        << data.co2 << ", "
        << data.pm25 << ", "
        << data.noise << ", "
        << data.light << ", '"
        << data.area << "', "
        << static_cast<int>(data.area_type) << ")";
        
    bool success = mysql_query(conn_, sql.str().c_str()) == 0;
    
    // 如果插入成功，检查是否需要进行数据聚合
    if (success) {
        time_t now = time(nullptr);
        static time_t last_hourly_aggregate = 0;
        static time_t last_daily_aggregate = 0;
        
        // 每小时聚合一次数据
        if (now - last_hourly_aggregate >= 3600) {
            aggregateHourlyData();
            last_hourly_aggregate = now;
        }
        
        // 每天凌晨聚合一次数据
        struct tm* tm = localtime(&now);
        if (tm->tm_hour == 0 && tm->tm_min == 0 && now - last_daily_aggregate >= 86400) {
            aggregateDailyData();
            last_daily_aggregate = now;
        }
    }
    
    return success;
}

std::vector<SensorData> Database::getHistoryData(const std::string& device_id, 
                                               time_t start_time, 
                                               time_t end_time) {
    time_t now = time(nullptr);
    time_t oneDay = 24 * 3600;
    time_t thirtyDays = 30 * oneDay;
    
    std::vector<SensorData> result;
    
    if (now - start_time <= oneDay) {
        result = queryRealtimeData(device_id, start_time, end_time);
    } else if (now - start_time <= thirtyDays) {
        result = queryHourlyData(device_id, start_time, end_time);
    } else {
        result = queryDailyData(device_id, start_time, end_time);
    }
    
    std::sort(result.begin(), result.end(), 
        [](const SensorData& a, const SensorData& b) {
            return a.timestamp < b.timestamp;
        });
    
    return result;
}

bool Database::aggregateHourlyData() {
    time_t now = time(nullptr);
    time_t oneHourAgo = now - 3600;
    
    std::stringstream sql;
    sql << "INSERT INTO sensor_data_hourly "
        << "(device_id, hour_timestamp, avg_temperature, avg_humidity, avg_co2, "
        << "avg_pm25, avg_noise, avg_light, max_temperature, min_temperature, "
        << "samples_count, area, area_type) "
        << "SELECT device_id, "
        << "FROM_UNIXTIME(UNIX_TIMESTAMP(timestamp) - MOD(UNIX_TIMESTAMP(timestamp), 3600)), "
        << "AVG(temperature), AVG(humidity), AVG(co2), "
        << "AVG(pm25), AVG(noise), AVG(light), "
        << "MAX(temperature), MIN(temperature), COUNT(*), "
        << "MAX(area), MAX(area_type) "
        << "FROM sensor_data_realtime "
        << "WHERE timestamp >= FROM_UNIXTIME(" << oneHourAgo << ") "
        << "AND timestamp < FROM_UNIXTIME(" << now << ") "
        << "GROUP BY device_id, FROM_UNIXTIME(UNIX_TIMESTAMP(timestamp) - MOD(UNIX_TIMESTAMP(timestamp), 3600))";
    
    return mysql_query(conn_, sql.str().c_str()) == 0;
}

bool Database::aggregateDailyData() {
    time_t now = time(nullptr);
    time_t oneDayAgo = now - 24 * 3600;
    
    std::stringstream sql;
    sql << "INSERT INTO sensor_data_daily "
        << "(device_id, date_timestamp, avg_temperature, avg_humidity, avg_co2, "
        << "avg_pm25, avg_noise, avg_light, max_temperature, min_temperature, "
        << "samples_count, area, area_type) "
        << "SELECT device_id, "
        << "DATE(hour_timestamp), "
        << "AVG(avg_temperature), AVG(avg_humidity), AVG(avg_co2), "
        << "AVG(avg_pm25), AVG(avg_noise), AVG(avg_light), "
        << "MAX(max_temperature), MIN(min_temperature), SUM(samples_count), "
        << "MAX(area), MAX(area_type) "
        << "FROM sensor_data_hourly "
        << "WHERE hour_timestamp >= FROM_UNIXTIME(" << oneDayAgo << ") "
        << "AND hour_timestamp < FROM_UNIXTIME(" << now << ") "
        << "GROUP BY device_id, DATE(hour_timestamp)";
    
    return mysql_query(conn_, sql.str().c_str()) == 0;
}

bool Database::cleanupOldData() {
    time_t now = time(nullptr);
    
    // 清理实时数据
    std::stringstream sql1;
    sql1 << "DELETE FROM sensor_data_realtime WHERE timestamp < FROM_UNIXTIME("
         << (now - REALTIME_DATA_RETENTION_HOURS * 3600) << ")";
    
    // 清理小时数据
    std::stringstream sql2;
    sql2 << "DELETE FROM sensor_data_hourly WHERE hour_timestamp < FROM_UNIXTIME("
         << (now - HOURLY_DATA_RETENTION_DAYS * 24 * 3600) << ")";
    
    // 清理每日数据
    std::stringstream sql3;
    sql3 << "DELETE FROM sensor_data_daily WHERE date_timestamp < DATE_SUB(CURDATE(), "
         << "INTERVAL " << DAILY_DATA_RETENTION_DAYS << " DAY)";
    
    return mysql_query(conn_, sql1.str().c_str()) == 0 &&
           mysql_query(conn_, sql2.str().c_str()) == 0 &&
           mysql_query(conn_, sql3.str().c_str()) == 0;
}

std::vector<SensorData> Database::queryRealtimeData(const std::string& device_id, 
                                                  time_t start_time, 
                                                  time_t end_time) {
    std::stringstream sql;
    // 获取当前时间
    time_t now = time(nullptr);
    // 计算查询的时间范围
    time_t query_start = now - (end_time - start_time);
    time_t query_end = now;
    
    sql << "SELECT UNIX_TIMESTAMP(timestamp) as ts, device_id, timestamp, "
        << "temperature, humidity, co2, pm25, noise, light, area, area_type "
        << "FROM sensor_data_realtime "
        << "WHERE device_id = '" << device_id << "' "
        << "AND timestamp BETWEEN FROM_UNIXTIME(" << query_start << ") "
        << "AND FROM_UNIXTIME(" << query_end << ") "
        << "ORDER BY timestamp ASC";
        
    std::cout << "Executing SQL: " << sql.str() << std::endl;
        
    std::vector<SensorData> result;
    if (mysql_query(conn_, sql.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn_);
        if (res) {
            MYSQL_ROW row;
            int row_count = 0;
            while ((row = mysql_fetch_row(res))) {
                SensorData data;
                data.timestamp = std::stoll(row[0]);  // ts
                data.device_id = row[1];              // device_id
                data.temperature = std::stof(row[3]); // temperature
                data.humidity = std::stof(row[4]);    // humidity
                data.co2 = std::stof(row[5]);        // co2
                data.pm25 = std::stof(row[6]);       // pm25
                data.noise = std::stof(row[7]);      // noise
                data.light = std::stof(row[8]);      // light
                data.area = row[9];                  // area
                data.area_type = static_cast<AreaType>(std::stoi(row[10])); // area_type
                result.push_back(data);
                row_count++;
            }
            mysql_free_result(res);
            std::cout << "Found " << row_count << " rows" << std::endl;
        }
    } else {
        std::cerr << "MySQL query error: " << mysql_error(conn_) << std::endl;
    }
    return result;
}

std::vector<SensorData> Database::queryHourlyData(const std::string& device_id, 
                                                time_t start_time, 
                                                time_t end_time) {
    std::stringstream sql;
    sql << "SELECT UNIX_TIMESTAMP(hour_timestamp) as ts, * FROM sensor_data_hourly "
        << "WHERE device_id = '" << device_id << "' "
        << "AND hour_timestamp BETWEEN FROM_UNIXTIME(" << start_time << ") "
        << "AND FROM_UNIXTIME(" << end_time << ") "
        << "ORDER BY hour_timestamp ASC";
        
    std::vector<SensorData> result;
    if (mysql_query(conn_, sql.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn_);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                SensorData data;
                data.timestamp = std::stoll(row[0]);  // 使用 UNIX_TIMESTAMP 的结果
                data.device_id = row[2];
                data.temperature = std::stof(row[4]);  // avg_temperature
                data.humidity = std::stof(row[5]);     // avg_humidity
                data.co2 = std::stof(row[6]);         // avg_co2
                data.pm25 = std::stof(row[7]);        // avg_pm25
                data.noise = std::stof(row[8]);       // avg_noise
                data.light = std::stof(row[9]);       // avg_light
                data.area = row[13];
                data.area_type = static_cast<AreaType>(std::stoi(row[14]));
                
                // 设置聚合数据标记
                data.has_aggregated_data = true;
                data.is_hourly = true;
                data.has_min_max = true;
                data.max_temperature = std::stof(row[10]);  // max_temperature
                data.min_temperature = std::stof(row[11]);  // min_temperature
                data.samples_count = std::stoi(row[12]);    // samples_count
                
                result.push_back(data);
            }
            mysql_free_result(res);
        }
    }
    return result;
}

std::vector<SensorData> Database::queryDailyData(const std::string& device_id, 
                                               time_t start_time, 
                                               time_t end_time) {
    std::stringstream sql;
    sql << "SELECT UNIX_TIMESTAMP(date_timestamp) as ts, * FROM sensor_data_daily "
        << "WHERE device_id = '" << device_id << "' "
        << "AND date_timestamp BETWEEN FROM_UNIXTIME(" << start_time << ") "
        << "AND FROM_UNIXTIME(" << end_time << ") "
        << "ORDER BY date_timestamp ASC";
        
    std::vector<SensorData> result;
    if (mysql_query(conn_, sql.str().c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn_);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                SensorData data;
                data.timestamp = std::stoll(row[0]);  // 使用 UNIX_TIMESTAMP 的结果
                data.device_id = row[2];
                data.temperature = std::stof(row[4]);  // avg_temperature
                data.humidity = std::stof(row[5]);     // avg_humidity
                data.co2 = std::stof(row[6]);         // avg_co2
                data.pm25 = std::stof(row[7]);        // avg_pm25
                data.noise = std::stof(row[8]);       // avg_noise
                data.light = std::stof(row[9]);       // avg_light
                data.area = row[13];
                data.area_type = static_cast<AreaType>(std::stoi(row[14]));
                
                // 设置聚合数据标记
                data.has_aggregated_data = true;
                data.is_hourly = false;  // 这是每日数据
                data.has_min_max = true;
                data.max_temperature = std::stof(row[10]);
                data.min_temperature = std::stof(row[11]);
                data.samples_count = std::stoi(row[12]);
                
                result.push_back(data);
            }
            mysql_free_result(res);
        }
    }
    return result;
} 