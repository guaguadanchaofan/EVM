#include "net.h"

// 构造函数
Network::Network(int port) : port(port), server_socket(-1), running(false) {}

// 析构函数
Network::~Network()
{
    stop();
}

// 启动服务器
bool Network::start()
{
    // 创建TCP套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "[-] 创建套接字失败" << std::endl;
        return false;
    }
    // 设置 SO_REUSEADDR 选项
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        std::cerr << "设置 SO_REUSEADDR 失败" << std::endl;
        close(server_socket);
        return false;
    }
    // 绑定IP和端口
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口
    server_address.sin_port = htons(port);

    if (bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "[-] 绑定失败" << std::endl;
        close(server_socket);
        return false;
    }

    // 开始监听
    if (listen(server_socket, 5) < 0)
    {
        std::cerr << "[-] 监听失败" << std::endl;
        close(server_socket);
        return false;
    }

    std::cout << "[*] 监听 0.0.0.0:" << port << " ..." << std::endl;
    running = true;

    // 接受客户端连接
    while (running)
    {
        sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0)
        {
            std::cerr << "[-] 接受连接失败" << std::endl;
            continue;
        }

        // 为每个客户端创建一个线程
        client_threads.emplace_back(&Network::handleClient, this, client_socket, client_address);
    }

    return true;
}

// 停止服务器
void Network::stop()
{
    running = false;
    for (auto &thread : client_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    close(server_socket);
    std::cout << "[*] 服务器已停止" << std::endl;
}

void Network::setDeviceManager(DeviceManager *deviceManager)
{
    this->deviceManager = deviceManager;
}

// 处理客户端连接
void Network::handleClient(int client_socket, sockaddr_in client_address)
{
    std::cout << "[+] 新连接: " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;

    while (running)
    {
        std::string data = receiveData(client_socket);
        if (data.empty())
        {
            break;
        }
        parseData(data);
        std::cout << "来自 " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << " 的数据: " << data << std::endl;
        // 发送响应
        sendData(client_socket, "数据已接收");
    }

    close(client_socket);
    std::cout << "[-] 连接关闭: " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;
}

// 发送数据
void Network::sendData(int client_socket, const std::string &data)
{
    send(client_socket, data.c_str(), data.size(), 0);
}

// 接收数据
std::string Network::receiveData(int client_socket)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0)
    {
        return ""; // 连接关闭或错误
    }
    return std::string(buffer, bytes_received);
}
std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> Network::parseData(const std::string &data)
{
    std::vector<std::string> fields = split(data, ' '); // 使用空格分割
    if (fields.size() != 8)
    {
        std::cerr << "[-] 数据格式错误" << std::endl;
        return {};
    }
    std::string name = fields[0];
    std::string type = fields[1];
    float temperature = std::stof(fields[2]);
    float humidity = std::stof(fields[3]);
    float noise = std::stof(fields[4]);
    float airQuality = std::stof(fields[5]);
    float light = std::stof(fields[6]);
    int online = std::stoi(fields[7]);
    // std::cout << "name: " << name << std::endl;
    // std::cout << "type: " << type << std::endl;
    // std::cout << "temperature: " << temperature << std::endl;
    // std::cout << "humidity: " << humidity << std::endl;
    // std::cout << "noise: " << noise << std::endl;
    // std::cout << "airQuality: " << airQuality << std::endl;
    // std::cout << "light: " << light << std::endl;
    // std::cout << "online: " << online << std::endl;
    deviceManager->addDevice(std::make_shared<Device>(name, type, temperature, humidity, noise, airQuality, light, online));
    return fields;
}


