#include "device_manager.h"
#include <algorithm>
#include <ctime>
#include <iostream>

bool DeviceManager::registerDevice(const std::string& device_id,
                                 const std::string& location_id,
                                 const std::string& device_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (devices_.find(device_id) != devices_.end()) {
        return false;
    }
    
    auto device = std::make_shared<DeviceInfo>();
    device->device_id = device_id;
    device->location_id = location_id;
    device->device_type = device_type;
    device->status = DeviceStatus::ONLINE;
    device->register_time = std::time(nullptr);
    device->last_heartbeat = device->register_time;
    device->last_seen = device->register_time;
    
    devices_[device_id] = device;
    return true;
}

void DeviceManager::updateDeviceStatus(const std::string& device_id, DeviceStatus status) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto it = devices_.find(device_id); it != devices_.end()) {
        it->second->status = status;
    }
}

void DeviceManager::updateHeartbeat(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto it = devices_.find(device_id); it != devices_.end()) {
        it->second->last_heartbeat = std::time(nullptr);
        if (it->second->status == DeviceStatus::OFFLINE) {
            it->second->status = DeviceStatus::ONLINE;
        }
    }
}

void DeviceManager::addSensorData(const std::string& device_id, const SensorData& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto it = devices_.find(device_id); it != devices_.end()) {
        it->second->recent_data.push_back(data);
        if (it->second->recent_data.size() > 100) {
            it->second->recent_data.erase(it->second->recent_data.begin());
        }
    }
}

std::shared_ptr<DeviceInfo> DeviceManager::getDeviceInfo(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto it = devices_.find(device_id); it != devices_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<DeviceInfo>> DeviceManager::getAllDevices() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<DeviceInfo>> result;
    result.reserve(devices_.size());
    
    for (const auto& [_, device] : devices_) {
        result.push_back(device);
    }
    return result;
}

std::vector<std::shared_ptr<DeviceInfo>> DeviceManager::getDevicesByLocation(
    const std::string& location_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::shared_ptr<DeviceInfo>> result;
    
    for (const auto& [_, device] : devices_) {
        if (device->location_id == location_id) {
            result.push_back(device);
        }
    }
    return result;
}

void DeviceManager::checkDevicesStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    time_t now = std::time(nullptr);
    
    for (auto& device : devices_) {
        // 如果超过10秒没有收到心跳，则将设备设置为离线
        if (now - device.second->last_heartbeat > 10) {
            if (device.second->status != DeviceStatus::OFFLINE) {
                std::cout << "[DeviceManager] Device " << device.first 
                         << " is offline (no heartbeat for " 
                         << (now - device.second->last_heartbeat) 
                         << " seconds)" << std::endl;
                device.second->status = DeviceStatus::OFFLINE;
            }
        } else {
            if (device.second->status != DeviceStatus::ONLINE) {
                std::cout << "[DeviceManager] Device " << device.first 
                         << " is online" << std::endl;
                device.second->status = DeviceStatus::ONLINE;
            }
        }
    }
}

bool DeviceManager::updateDeviceConfig(const std::string& device_id,
                                     const DeviceInfo::Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto it = devices_.find(device_id); it != devices_.end()) {
        it->second->config = config;
        return true;
    }
    return false;
}

bool DeviceManager::unregisterDevice(const std::string& device_id) {
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
        devices_.erase(it);
        return true;
    }
    return false;
} 