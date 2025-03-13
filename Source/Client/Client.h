#pragma once

#include <boost/asio.hpp>
#include <filesystem>

enum class MessageType
{
    TEXT,
    FILE,
};

class Client
{
public:
    Client() = default;
    Client(const std::string& ip, const std::string& port);

    void connect(const std::string& ip, const std::string& port);
    void sendData(const std::string& type, const std::string& data);

private:
    static size_t getFileSize(std::ifstream& file);

private:
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;
};