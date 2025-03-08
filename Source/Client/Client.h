#pragma once

#include <boost/asio.hpp>
#include <filesystem>

class Client
{
public:
    Client() = default;
    Client(const std::string& ip, const std::string& port);

    void connect(const std::string& ip, const std::string& port);

    void sendText(const std::string& message);
    void sendFile(const std::filesystem::path& filePath);

private:
    static size_t getFileSize(std::ifstream& file);

private:
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;
};