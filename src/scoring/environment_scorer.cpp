#include "environment_scorer.h"

EnvironmentScorer::EnvironmentScorer(SceneType scene)
    : scene_(scene)
{
    // 初始化评分映射
    factorScores_["temperature"] = 0.0;
    factorScores_["humidity"] = 0.0;
    factorScores_["co2"] = 0.0;
    factorScores_["pm25"] = 0.0;
    factorScores_["noise"] = 0.0;
    factorScores_["light"] = 0.0;
}

double EnvironmentScorer::calculateScore(const SensorData& data) {
    // 计算各项指标的评分
    double temp_score = calculateTemperatureScore(data.temperature, data.area_type);
    double humidity_score = calculateHumidityScore(data.humidity);
    double co2_score = calculateCO2Score(data.co2);
    double pm25_score = calculatePM25Score(data.pm25);
    double noise_score = calculateNoiseScore(data.noise, data.area_type);
    double light_score = calculateLightScore(data.light, data.area_type);
    
    // 根据不同区域类型调整权重
    double temp_weight = 0.2;
    double humidity_weight = 0.1;
    double co2_weight = 0.2;
    double pm25_weight = 0.2;
    double noise_weight = 0.15;
    double light_weight = 0.15;
    
    switch (data.area_type) {
        case AreaType::LIVING:
            // 生活区更注重温度和空气质量
            temp_weight = 0.25;
            humidity_weight = 0.15;
            co2_weight = 0.2;
            pm25_weight = 0.2;
            noise_weight = 0.1;
            light_weight = 0.1;
            break;
            
        case AreaType::TEACHING:
            // 教学区更注重光照和噪音
            temp_weight = 0.2;
            humidity_weight = 0.1;
            co2_weight = 0.15;
            pm25_weight = 0.15;
            noise_weight = 0.2;
            light_weight = 0.2;
            break;
            
        case AreaType::RECREATION:
            // 娱乐区权重均衡
            temp_weight = 0.2;
            humidity_weight = 0.1;
            co2_weight = 0.2;
            pm25_weight = 0.2;
            noise_weight = 0.15;
            light_weight = 0.15;
            break;
    }
    
    // 计算加权总分
    double total_score = 
        temp_score * temp_weight +
        humidity_score * humidity_weight +
        co2_score * co2_weight +
        pm25_score * pm25_weight +
        noise_score * noise_weight +
        light_score * light_weight;
    
    return std::min(100.0, std::max(0.0, total_score));
}

double EnvironmentScorer::calculateTemperatureScore(double temperature, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区温度要求更舒适 (最佳: 22-26℃)
            if (temperature >= 22 && temperature <= 26) return 100;
            else if (temperature >= 18 && temperature < 22) 
                return std::min(100.0, std::max(0.0, 80 + (temperature - 18) * 5));  // 18-22℃ 线性增加
            else if (temperature > 26 && temperature <= 30)
                return std::min(100.0, std::max(0.0, 80 - (temperature - 26) * 5));  // 26-30℃ 线性减少
            else if (temperature < 18)
                return std::min(100.0, std::max(0.0, 40 + temperature * 2));  // <18℃ 快速下降
            else
                return std::min(100.0, std::max(0.0, 60 - (temperature - 30) * 3));  // >30℃ 快速下降
            
        case AreaType::TEACHING:
            // 教学区温度要求适中 (最佳: 20-25℃)
            if (temperature >= 20 && temperature <= 25) return 100;
            else if (temperature >= 16 && temperature < 20)
                return std::min(100.0, std::max(0.0, 80 + (temperature - 16) * 5));
            else if (temperature > 25 && temperature <= 29)
                return std::min(100.0, std::max(0.0, 80 - (temperature - 25) * 5));
            else if (temperature < 16)
                return std::min(100.0, std::max(0.0, 40 + temperature * 2));
            else
                return std::min(100.0, std::max(0.0, 60 - (temperature - 29) * 3));
            
        case AreaType::RECREATION:
            // 娱乐区温度容许范围更大 (最佳: 18-27℃)
            if (temperature >= 18 && temperature <= 27) return 100;
            else if (temperature >= 14 && temperature < 18)
                return std::min(100.0, std::max(0.0, 80 + (temperature - 14) * 5));
            else if (temperature > 27 && temperature <= 31)
                return std::min(100.0, std::max(0.0, 80 - (temperature - 27) * 5));
            else if (temperature < 14)
                return std::min(100.0, std::max(0.0, 40 + temperature * 2));
            else
                return std::min(100.0, std::max(0.0, 60 - (temperature - 31) * 3));
    }
    return 0;
}

