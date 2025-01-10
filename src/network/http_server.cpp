#include "http_server.h"
#include <iostream>
#include <boost/beast/version.hpp>
#include <jsoncpp/json/json.h>
#include <fstream>

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
    response->set(http::field::access_control_allow_origin, "*");
    response->set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    response->set(http::field::access_control_allow_headers, "Content-Type, Accept");
    response->set(http::field::access_control_max_age, "3600");
    response->set(http::field::connection, "keep-alive");  // 保持连接
    
    // 处理预检请求
    if (req.method() == http::verb::options) {
        response->result(http::status::no_content);
        response->prepare_payload();
        beast::error_code ec;
        http::write(socket, *response, ec);
        return;
    }
    
    try {
        if (req.target() == "/") {
            static std::string cached_index;  // 缓存index.html
            if (cached_index.empty()) {
                std::string filepath = "../web/index.html";
                std::ifstream file(filepath);
                if (file) {
                    cached_index = std::string(
                        std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>()
                    );
                }
            }
            
            if (!cached_index.empty()) {
                response->set(http::field::content_type, "text/html");
                response->body() = cached_index;
            } else {
                response->result(http::status::not_found);
                response->body() = "404 Not Found\n";
            }
        }
        else if (req.target() == "/api/devices" && req.method() == http::verb::get) {
            response->set(http::field::content_type, "application/json");
            handleGetDevices(*response);
        }
        else if (req.target() == "/api/data/realtime" && req.method() == http::verb::get) {
            response->set(http::field::content_type, "application/json");
            handleGetRealtimeData(req, *response);
        }
        else if (req.target() == "/api/score/realtime" && req.method() == http::verb::get) {
            handleGetRealtimeScore(req, *response);
        }
        else if (req.target().starts_with("/api/device/") && req.target().find("/history") == std::string::npos) {
            // 处理单个设备的实时数据请求
            std::string device_id = std::string(req.target()).substr(12);  // 移除 "/api/device/"
            handleGetDeviceData(device_id, *response);
        }
        else if (req.target().starts_with("/api/device/") && req.target().find("/history") != std::string::npos) {
            // 处理历史数据请求
            std::string path = std::string(req.target());
            size_t historyPos = path.find("/history");
            std::string device_id = path.substr(12, historyPos - 12);  // 提取设备ID
            
            // 解析数据类型参数
            std::string dataType = "realtime";  // 默认类型
            time_t start_time = 0, end_time = time(nullptr);
            
            if (path.find('?') != std::string::npos) {
                std::string query = path.substr(path.find('?') + 1);
                std::istringstream iss(query);
                std::string param;
                while (std::getline(iss, param, '&')) {
                    if (param.substr(0, 6) == "start=") {
                        start_time = std::stoll(param.substr(6));
                    } else if (param.substr(0, 4) == "end=") {
                        end_time = std::stoll(param.substr(4));
                    } else if (param.substr(0, 5) == "type=") {
                        dataType = param.substr(5);
                    }
                }
            }
            
            handleGetDeviceHistory(device_id, dataType, start_time, end_time, *response);
        }
        else {
            response->result(http::status::not_found);
            response->body() = "404 Not Found\n";
        }
    }
    catch (const std::exception& e) {
        response->result(http::status::internal_server_error);
        Json::Value error;
        error["error"] = e.what();
        Json::FastWriter writer;
        response->body() = writer.write(error);
    }
    
    response->prepare_payload();
    
    // 使用异步写入
    auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
    http::async_write(
        *socket_ptr,
        *response,
        [response, socket_ptr](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "[HTTP] Write error: " << ec.message() << std::endl;
            }
        });
}

