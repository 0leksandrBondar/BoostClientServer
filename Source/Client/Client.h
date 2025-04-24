#pragma once

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>

class Client
{
public:
    Client(const std::string& host, const std::string& port);

    void sendText(const std::string& receiver, const std::string& message);
    void sendFile(const std::string& receiver, const std::string& filename);

private:
    void sendJson(const nlohmann::json& j);
    std::string base64Encode(const std::vector<unsigned char>& bytes_to_encode);

private:
    std::string _senderName{ "unknown" };
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::socket _socket;
};
