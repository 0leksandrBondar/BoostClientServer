#include "Client.h"

#include <spdlog/spdlog.h>

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        Client client("127.0.0.1", "12345");
        client.setName("oleg");
        client.registerName();
        client.startReceiving();

        std::thread ioThread([&]() { client.run(); });

        std::string receiver, message, type;

        std::cout << "To: ";
        std::getline(std::cin, receiver);
        std::cout << "file or text: ";
        std::getline(std::cin, type);
        if (type == "file")
        {
            client.sendFile(receiver, "path/to/file");
        }
        else if (type == "text")
        {
            std::cout << "Message: ";
            std::getline(std::cin, message);
            client.sendText(receiver, message);
        }

        ioThread.join();
    }
    catch (const std::exception& e)
    {
        spdlog::error("Client crashed: {}", e.what());
    }

    return 0;
}
