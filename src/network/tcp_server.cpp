#include "tcp_server.h"
#include <jsoncpp/json/json.h>
#include <iostream>
#include <ctime>
#include "../scoring/environment_scorer.h"
#include "../utils/json_helper.h"
#include "../device/device_manager.h"

TCPServer::TCPServer(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
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

void TCPServer::handle_read(std::shared_ptr<tcp::socket> socket,
                          std::shared_ptr<std::vector<char>> buffer,
                          const boost::system::error_code& error,
                          size_t bytes_transferred) {
    if (!error) {
        try {
            std::string data(buffer->begin(), buffer->begin() + bytes_transferred);
            std::cout << "\n[TCP Server] Received raw data (" << bytes_transferred << " bytes): " 
                     << data << std::endl;
            
            Json::Value root;
            if (!JsonHelper::parseJson(data, root)) {
                std::cerr << "[TCP Server] Failed to parse JSON data" << std::endl;
                return;
            }
            
            std::string device_id = root["device_id"].asString();
            std::cout << "[TCP Server] Processing data from device: " << device_id << std::endl;
            
            // 检查设备是否存在，不存在则注册
            auto& deviceManager = DeviceManager::getInstance();
            if (!deviceManager.getDeviceInfo(device_id)) {
                std::cout << "[TCP Server] New device detected, registering..." << std::endl;
                deviceManager.registerDevice(device_id, "default_location", "sensor");
            }
            
            // 更新设备心跳
            deviceManager.updateHeartbeat(device_id);
            
            SensorData sensor_data{
                device_id,
                root["temperature"].asDouble(),
                root["humidity"].asDouble(),
                root["co2"].asDouble(),
                root["pm25"].asDouble(),
                root["timestamp"].asInt64(),
                0.0
            };
            
            // 打印传感器数据
            std::cout << "[TCP Server] Sensor readings:" << std::endl
                     << "  Temperature: " << sensor_data.temperature << "°C" << std::endl
                     << "  Humidity: " << sensor_data.humidity << "%" << std::endl
                     << "  CO2: " << sensor_data.co2 << "ppm" << std::endl
                     << "  PM2.5: " << sensor_data.pm25 << "μg/m³" << std::endl;
            
            // 计算环境评分
            sensor_data.score = EnvironmentScorer::calculateScore(sensor_data);
            std::cout << "[TCP Server] Environment score: " << sensor_data.score << std::endl;
            
            // 添加传感器数据到设备管理器
            deviceManager.addSensorData(device_id, sensor_data);
            
            // 获取设备信息并打印数据点数量
            if (auto device = deviceManager.getDeviceInfo(device_id)) {
                std::cout << "[TCP] Added data for device " << device_id 
                         << ", current data points: " 
                         << device->recent_data.size() 
                         << std::endl;
            }
            
            if (data_callback_) {
                data_callback_(sensor_data);
            }
            
            std::cout << "[TCP Server] Data processing completed for device: " << device_id << "\n" 
                     << "----------------------------------------" << std::endl;
            
            // 继续监听下一个数据包
            socket->async_read_some(
                boost::asio::buffer(*buffer),
                [this, socket, buffer](const boost::system::error_code& error,
                                     size_t bytes_transferred) {
                    handle_read(socket, buffer, error, bytes_transferred);
                });
            
        } catch (const std::exception& e) {
            std::cerr << "[TCP Server] Error processing data: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "[TCP Server] Read error: " << error.message() << std::endl;
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