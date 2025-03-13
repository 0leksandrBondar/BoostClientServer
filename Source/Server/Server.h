#pragma once

#include <boost/asio.hpp>
#include <filesystem>

class Server
{
public:
    Server();

    void run();

private:
    void accept();

    void receiveData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void receiveFile(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    void listenForData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    std::filesystem::path getDesktopPath();

private:
    boost::asio::io_context _ioContext;
    boost::asio::ip::tcp::acceptor _acceptor;
};