void HTTPServer::handleGetRealtimeData(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    Json::Value root;
    root["data"] = Json::Value(Json::arrayValue);
    
    auto& devices = DeviceManager::getInstance().getDevices();
    std::cout << "[HTTP] GetRealtimeData: found " << devices.size() << " devices" << std::endl;
    
    for (const auto& device : devices) {
        std::cout << "[HTTP] Processing device " << device.first << std::endl;
        
        if (!device.second->recent_data.empty()) {
            const auto& latest_data = device.second->recent_data.back();
            
            Json::Value deviceData;
            deviceData["device_id"] = device.first;
            deviceData["area"] = latest_data.area;
            deviceData["area_type"] = static_cast<int>(latest_data.area_type);
            // 根据最后心跳时间判断设备状态
            time_t now = time(nullptr);
            bool isOnline = (now - device.second->last_heartbeat) <= 30;  // 30秒内有心跳就认为在线
            deviceData["device_status"] = isOnline ? 1 : 0;  // 1表示在线，0表示离线
            
            deviceData["temperature"] = latest_data.temperature;
            deviceData["humidity"] = latest_data.humidity;
            deviceData["co2"] = latest_data.co2;
            deviceData["pm25"] = latest_data.pm25;
            deviceData["noise"] = latest_data.noise;
            deviceData["light"] = latest_data.light;
            deviceData["timestamp"] = static_cast<Json::Int64>(latest_data.timestamp);
            
            // 添加评分数据
            deviceData["scores"] = Json::Value();
            deviceData["scores"]["temperature"] = latest_data.scores.temperature;
            deviceData["scores"]["humidity"] = latest_data.scores.humidity;
            deviceData["scores"]["co2"] = latest_data.scores.co2;
            deviceData["scores"]["pm25"] = latest_data.scores.pm25;
            deviceData["scores"]["noise"] = latest_data.scores.noise;
            deviceData["scores"]["light"] = latest_data.scores.light;
            deviceData["scores"]["overall"] = latest_data.scores.overall;
            
            // 添加状态数据
            deviceData["status"] = Json::Value();
            deviceData["status"]["temperature"] = latest_data.status.temperature;
            deviceData["status"]["humidity"] = latest_data.status.humidity;
            deviceData["status"]["co2"] = latest_data.status.co2;
            deviceData["status"]["pm25"] = latest_data.status.pm25;
            deviceData["status"]["noise"] = latest_data.status.noise;
            deviceData["status"]["light"] = latest_data.status.light;
            
            // 添加建议
            Json::Value suggestionsArray(Json::arrayValue);
            for (const auto& suggestion : latest_data.suggestions) {
                suggestionsArray.append(suggestion);
            }
            deviceData["suggestions"] = suggestionsArray;
            
            root["data"].append(deviceData);
            std::cout << "[HTTP] Added device data to response" << std::endl;
        } else {
            std::cout << "[HTTP] Device has no data" << std::endl;
        }
    }
    
    Json::FastWriter writer;
    std::string jsonStr = writer.write(root);
    
    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    res.body() = jsonStr;
}

void HTTPServer::handleGetRealtimeScore(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    Json::Value root;
    root["data"] = Json::Value(Json::arrayValue);
    
    auto& devices = DeviceManager::getInstance().getDevices();
    
    for (const auto& device : devices) {
        if (!device.second->recent_data.empty()) {
            const auto& latest_data = device.second->recent_data.back();
            
            Json::Value scoreData;
            scoreData["device_id"] = device.first;
            scoreData["scores"] = Json::Value();
            scoreData["scores"]["temperature"] = latest_data.scores.temperature;
            scoreData["scores"]["humidity"] = latest_data.scores.humidity;
            scoreData["scores"]["co2"] = latest_data.scores.co2;
            scoreData["scores"]["pm25"] = latest_data.scores.pm25;
            scoreData["scores"]["noise"] = latest_data.scores.noise;
            scoreData["scores"]["light"] = latest_data.scores.light;
            scoreData["scores"]["overall"] = latest_data.scores.overall;
            
            // 添加各项指标的评分详情
            Json::Value tempDetails;
            tempDetails["value"] = latest_data.temperature;
            tempDetails["score"] = latest_data.scores.temperature;
            tempDetails["status"] = getTemperatureStatus(latest_data.temperature, latest_data.area_type);
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
            
            // 添加噪音评分
            Json::Value noiseDetails;
            noiseDetails["value"] = latest_data.noise;
            noiseDetails["score"] = calculateNoiseScore(latest_data.noise, latest_data.area_type);
            noiseDetails["status"] = getNoiseStatus(latest_data.noise, latest_data.area_type);
            scoreData["details"]["noise"] = noiseDetails;
            
            // 添加光照评分
            Json::Value lightDetails;
            lightDetails["value"] = latest_data.light;
            lightDetails["score"] = calculateLightScore(latest_data.light, latest_data.area_type);
            lightDetails["status"] = getLightStatus(latest_data.light, latest_data.area_type);
            scoreData["details"]["light"] = lightDetails;
            
            scoreData["timestamp"] = static_cast<Json::Int64>(latest_data.timestamp);
            root["data"].append(scoreData);
        }
    }
    
    Json::FastWriter writer;
    res.body() = writer.write(root);
}

