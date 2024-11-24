#pragma once
#include <WinSock2.h>
#include <string>

using namespace std;

class Client
{
private:
    SOCKET clientSocket;
    string serverAddr;
    int serverPort;
    bool isIPv6;
    sockaddr_storage sockAddrStorage;

    void logError(const char* msg, bool fatal = false);

    void initializeSocket();

    void receive();

    sockaddr* getSockAddr();

public:
    Client();

    ~Client();

    void connectToServer(string addr, int port, bool isIPv6);
};