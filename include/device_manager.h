#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm> 
#include "device.h"

class DeviceManager {
public:
    void addDevice(const std::shared_ptr<Device>& device) {
        devices.push_back(device);
    }

    void removeDevice(const std::string& name) {
        devices.erase(std::remove_if(devices.begin(), devices.end(),
            [&name](const std::shared_ptr<Device>& device) {
                return device->getName() == name;
            }), devices.end());
    }

    std::shared_ptr<Device> getDevice(const std::string& name) const {
        if(devices.empty()) {
            return nullptr;
        }
        for (const auto& device : devices) {
            if (device->getName() == name) {
                return device;
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Device>> getAllDevices() const {
        return devices;
    }

private:
    std::vector<std::shared_ptr<Device>> devices;
};
