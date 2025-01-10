#include <boost/asio.hpp>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include "../src/models/sensor_data.h"

using boost::asio::ip::tcp;

double generateRandomValue(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // 增加随机波动
    static double last_value = (min + max) / 2;
    double variation = (max - min) * 0.7;  // 允许70%的波动范围
    double new_value = last_value + variation * (dis(gen) * 2 - 1);
    
    // 有20%的概率产生极端值
    if (dis(gen) < 0.2) {
        new_value = dis(gen) < 0.5 ? min : max;
    }
    
    // 确保值在范围内
    new_value = std::max(min, std::min(max, new_value));
    last_value = new_value;
    return new_value;
}

bool connectToServer(tcp::socket& socket, boost::asio::io_context& io_context) {
    try {
        socket.connect(tcp::endpoint(
            boost::asio::ip::address::from_string("127.0.0.1"), 8888));
        std::cout << "Connected to server successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void simulateDevice(const std::string& device_id, const std::string& area, AreaType area_type) {
    while (true) {  // 外层循环，确保设备永远运行
        try {
            boost::asio::io_context io_context;
            tcp::socket socket(io_context);
            
            // 尝试连接服务器，失败后等待5秒重试
            while (!connectToServer(socket, io_context)) {
                std::cerr << "Retrying in 5 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                socket.close();
                socket = tcp::socket(io_context);
            }
            
            // 连接成功后的数据发送循环
            while (true) {
                try {
                    // 生成模拟数据
                    Json::Value root;
                    root["device_id"] = device_id;
                    root["timestamp"] = static_cast<Json::Int64>(time(nullptr));
                    
                    // 四季通用的极端数据范围
                    root["temperature"] = generateRandomValue(-20, 40);
                    root["humidity"] = generateRandomValue(10, 95);
                    root["co2"] = generateRandomValue(350, 5000);
                    root["pm25"] = generateRandomValue(0, 500);
                    root["noise"] = generateRandomValue(20, 120);
                    root["light"] = generateRandomValue(0, 100000);
                    
                    root["area"] = area;
                    root["area_type"] = area_type == AreaType::LIVING ? "living" :
                                      area_type == AreaType::TEACHING ? "teaching" : "recreation";
                    
                    // 转换为字符串
                    Json::FastWriter writer;
                    std::string json_str = writer.write(root);
                    
                    // 发送数据
                    boost::asio::write(socket, boost::asio::buffer(json_str));
                    std::cout << "Sent data: " << json_str;
                    
                    // 接收响应
                    std::vector<char> response(1024);
                    size_t len = socket.read_some(boost::asio::buffer(response));
                    if (len > 0) {
                        std::cout << "Data sent successfully" << std::endl;
                    }
                    
                    // 固定5秒发送一次
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    
                } catch (const std::exception& e) {
                    std::cerr << "Error during data transmission: " << e.what() << std::endl;
                    std::cerr << "Connection lost, attempting to reconnect..." << std::endl;
                    break;  // 跳出内层循环，重新连接
                }
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Fatal error: " << e.what() << std::endl;
            std::cerr << "Restarting device simulation..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <device_id> <area> <area_type>\n";
        std::cerr << "Area type: living, teaching, recreation\n";
        return 1;
    }
    
    std::string device_id = argv[1];
    std::string area = argv[2];
    std::string area_type_str = argv[3];
    AreaType area_type;
    
    // 将字符串转换为 AreaType
    if (area_type_str == "living") {
        area_type = AreaType::LIVING;
    } else if (area_type_str == "teaching") {
        area_type = AreaType::TEACHING;
    } else if (area_type_str == "recreation") {
        area_type = AreaType::RECREATION;
    } else {
        std::cerr << "Invalid area type. Must be one of: living, teaching, recreation\n";
        return 1;
    }
    
    simulateDevice(device_id, area, area_type);
    
    return 0;
} 