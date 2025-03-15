#include "Client.h"

int main()
{
    Client client("127.0.0.1", "12345");

    client.sendData("AUTHENTICATION", "UserName: Sasha, Password: 123456");

    return 0;
}
