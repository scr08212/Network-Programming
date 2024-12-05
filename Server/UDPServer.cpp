#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma once
#include "UDPServer.h"
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <algorithm> 

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 2048
#define HEADERSIZE 5

#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03
#define CLEARCANVAS 0x04

#define MULTICASTIP "::ffff:235.7.8.9"
#define MULTICASTIP_V6 "FF12::1:2:3:4"
#define REMOTEPORT 9002

using namespace std;

void UDPServer::run(int port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    _sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (_sock == INVALID_SOCKET)
        logError("socket()", true);

    DWORD optval = 0;
    if (setsockopt(_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        logError("setsockopt()", true);

    DWORD ttl = 2;
    if (setsockopt(_sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char*)&ttl, sizeof(ttl)))
        logError("setsockopt()", true);

    sockaddr_in6 serverAddr = {};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_addr = in6addr_any;
    serverAddr.sin6_port = htons(port);

    if (bind(_sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        logError("bind()", true);

    printf("UDP server is running on port %d\n", port);

    receive();
    stop();
}

void UDPServer::receive()
{
    sockaddr_in6 clientAddr = {};
    int addrlen;
    char buf[BUFSIZE];
    int recvBytes;

    struct sockaddr_in6 remoteaddr_in6;
    memset(&remoteaddr_in6, 0, sizeof(remoteaddr_in6));
    remoteaddr_in6.sin6_family = AF_INET6;
    remoteaddr_in6.sin6_port = htons(REMOTEPORT);

    while (true)
    {
        addrlen = sizeof(clientAddr);
        recvBytes = recvfrom(_sock, buf, BUFSIZE, 0, (sockaddr*)&clientAddr, &addrlen);
        if (recvBytes <= 0)
        {
            logError("recvfrom()");
            continue;
        }

        // 송신자 정보 받아오기.
        ClientInfo clientInfo;
        char addr[INET6_ADDRSTRLEN];
        sockaddr_in6* s = (sockaddr_in6*)&clientAddr;
        if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr))
        {
            inet_pton(AF_INET6, MULTICASTIP, &remoteaddr_in6.sin6_addr);
            struct in_addr ipv4Addr;
            memcpy(&ipv4Addr, &s->sin6_addr.s6_addr[12], sizeof(ipv4Addr));
            inet_ntop(AF_INET, &ipv4Addr, addr, sizeof(addr));
            clientInfo.ipVersion = IPVersion::IPv4;
        }
        else
        {
            inet_pton(AF_INET6, MULTICASTIP_V6, &remoteaddr_in6.sin6_addr);
            inet_ntop(AF_INET6, &s->sin6_addr, addr, sizeof(addr));
            clientInfo.ipVersion = IPVersion::IPv6;
        }
        clientInfo.address = addr;
        clientInfo.port = ntohs(clientAddr.sin6_port);

        uint8_t type = buf[0];
        uint32_t length;
        memcpy(&length, buf + 1, 4);
        string data(buf + 5, recvBytes - HEADERSIZE);

        int sendBytes = sendto(_sock, buf, recvBytes, 0, (sockaddr*)&remoteaddr_in6, sizeof(remoteaddr_in6));
        if (sendBytes == SOCKET_ERROR)
        {
            logError("sendto() failed");
        }                      
    }
}