#pragma once
#include <mysql/mysql.h>
#include <memory>
#include <string>
#include <vector>
#include "../models/sensor_data.h"

class Database {
public:
    static Database& getInstance();
    
    // 数据插入
    bool insertSensorData(const SensorData& data);
    bool batchInsertSensorData(const std::vector<SensorData>& data);
    
    // 数据查询
    std::vector<SensorData> getHistoryData(const std::string& device_id, 
                                         time_t start_time, 
                                         time_t end_time);
    
    // 数据维护
    bool aggregateHourlyData();
    bool aggregateDailyData();
    bool cleanupOldData();
    
    // 将查询方法移到 public 区域
    std::vector<SensorData> queryRealtimeData(const std::string& device_id,
                                            time_t start_time,
                                            time_t end_time);
    std::vector<SensorData> queryHourlyData(const std::string& device_id,
                                          time_t start_time,
                                          time_t end_time);
    std::vector<SensorData> queryDailyData(const std::string& device_id,
                                         time_t start_time,
                                         time_t end_time);

private:
    Database(const std::string& host, const std::string& user,
            const std::string& password, const std::string& database);
    ~Database();
    
    // 禁止拷贝
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool initTables();
    MYSQL* conn_;
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    
    static constexpr int REALTIME_DATA_RETENTION_HOURS = 24;
    static constexpr int HOURLY_DATA_RETENTION_DAYS = 30;
    static constexpr int DAILY_DATA_RETENTION_DAYS = 365;
    static constexpr int MAX_BATCH_SIZE = 1000;
}; 