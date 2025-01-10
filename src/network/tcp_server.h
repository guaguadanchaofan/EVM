#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include "../models/sensor_data.h"
#include "../scoring/environment_scorer.h"
#include "../database/database.h"
#include "../services/environment_service.h"

using boost::asio::ip::tcp;

class TCPServer {
public:
    TCPServer(boost::asio::io_context& io_context, short port, Database& db);
    void start();

private:
    void start_accept();
    void handle_accept(std::shared_ptr<tcp::socket> socket,
                      const boost::system::error_code& error);
    void start_read(std::shared_ptr<tcp::socket> socket);
    void handle_read(std::shared_ptr<tcp::socket> socket,
                    std::shared_ptr<std::vector<char>> buffer,
                    const boost::system::error_code& error,
                    size_t bytes_transferred);
    
    EnvironmentScorer::TimeSlot determineTimeSlot(time_t timestamp);

    tcp::acceptor acceptor_;
    std::function<void(const SensorData&)> data_callback_;
    EnvironmentService environment_service_;
    Database& database_;
}; 