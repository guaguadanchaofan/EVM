#pragma once
#include "../models/sensor_data.h"
#include <map>
#include <string>

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
        MORNING_CLASS,     // 上午课程
        AFTERNOON_CLASS,   // 下午课程
        EVENING_CLASS,     // 晚自习
        REST_TIME,         // 休息时间
        SLEEPING_TIME      // 睡眠时间
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
    
private:
    // 单项指标评分计算
    double calculateTemperatureScore(double temperature);
    double calculateHumidityScore(double humidity);
    double calculateCO2Score(double co2);
    double calculatePM25Score(double pm25);
    
    // 获取权重
    std::map<std::string, double> getWeights(TimeSlot timeSlot) const;
    
    SceneType scene_;
    std::map<std::string, double> factorScores_;
}; 