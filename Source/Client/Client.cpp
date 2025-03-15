#include "Client.h"

#include <spdlog/spdlog.h>
#include <fstream>

Client::Client(const std::string& ip, const std::string& port)
    : _socket{ _ioContext }, _resolver{ _ioContext }
{
    connect(ip, port);
    _ioContext.run();
}

void Client::connect(const std::string& ip, const std::string& port)
{
    try
    {
        const auto endpoints = _resolver.resolve(ip, port);
        boost::asio::connect(_socket, endpoints);
        spdlog::info("Connected to {}:{}", ip, port);
    }
    catch (const std::exception& e)
    {
        spdlog::error("Connection failed: {}", e.what());
    }
}

void Client::sendData(const std::string& type, const std::string& data)
{
    try
    {
        sendHeader(type);

        if (type == "TEXT")
        {
            sendText(data);
        }
        else if (type == "FILE")
        {
            sendFile(data);
        }
        else
        {
            spdlog::warn("Unknown data type: {}", type);
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Data send failed: {}", e.what());
    }
}

void Client::sendHeader(const std::string& type)
{
    uint8_t headerSize = static_cast<uint8_t>(type.size());
    boost::asio::write(_socket, boost::asio::buffer(&headerSize, sizeof(headerSize)));
    boost::asio::write(_socket, boost::asio::buffer(type));
}

void Client::sendText(const std::string& text)
{
    uint32_t textSize = static_cast<uint32_t>(text.size());
    boost::asio::write(_socket, boost::asio::buffer(&textSize, sizeof(textSize)));
    boost::asio::write(_socket, boost::asio::buffer(text));
    spdlog::info("Sent text: {} ({} bytes)", text, text.size());
}

void Client::sendFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        spdlog::error("Error: Could not open file {}", filePath);
        return;
    }

    size_t fileSize = getFileSize(file);
    boost::asio::write(_socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));
    sendFileExtension(filePath);
    sendFileContent(file, fileSize);
}

void Client::sendFileExtension(const std::string& filePath)
{
    std::string fileExtension = std::filesystem::path(filePath).extension().string();
    uint8_t extensionSize = static_cast<uint8_t>(fileExtension.size());
    boost::asio::write(_socket, boost::asio::buffer(&extensionSize, sizeof(extensionSize)));
    boost::asio::write(_socket, boost::asio::buffer(fileExtension));
}

void Client::sendFileContent(std::ifstream& file, size_t fileSize)
{
    constexpr size_t bufferSize = 4096;
    char buffer[bufferSize];

    spdlog::info("Sending file ({} bytes)", fileSize);

    while (file)
    {
        file.read(buffer, bufferSize);
        std::streamsize bytesRead = file.gcount();
        boost::asio::write(_socket, boost::asio::buffer(buffer, bytesRead));
    }

    spdlog::info("File sent successfully!");
}

size_t Client::getFileSize(std::ifstream& file)
{
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    return fileSize;
}
