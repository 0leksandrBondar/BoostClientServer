#include "Client.h"

int main()
{
    Client client("127.0.0.1", "12345");

    client.sendText("Hello, Server!");
    //client.sendFile("C:\\Users\\aleks\\Desktop\\Data.mp4");
    client.sendFile("C:\\Users\\aleks\\Downloads\\VulkanSDK-1.4.304.1-Installer.exe");

    return 0;
}
