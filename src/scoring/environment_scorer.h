#pragma once
#include "../models/sensor_data.h"
#include <map>
#include <string>
#include <vector>

class EnvironmentScorer {
public:
    // 场景类型
    enum class SceneType {
        CLASSROOM,      // 教室
        LIBRARY,       // 图书馆
        LABORATORY,    // 实验室
        OFFICE,        // 办公室
        DORMITORY      // 宿舍
    };

    // 时段类型
    enum class TimeSlot {
        MORNING_CLASS,    // 8:00-12:00
        AFTERNOON_CLASS,  // 14:00-18:00
        EVENING_CLASS,    // 19:00-22:00
        SLEEPING_TIME,    // 22:00-6:00
        REST_TIME         // 其他时间
    };

    EnvironmentScorer(SceneType scene = SceneType::CLASSROOM);
    
    // 计算综合评分
    double calculateScore(const SensorData& data, TimeSlot timeSlot);
    
    // 获取各项指标的评分
    std::map<std::string, double> getFactorScores() const { return factorScores_; }
    
    static double calculateScore(const SensorData& data);
    static double evaluateTemperature(double temp);
    static double evaluateHumidity(double humidity);
    static double evaluateCO2(double co2);
    static double evaluatePM25(double pm25);
    
    static double calculateTemperatureScore(double temperature, AreaType area_type);
    static double calculateHumidityScore(double humidity);
    static double calculateCO2Score(double co2);
    static double calculatePM25Score(double pm25);
    static double calculateNoiseScore(double noise, AreaType area_type);
    static double calculateLightScore(double light, AreaType area_type);
    
    std::string getTemperatureStatus(double temp, AreaType type);
    std::string getHumidityStatus(double humidity);
    std::string getCO2Status(double co2);
    std::string getPM25Status(double pm25);
    std::string getNoiseStatus(double noise, AreaType type);
    std::string getLightStatus(double light, AreaType type);
    
    std::vector<std::string> generateSuggestions(const SensorData& data, TimeSlot time_slot);
    
private:
    // 获取权重
    std::map<std::string, double> getWeights(TimeSlot timeSlot) const;
    
    SceneType scene_;
    std::map<std::string, double> factorScores_;
    
    // 辅助函数
    bool isWorkingHours(TimeSlot time_slot);
    bool isSleepingHours(TimeSlot time_slot);
}; 