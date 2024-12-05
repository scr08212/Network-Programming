#pragma once
#include <WinSock2.h>
#include <string>

using namespace std;

enum class IPVersion : uint8_t
{
    IPv4 = 0x01,
    IPv6 = 0x02
};

struct ClientInfo
{
    SOCKET socket;
    IPVersion ipVersion;
    string address;
    int port;

    bool operator==(const ClientInfo& other) const
    {
        return socket == other.socket &&
            ipVersion == other.ipVersion &&
            address == other.address &&
            port == other.port;
    }
};

class BaseServer
{
protected:
    SOCKET _sock;
    int _port;

    void logError(const char* msg, bool fatal = false);

public:
    virtual void run(int port) = 0;
    virtual void stop();
};