#include "environment_service.h"
#include <algorithm>
#include <numeric>

void EnvironmentService::processEnvironmentData(SensorData& data) {
    calculateScores(data);
    determineStatus(data);
    generateSuggestions(data);
}

void EnvironmentService::calculateScores(SensorData& data) {
    // 计算各项指标的评分
    data.scores.temperature = calculateTemperatureScore(data.temperature, data.area_type);
    data.scores.humidity = calculateHumidityScore(data.humidity);
    data.scores.co2 = calculateCO2Score(data.co2);
    data.scores.pm25 = calculatePM25Score(data.pm25);
    data.scores.noise = calculateNoiseScore(data.noise, data.area_type);
    data.scores.light = calculateLightScore(data.light, data.area_type);
    
    // 计算总体评分（加权平均）
    const double weights[] = {0.2, 0.15, 0.15, 0.15, 0.15, 0.2};  // 权重之和为1
    data.scores.overall = 
        data.scores.temperature * weights[0] +
        data.scores.humidity * weights[1] +
        data.scores.co2 * weights[2] +
        data.scores.pm25 * weights[3] +
        data.scores.noise * weights[4] +
        data.scores.light * weights[5];
}

double EnvironmentService::calculateTemperatureScore(double temp, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (temp >= 22 && temp <= 26) return 100;
            if (temp < 22) return 100 - (22 - temp) * 12;
            return 100 - (temp - 26) * 12;
            
        case AreaType::TEACHING:
            if (temp >= 20 && temp <= 25) return 100;
            if (temp < 20) return 100 - (20 - temp) * 10;
            return 100 - (temp - 25) * 10;
            
        case AreaType::RECREATION:
            if (temp >= 18 && temp <= 27) return 100;
            if (temp < 18) return 100 - (18 - temp) * 8;
            return 100 - (temp - 27) * 8;
    }
    return 0;
}

double EnvironmentService::calculateHumidityScore(double humidity) {
    if (humidity >= 40 && humidity <= 60) return 100;
    if (humidity < 40) return 100 - (40 - humidity) * 2;
    return 100 - (humidity - 60) * 2;
}

double EnvironmentService::calculateCO2Score(double co2) {
    if (co2 <= 800) return 100;
    if (co2 <= 1000) return 80;
    if (co2 <= 1500) return 60;
    if (co2 <= 2000) return 40;
    return 20;
}

double EnvironmentService::calculatePM25Score(double pm25) {
    if (pm25 <= 35) return 100;
    if (pm25 <= 75) return 80;
    if (pm25 <= 115) return 60;
    if (pm25 <= 150) return 40;
    return 20;
}

double EnvironmentService::calculateNoiseScore(double noise, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (noise <= 40) return 100;
            if (noise <= 50) return 80;
            if (noise <= 60) return 60;
            if (noise <= 70) return 40;
            return 20;
            
        case AreaType::TEACHING:
            if (noise <= 45) return 100;
            if (noise <= 55) return 80;
            if (noise <= 65) return 60;
            if (noise <= 75) return 40;
            return 20;
            
        case AreaType::RECREATION:
            if (noise <= 55) return 100;
            if (noise <= 65) return 80;
            if (noise <= 75) return 60;
            if (noise <= 85) return 40;
            return 20;
    }
    return 0;
}

double EnvironmentService::calculateLightScore(double light, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (light >= 200 && light <= 500) return 100;
            if (light < 200) return 60 + (light / 200) * 40;
            if (light <= 750) return 80;
            if (light <= 1000) return 60;
            return 40;
            
        case AreaType::TEACHING:
            if (light >= 400 && light <= 750) return 100;
            if (light < 400) return 60 + (light / 400) * 40;
            if (light <= 1000) return 80;
            if (light <= 1500) return 60;
            return 40;
            
        case AreaType::RECREATION:
            if (light >= 300 && light <= 1000) return 100;
            if (light < 300) return 60 + (light / 300) * 40;
            if (light <= 1500) return 80;
            if (light <= 2000) return 60;
            return 40;
    }
    return 0;
}

void EnvironmentService::determineStatus(SensorData& data) {
    data.status.temperature = getTemperatureStatus(data.temperature, data.area_type);
    data.status.humidity = getHumidityStatus(data.humidity);
    data.status.co2 = getCO2Status(data.co2);
    data.status.pm25 = getPM25Status(data.pm25);
    data.status.noise = getNoiseStatus(data.noise, data.area_type);
    data.status.light = getLightStatus(data.light, data.area_type);
}

std::string EnvironmentService::getTemperatureStatus(double temp, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (temp >= 22 && temp <= 26) return "适宜";
            if (temp < 22) return "偏冷";
            return "偏热";
            
        case AreaType::TEACHING:
            if (temp >= 20 && temp <= 25) return "适宜";
            if (temp < 20) return "偏冷";
            return "偏热";
            
        case AreaType::RECREATION:
            if (temp >= 18 && temp <= 27) return "适宜";
            if (temp < 18) return "偏冷";
            return "偏热";
    }
    return "异常";
}

std::string EnvironmentService::getHumidityStatus(double humidity) {
    if (humidity >= 40 && humidity <= 60) return "正常";
    if (humidity < 40) return "偏干";
    return "偏湿";
}

std::string EnvironmentService::getCO2Status(double co2) {
    if (co2 <= 800) return "优";
    if (co2 <= 1000) return "良";
    if (co2 <= 1500) return "中";
    if (co2 <= 2000) return "差";
    return "很差";
}

