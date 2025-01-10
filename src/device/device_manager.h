#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../models/sensor_data.h"

// 设备状态
enum class DeviceStatus {
    ONLINE,     // 在线
    OFFLINE,    // 离线
    FAULT,      // 故障
    MAINTENANCE // 维护中
};

// 设备信息
struct DeviceInfo {
    std::string device_id;
    std::string location_id;
    std::string device_type;
    DeviceStatus status;
    time_t last_heartbeat;
    time_t last_seen;
    time_t register_time;
    std::vector<SensorData> recent_data;  // 最近的数据缓存
    
    // 设备配置
    struct Config {
        int data_interval;     // 数据上报间隔(秒)
        int heartbeat_interval;// 心跳间隔(秒)
        double alert_temp_min; // 温度下限
        double alert_temp_max; // 温度上限
        double alert_hum_min;  // 湿度下限
        double alert_hum_max;  // 湿度上限
        double alert_co2_max;  // CO2上限
        double alert_pm25_max; // PM2.5上限
    } config;
};

class DeviceManager {
public:
    static DeviceManager& getInstance() {
        static DeviceManager instance;
        return instance;
    }

    // 设备注册
    bool registerDevice(const std::string& device_id, 
                       const std::string& location_id,
                       const std::string& device_type);
    
    // 更新设备状态
    void updateDeviceStatus(const std::string& device_id, DeviceStatus status);
    
    // 更新设备心跳
    void updateHeartbeat(const std::string& device_id);
    
    // 添加传感器数据
    void addSensorData(const std::string& device_id, const SensorData& data);
    
    // 获取设备信息
    std::shared_ptr<DeviceInfo> getDeviceInfo(const std::string& device_id);
    
    // 获取所有设备
    std::vector<std::shared_ptr<DeviceInfo>> getAllDevices();
    
    // 获取指定位置的设备
    std::vector<std::shared_ptr<DeviceInfo>> getDevicesByLocation(const std::string& location_id);
    
    // 检查设备状态
    void checkDevicesStatus();
    
    // 更新设备配置
    bool updateDeviceConfig(const std::string& device_id, const DeviceInfo::Config& config);

    bool unregisterDevice(const std::string& device_id);

    const std::map<std::string, std::shared_ptr<DeviceInfo>>& getDevices() const { return devices_; }

private:
    DeviceManager() = default;
    ~DeviceManager() = default;
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    std::map<std::string, std::shared_ptr<DeviceInfo>> devices_;
    mutable std::mutex mutex_;
}; 