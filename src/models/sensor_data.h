#pragma once
#include <string>
#include <ctime>
#include <map>
#include <vector>

enum class AreaType {
    LIVING,     // 生活区
    TEACHING,   // 教学区
    RECREATION  // 娱乐区
};

struct SensorData {
    std::string device_id;
    time_t timestamp;
    double temperature;
    double humidity;
    double co2;
    double pm25;
    double noise;
    double light;
    std::string area;
    AreaType area_type;
    
    // 聚合数据相关字段
    bool has_aggregated_data = false;  // 标记是否为聚合数据
    bool has_min_max = false;          // 标记是否有最大最小值
    bool is_hourly = false;            // 标记是否为小时聚合数据
    
    // 最大最小值（仅温度）
    double max_temperature = 0;
    double min_temperature = 0;
    
    // 样本数量
    int samples_count = 0;
    
    // 评分数据
    struct {
        double temperature;
        double humidity;
        double co2;
        double pm25;
        double noise;
        double light;
        double overall;
    } scores;
    
    // 状态描述
    struct {
        std::string temperature;
        std::string humidity;
        std::string co2;
        std::string pm25;
        std::string noise;
        std::string light;
    } status;
    
    // 环境建议
    std::vector<std::string> suggestions;
};

struct EnvironmentScore {
    std::string location_id;
    double score;
    std::string grade;
    std::map<std::string, double> factor_scores;
}; 