std::string EnvironmentService::getPM25Status(double pm25) {
    if (pm25 <= 35) return "优";
    if (pm25 <= 75) return "良";
    if (pm25 <= 115) return "中";
    if (pm25 <= 150) return "差";
    return "很差";
}

std::string EnvironmentService::getNoiseStatus(double noise, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (noise <= 40) return "安静";
            if (noise <= 50) return "适中";
            if (noise <= 60) return "较吵";
            return "很吵";
            
        case AreaType::TEACHING:
            if (noise <= 45) return "安静";
            if (noise <= 55) return "适中";
            if (noise <= 65) return "较吵";
            return "很吵";
            
        case AreaType::RECREATION:
            if (noise <= 55) return "适中";
            if (noise <= 65) return "正常";
            if (noise <= 75) return "较吵";
            return "很吵";
    }
    return "异常";
}

std::string EnvironmentService::getLightStatus(double light, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (light >= 200 && light <= 500) return "适宜";
            if (light < 200) return "偏暗";
            return "偏亮";
            
        case AreaType::TEACHING:
            if (light >= 400 && light <= 750) return "适宜";
            if (light < 400) return "偏暗";
            return "偏亮";
            
        case AreaType::RECREATION:
            if (light >= 300 && light <= 1000) return "适宜";
            if (light < 300) return "偏暗";
            return "偏亮";
    }
    return "异常";
}

void EnvironmentService::generateSuggestions(SensorData& data) {
    data.suggestions.clear();
    
    switch (data.area_type) {
        case AreaType::LIVING:
            if (data.temperature < 22) {
                data.suggestions.push_back("生活区温度偏低，建议适当提高温度，注意保暖");
            } else if (data.temperature > 26) {
                data.suggestions.push_back("生活区温度偏高，建议开启空调或加强通风");
            }
            
            if (data.noise > 50) {
                data.suggestions.push_back("生活区噪音较大，建议查找噪音源并采取措施");
                if (data.noise > 60) {
                    data.suggestions.push_back("生活区噪音严重超标，可能影响休息，建议立即处理");
                }
            }
            
            if (data.light < 200) {
                data.suggestions.push_back("生活区光线不足，建议适当增加照明");
            } else if (data.light > 500) {
                data.suggestions.push_back("生活区光线过强，建议适当调整");
            }
            break;
            
        case AreaType::TEACHING:
            if (data.temperature < 20) {
                data.suggestions.push_back("教学区温度偏低，建议适当提高温度");
            } else if (data.temperature > 25) {
                data.suggestions.push_back("教学区温度偏高，建议开启空调或通风");
            }
            
            if (data.noise > 55) {
                data.suggestions.push_back("教学区噪音较大，可能影响教学质量");
                if (data.noise > 65) {
                    data.suggestions.push_back("教学区噪音过大，建议暂停喧哗活动，维护教学秩序");
                }
            }
            
            if (data.light < 400) {
                data.suggestions.push_back("教室光线不足，可能影响阅读学习，建议增加照明");
            } else if (data.light > 750) {
                data.suggestions.push_back("教室光线过强，注意防止投影反光，可适当调整窗帘");
            }
            break;
            
        case AreaType::RECREATION:
            if (data.temperature < 18) {
                data.suggestions.push_back("活动区温度偏低，建议适当提高温度，注意运动时的保暖");
            } else if (data.temperature > 27) {
                data.suggestions.push_back("活动区温度偏高，建议加强通风或开启空调，预防中暑");
            }
            
            if (data.noise > 65) {
                data.suggestions.push_back("娱乐区噪音超标，建议控制活动音量");
                if (data.noise > 75) {
                    data.suggestions.push_back("娱乐区噪音严重超标，建议佩戴护耳设备或调整活动方式");
                }
            }
            
            if (data.light < 300) {
                data.suggestions.push_back("活动区光线不足，注意安全，建议增加照明");
            } else if (data.light > 1000) {
                data.suggestions.push_back("活动区光线过强，建议适当遮挡，避免眩光影响活动");
            }
            break;
    }
    
    // 通用环境建议
    if (data.humidity < 40) {
        data.suggestions.push_back("空气偏干，建议使用加湿器改善空气湿度");
    } else if (data.humidity > 60) {
        data.suggestions.push_back("湿度偏高，建议开启除湿或通风");
    }
    
    if (data.co2 > 1000) {
        data.suggestions.push_back("CO2浓度偏高，建议开窗通风，保持空气流通");
        if (data.co2 > 1500) {
            if (data.area_type == AreaType::TEACHING) {
                data.suggestions.push_back("CO2浓度严重超标，建议立即通风并考虑暂时休息");
            } else {
                data.suggestions.push_back("CO2浓度过高，建议立即通风，并减少房间内人员密度");
            }
        }
    }
    
    if (data.pm25 > 75) {
        data.suggestions.push_back("PM2.5浓度较高，建议开启空气净化器");
        if (data.pm25 > 115) {
            if (data.area_type == AreaType::RECREATION) {
                data.suggestions.push_back("空气质量差，建议暂停剧烈运动，改为室内低强度活动");
            } else {
                data.suggestions.push_back("空气质量差，建议戴口罩，减少室内活动");
            }
        }
    }
    
    // 如果环境良好，给出积极反馈
    if (data.suggestions.empty()) {
        switch (data.area_type) {
            case AreaType::LIVING:
                data.suggestions.push_back("生活区环境舒适宜人，请继续保持");
                break;
            case AreaType::TEACHING:
                data.suggestions.push_back("教学区环境适宜，有助于专注学习");
                break;
            case AreaType::RECREATION:
                data.suggestions.push_back("活动区环境良好，适合开展各类活动");
                break;
        }
    }
} 