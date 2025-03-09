#include "Server.h"

#include <spdlog/spdlog.h>
#include <fstream>

Server::Server()
    : _ioContext{},
      _acceptor{ _ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12345) }
{
    accept();
}

void Server::run() { _ioContext.run(); }

void Server::accept()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioContext);
    _acceptor.async_accept(*socket,
                           [this, socket](boost::system::error_code ec)
                           {
                               if (!ec)
                               {
                                   spdlog::info("New client connected!");
                                   listenForData(socket);
                               }
                               accept();
                           });
}

void Server::receiveFile(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    try
    {
        size_t fileSize;
        boost::asio::read(*socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));
        spdlog::info("Receiving file of size: {} bytes", fileSize);

        std::filesystem::path desktopPath = getDesktopPath();
        if (desktopPath.empty())
        {
            spdlog::error("Could not determine desktop path!");
            return;
        }
        std::filesystem::path filePath = desktopPath / "receivedData.mp4";
        std::ofstream outputFile(filePath, std::ios::binary);
        if (!outputFile)
        {
            spdlog::error("Could not create file at {}", filePath.string());
            return;
        }
        constexpr size_t bufferSize = 4096;
        char buffer[bufferSize];
        size_t totalReceived = 0;
        while (totalReceived < fileSize)
        {
            size_t bytesToRead = std::min(bufferSize, fileSize - totalReceived);
            size_t bytesRead = boost::asio::read(*socket, boost::asio::buffer(buffer, bytesToRead));
            outputFile.write(buffer, bytesRead);
            totalReceived += bytesRead;
            spdlog::info("Received packet: {} bytes (Total: {} / {} bytes)", bytesRead,
                         totalReceived, fileSize);
        }
        spdlog::info("File received successfully! Saved to: {}", filePath.string());
        listenForData(socket);
    }
    catch (const std::exception& e)
    {
        spdlog::error("File receive failed: {}", e.what());
    }
}

void Server::receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto buffer = std::make_shared<std::array<char, 1024>>();
    socket->async_read_some(boost::asio::buffer(*buffer),
                            [this, socket, buffer](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    std::string message(buffer->data(), length);
                                    spdlog::info("Received text: {} ({} bytes)", message, length);
                                    listenForData(socket);
                                }
                                else
                                {
                                    spdlog::error("Receiving text failed: {}", ec.message());
                                }
                            });
}

void Server::listenForData(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto buffer = std::make_shared<std::array<char, 4>>();
    boost::asio::async_read(*socket, boost::asio::buffer(*buffer),
                            [this, socket, buffer](boost::system::error_code ec,
                                                   std::size_t bytesRead)
                            {
                                if (!ec)
                                {
                                    std::string header(buffer->data(), 4);
                                    spdlog::info("Received header: {} ({} bytes)", header,
                                                 bytesRead);
                                    if (header == "TEXT")
                                    {
                                        receiveText(socket);
                                    }
                                    else if (header == "FILE")
                                    {
                                        receiveFile(socket);
                                    }
                                    else
                                    {
                                        spdlog::error("Unknown message type!");
                                        listenForData(socket);
                                    }
                                }
                                else
                                {
                                    spdlog::warn("Client disconnected: {}", ec.message());
                                }
                            });
}

std::filesystem::path Server::getDesktopPath()
{
#ifdef _WIN32
    char* userProfile = std::getenv("USERPROFILE");
    if (userProfile)
    {
        return std::filesystem::path(userProfile) / "Desktop";
    }
#elif __linux__
    char* home = std::getenv("HOME");
    if (home)
    {
        return std::filesystem::path(home) / "Desktop";
    }
#endif
    return {};
}
