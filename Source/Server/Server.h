#pragma once

#include <boost/asio.hpp>
#include <filesystem>

class Server
{
public:
    Server();

    void run();

private:
    void startAccept();

    void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void readHeaderSize(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void readHeader(std::shared_ptr<boost::asio::ip::tcp::socket> socket, uint8_t headerSize);
    void processHeader(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                       const std::string& header);

    void receiveData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void receiveFile(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    void listenForData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    std::filesystem::path getDesktopPath();

private:
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::unordered_map<std::string, std::shared_ptr<boost::asio::ip::tcp::socket>> _clients;
};