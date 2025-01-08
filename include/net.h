#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <arpa/inet.h>
#include "device_manager.h"
class Network {
public:
    Network(int port); // 构造函数
    ~Network();        // 析构函数

    bool start();               // 启动服务器
    void stop();                // 停止服务器
    void setDeviceManager(DeviceManager *deviceManager); // 设置设备管理器
    void sendData(int client_socket, const std::string& data); // 发送数据
    std::string receiveData(int client_socket);                // 接收数据

    // 解析数据
    std::vector<std::string> parseData(const std::string& data);

private:
    int port;                   // 监听端口
    int server_socket;          // 服务器套接字
    bool running;               // 服务器运行状态
    std::vector<std::thread> client_threads; // 客户端线程池
    DeviceManager *deviceManager; // 设备管理器
    void handleClient(int client_socket, sockaddr_in client_address); // 处理客户端连接
};