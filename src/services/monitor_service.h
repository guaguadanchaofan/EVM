#pragma once
#include "../models/sensor_data.h"
#include "../database/database.h"
#include "environment_service.h"

class MonitorService {
public:
    MonitorService(Database& db);
    void processSensorData(SensorData& data);
    
private:
    Database& database_;
    EnvironmentService environment_service_;
};