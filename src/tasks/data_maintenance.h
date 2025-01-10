#pragma once
#include <thread>
#include <atomic>
#include "../database/database.h"

class DataMaintenanceTask {
public:
    static DataMaintenanceTask& getInstance();
    void start();
    void stop();

private:
    DataMaintenanceTask() = default;
    ~DataMaintenanceTask();
    void run();
    
    std::atomic<bool> running_{false};
    std::thread worker_;
}; 