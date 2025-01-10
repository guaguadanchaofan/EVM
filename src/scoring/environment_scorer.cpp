#include "environment_scorer.h"

double EnvironmentScorer::calculateScore(const SensorData& data) {
    double temp_score = evaluateTemperature(data.temperature);
    double hum_score = evaluateHumidity(data.humidity);
    double co2_score = evaluateCO2(data.co2);
    double pm25_score = evaluatePM25(data.pm25);
    
    return (temp_score + hum_score + co2_score + pm25_score) / 4.0;
}

double EnvironmentScorer::evaluateTemperature(double temp) {
    if (temp >= 20 && temp <= 26) return 100;
    if (temp < 18 || temp > 28) return 0;
    if (temp < 20) return (temp - 18) * 50;
    return (28 - temp) * 50;
}

double EnvironmentScorer::evaluateHumidity(double hum) {
    if (hum >= 40 && hum <= 70) return 100;
    if (hum < 30 || hum > 80) return 0;
    if (hum < 40) return (hum - 30) * 10;
    return (80 - hum) * 10;
}

double EnvironmentScorer::evaluateCO2(double co2) {
    if (co2 <= 1000) return 100;
    if (co2 >= 2000) return 0;
    return (2000 - co2) / 10;
}

double EnvironmentScorer::evaluatePM25(double pm25) {
    if (pm25 <= 35) return 100;
    if (pm25 >= 150) return 0;
    return (150 - pm25) * 100 / 115;
} 