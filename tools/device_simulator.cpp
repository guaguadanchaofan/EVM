#include <boost/asio.hpp>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

using boost::asio::ip::tcp;

class DeviceSimulator {
public:
    DeviceSimulator(const std::string& device_id, 
                   const std::string& host = "127.0.0.1", 
                   int port = 8888)
        : device_id_(device_id)
        , host_(host)
        , port_(port)
        , io_context_()
        , socket_(io_context_)
    {
        // 初始化随机数生成器
        std::random_device rd;
        gen_ = std::mt19937(rd());
        
        // 设置各项指标的正态分布范围
        temp_dist_ = std::normal_distribution<>(23.0, 2.0);    // 温度范围约19-27℃
        hum_dist_ = std::normal_distribution<>(55.0, 10.0);    // 湿度范围约35-75%
        co2_dist_ = std::normal_distribution<>(800.0, 200.0);  // CO2范围约400-1200ppm
        pm25_dist_ = std::normal_distribution<>(50.0, 15.0);   // PM2.5范围约20-80μg/m³
    }
    
    void connect() {
        try {
            std::cout << "Attempting to connect to " << host_ << ":" << port_ << std::endl;
            tcp::resolver resolver(io_context_);
            auto endpoints = resolver.resolve(host_, std::to_string(port_));
            boost::asio::connect(socket_, endpoints);
            std::cout << "Connected to server successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void start(int interval_seconds = 5) {
        while (true) {
            try {
                sendData();
                std::cout << "Data sent successfully" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Failed to send data: " << e.what() << std::endl;
                try {
                    connect(); // 尝试重新连接
                } catch (...) {
                    std::cerr << "Reconnection failed, waiting..." << std::endl;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
        }
    }
    
private:
    void sendData() {
        // 生成模拟数据
        double temperature = std::max(15.0, std::min(30.0, temp_dist_(gen_)));
        double humidity = std::max(30.0, std::min(80.0, hum_dist_(gen_)));
        double co2 = std::max(400.0, std::min(2000.0, co2_dist_(gen_)));
        double pm25 = std::max(0.0, std::min(150.0, pm25_dist_(gen_)));
        
        // 构建JSON数据
        Json::Value data;
        data["device_id"] = device_id_;
        data["timestamp"] = std::time(nullptr);
        data["temperature"] = temperature;
        data["humidity"] = humidity;
        data["co2"] = co2;
        data["pm25"] = pm25;
        
        Json::FastWriter writer;
        std::string json_str = writer.write(data);
        
        // 发送数据
        boost::asio::write(socket_, boost::asio::buffer(json_str));
        
        // 打印发送的数据
        std::cout << "Sent data: " << json_str;
    }
    
    std::string device_id_;
    std::string host_;
    int port_;
    boost::asio::io_context io_context_;
    tcp::socket socket_;
    
    std::mt19937 gen_;
    std::normal_distribution<> temp_dist_;
    std::normal_distribution<> hum_dist_;
    std::normal_distribution<> co2_dist_;
    std::normal_distribution<> pm25_dist_;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <device_id> [host] [port]" << std::endl;
        return 1;
    }
    
    std::string device_id = argv[1];
    std::string host = argc > 2 ? argv[2] : "127.0.0.1";
    int port = argc > 3 ? std::stoi(argv[3]) : 8888;
    
    try {
        DeviceSimulator simulator(device_id, host, port);
        simulator.connect();
        simulator.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 