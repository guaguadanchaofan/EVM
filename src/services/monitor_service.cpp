#include "monitor_service.h"

MonitorService::MonitorService(Database& db)
    : database_(db)
{
}

void MonitorService::processSensorData(SensorData& data) {
    // 处理环境数据
    environment_service_.processEnvironmentData(data);
    
    // 存储数据
    database_.insertSensorData(data);
} 