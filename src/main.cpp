#include "network/tcp_server.h"
#include "network/http_server.h"
#include <iostream>
#include <thread>
#include <csignal>

volatile sig_atomic_t running = true;

void signal_handler(int) {
    running = false;
}

int main() {
    try {
        // 为 TCP 和 HTTP 服务器创建独立的 io_context
        boost::asio::io_context tcp_io_context;
        boost::asio::io_context http_io_context;
        
        // TCP 服务器用于接收设备数据
        std::cout << "[Main] Creating TCP server on port 8888..." << std::endl;
        TCPServer tcp_server(tcp_io_context, 8888);
        tcp_server.start();
        
        // HTTP 服务器用于 API 接口
        std::cout << "[Main] Creating HTTP server on port 8080..." << std::endl;
        HTTPServer http_server(8080);
        http_server.start();  // 只启动接受器
        
        // 创建工作线程来运行 TCP io_context
        std::cout << "[Main] Starting TCP io_context..." << std::endl;
        std::vector<std::thread> tcp_threads;
        for (int i = 0; i < 2; ++i) {
            tcp_threads.emplace_back([&tcp_io_context]() {
                try {
                    tcp_io_context.run();
                } catch (const std::exception& e) {
                    std::cerr << "[TCP Worker] Error: " << e.what() << std::endl;
                }
            });
        }
        
        // 创建工作线程来运行 HTTP io_context
        std::cout << "[Main] Starting HTTP io_context..." << std::endl;
        std::vector<std::thread> http_threads;
        for (int i = 0; i < 2; ++i) {
            http_threads.emplace_back([&http_server]() {
                try {
                    http_server.run();  // 在线程中运行 io_context
                } catch (const std::exception& e) {
                    std::cerr << "[HTTP Worker] Error: " << e.what() << std::endl;
                }
            });
        }
        
        std::cout << "[Main] All services started successfully" << std::endl;
        std::cout << "[Main] Press Ctrl+C to exit" << std::endl;
        
        // 主循环
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // 清理
        std::cout << "\n[Main] Shutting down..." << std::endl;
        tcp_io_context.stop();
        http_server.stop();  // 停止 HTTP 服务器
        
        // 等待所有线程完成
        for (auto& thread : tcp_threads) {
            thread.join();
        }
        for (auto& thread : http_threads) {
            thread.join();
        }
        
        std::cout << "[Main] Shutdown complete" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 