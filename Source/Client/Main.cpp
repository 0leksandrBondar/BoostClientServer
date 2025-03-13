#include "Client.h"

int main()
{
    Client client("127.0.0.1", "12345");

    client.sendData("TEXT", "Hello, Server!");
    client.sendData("FILE", "C:\\Users\\aleks\\Desktop\\Data.mp4");

    return 0;
}