double EnvironmentScorer::calculateHumidityScore(double humidity) {
    // 最佳湿度范围：40-60%
    if (humidity >= 40 && humidity <= 60) return 100;
    else if (humidity >= 30 && humidity < 40)
        return std::min(100.0, std::max(0.0, 80 + (humidity - 30) * 2));
    else if (humidity > 60 && humidity <= 70)
        return std::min(100.0, std::max(0.0, 80 - (humidity - 60) * 2));
    else if (humidity < 30)
        return std::min(100.0, std::max(0.0, 40 + humidity * 1.5));
    else
        return std::min(100.0, std::max(0.0, 60 - (humidity - 70) * 1.5));
}

double EnvironmentScorer::calculateCO2Score(double co2) {
    // 最佳CO2浓度：400-800ppm
    if (co2 <= 800) return 100;
    else if (co2 <= 1000) return 90;
    else if (co2 <= 1500) return 75;
    else if (co2 <= 2000) return 60;
    else if (co2 <= 3000) return 40;
    else if (co2 <= 4000) return 20;
    else return std::min(100.0, std::max(0.0, 20 - (co2 - 4000) / 200));
}

double EnvironmentScorer::calculatePM25Score(double pm25) {
    if (pm25 <= 35) return 100;
    if (pm25 <= 75) return 80;
    if (pm25 <= 150) return 60;
    if (pm25 <= 250) return 40;
    if (pm25 <= 350) return 20;
    if (pm25 <= 500) return 10;
    return 0;
}

double EnvironmentScorer::calculateNoiseScore(double noise, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区要求安静
            if (noise <= 40) return 100;
            if (noise <= 60) return 80;
            if (noise <= 80) return 60;
            if (noise <= 100) return 40;
            if (noise <= 120) return 20;
            return 0;
            
        case AreaType::TEACHING:
            // 教学区要求较安静
            if (noise <= 45) return 100;
            if (noise <= 65) return 80;
            if (noise <= 85) return 60;
            if (noise <= 105) return 40;
            if (noise <= 120) return 20;
            return 0;
            
        case AreaType::RECREATION:
            // 娱乐区允许较大噪音
            if (noise <= 60) return 100;
            if (noise <= 80) return 80;
            if (noise <= 100) return 60;
            if (noise <= 110) return 40;
            if (noise <= 120) return 20;
            return 0;
    }
    return 0;
}

double EnvironmentScorer::calculateLightScore(double light, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区光照要求舒适
            if (light >= 200 && light <= 1000) return 100;
            if (light < 200) return 60 + (light / 200) * 40;
            if (light <= 10000) return 80;
            if (light <= 50000) return 60;
            if (light <= 100000) return 40;
            return 20;
            
        case AreaType::TEACHING:
            // 教学区要求充足明亮
            if (light >= 400 && light <= 2000) return 100;
            if (light < 400) return 60 + (light / 400) * 40;
            if (light <= 20000) return 80;
            if (light <= 60000) return 60;
            if (light <= 100000) return 40;
            return 20;
            
        case AreaType::RECREATION:
            // 娱乐区光照要求灵活
            if (light >= 300 && light <= 3000) return 100;
            if (light < 300) return 60 + (light / 300) * 40;
            if (light <= 30000) return 80;
            if (light <= 70000) return 60;
            if (light <= 100000) return 40;
            return 20;
    }
    return 0;
}

std::string EnvironmentScorer::getTemperatureStatus(double temp, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (temp >= 20 && temp <= 26) return "适宜";
            if (temp < 20) {
                if (temp < 0) return "极寒";
                if (temp < 10) return "严寒";
                return "偏冷";
            }
            if (temp > 35) return "极热";
            if (temp > 30) return "严热";
            return "偏热";
            
        case AreaType::TEACHING:
            if (temp >= 18 && temp <= 25) return "适宜";
            if (temp < 18) {
                if (temp < 0) return "极寒";
                if (temp < 10) return "严寒";
                return "偏冷";
            }
            if (temp > 35) return "极热";
            if (temp > 30) return "严热";
            return "偏热";
            
        case AreaType::RECREATION:
            if (temp >= 16 && temp <= 28) return "适宜";
            if (temp < 16) {
                if (temp < 0) return "极寒";
                if (temp < 10) return "严寒";
                return "偏冷";
            }
            if (temp > 35) return "极热";
            if (temp > 30) return "严热";
            return "偏热";
    }
    return "异常";
}

