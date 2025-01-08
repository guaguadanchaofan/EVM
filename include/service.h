#pragma once
#include <string>
#include "device_manager.h"
#include "net.h"
class Service {
public:
    Service(const std::string& name , int port): network(port) {
        deviceManager = new DeviceManager();
        network.setDeviceManager(deviceManager);
    }
    std::shared_ptr<Device> getDeviceData(std::string name)
    {
        return deviceManager->getDevice(name);
    }
    void start();
    void stop();
private:
    DeviceManager *deviceManager;
    Network network;
};

