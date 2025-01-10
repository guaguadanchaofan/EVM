#include "network/tcp_server.h"
#include "network/http_server.h"
#include "tasks/data_maintenance.h"
#include <iostream>
#include <thread>
#include <csignal>
#include <boost/asio.hpp>
#include <vector>

volatile sig_atomic_t running = true;

void signal_handler(int) {
    running = false;
}

int main() {
    try {
        // 为 TCP 和 HTTP 服务器创建独立的 io_context
        boost::asio::io_context tcp_io_context;
        boost::asio::io_context http_io_context;
        
        // 创建 work guard 防止 io_context 过早退出
        auto tcp_work_guard = boost::asio::make_work_guard(tcp_io_context);
        auto http_work_guard = boost::asio::make_work_guard(http_io_context);
        
        // 获取数据库单例
        auto& db = Database::getInstance();
        
        // 启动数据维护任务
        DataMaintenanceTask::getInstance().start();
        
        // 启动 TCP 服务器
        TCPServer tcp_server(tcp_io_context, 8888, db);
        tcp_server.start();
        
        // 启动 HTTP 服务器
        HTTPServer http_server(8080);
        http_server.start();
        
        // 创建工作线程池
        std::vector<std::thread> tcp_threads;
        std::vector<std::thread> http_threads;
        const int num_threads = std::thread::hardware_concurrency();
        
        // 启动 TCP 工作线程
        for (int i = 0; i < num_threads - 1; ++i) {
            tcp_threads.emplace_back([&tcp_io_context]() {
                try {
                    tcp_io_context.run();
                } catch (const std::exception& e) {
                    std::cerr << "TCP worker thread error: " << e.what() << std::endl;
                }
            });
        }
        
        // 启动 HTTP 工作线程
        for (int i = 0; i < num_threads - 1; ++i) {
            http_threads.emplace_back([&http_server]() {
                try {
                    http_server.run();
                } catch (const std::exception& e) {
                    std::cerr << "HTTP worker thread error: " << e.what() << std::endl;
                }
            });
        }
        
        // 设置信号处理
        // signal(SIGINT, signal_handler);
        // signal(SIGTERM, signal_handler);
        
        // 主线程运行 TCP io_context
        try {
            tcp_io_context.run();
        } catch (const std::exception& e) {
            std::cerr << "Main thread error: " << e.what() << std::endl;
        }
        
        // 等待所有工作线程完成
        for (auto& thread : tcp_threads) {
            thread.join();
        }
        for (auto& thread : http_threads) {
            thread.join();
        }
        
        // 停止数据维护任务
        DataMaintenanceTask::getInstance().stop();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 