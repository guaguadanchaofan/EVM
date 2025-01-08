#include "device.h"

Device::Device(const std::string &name, const std::string &type, float temperature, float humidity, float noise, float airQuality, float light, int online)
{
    this->name = name;
    this->type = type;
    this->temperature = temperature;
    this->humidity = humidity;
    this->noise = noise;
    this->airQuality = airQuality;
    this->light = light;
    this->online = online;
}

std::string Device::getName() const
{
    return this->name;
}

std::string Device::getType() const
{
    return this->type;
}

float Device::getTemperature() const
{
    return this->temperature;
}

float Device::getHumidity() const
{
    return this->humidity;
;
}

float Device::getNoise() const
{
    return this->noise;
}

float Device::getAirQuality() const
{
    return this->airQuality;
}

float Device::getLight() const
{
    return this->light;
}

bool Device::isOnline() const
{
    return this->online;
}