std::string EnvironmentScorer::getHumidityStatus(double humidity) {
    if (humidity >= 35 && humidity <= 65) return "适宜";
    if (humidity < 35) {
        if (humidity < 20) return "极干";
        return "偏干";
    }
    if (humidity > 85) return "极湿";
    if (humidity > 75) return "很湿";
    return "偏湿";
}

std::string EnvironmentScorer::getCO2Status(double co2) {
    if (co2 <= 600) return "优";
    if (co2 <= 1000) return "良";
    if (co2 <= 2000) return "中";
    if (co2 <= 3000) return "差";
    if (co2 <= 4000) return "很差";
    return "危险";
}

std::string EnvironmentScorer::getPM25Status(double pm25) {
    if (pm25 <= 35) return "优";
    if (pm25 <= 75) return "良";
    if (pm25 <= 150) return "轻度污染";
    if (pm25 <= 250) return "中度污染";
    if (pm25 <= 350) return "重度污染";
    return "严重污染";
}

std::string EnvironmentScorer::getNoiseStatus(double noise, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (noise <= 40) return "安静";
            if (noise <= 60) return "适中";
            if (noise <= 80) return "嘈杂";
            if (noise <= 100) return "很吵";
            return "危险";
            
        case AreaType::TEACHING:
            if (noise <= 45) return "安静";
            if (noise <= 65) return "适中";
            if (noise <= 85) return "嘈杂";
            if (noise <= 105) return "很吵";
            return "危险";
            
        case AreaType::RECREATION:
            if (noise <= 60) return "适中";
            if (noise <= 80) return "正常";
            if (noise <= 100) return "嘈杂";
            if (noise <= 110) return "很吵";
            return "危险";
    }
    return "异常";
}

std::string EnvironmentScorer::getLightStatus(double light, AreaType type) {
    switch (type) {
        case AreaType::LIVING:
            if (light >= 200 && light <= 1000) return "适宜";
            if (light < 200) {
                if (light < 50) return "黑暗";
                return "偏暗";
            }
            if (light > 50000) return "强光";
            if (light > 10000) return "很亮";
            return "偏亮";
            
        case AreaType::TEACHING:
            if (light >= 400 && light <= 2000) return "适宜";
            if (light < 400) {
                if (light < 100) return "黑暗";
                return "偏暗";
            }
            if (light > 60000) return "强光";
            if (light > 20000) return "很亮";
            return "偏亮";
            
        case AreaType::RECREATION:
            if (light >= 300 && light <= 3000) return "适宜";
            if (light < 300) {
                if (light < 100) return "黑暗";
                return "偏暗";
            }
            if (light > 70000) return "强光";
            if (light > 30000) return "很亮";
            return "偏亮";
    }
    return "异常";
}

bool EnvironmentScorer::isWorkingHours(TimeSlot time_slot) {
    return time_slot == TimeSlot::MORNING_CLASS ||
           time_slot == TimeSlot::AFTERNOON_CLASS ||
           time_slot == TimeSlot::EVENING_CLASS;
}

bool EnvironmentScorer::isSleepingHours(TimeSlot time_slot) {
    return time_slot == TimeSlot::SLEEPING_TIME;
}

