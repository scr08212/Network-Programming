#pragma once
#include "Client.h"

int main(int argc, char* argv[])
{
    char* SERVERIPv4 = (char*)"127.0.0.1";
    char* SERVERIPv6 = (char*)"::1";
    int port = 9000;

    Client client;
    client.connectToServer(SERVERIPv4, port, false);

    return 0;
}