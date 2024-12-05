#pragma once
#include "BaseServer.h"
#include <vector>

class TCPServer : public BaseServer
{
private:
    vector<ClientInfo> clients; // 접속한 클라이언트

    void acceptClient();
    void receiveThread(ClientInfo clientInfo);
    void broadCast(SOCKET from, const char* buf, int len, bool loop_back); // 전체 송신

public:
    void run(int port) override;
};