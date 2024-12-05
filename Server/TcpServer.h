#pragma once
#include "BaseServer.h"
#include <vector>

class TCPServer : public BaseServer
{
private:
    vector<ClientInfo> clients; // ������ Ŭ���̾�Ʈ

    void acceptClient();
    void receiveThread(ClientInfo clientInfo);
    void broadCast(SOCKET from, const char* buf, int len, bool loop_back); // ��ü �۽�

public:
    void run(int port) override;
};