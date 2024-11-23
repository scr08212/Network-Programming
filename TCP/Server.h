#pragma once
#include <vector>
#include <WinSock2.h> // DWORD 

using namespace std;

class Server
{
public:
    Server();
    ~Server();
    void Start();

private:
    static void LogError(const char* msg, bool fatal = false); // false: Display, true: Quit
    bool AcceptClient(SOCKET listen_sock);
    DWORD WINAPI HandleClient(LPVOID arg);
    void SendMsg(SOCKET from, char* buf, int len, bool loop_back = true);

private:
    vector<SOCKET> client_sockets;
};