#include "Client.h"

#include <spdlog/spdlog.h>

#include <fstream>

Client::Client(const std::string& host, const std::string& port) : _socket{ _ioContext }
{
    boost::asio::ip::tcp::resolver resolver(_ioContext);
    boost::asio::connect(_socket, resolver.resolve(host, port));
}

void Client::registerName()
{
    nlohmann::json msg = { { "type", "REGISTER" }, { "sender", _senderName } };
    sendJson(msg);
}

void Client::startReceiving()
{
    boost::asio::async_read(
        _socket, boost::asio::buffer(&_incomingLength, sizeof(_incomingLength)),
        [this](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                if (_incomingLength > 10 * 1024 * 1024)
                {
                    spdlog::warn("Incoming message too large: {} bytes", _incomingLength);
                    _incomingLength = 0;
                    startReceiving();
                    return;
                }

                _incomingData.resize(_incomingLength);
                boost::asio::async_read(
                    _socket, boost::asio::buffer(_incomingData),
                    [this](boost::system::error_code ec, std::size_t)
                    {
                        if (!ec)
                        {
                            std::string json_text(reinterpret_cast<char*>(_incomingData.data()),
                                                  _incomingData.size());
                            spdlog::info("RAW JSON: {}", json_text);

                            try
                            {
                                nlohmann::json msg = nlohmann::json::parse(json_text);
                                std::string type = msg["type"];
                                std::string sender = msg.value("sender", "unknown");
                                spdlog::info("Start handle message type: {}", type);
                                if (type == "TEXT")
                                {
                                    spdlog::info("Incoming TEXT type message");
                                    std::string encoded = msg["data"];
                                    std::vector<unsigned char> decoded = base64Decode(encoded);
                                    std::string message(decoded.begin(), decoded.end());
                                    spdlog::info("[from {}]: {}", sender, message);
                                }
                                else if (type == "FILE")
                                {
                                    spdlog::info("Incoming FILE type message");
                                    std::string filename = msg["filename"];
                                    std::string encoded = msg["data"];
                                    std::vector<unsigned char> decoded = base64Decode(encoded);
                                    spdlog::info("[file from {}]: {} ({} bytes)", sender, filename,
                                                 decoded.size());
                                    // можно здесь сохранить файл
                                }
                                else
                                {
                                    spdlog::warn("Unknown message type: {}", type);
                                }
                            }
                            catch (const std::exception& e)
                            {
                                spdlog::error("Failed to parse message: {}", e.what());
                            }

                            startReceiving(); // продолжаем слушать
                        }
                        else
                        {
                            spdlog::error("Read message error: {}", ec.message());
                        }
                    });
            }
            else
            {
                spdlog::error("Read length error: {}", ec.message());
            }
        });
}

void Client::sendText(const std::string& receiver, const std::string& message)
{
    const std::string encoded
        = base64Encode(std::vector<unsigned char>(message.begin(), message.end()));
    const nlohmann::json msg = {
        { "sender", _senderName }, { "receiver", receiver }, { "type", "TEXT" }, { "data", encoded }
    };

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
std::vector<unsigned char> Client::base64Decode(std::string const& encoded_string)
{
    static const std::string base64_chars
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz"
          "0123456789+/";

    int in_len = encoded_string.size();
    int i = 0, j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}

bool Client::is_base64(unsigned char c) { return isalnum(c) || c == '+' || c == '/'; }