double HTTPServer::calculateTemperatureScore(double temp, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区温度要求更舒适
            if (temp >= 22 && temp <= 26) return 100;
            if (temp < 22) return 100 - (22 - temp) * 12;
            return 100 - (temp - 26) * 12;
            
        case AreaType::TEACHING:
            // 教学区温度要求适中
            if (temp >= 20 && temp <= 25) return 100;
            if (temp < 20) return 100 - (20 - temp) * 10;
            return 100 - (temp - 25) * 10;
            
        case AreaType::RECREATION:
            // 娱乐区温度容许范围更大
            if (temp >= 18 && temp <= 27) return 100;
            if (temp < 18) return 100 - (18 - temp) * 8;
            return 100 - (temp - 27) * 8;
    }
    return 0;
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

double HTTPServer::calculateNoiseScore(double noise, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区要求安静
            if (noise <= 40) return 100;
            if (noise <= 50) return 80;
            if (noise <= 60) return 60;
            if (noise <= 70) return 40;
            return 20;
            
        case AreaType::TEACHING:
            // 教学区要求较安静
            if (noise <= 45) return 100;
            if (noise <= 55) return 80;
            if (noise <= 65) return 60;
            if (noise <= 75) return 40;
            return 20;
            
        case AreaType::RECREATION:
            // 娱乐区允许较大噪音
            if (noise <= 55) return 100;
            if (noise <= 65) return 80;
            if (noise <= 75) return 60;
            if (noise <= 85) return 40;
            return 20;
    }
    return 0;
}

double HTTPServer::calculateLightScore(double light, AreaType area_type) {
    switch (area_type) {
        case AreaType::LIVING:
            // 生活区光照要求舒适
            if (light >= 200 && light <= 500) return 100;
            if (light < 200) return 60 + (light / 200) * 40;
            if (light <= 750) return 80;
            if (light <= 1000) return 60;
            return 40;
            
        case AreaType::TEACHING:
            // 教学区要求充足明亮
            if (light >= 400 && light <= 750) return 100;
            if (light < 400) return 60 + (light / 400) * 40;
            if (light <= 1000) return 80;
            if (light <= 1500) return 60;
            return 40;
            
        case AreaType::RECREATION:
            // 娱乐区光照要求灵活
            if (light >= 300 && light <= 1000) return 100;
            if (light < 300) return 60 + (light / 300) * 40;
            if (light <= 1500) return 80;
            if (light <= 2000) return 60;
            return 40;
    }
    return 0;
}

