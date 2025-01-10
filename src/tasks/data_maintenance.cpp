#include "data_maintenance.h"

DataMaintenanceTask& DataMaintenanceTask::getInstance() {
    static DataMaintenanceTask instance;
    return instance;
}

DataMaintenanceTask::~DataMaintenanceTask() {
    stop();
}

void DataMaintenanceTask::start() {
    running_ = true;
    worker_ = std::thread(&DataMaintenanceTask::run, this);
}

void DataMaintenanceTask::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void DataMaintenanceTask::run() {
    while (running_) {
        time_t now = time(nullptr);
        struct tm* tm = localtime(&now);
        
        // 每小时执行一次数据聚合
        if (tm->tm_min == 0) {
            Database::getInstance().aggregateHourlyData();
        }
        
        // 每天凌晨执行一次数据聚合和清理
        if (tm->tm_hour == 0 && tm->tm_min == 0) {
            Database::getInstance().aggregateDailyData();
            Database::getInstance().cleanupOldData();
        }
        
        // 休眠到下一分钟
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
} 