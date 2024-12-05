#pragma once
#include "BaseServer.h"
#include <vector>

class UDPServer : public BaseServer
{
private:
    void receive();

public:
    void run(int port) override;
};