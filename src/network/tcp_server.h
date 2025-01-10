#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include "../models/sensor_data.h"
#include "../scoring/environment_scorer.h"

using boost::asio::ip::tcp;

class TCPServer {
public:
    TCPServer(boost::asio::io_context& io_context, short port);
    void start();

private:
    void start_accept();
    void handle_accept(std::shared_ptr<tcp::socket> socket,
                      const boost::system::error_code& error);
    void handle_read(std::shared_ptr<tcp::socket> socket,
                    std::shared_ptr<std::vector<char>> buffer,
                    const boost::system::error_code& error,
                    size_t bytes_transferred);
    
    EnvironmentScorer::TimeSlot determineTimeSlot(time_t timestamp);

    tcp::acceptor acceptor_;
    std::function<void(const SensorData&)> data_callback_;
}; 