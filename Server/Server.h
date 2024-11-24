#pragma once
#include <vector>
#include <WinSock2.h> // DWORD 
#include <string>

using namespace std;

class Server
{
private:
    const int port;
    SOCKET serverSocket;
    vector<SOCKET> clients;

    void logError(const char* msg, bool fatal = false);

    void initializeSocket();

    void acceptClients();

    void broadCast(SOCKET from, const char* buf, int len, bool loopBack);

    void handleClient(SOCKET client);

    void handleData(uint8_t type, string data);

public:
    Server(int serverPort);

    ~Server();

    void start();
};