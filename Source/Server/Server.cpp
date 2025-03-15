#include "Server.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <memory>

Server::Server()
    : _ioContext{},
      _acceptor{ _ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12345) }
{
    startAccept();
}

void Server::run() { _ioContext.run(); }

void Server::startAccept()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioContext);
    _acceptor.async_accept(*socket,
                           [this, socket](boost::system::error_code ec)
                           {
                               if (!ec)
                               {
                                   spdlog::info("New client connected!");
                                   handleClient(socket);
                               }
                               else
                               {
                                   spdlog::error("Accept error: {}", ec.message());
                               }
                               startAccept(); // Accept next connection
                           });
}

void Server::handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    readHeaderSize(socket);
}

void Server::readHeaderSize(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto headerSizeBuffer = std::make_shared<uint8_t>(0);

    auto handler = [this, socket, headerSizeBuffer](boost::system::error_code ec, std::size_t)
    {
        if (!ec)
        {
            uint8_t headerSize = *headerSizeBuffer;
            if (headerSize == 0 || headerSize > 50)
            {
                spdlog::error("Invalid header size: {}", headerSize);
                return;
            }
            readHeader(socket, headerSize);
        }
        else
        {
            spdlog::warn("Client disconnected (header size): {}", ec.message());
        }
    };

    boost::asio::async_read(*socket, boost::asio::buffer(headerSizeBuffer.get(), sizeof(uint8_t)),
                            handler);
}

void Server::readHeader(std::shared_ptr<boost::asio::ip::tcp::socket> socket, uint8_t headerSize)
{
    auto headerBuffer = std::make_shared<std::vector<char>>(headerSize);

    auto handler = [this, socket, headerBuffer](boost::system::error_code ec, std::size_t)
    {
        if (!ec)
        {
            std::string header(headerBuffer->begin(), headerBuffer->end());
            spdlog::info("Received header: {}", header);
            processHeader(socket, header);
        }
        else
        {
            spdlog::warn("Client disconnected (header): {}", ec.message());
        }
    };

    boost::asio::async_read(*socket, boost::asio::buffer(*headerBuffer), handler);
}

void Server::processHeader(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                           const std::string& header)
{
    if (header == "AUTHENTICATION")
    {
        auto textSizeBuffer = std::make_shared<uint32_t>(0);

        _clients[] = socket;
    }
    else if (header == "TEXT")
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
        handleClient(socket); // Continue listening for new data
    }
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

        std::filesystem::path filePath = getDesktopPath() / ("receivedData" + fileExtension);
        std::ofstream outputFile(filePath, std::ios::binary);
        if (!outputFile)
        {
            spdlog::error("Could not create file at {}", filePath.string());
            return;
        }

        constexpr size_t bufferSize = 4096;
        std::vector<char> buffer(bufferSize);
        size_t totalReceived = 0;
        while (totalReceived < fileSize)
        {
            size_t bytesToRead = std::min(bufferSize, fileSize - totalReceived);
            size_t bytesRead
                = boost::asio::read(*socket, boost::asio::buffer(buffer.data(), bytesToRead));
            outputFile.write(buffer.data(), bytesRead);
            totalReceived += bytesRead;
            spdlog::info("Received packet: {} bytes (Total: {} / {} bytes)", bytesRead,
                         totalReceived, fileSize);
        }

        spdlog::info("File received successfully! Saved to: {}", filePath.string());
        handleClient(socket); // Continue listening for new data
    }
    catch (const std::exception& e)
    {
        spdlog::error("File receive failed: {}", e.what());
    }
}

void Server::receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto textSizeBuffer = std::make_shared<uint32_t>(0);

    auto textSizeHandler = [this, socket, textSizeBuffer](boost::system::error_code ec, std::size_t)
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

            auto textHandler
                = [this, socket, buffer](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::string message(buffer->begin(), buffer->end());
                    spdlog::info("Received text: {} ({} bytes)", message, length);
                    handleClient(socket);
                }
                else
                {
                    spdlog::error("Receiving text failed: {}", ec.message());
                }
            };

            boost::asio::async_read(*socket, boost::asio::buffer(*buffer), textHandler);
        }
        else
        {
            spdlog::error("Receiving text size failed: {}", ec.message());
        }
    };

    boost::asio::async_read(*socket, boost::asio::buffer(textSizeBuffer.get(), sizeof(uint32_t)),
                            textSizeHandler);
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