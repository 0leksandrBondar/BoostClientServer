#include "Client.h"

#include <spdlog/spdlog.h>

int main()
{
    try
    {
        Client client{ "127.0.0.1", "12345" };
        client.sendText("server", "Hello from client");
        client.sendFile("server", "C:\\Users\\aleks\\Desktop\\Data.mp4");
        client.sendFile("server", "C:\\Users\\aleks\\Downloads\\OBS-Studio-31.0.1-Windows-Installer.exe");
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error {}", e.what());
    }
    return 0;
}
