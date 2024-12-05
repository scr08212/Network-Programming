#pragma once
#include "BaseServer.h"
#include <vector>

class TCPServer : public BaseServer
{
private:
    vector<ClientInfo> clients;

    void acceptClient();
    void receiveThread(ClientInfo clientInfo);
    void broadCast(SOCKET from, const char* buf, int len, bool loop_back);

public:
    void run(int port) override;
};