std::vector<std::string> EnvironmentScorer::generateSuggestions(const SensorData& data, TimeSlot time_slot) {
    std::vector<std::string> suggestions;
    
    // 根据区域类型调整建议阈值
    switch (data.area_type) {
        case AreaType::LIVING:
            // 生活区建议
            if (data.temperature < 18) {
                suggestions.push_back("室温过低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启暖气；2. 关闭门窗减少热量流失；" +
                                    "3. 使用加热设备提升温度");
            } else if (data.temperature < 22) {
                suggestions.push_back("室温偏低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 调高暖气温度至22-26℃；2. 关闭门窗减少热量流失；" +
                                    "3. 可以使用加热设备临时提升温度");
            } else if (data.temperature > 30) {
                suggestions.push_back("室温过高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启空调降温；2. 加强通风；" +
                                    "3. 关闭发热设备；4. 避免剧烈活动");
            } else if (data.temperature > 26) {
                suggestions.push_back("室温偏高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 开启空调调节至22-26℃；2. 适当开窗通风；" +
                                    "3. 调整或关闭发热设备");
            }
            break;
            
        case AreaType::TEACHING:
            // 教学区建议
            if (data.temperature < 16) {
                suggestions.push_back("教室温度过低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启暖气；2. 暂停教学活动；" +
                                    "3. 转移到其他教室；4. 注意保暖");
            } else if (data.temperature < 20) {
                suggestions.push_back("教室温度偏低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 调高暖气温度至20-25℃；2. 课间可以进行适当运动；" +
                                    "3. 建议穿着保暖衣物");
            } else if (data.temperature > 29) {
                suggestions.push_back("教室温度过高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启空调降温；2. 转移到其他教室；" +
                                    "3. 暂停剧烈活动；4. 注意补充水分");
            } else if (data.temperature > 25) {
                suggestions.push_back("教室温度偏高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 开启空调调节至20-25℃；2. 课间开窗通风；" +
                                    "3. 减少教室内的人员密度");
            }
            break;
            
        case AreaType::RECREATION:
            // 娱乐区建议
            if (data.temperature < 14) {
                suggestions.push_back("活动区温度过低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启暖气；2. 暂停室内活动；" +
                                    "3. 转移到其他区域；4. 注意保暖");
            } else if (data.temperature < 18) {
                suggestions.push_back("活动区温度偏低（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 调高室温至18-27℃；2. 进行适度运动增加体温；" +
                                    "3. 注意保暖，可以增加衣物");
            } else if (data.temperature > 31) {
                suggestions.push_back("活动区温度过高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 立即开启空调降温；2. 暂停室内活动；" +
                                    "3. 转移到其他区域；4. 注意防暑降温");
            } else if (data.temperature > 27) {
                suggestions.push_back("活动区温度偏高（当前" + std::to_string(int(data.temperature)) + 
                                    "℃），建议：1. 开启空调降温；2. 加强通风；3. 减少剧烈运动");
            }
            break;
    }
    
    // 湿度建议
    if (data.humidity < 30) {
        suggestions.push_back("空气严重干燥（当前" + std::to_string(int(data.humidity)) + 
                            "%），建议：1. 立即开启加湿器；2. 暂时离开房间；" +
                            "3. 多补充水分；4. 使用湿毛巾提高湿度");
    } else if (data.humidity < 40) {
        suggestions.push_back("空气偏干燥（当前" + std::to_string(int(data.humidity)) + 
                            "%），建议：1. 使用加湿器提高湿度至40-60%；2. 放置绿植增加自然湿度；" +
                            "3. 多补充水分，保持皮肤湿润");
    } else if (data.humidity > 70) {
        suggestions.push_back("湿度过高（当前" + std::to_string(int(data.humidity)) + 
                            "%），建议：1. 立即开启除湿机和空调；2. 检查是否有漏水；" +
                            "3. 暂时离开房间；4. 注意防霉防潮");
    } else if (data.humidity > 60) {
        suggestions.push_back("湿度偏高（当前" + std::to_string(int(data.humidity)) + 
                            "%），建议：1. 开启除湿机；2. 加强通风换气；" +
                            "3. 检查是否有漏水或渗水现象");
    }
    
    // CO2建议
    if (data.co2 > 800) {
        if (data.co2 > 2000) {
            suggestions.push_back("CO2浓度严重超标（当前" + std::to_string(int(data.co2)) + 
                                "ppm），建议：1. 立即疏散人员；2. 全面开窗通风；" +
                                "3. 开启新风系统最大功率；4. 检查通风设备");
        } else if (data.co2 > 1500) {
            suggestions.push_back("CO2浓度严重超标（当前" + std::to_string(int(data.co2)) + 
                                "ppm），建议：1. 立即开窗通风；2. 减少室内人员数量；" +
                                "3. 开启新风系统；4. 暂时离开房间，等待空气改善");
        } else if (data.co2 > 1000) {
            suggestions.push_back("CO2浓度偏高（当前" + std::to_string(int(data.co2)) + 
                                "ppm），建议：1. 开窗通风15-20分钟；2. 控制房间人数；" +
                                "3. 增加绿植吸收CO2");
        }
    }
    
    // PM2.5建议
    if (data.pm25 > 75) {
        if (data.pm25 > 115) {
            suggestions.push_back("PM2.5浓度严重超标（当前" + std::to_string(int(data.pm25)) + 
                                "μg/m³），建议：1. 开启空气净化器并设置为最大功率；" +
                                "2. 关闭门窗，避免室外污染；3. 佩戴防护口罩；" +
                                "4. 避免剧烈运动，减少深呼吸");
        } else {
            suggestions.push_back("PM2.5浓度较高（当前" + std::to_string(int(data.pm25)) + 
                                "μg/m³），建议：1. 开启空气净化器；2. 保持门窗关闭；" +
                                "3. 定期清洁空气净化器滤网");
        }
    }
    
    // 根据区域类型和时段给出噪音建议
    if (isSleepingHours(time_slot)) {
        if (data.noise > 45) {
            suggestions.push_back("夜间噪音较大（当前" + std::to_string(int(data.noise)) + 
                                "dB），建议：1. 保持安静，避免大声说话；2. 使用隔音耳塞；" +
                                "3. 关闭不必要的电器设备；4. 检查是否有异常噪音源");
        }
    } else {
        switch (data.area_type) {
            case AreaType::LIVING:
                if (data.noise > 50) {
                    suggestions.push_back("生活区噪音较大（当前" + std::to_string(int(data.noise)) + 
                                        "dB），建议：1. 降低音响、电视音量；2. 避免大声喧哗；" +
                                        "3. 使用隔音材料；4. 合理安排家务活动时间");
                }
                break;
            case AreaType::TEACHING:
                if (data.noise > 55) {
                    suggestions.push_back("教学区噪音超标（当前" + std::to_string(int(data.noise)) + 
                                        "dB），建议：1. 保持课堂纪律；2. 关闭门窗隔绝外部噪音；" +
                                        "3. 使用麦克风代替提高音量；4. 采用互动式教学减少集体朗读");
                }
                break;
            case AreaType::RECREATION:
                if (data.noise > 65) {
                    suggestions.push_back("活动区噪音过大（当前" + std::to_string(int(data.noise)) + 
                                        "dB），建议：1. 控制活动音量；2. 分散活动人群；" +
                                        "3. 设置隔音设施；4. 避免在同一时间开展多个高噪音活动");
                }
                break;
        }
    }
    
    // 根据区域类型和时段给出光照建议
    if (isWorkingHours(time_slot)) {
        switch (data.area_type) {
            case AreaType::LIVING:
                if (data.light < 200) {
                    suggestions.push_back("生活区光照不足（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 开启主照明；2. 拉开窗帘增加自然光；" +
                                        "3. 调整家具布局，避免遮挡光源；4. 使用台灯补充局部照明");
                } else if (data.light > 500) {
                    suggestions.push_back("生活区光照过强（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 适当调暗照明；2. 使用窗帘调节自然光；" +
                                        "3. 避免直射光线；4. 调整显示器亮度");
                }
                break;
                
            case AreaType::TEACHING:
                if (data.light < 400) {
                    suggestions.push_back("教室光照不足（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 开启全部照明设备；2. 打开窗帘增加自然光；" +
                                        "3. 调整座位，避免处于阴暗区域；4. 定期清洁照明设备");
                } else if (data.light > 750) {
                    suggestions.push_back("教室光照过强（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 调整照明亮度；2. 使用窗帘调节自然光；" +
                                        "3. 避免阳光直射黑板；4. 调整投影仪亮度");
                }
                break;
                
            case AreaType::RECREATION:
                if (data.light < 300) {
                    suggestions.push_back("活动区光照不足（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 增加照明设备；2. 优化自然采光；" +
                                        "3. 调整活动区域布局；4. 增加反光材料提升亮度");
                } else if (data.light > 1000) {
                    suggestions.push_back("活动区光照过强（当前" + std::to_string(int(data.light)) + 
                                        "lux），建议：1. 降低照明强度；2. 安装遮光设施；" +
                                        "3. 调整活动时间避开强光时段；4. 佩戴防护眼镜");
                }
                break;
        }
    } else if (isSleepingHours(time_slot)) {
        if (data.light > 100) {
            suggestions.push_back("夜间光照过强（当前" + std::to_string(int(data.light)) + 
                                "lux），建议：1. 关闭主要照明；2. 使用柔和夜灯；" +
                                "3. 确保窗帘完全遮光；4. 避免使用蓝光设备");
        }
    }
    
    return suggestions;
} 