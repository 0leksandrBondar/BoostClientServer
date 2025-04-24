#include "Client.h"

#include <spdlog/spdlog.h>

#include <fstream>

Client::Client(const std::string& host, const std::string& port) : _socket{ _ioContext }
{
    boost::asio::ip::tcp::resolver resolver(_ioContext);
    boost::asio::connect(_socket, resolver.resolve(host, port));
}

void Client::sendText(const std::string& receiver, const std::string& message)
{
    const std::string encoded
        = base64Encode(std::vector<unsigned char>(message.begin(), message.end()));
    const nlohmann::json msg
        = { { "sender", _senderName },{ "receiver", receiver }, { "type", "TEXT" }, { "data", encoded } };

    sendJson(msg);
}

void Client::sendFile(const std::string& receiver, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        spdlog::error("File not found: {}", filename);
        return;
    }

    std::vector<unsigned char> file_data((std::istreambuf_iterator<char>(file)),
                                         std::istreambuf_iterator<char>());
    std::string encoded = base64Encode(file_data);

    nlohmann::json msg = {
        { "receiver", receiver }, { "type", "FILE" }, { "filename", filename }, { "data", encoded }
    };

    sendJson(msg);
}

void Client::sendJson(const nlohmann::json& j)
{
    std::string serialized = j.dump();
    uint32_t len = serialized.size();
    boost::asio::write(_socket, boost::asio::buffer(&len, sizeof(len)));
    boost::asio::write(_socket, boost::asio::buffer(serialized));
    spdlog::info("Sent {} bytes", len);
}

std::string Client::base64Encode(const std::vector<unsigned char>& bytes_to_encode)
{
    const std::string base64_chars
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz"
          "0123456789+/";

    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    auto it = bytes_to_encode.begin();
    while (it != bytes_to_encode.end())
    {
        int j = 0;
        while (j < 3 && it != bytes_to_encode.end())
        {
            char_array_3[j++] = *(it++);
        }
        while (j < 3)
        {
            char_array_3[j++] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int k = 0; k < 4; k++)
        {
            if (k < j + 1)
                ret += base64_chars[char_array_4[k]];
            else
                ret += '=';
        }
    }

    return ret;
}
