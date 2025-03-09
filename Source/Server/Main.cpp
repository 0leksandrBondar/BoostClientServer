// #include <boost/asio.hpp>
// #include <filesystem>
// #include <fstream>
// #include <iostream>

// class Server
// {
// public:
//     explicit Server(const short port)
//         : _ioContext{},
//           _acceptor{ _ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)
//           }
//     {
//         accept();
//     }
//
//     void run() { _ioContext.run(); }
//
// private:
//     void accept()
//     {
//         auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_ioContext);
//         _acceptor.async_accept(*socket,
//                                [this, socket](boost::system::error_code ec)
//                                {
//                                    if (!ec)
//                                    {
//                                        std::cout << "[+] New client connected" << std::endl;
//                                        listenForData(socket);
//                                    }
//                                    accept();
//                                });
//     }
//
//     void listenForData(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
//     {
//         auto buffer = std::make_shared<std::array<char, 4>>();
//
//         boost::asio::async_read(*socket, boost::asio::buffer(*buffer),
//                                 [this, socket, buffer](boost::system::error_code ec,
//                                                        std::size_t bytesRead)
//                                 {
//                                     if (!ec)
//                                     {
//                                         std::string header(buffer->data(), 4);
//                                         std::cout << "[LOG] Received header: " << header << " ("
//                                                   << bytesRead << " bytes)" << std::endl;
//
//                                         if (header == "TEXT")
//                                         {
//                                             receiveText(socket);
//                                         }
//                                         else if (header == "FILE")
//                                         {
//                                             receiveFile(socket);
//                                         }
//                                         else
//                                         {
//                                             std::cerr << "[ERROR] Unknown message type!"
//                                                       << std::endl;
//                                             listenForData(socket);
//                                         }
//                                     }
//                                     else
//                                     {
//                                         std::cerr << "[-] Client disconnected: " << ec.message()
//                                                   << std::endl;
//                                     }
//                                 });
//     }
//
//     void receiveText(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
//     {
//         auto buffer = std::make_shared<std::array<char, 1024>>();
//         socket->async_read_some(boost::asio::buffer(*buffer),
//                                 [this, socket, buffer](boost::system::error_code ec,
//                                                        std::size_t length)
//                                 {
//                                     if (!ec)
//                                     {
//                                         std::string message(buffer->data(), length);
//                                         std::cout << "[LOG] Received text: " << message << " ("
//                                                   << length << " bytes)" << std::endl;
//
//                                         listenForData(socket); // Чекаємо наступне повідомлення
//                                     }
//                                     else
//                                     {
//                                         std::cerr
//                                             << "[ERROR] Receiving text failed: " << ec.message()
//                                             << std::endl;
//                                     }
//                                 });
//     }
//
//     void receiveFile(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
//     {
//         try
//         {
//             size_t fileSize;
//             boost::asio::read(*socket, boost::asio::buffer(&fileSize, sizeof(fileSize)));
//             std::cout << "[LOG] Receiving file of size: " << fileSize << " bytes" << std::endl;
//
//             // Отримання шляху до робочого столу
//             std::filesystem::path desktopPath = getDesktopPath();
//             if (desktopPath.empty())
//             {
//                 std::cerr << "[ERROR] Could not determine desktop path!" << std::endl;
//                 return;
//             }
//
//             std::filesystem::path filePath = desktopPath / "receivedData.mp4";
//             std::ofstream outputFile(filePath, std::ios::binary);
//             if (!outputFile)
//             {
//                 std::cerr << "[ERROR] Could not create file at " << filePath << std::endl;
//                 return;
//             }
//
//             constexpr size_t bufferSize = 4096;
//             char buffer[bufferSize];
//             size_t totalReceived = 0;
//
//             while (totalReceived < fileSize)
//             {
//                 size_t bytesToRead = std::min(bufferSize, fileSize - totalReceived);
//                 size_t bytesRead
//                     = boost::asio::read(*socket, boost::asio::buffer(buffer, bytesToRead));
//                 outputFile.write(buffer, bytesRead);
//                 totalReceived += bytesRead;
//
//                 std::cout << "[LOG] Received packet: " << bytesRead
//                           << " bytes (Total: " << totalReceived << " / " << fileSize << " bytes)"
//                           << std::endl;
//             }
//
//             std::cout << "[+] File received successfully! Saved to: " << filePath << std::endl;
//
//             listenForData(socket); // Чекаємо наступне повідомлення
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "[ERROR] File receive failed: " << e.what() << std::endl;
//         }
//     }
//
//     // Функція для визначення шляху до робочого столу
//     std::filesystem::path getDesktopPath()
//     {
// #ifdef _WIN32
//         char* userProfile = std::getenv("USERPROFILE");
//         if (userProfile)
//         {
//             return std::filesystem::path(userProfile) / "Desktop";
//         }
// #elif __linux__
//         char* home = std::getenv("HOME");
//         if (home)
//         {
//             return std::filesystem::path(home) / "Desktop";
//         }
// #endif
//         return {};
//     }
//
//     boost::asio::io_context _ioContext;
//     boost::asio::ip::tcp::acceptor _acceptor;
// };

#include "Server.h"

int main()
{
    Server server;
    server.run();

    return 0;
}
