#include "Client.h"

#include <fstream>
#include <iostream>

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
        std::cout << "Connected to " << ip << ":" << port << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Connection failed: " << e.what() << std::endl;
    }
}

void Client::sendText(const std::string& message)
{
    try
    {
        std::string header = "TEXT";
        boost::asio::write(_socket, boost::asio::buffer(header));
        boost::asio::write(_socket, boost::asio::buffer(message));

        std::cout << "Sent text: " << message << " (" << message.size() << " bytes)" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Send failed: " << e.what() << std::endl;
    }
}

void Client::sendFile(const std::filesystem::path& filePath)
{
    try
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file)
        {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return;
        }

        size_t fileSize = getFileSize(file);

        std::string header = "FILE";
        boost::asio::write(_socket, boost::asio::buffer(header));
        boost::asio::write(_socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));

        std::cout << "Sending file: " << filePath.filename() << " (" << fileSize << " bytes)"
                  << std::endl;

        constexpr size_t bufferSize = 4096;
        char buffer[bufferSize];

        while (file)
        {
            file.read(buffer, bufferSize);
            const std::streamsize bytesRead = file.gcount();
            boost::asio::write(_socket, boost::asio::buffer(buffer, bytesRead));
        }

        std::cout << "File sent successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "File send failed: " << e.what() << std::endl;
    }
}

size_t Client::getFileSize(std::ifstream& file)
{
    file.seekg(0, std::ios::end);
    const size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    return fileSize;
}
