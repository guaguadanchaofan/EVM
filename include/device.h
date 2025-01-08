#pragma once
#include <string>

class Device {
public:
    Device(const std::string& name, const std::string& type, float temperature, float humidity, float noise, float airQuality, float light, int online);
    ~Device(){}
    std::string getName() const;
    std::string getType() const;
    float getTemperature() const;
    float getHumidity() const;
    float getNoise() const;
    float getAirQuality() const;
    float getLight() const;
    bool isOnline() const;

    void setName(const std::string& name){this->name = name;}
    void setType(const std::string& type){this->type = type;}
    void setTemperature(float temperature){this->temperature = temperature;}
    void setHumidity(float humidity){this->humidity = humidity;}
    void setNoise(float noise){this->noise = noise;}
    void setAirQuality(float airQuality){this->airQuality = airQuality;}
    void setLight(float light){this->light = light;}
    void setOnline(bool online){this->online = online;}

private:
    std::string name;
    std::string type;
    float temperature;
    float humidity;
    float noise;
    float airQuality;
    float light;
    bool online;
};