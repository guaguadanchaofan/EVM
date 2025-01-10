#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include "../models/sensor_data.h"
#include "../device/device_manager.h"
#include "../scoring/environment_scorer.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class HTTPServer {
public:
    explicit HTTPServer(int port);
    void start();
    void run();
    void stop();

private:
    void do_accept();
    void handle_request(http::request<http::string_body>&& req, tcp::socket& socket);
    
    // 设备管理接口
    void handleRegisterDevice(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleUnregisterDevice(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetDevices(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetDeviceStatus(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleUpdateDeviceConfig(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    
    // 数据查询接口
    void handleGetRealtimeData(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetHistoryData(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetStatistics(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handlePostData(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    
    // 评分和建议接口
    void handleGetRealtimeScore(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetHistoryScores(const http::request<http::string_body>& req, http::response<http::string_body>& res);
    void handleGetSuggestions(const http::request<http::string_body>& req, http::response<http::string_body>& res);

    // 添加评分辅助函数声明
    double calculateTemperatureScore(double temp);
    double calculateHumidityScore(double humidity);
    double calculateCO2Score(double co2);
    double calculatePM25Score(double pm25);
    
    std::string getTemperatureStatus(double temp);
    std::string getHumidityStatus(double humidity);
    std::string getCO2Status(double co2);
    std::string getPM25Status(double pm25);

    // 成员变量按照初始化顺序声明
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    beast::flat_buffer buffer_;
    int port_;
}; 