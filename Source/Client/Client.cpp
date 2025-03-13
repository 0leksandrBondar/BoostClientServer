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
        uint8_t headerSize = static_cast<uint8_t>(type.size());
        boost::asio::write(_socket, boost::asio::buffer(&headerSize, sizeof(headerSize)));
        boost::asio::write(_socket, boost::asio::buffer(type));

        if (type == "TEXT")
        {
            uint32_t textSize = static_cast<uint32_t>(data.size());
            boost::asio::write(_socket, boost::asio::buffer(&textSize, sizeof(textSize)));
            boost::asio::write(_socket, boost::asio::buffer(data));
            spdlog::info("Sent text: {} ({} bytes)", data, data.size());
        }
        else if (type == "FILE")
        {
            std::ifstream file(data, std::ios::binary);
            if (!file)
            {
                spdlog::error("Error: Could not open file {}", data);
                return;
            }

            size_t fileSize = getFileSize(file);
            boost::asio::write(_socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));

            spdlog::info("Sending file: {} ({} bytes)", data, fileSize);

            constexpr size_t bufferSize = 4096;
            char buffer[bufferSize];

            while (file)
            {
                file.read(buffer, bufferSize);
                const std::streamsize bytesRead = file.gcount();
                boost::asio::write(_socket, boost::asio::buffer(buffer, bytesRead));
            }

            spdlog::info("File sent successfully!");
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



size_t Client::getFileSize(std::ifstream& file)
{
    file.seekg(0, std::ios::end);
    const size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    return fileSize;
}
