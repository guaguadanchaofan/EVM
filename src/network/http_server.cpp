#include "http_server.h"
#include <iostream>
#include <boost/beast/version.hpp>
#include <jsoncpp/json/json.h>

HTTPServer::HTTPServer(int port)
    : ioc_()
    , acceptor_(ioc_, {net::ip::make_address("0.0.0.0"), static_cast<unsigned short>(port)})
    , buffer_()
    , port_(port)
{
}

void HTTPServer::start() {
    do_accept();
    std::cout << "[HTTP] Server started on port " << port_ << std::endl;
}

void HTTPServer::run() {
    ioc_.run();
}

void HTTPServer::stop() {
    ioc_.stop();
}

void HTTPServer::do_accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
                auto req = std::make_shared<http::request<http::string_body>>();
                
                http::async_read(*socket_ptr, buffer_, *req,
                    [this, req, socket_ptr](beast::error_code ec, std::size_t) {
                        if (!ec) {
                            handle_request(std::move(*req), *socket_ptr);
                        } else {
                            std::cerr << "[HTTP] Read error: " << ec.message() << std::endl;
                        }
                    });
            } else {
                std::cerr << "[HTTP] Accept error: " << ec.message() << std::endl;
            }
            
            do_accept();
        });
}

void HTTPServer::handle_request(http::request<http::string_body>&& req, tcp::socket& socket) {
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(req.version());
    response->set(http::field::server, "EVM Monitor");
    response->set(http::field::content_type, "application/json");
    response->set(http::field::access_control_allow_origin, "*");
    
    try {
        if (req.target() == "/api/data/realtime" && req.method() == http::verb::get) {
            handleGetRealtimeData(req, *response);
        }
        else if (req.target() == "/api/score/realtime" && req.method() == http::verb::get) {
            handleGetRealtimeScore(req, *response);
        }
        else {
            response->result(http::status::not_found);
            response->body() = "404 Not Found\n";
        }
    }
    catch (const std::exception& e) {
        response->result(http::status::internal_server_error);
        response->body() = std::string("Error: ") + e.what() + "\n";
    }
    
    response->prepare_payload();
    
    http::async_write(socket, *response,
        [response](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "[HTTP] Write error: " << ec.message() << std::endl;
            }
        });
}

void HTTPServer::handleGetRealtimeData(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    Json::Value response(Json::arrayValue);
    auto& devices = DeviceManager::getInstance().getDevices();
    
    std::cout << "[HTTP] GetRealtimeData: found " << devices.size() << " devices" << std::endl;
    
    for (const auto& device : devices) {
        std::cout << "[HTTP] Device " << device.first << " has " 
                  << device.second->recent_data.size() << " data points" << std::endl;
        
        if (!device.second->recent_data.empty()) {
            const auto& latest_data = device.second->recent_data.back();
            
            Json::Value deviceData;
            deviceData["device_id"] = device.first;
            deviceData["temperature"] = latest_data.temperature;
            deviceData["humidity"] = latest_data.humidity;
            deviceData["co2"] = latest_data.co2;
            deviceData["pm25"] = latest_data.pm25;
            deviceData["score"] = latest_data.score;
            deviceData["timestamp"] = static_cast<Json::Int64>(latest_data.timestamp);
            deviceData["status"] = static_cast<int>(device.second->status);
            
            response.append(deviceData);
        }
    }
    
    Json::FastWriter writer;
    res.body() = writer.write(response);
}

void HTTPServer::handleGetRealtimeScore(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    Json::Value response(Json::arrayValue);
    auto& devices = DeviceManager::getInstance().getDevices();
    
    for (const auto& device : devices) {
        if (!device.second->recent_data.empty()) {
            const auto& latest_data = device.second->recent_data.back();
            
            Json::Value scoreData;
            scoreData["device_id"] = device.first;
            scoreData["score"] = latest_data.score;
            
            // 添加各项指标的评分详情
            Json::Value tempDetails;
            tempDetails["value"] = latest_data.temperature;
            tempDetails["score"] = calculateTemperatureScore(latest_data.temperature);
            tempDetails["status"] = getTemperatureStatus(latest_data.temperature);
            scoreData["details"]["temperature"] = tempDetails;
            
            Json::Value humidityDetails;
            humidityDetails["value"] = latest_data.humidity;
            humidityDetails["score"] = calculateHumidityScore(latest_data.humidity);
            humidityDetails["status"] = getHumidityStatus(latest_data.humidity);
            scoreData["details"]["humidity"] = humidityDetails;
            
            Json::Value co2Details;
            co2Details["value"] = latest_data.co2;
            co2Details["score"] = calculateCO2Score(latest_data.co2);
            co2Details["status"] = getCO2Status(latest_data.co2);
            scoreData["details"]["co2"] = co2Details;
            
            Json::Value pm25Details;
            pm25Details["value"] = latest_data.pm25;
            pm25Details["score"] = calculatePM25Score(latest_data.pm25);
            pm25Details["status"] = getPM25Status(latest_data.pm25);
            scoreData["details"]["pm25"] = pm25Details;
            
            scoreData["timestamp"] = static_cast<Json::Int64>(latest_data.timestamp);
            response.append(scoreData);
        }
    }
    
    Json::FastWriter writer;
    res.body() = writer.write(response);
}

double HTTPServer::calculateTemperatureScore(double temp) {
    if (temp >= 20 && temp <= 26) return 100;
    if (temp < 20) return 100 - (20 - temp) * 10;
    return 100 - (temp - 26) * 10;
}

double HTTPServer::calculateHumidityScore(double humidity) {
    if (humidity >= 40 && humidity <= 60) return 100;
    if (humidity < 40) return 100 - (40 - humidity) * 2;
    return 100 - (humidity - 60) * 2;
}

double HTTPServer::calculateCO2Score(double co2) {
    if (co2 <= 800) return 100;
    if (co2 <= 1000) return 80;
    if (co2 <= 1500) return 60;
    if (co2 <= 2000) return 40;
    return 20;
}

double HTTPServer::calculatePM25Score(double pm25) {
    if (pm25 <= 35) return 100;
    if (pm25 <= 75) return 80;
    if (pm25 <= 115) return 60;
    if (pm25 <= 150) return 40;
    return 20;
}

std::string HTTPServer::getTemperatureStatus(double temp) {
    if (temp >= 20 && temp <= 26) return "正常";
    if (temp < 20) return "偏冷";
    return "偏热";
}

std::string HTTPServer::getHumidityStatus(double humidity) {
    if (humidity >= 40 && humidity <= 60) return "正常";
    if (humidity < 40) return "偏干";
    return "偏湿";
}

std::string HTTPServer::getCO2Status(double co2) {
    if (co2 <= 800) return "优";
    if (co2 <= 1000) return "良";
    if (co2 <= 1500) return "中";
    if (co2 <= 2000) return "差";
    return "很差";
}

std::string HTTPServer::getPM25Status(double pm25) {
    if (pm25 <= 35) return "优";
    if (pm25 <= 75) return "良";
    if (pm25 <= 115) return "中";
    if (pm25 <= 150) return "差";
    return "很差";
} 