#include "Client.h"

int main()
{
    Client client("127.0.0.1", "12345");

    client.sendData("TEXT", "Hello, Server!");

    return 0;
}
