    #define _CRT_SECURE_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _WIN32_WINNT 0x0601

    #pragma once
    #include "TCPServer.h"
    #include <ws2tcpip.h>
    #include <iostream>
    #include <thread>

    #pragma comment(lib, "ws2_32")

    #define HEADERSIZE 5
    #define MESSAGE 0X01
    #define FILE 0x02
    #define DRAWING 0X03
    #define CLEARCANVAS 0x04

    using namespace std;

    void TCPServer::run(int port)
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return;

        _sock = socket(AF_INET6, SOCK_STREAM, 0);
        if (_sock == INVALID_SOCKET)
            logError("socket()", true);

        // Dual socket 사용
        DWORD optval = 0;
        if (setsockopt(_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
            logError("setsockopt()", true);

        sockaddr_in6 serverAddr = {};
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_addr = in6addr_any;
        serverAddr.sin6_port = htons(port);

        if (bind(_sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            logError("bind()", true);

        if (listen(_sock, SOMAXCONN) == SOCKET_ERROR)
            logError("listen()", true);

        printf("TCP server is running on port %d\n", port);
        acceptClient();
        stop();
    }

    void TCPServer::acceptClient()
    {
        while (true)
        {
            struct sockaddr_in6 clientAddr = {};
            int addrlen = sizeof(clientAddr);
            SOCKET clientSock = accept(_sock, (sockaddr*)&clientAddr, &addrlen);
            if (clientSock == INVALID_SOCKET)
            {
                logError("accept()");
                continue;
            }

            ClientInfo clientInfo;
            char addr[INET6_ADDRSTRLEN];
            sockaddr_in6* s = (sockaddr_in6*)&clientAddr;
            if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr))
            {
                struct in_addr ipv4Addr;
                memcpy(&ipv4Addr, &s->sin6_addr.s6_addr[12], sizeof(ipv4Addr));
                inet_ntop(AF_INET, &ipv4Addr, addr, sizeof(addr));
                clientInfo.ipVersion = IPVersion::IPv4;
            }
            else
            {
                inet_ntop(AF_INET6, &s->sin6_addr, addr, sizeof(addr));
                clientInfo.ipVersion = IPVersion::IPv6;
            }
            clientInfo.socket = clientSock;
            clientInfo.address = addr; 
            clientInfo.port = ntohs(clientAddr.sin6_port);

            clients.push_back(clientInfo);
            printf("\n[TCP/%s] 클라이언트 접속: IP 주소= %s, 포트 번호 = %d\n", (clientInfo.ipVersion == IPVersion::IPv4 ? "IPv4" : "IPv6"), clientInfo.address.c_str(), clientInfo.port);
            thread(&TCPServer::receiveThread, this, clientInfo).detach();
        }
    }

    void TCPServer::receiveThread(ClientInfo clientInfo)
    {
        SOCKET clientSock = clientInfo.socket;
        while (true)
        {
            char header[HEADERSIZE]; // 0: type 1-4: dataSize
            if (recv(clientSock, header, HEADERSIZE, 0) <= 0)
            {
                logError("recv()");
                break;
            }

            uint8_t type = header[0];
            uint32_t length;
            memcpy(&length, header + 1, HEADERSIZE - 1);

            string data(length, '\0');
            if (recv(clientSock, &data[0], length, 0) <= 0)
            {
                logError("recv()");
                break;
            }

            string msg(header, HEADERSIZE);
            msg += data;

            bool loopback = false;
            switch (type)
            {
            case MESSAGE:
                printf("message received: %s\n", data.c_str());
                loopback = true;
                break;
            case FILE:
                printf("File received\n");
                loopback = false;
                break;
            case DRAWING:
                printf("Drawing command received\n");
                break;
            case CLEARCANVAS:
                printf("Clear canvas command received\n");
                break;
            default:
                printf("Unknown Type: %d\n", type);
                break;
            }

            broadCast(clientSock, msg.c_str(), HEADERSIZE + length, loopback);
        }

        clients.erase(remove(clients.begin(), clients.end(), clientInfo), clients.end());
        printf("\n[TCP/%s] 클라이언트 종료: IP 주소= %s, 포트 번호 = %d\n", (clientInfo.ipVersion == IPVersion::IPv4 ? "IPv4" : "IPv6"), clientInfo.address.c_str(), clientInfo.port);
        closesocket(clientSock);
    }

    void TCPServer::broadCast(SOCKET from, const char* buf, int len, bool loop_back)
    {
        for (auto& clientInfo : clients)
        {
            SOCKET client = clientInfo.socket;
            if (!loop_back && client == from)
                continue;
            if (send(client, buf, len, 0) == SOCKET_ERROR)
                logError("send()");
        }
    }