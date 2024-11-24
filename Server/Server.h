#pragma once
#include <vector>
#include <WinSock2.h> // DWORD 
#include <string>
#include <ws2tcpip.h>

using namespace std;

class Server
{
    struct ClientInfo {
        bool isIPv4;
        u_short port;
        string address;
    };

private:
    const int port;

    SOCKET serverSocket;
    vector<SOCKET> clients;

    void logError(const char* msg, bool fatal = false);

    void initializeSocket();

    void acceptClients();

    void broadCast(SOCKET from, const char* buf, int len, bool loopBack);

    void handleClient(SOCKET client, sockaddr_in6 sock_addr);

    void handleData(uint8_t type, string data);

    ClientInfo getClientInfo(sockaddr_in6 sockAddr);

public:
    Server(int serverPort);

    ~Server();

    void start();
};