std::string HTTPServer::getTemperatureStatus(double temp, AreaType type) {
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

std::string HTTPServer::getNoiseStatus(double noise, AreaType type) {
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

std::string HTTPServer::getLightStatus(double light, AreaType type) {
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

void HTTPServer::handleGetDevices(http::response<http::string_body>& response) {
    Json::Value devices(Json::arrayValue);  // 直接返回数组
    
    auto& deviceMap = DeviceManager::getInstance().getDevices();
    for (const auto& device : deviceMap) {
        Json::Value deviceJson;
        deviceJson["device_id"] = device.first;
        if (!device.second->recent_data.empty()) {
            const auto& latest_data = device.second->recent_data.back();
            deviceJson["area"] = latest_data.area;
            deviceJson["area_type"] = static_cast<int>(latest_data.area_type);
        } else {
            deviceJson["area"] = "";
            deviceJson["area_type"] = 0;
        }
        deviceJson["status"] = static_cast<int>(device.second->status);
        deviceJson["last_update"] = static_cast<Json::Int64>(device.second->last_heartbeat);
        
        devices.append(deviceJson);  // 直接添加到数组
    }
    
    response.result(http::status::ok);
    response.set(http::field::content_type, "application/json");
    response.body() = devices.toStyledString();
}

void HTTPServer::handleGetDeviceData(const std::string& device_id,
                                   http::response<http::string_body>& response) {
    Json::Value root;
    root["code"] = 0;
    root["message"] = "success";
    root["data"] = Json::Value(Json::arrayValue);
    
    auto& devices = DeviceManager::getInstance().getDevices();
    auto it = devices.find(device_id);
    if (it != devices.end() && it->second) {
        const auto& device = it->second;
        if (!device->recent_data.empty()) {
            const auto& latest_data = device->recent_data.back();
            
            Json::Value dataJson;
            dataJson["device_id"] = device_id;
            dataJson["area"] = latest_data.area;
            dataJson["area_type"] = static_cast<int>(latest_data.area_type);
            dataJson["device_status"] = (time(nullptr) - device->last_heartbeat) <= 30 ? 1 : 0;  // 30秒内有心跳就认为在线
            dataJson["temperature"] = latest_data.temperature;
            dataJson["humidity"] = latest_data.humidity;
            dataJson["co2"] = latest_data.co2;
            dataJson["pm25"] = latest_data.pm25;
            dataJson["noise"] = latest_data.noise;
            dataJson["light"] = latest_data.light;
            dataJson["timestamp"] = static_cast<Json::Int64>(latest_data.timestamp);
            
            // 添加评分数据
            dataJson["scores"] = Json::Value();
            dataJson["scores"]["temperature"] = latest_data.scores.temperature;
            dataJson["scores"]["humidity"] = latest_data.scores.humidity;
            dataJson["scores"]["co2"] = latest_data.scores.co2;
            dataJson["scores"]["pm25"] = latest_data.scores.pm25;
            dataJson["scores"]["noise"] = latest_data.scores.noise;
            dataJson["scores"]["light"] = latest_data.scores.light;
            dataJson["scores"]["overall"] = latest_data.scores.overall;
            
            // 添加状态数据
            dataJson["status"] = Json::Value();
            dataJson["status"]["temperature"] = latest_data.status.temperature;
            dataJson["status"]["humidity"] = latest_data.status.humidity;
            dataJson["status"]["co2"] = latest_data.status.co2;
            dataJson["status"]["pm25"] = latest_data.status.pm25;
            dataJson["status"]["noise"] = latest_data.status.noise;
            dataJson["status"]["light"] = latest_data.status.light;
            
            // 添加建议
            Json::Value suggestionsArray(Json::arrayValue);
            for (const auto& suggestion : latest_data.suggestions) {
                suggestionsArray.append(suggestion);
            }
            dataJson["suggestions"] = suggestionsArray;
            
            root["data"].append(dataJson);
        }
    }
    
    response.result(http::status::ok);
    response.set(http::field::content_type, "application/json");
    response.body() = root.toStyledString();
}

void HTTPServer::handleGetDeviceHistory(const std::string& device_id,
                                       const std::string& dataType,
                                       time_t start_time,
                                       time_t end_time,
                                       http::response<http::string_body>& response) {
    std::cout << "[HTTP] Handling history request - Device: " << device_id 
              << ", Type: " << dataType << std::endl;
    
    std::vector<SensorData> history_data;
    
    // 根据数据类型选择不同的查询方法
    if (dataType == "realtime") {
        history_data = Database::getInstance().queryRealtimeData(device_id, start_time, end_time);
    } else if (dataType == "hourly") {
        history_data = Database::getInstance().queryHourlyData(device_id, start_time, end_time);
    } else if (dataType == "daily") {
        history_data = Database::getInstance().queryDailyData(device_id, start_time, end_time);
    }
    
    // 如果没有数据，返回空数组
    if (history_data.empty()) {
        Json::Value root;
        root["data"] = Json::Value(Json::arrayValue);
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        response.set(http::field::access_control_allow_origin, "*");
        response.body() = root.toStyledString();
        return;
    }
    
    // 构建JSON响应
    Json::Value root;
    Json::Value data_array(Json::arrayValue);
    
    for (const auto& data : history_data) {
        Json::Value dataJson;
        dataJson["timestamp"] = Json::Int64(data.timestamp);
        dataJson["temperature"] = data.temperature;
        dataJson["humidity"] = data.humidity;
        dataJson["co2"] = data.co2;
        dataJson["pm25"] = data.pm25;
        dataJson["noise"] = data.noise;
        dataJson["light"] = data.light;
        data_array.append(dataJson);
    }
    
    root["data"] = data_array;
    
    std::cout << "[HTTP] Returning " << history_data.size() << " records" << std::endl;
    
    response.result(http::status::ok);
    response.set(http::field::content_type, "application/json");
    response.set(http::field::access_control_allow_origin, "*");
    response.body() = root.toStyledString();
}