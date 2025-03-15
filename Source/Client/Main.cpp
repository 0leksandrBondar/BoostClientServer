#include "Client.h"

int main()
{
    Client client("127.0.0.1", "12345");

    client.sendData("TEXT", "Hello, Server!");
    client.sendData("FILE", "C:\\Users\\aleks\\Downloads\\DS (2).ico");
    client.sendData("FILE", "C:\\Users\\aleks\\Desktop\\Data.mp4");
    client.sendData("FILE", "C:\\Users\\aleks\\Downloads\\VulkanSDK-1.4.304.1-Installer.exe");

    return 0;
}
