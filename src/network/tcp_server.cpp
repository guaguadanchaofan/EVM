#include "tcp_server.h"
#include <jsoncpp/json/json.h>
#include <iostream>
#include <ctime>
#include "../scoring/environment_scorer.h"
#include "../utils/json_helper.h"
#include "../device/device_manager.h"

TCPServer::TCPServer(boost::asio::io_context& io_context, short port, Database& db)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , database_(db)
{
    start_accept();
}

void TCPServer::start() {
    std::cout << "TCP Server started on port " 
              << acceptor_.local_endpoint().port() << std::endl;
}

void TCPServer::start_accept() {
    auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*socket,
        [this, socket](const boost::system::error_code& error) {
            handle_accept(socket, error);
        });
}

void TCPServer::handle_accept(std::shared_ptr<tcp::socket> socket,
                            const boost::system::error_code& error) {
    if (!error) {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        socket->async_read_some(
            boost::asio::buffer(*buffer),
            [this, socket, buffer](const boost::system::error_code& error,
                                 size_t bytes_transferred) {
                handle_read(socket, buffer, error, bytes_transferred);
            });
    }
    
    start_accept();
}

void TCPServer::start_read(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024);
    socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& error,
                             size_t bytes_transferred) {
            handle_read(socket, buffer, error, bytes_transferred);
        });
}

void TCPServer::handle_read(std::shared_ptr<tcp::socket> socket,
                          std::shared_ptr<std::vector<char>> buffer,
                          const boost::system::error_code& error,
                          size_t bytes_transferred) {
    if (!error) {
        try {
            // 解析 JSON 数据
            Json::Value root;
            Json::Reader reader;
            std::string data(buffer->data(), bytes_transferred);            
            if (reader.parse(data, root)) {
                // 创建传感器数据对象
                SensorData sensor_data;
                sensor_data.device_id = root["device_id"].asString();
                sensor_data.timestamp = root["timestamp"].asInt64();
                sensor_data.temperature = root["temperature"].asDouble();
                sensor_data.humidity = root["humidity"].asDouble();
                sensor_data.co2 = root["co2"].asDouble();
                sensor_data.pm25 = root["pm25"].asDouble();
                sensor_data.noise = root["noise"].asDouble();
                sensor_data.light = root["light"].asDouble();
                sensor_data.area = root["area"].asString();
                
                // 将字符串转换为 AreaType
                std::string area_type = root["area_type"].asString();
                if (area_type == "living") {
                    sensor_data.area_type = AreaType::LIVING;
                } else if (area_type == "teaching") {
                    sensor_data.area_type = AreaType::TEACHING;
                } else if (area_type == "recreation") {
                    sensor_data.area_type = AreaType::RECREATION;
                } else {
                    std::cerr << "[TCP] Unknown area type: " << area_type << std::endl;
                    sensor_data.area_type = AreaType::TEACHING;  // 默认值
                }
                
                // 计算环境评分
                EnvironmentScorer scorer(EnvironmentScorer::SceneType::CLASSROOM);  // 使用默认场景
                auto time_slot = determineTimeSlot(sensor_data.timestamp);
                
                // 计算各项指标的评分
                sensor_data.scores.temperature = scorer.calculateTemperatureScore(sensor_data.temperature, sensor_data.area_type);
                sensor_data.scores.humidity = scorer.calculateHumidityScore(sensor_data.humidity);
                sensor_data.scores.co2 = scorer.calculateCO2Score(sensor_data.co2);
                sensor_data.scores.pm25 = scorer.calculatePM25Score(sensor_data.pm25);
                sensor_data.scores.noise = scorer.calculateNoiseScore(sensor_data.noise, sensor_data.area_type);
                sensor_data.scores.light = scorer.calculateLightScore(sensor_data.light, sensor_data.area_type);
                
                // 计算总体评分
                sensor_data.scores.overall = (
                    sensor_data.scores.temperature * 0.2 +
                    sensor_data.scores.humidity * 0.1 +
                    sensor_data.scores.co2 * 0.2 +
                    sensor_data.scores.pm25 * 0.2 +
                    sensor_data.scores.noise * 0.15 +
                    sensor_data.scores.light * 0.15
                );
                
                // 添加状态描述
                sensor_data.status.temperature = scorer.getTemperatureStatus(sensor_data.temperature, sensor_data.area_type);
                sensor_data.status.humidity = scorer.getHumidityStatus(sensor_data.humidity);
                sensor_data.status.co2 = scorer.getCO2Status(sensor_data.co2);
                sensor_data.status.pm25 = scorer.getPM25Status(sensor_data.pm25);
                sensor_data.status.noise = scorer.getNoiseStatus(sensor_data.noise, sensor_data.area_type);
                sensor_data.status.light = scorer.getLightStatus(sensor_data.light, sensor_data.area_type);
                
                // 生成环境建议
                sensor_data.suggestions = scorer.generateSuggestions(sensor_data, time_slot);
                
                // 更新设备状态
                auto& deviceManager = DeviceManager::getInstance();
                
                if (!deviceManager.getDeviceInfo(sensor_data.device_id)) {
                    deviceManager.registerDevice(sensor_data.device_id, sensor_data.area, "sensor");
                }
                
                auto device = deviceManager.getDeviceInfo(sensor_data.device_id);
                if (device) {
                    // 更新设备的最新数据
                    device->last_heartbeat = time(nullptr);
                    device->status = DeviceStatus::ONLINE;  // 更新设备状态
                    device->location_id = sensor_data.area; // 更新位置信息
                    device->recent_data.push_back(sensor_data);
                    // 保持最近数据的数量限制
                    if (device->recent_data.size() > 100) {  // 保留最近100条数据
                        device->recent_data.erase(device->recent_data.begin());
                    }
                    
                    // 调用设备管理器的数据添加方法
                    deviceManager.addSensorData(sensor_data.device_id, sensor_data);
                }
                
                // 保存数据到数据库
                Database::getInstance().insertSensorData(sensor_data);
                
                // 发送响应
                std::string response = "OK\n";
                boost::asio::async_write(*socket,
                    boost::asio::buffer(response),
                    [](const boost::system::error_code& error, std::size_t) {
                        if (error) {
                            std::cerr << "[TCP] Write error: " << error.message() << std::endl;
                        }
                    });
            }
        } catch (const std::exception& e) {
            std::cerr << "[TCP] Error processing data: " << e.what() << std::endl;
        }
        
        // 继续读取下一个数据包
        start_read(socket);
    } else if (error != boost::asio::error::operation_aborted) {
        std::cerr << "[TCP] Read error: " << error.message() << std::endl;
    }
}

EnvironmentScorer::TimeSlot TCPServer::determineTimeSlot(time_t timestamp) {
    // 将时间戳转换为本地时间
    struct tm* lt = localtime(&timestamp);
    int hour = lt->tm_hour;
    
    if (hour >= 8 && hour < 12) {
        return EnvironmentScorer::TimeSlot::MORNING_CLASS;
    } else if (hour >= 14 && hour < 18) {
        return EnvironmentScorer::TimeSlot::AFTERNOON_CLASS;
    } else if (hour >= 19 && hour < 22) {
        return EnvironmentScorer::TimeSlot::EVENING_CLASS;
    } else if (hour >= 22 || hour < 6) {
        return EnvironmentScorer::TimeSlot::SLEEPING_TIME;
    } else {
        return EnvironmentScorer::TimeSlot::REST_TIME;
    }
} 