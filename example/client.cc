#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

const char* SERVER_IP = "127.0.0.1";
const int PORT = 12345;

int main() {
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "创建套接字失败" << std::endl;
        return 1;
    }

    // 连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "连接服务器失败" << std::endl;
        close(client_socket);
        return 1;
    }

    std::cout << "已连接到服务器" << std::endl;

    // 发送数据
    const char* message = "device test 25.0 50.0 30.0 80.0 100.0 1";
    if (send(client_socket, message, strlen(message), 0) < 0) {
        std::cerr << "发送数据失败" << std::endl;
    } else {
        std::cout << "数据已发送: " << message << std::endl;
    }

    // 关闭套接字
    close(client_socket);
    return 0;
}