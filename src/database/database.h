#pragma once
#include <mysql/mysql.h>
#include <memory>
#include <string>
#include <vector>
#include "../models/sensor_data.h"
#include "../device/device_manager.h"

class Database {
public:
    static Database& getInstance() {
        static Database instance("localhost", "root", "password", "evm_db");
        return instance;
    }
    
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    Database(const std::string& host, const std::string& user,
            const std::string& password, const std::string& database);
    ~Database();

    bool connect();
    
    // 设备相关操作
    bool saveDevice(const std::string& device_id, 
                   const std::string& location_id,
                   const std::string& device_type);
    bool removeDevice(const std::string& device_id);
    bool updateDeviceStatus(const std::string& device_id, int status);
    bool updateDeviceConfig(const std::string& device_id, const DeviceInfo::Config& config);
    
    // 传感器数据操作
    bool saveSensorData(const SensorData& data);
    std::vector<SensorData> getHistoryData(const std::string& device_id,
                                         time_t start_time,
                                         time_t end_time);
    bool getLatestData(const std::string& device_id, SensorData& data);

    // 插入传感器数据
    bool insertSensorData(const std::string& device_id, const SensorData& data);
    
    // 查询历史数据
    std::vector<SensorData> getSensorData(const std::string& device_id, 
                                        time_t start_time, 
                                        time_t end_time);

private:
    bool initTables();  // 初始化数据库表
    
    MYSQL* conn_;
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
}; 