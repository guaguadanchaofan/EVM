#include <iostream>
#include "service.h"
#include <thread>
#include "device.h"

void run(Service *myService)
{
    myService->start();
}

int main()
{
    Service *myService = new Service("service", 12345);
    std::thread mythread(run, myService);
    // 主线程可以执行其他任务
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        auto device = myService->getDeviceData("device");
        if (device != nullptr)
        {
            std::cout << "FOUND!" << std::endl;
            std::string name = device -> getName();
            std::cout << "name: " << name << std::endl;
            std::string type = device -> getType();
            std::cout << "type: " << type << std::endl;
            float Temperature = device -> getTemperature();
            std::cout << "Temperature: " << Temperature << std::endl;
            float Humidity = device -> getHumidity();
            std::cout << "Humidity: " << Humidity << std::endl;
            float Noise = device -> getNoise();
            std::cout << "Noise: " << Noise << std::endl;
            float AirQuality = device -> getAirQuality();
            std::cout << "AirQuality: " << AirQuality << std::endl;
            float Light = device -> getLight();
            std::cout << "Light: " << Light << std::endl;
            bool Online = device -> isOnline();
            std::cout << "Online: " << Online << std::endl;
        }
        else
        {
            std::cout << "NOT FOUND!" << std::endl;
        }
    }
    return 0;
}