#pragma once
#include "../models/sensor_data.h"

class EnvironmentService {
public:
    void processEnvironmentData(SensorData& data);
    
private:
    void calculateScores(SensorData& data);
    void determineStatus(SensorData& data);
    void generateSuggestions(SensorData& data);
    
    // 评分计算函数
    double calculateTemperatureScore(double temp, AreaType type);
    double calculateHumidityScore(double humidity);
    double calculateCO2Score(double co2);
    double calculatePM25Score(double pm25);
    double calculateNoiseScore(double noise, AreaType type);
    double calculateLightScore(double light, AreaType type);
    
    // 状态判断函数
    std::string getTemperatureStatus(double temp, AreaType type);
    std::string getHumidityStatus(double humidity);
    std::string getCO2Status(double co2);
    std::string getPM25Status(double pm25);
    std::string getNoiseStatus(double noise, AreaType type);
    std::string getLightStatus(double light, AreaType type);
}; 