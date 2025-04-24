#include "Session.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <memory>

// TEST
#include <shlobj.h>
#include <windows.h>

Session::Session(boost::asio::ip::tcp::socket socket) : _socket(std::move(socket)) {}

void Session::start() { readHeader(); }

void Session::readHeader()
{
    auto self = shared_from_this();
    boost::asio::async_read(_socket, boost::asio::buffer(&_dataLen, sizeof(_dataLen)),
                            [this, self](const boost::system::error_code& ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    _data.resize(_dataLen);
                                    readBody();
                                }
                            });
}

void Session::readBody()
{
    auto self = shared_from_this();
    boost::asio::async_read(_socket, boost::asio::buffer(_data),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    std::string json_text(_data.begin(), _data.end());
                                    spdlog::info("Raw data size: {}", _data.size());
                                    spdlog::info("JSON preview: {}", json_text.substr(0, 200));

                                    try
                                    {
                                        handleMessage(json_text);
                                    }
                                    catch (const std::exception& e)
                                    {
                                        spdlog::error("Message processing failed: {}", e.what());
                                    }

                                    readHeader();
                                }
                                else
                                {
                                    spdlog::error("Read error: {}", ec.message());
                                }
                            });
}

bool Session::is_base64(unsigned char c) { return isalnum(c) || c == '+' || c == '/'; }

void Session::handleMessage(const std::string& json_text)
{
    const nlohmann::json msg = nlohmann::json::parse(json_text);

    const std::string type = msg["type"];
    const std::string encoded_data = msg["data"];
    const std::string receiver = msg["receiver"];
    const std::string sender = msg.value("sender", "unknown");
    const std::string filename = msg.value("filename", "unnamed");

    auto decoded = base64Decode(encoded_data);

    if (type == "FILE")
    {
        processFile(sender, receiver, filename, decoded);
    }
    else if (type == "TEXT")
    {
        std::string message(decoded.begin(), decoded.end());
        processText(sender, receiver, message);
    }
    else
    {
        spdlog::warn("Unknown message type: {}", type);
    }
}

void Session::processFile(const std::string& sender, const std::string& receiver,
                          const std::string& filename_raw, const std::vector<unsigned char>& data)
{
    std::string filename = sanitizeFilename(filename_raw);
    std::string full_path = getDesktopPath() + "\\Received from client " + filename;

    std::ofstream out(full_path, std::ios::binary);
    if (!out)
    {
        spdlog::error("Cannot open file for writing: {}", full_path);
        return;
    }

    out.write(reinterpret_cast<const char*>(data.data()), data.size());

    spdlog::info("Sender: {}, Receiver: {}, Sent FILE: '{}', Size: {} bytes", sender, receiver,
                 filename, data.size());
}

void Session::processText(const std::string& sender, const std::string& receiver,
                          const std::string& message)
{
    spdlog::info("Sender: {}, Receiver: {}, Sent TEXT: '{}'", sender, receiver, message);
}

std::string Session::sanitizeFilename(std::string filename)
{
    for (char& c : filename)
    {
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<'
            || c == '>' || c == '|')
        {
            c = '_';
        }
    }
    return filename;
}

std::vector<unsigned char> Session::base64Decode(std::string const& encoded_string)
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

std::string Session::getDesktopPath()
{
    PWSTR path_tmp;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &path_tmp);
    if (SUCCEEDED(hr))
    {
        char path_utf8[MAX_PATH];
        wcstombs(path_utf8, path_tmp, MAX_PATH);
        CoTaskMemFree(path_tmp);
        return std::string(path_utf8);
    }
    return ".";
}
