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
                                   receiveData(socket);
                               }
                               accept();
                           });
}

void Server::receiveData(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto headerSizeBuffer = std::make_shared<uint8_t>(0);
    boost::asio::async_read(
        *socket, boost::asio::buffer(headerSizeBuffer.get(), sizeof(uint8_t)),
        [this, socket, headerSizeBuffer](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                uint8_t headerSize = *headerSizeBuffer;

                if (headerSize == 0 || headerSize > 50)
                {
                    spdlog::error("Invalid header size: {}", headerSize);
                    return;
                }

                auto headerBuffer = std::make_shared<std::vector<char>>(headerSize);
                boost::asio::async_read(*socket, boost::asio::buffer(*headerBuffer),
                                        [this, socket, headerBuffer](boost::system::error_code ec,
                                                                     std::size_t bytesRead)
                                        {
                                            if (!ec)
                                            {
                                                std::string header(headerBuffer->begin(),
                                                                   headerBuffer->end());
                                                spdlog::info("Received header: {}", header);

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
                                                    spdlog::warn("Unknown data type: {}", header);
                                                    receiveData(socket);
                                                }
                                            }
                                            else
                                            {
                                                spdlog::warn("Client disconnected: {}",
                                                             ec.message());
                                            }
                                        });
            }
            else
            {
                spdlog::warn("Client disconnected: {}", ec.message());
            }
        });
}

void Server::receiveFile(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    try
    {
        size_t fileSize;
        boost::asio::read(*socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));
        spdlog::info("Receiving file of size: {} bytes", fileSize);

        uint8_t extensionSize;
        boost::asio::read(*socket, boost::asio::buffer(&extensionSize, sizeof(extensionSize)));
        std::vector<char> extensionBuffer(extensionSize);
        boost::asio::read(*socket, boost::asio::buffer(extensionBuffer));
        std::string fileExtension(extensionBuffer.begin(), extensionBuffer.end());

        spdlog::info("File extension: {}", fileExtension);

        std::filesystem::path desktopPath = getDesktopPath();
        if (desktopPath.empty())
        {
            spdlog::error("Could not determine desktop path!");
            return;
        }
        std::filesystem::path filePath = desktopPath / ("receivedData" + fileExtension);
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
        receiveData(socket);
    }
    catch (const std::exception& e)
    {
        spdlog::error("File receive failed: {}", e.what());
    }
}

void Server::receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto textSizeBuffer = std::make_shared<uint32_t>(0);
    boost::asio::async_read(
        *socket, boost::asio::buffer(textSizeBuffer.get(), sizeof(uint32_t)),
        [this, socket, textSizeBuffer](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                uint32_t textSize = *textSizeBuffer;
                if (textSize == 0 || textSize > 4096)
                {
                    spdlog::error("Invalid text size: {}", textSize);
                    return;
                }

                auto buffer = std::make_shared<std::vector<char>>(textSize);
                boost::asio::async_read(*socket, boost::asio::buffer(*buffer),
                                        [this, socket, buffer](boost::system::error_code ec,
                                                               std::size_t length)
                                        {
                                            if (!ec)
                                            {
                                                std::string message(buffer->begin(), buffer->end());
                                                spdlog::info("Received text: {} ({} bytes)",
                                                             message, length);
                                                receiveData(socket);
                                            }
                                            else
                                            {
                                                spdlog::error("Receiving text failed: {}",
                                                              ec.message());
                                            }
                                        });
            }
            else
            {
                spdlog::error("Receiving text size failed: {}", ec.message());
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
