#pragma once
#include <string>
#include <ctime>
#include <map>
struct SensorData {
    std::string device_id;
    double temperature;
    double humidity;
    double co2;
    double pm25;
    time_t timestamp;
    double score;
};

struct EnvironmentScore {
    std::string location_id;
    double score;
    std::string grade;
    std::map<std::string, double> factor_scores;
}; 