#pragma once
#include "BaseServer.h"
#include <vector>
#include <ws2tcpip.h>

class UDPServer : public BaseServer
{
private:

    void receive();

public:
    void run(int port) override;
};