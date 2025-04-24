#pragma once

#include <boost/asio.hpp>

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket socket);

    void start();

private:
    void readHeader();
    void readBody();

    bool is_base64(unsigned char c);
    void handleMessage(const std::string& json_text);
    void processFile(const std::string& sender, const std::string& receiver,
                     const std::string& filename_raw, const std::vector<unsigned char>& data);
    void processText(const std::string& sender, const std::string& receiver,
                     const std::string& message);

    std::string sanitizeFilename(std::string filename);
    std::vector<unsigned char> base64Decode(std::string const& encoded_string);

    // TEST
    std::string getDesktopPath();

private:
    uint32_t _dataLen = 0;
    std::vector<char> _data;
    boost::asio::ip::tcp::socket _socket;
};