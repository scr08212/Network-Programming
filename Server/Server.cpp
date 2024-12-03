#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma once
#include "Server.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

Server::Server(int server_port)
    :port(server_port)
{
    serverSocket = NULL;
    clients = {};
}

Server::~Server()
{
}

void Server::logError(const char* msg, bool fatal)
{
    LPVOID lpMsgBuf = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);

    if (fatal)
    {
        MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
        LocalFree(lpMsgBuf);
        exit(1);
    }
    else
    {
        cout << "[" << msg << "] " << (char*)lpMsgBuf << endl;
        LocalFree(lpMsgBuf);
    }
}

void Server::initializeSocket()
{
    serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
        logError("socket()", true);

    int optval = 0;
    if (setsockopt(serverSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        logError("setsockopt()", true);

    sockaddr_in6 server_addr = {};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        logError("bind()", true);

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
        logError("listen()", true);
}

void Server::start()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    initializeSocket();
    cout << "Server is running on port " << port << endl;

    acceptClients();

    closesocket(serverSocket);
    WSACleanup();

    return;
}

void Server::acceptClients()
{
    while (true)
    {
        struct sockaddr_in6 client_addr = {};
        int addrlen = sizeof(client_addr);
        SOCKET client_sock = accept(serverSocket, (sockaddr*)&client_addr, &addrlen);

        if (client_sock == INVALID_SOCKET)
        {
            logError("accept()");
            break;
        }
        ClientInfo info = getClientInfo(client_addr);
        printf("\n[TCP/%s 서버] 클라이언트 접속: IP 주소= %s, 포트 번호 = %d\n", (info.isIPv4 ? "IPv4" : "IPv6"), info.address.c_str(), info.port);
        clients.push_back(client_sock);
        std::thread(&Server::handleClient, this, client_sock, client_addr).detach();
    }

    return;
}

void Server::handleClient(SOCKET client, sockaddr_in6 sock_addr)
{
    while (true)
    {
        char header[5]; // 0: type 1~4: dataSize
        int received = recv(client, header, 5, 0);
        if (received <= 0)
        {
            logError("recv()");
            break;
        }

        uint8_t type = header[0];
        uint32_t length;
        memcpy(&length, header + 1, 4);

        string data(length, '\0');
        received = recv(client, &data[0], length, 0);
        if (received <= 0)
        {
            logError("recv()");
            break;
        }

        handleData(type, data);

        string msg(header, 5);
        msg += data;

        broadCast(client, msg.c_str(), 5 + length, true);
    }

    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());

    ClientInfo info = getClientInfo(sock_addr);
    printf("\n[TCP/%s 서버] 클라이언트 종료: IP 주소= %s, 포트 번호 = %d\n", (info.isIPv4 ? "IPv4" : "IPv6"), info.address.c_str(), info.port);
    closesocket(client);
    return;
}

void Server::broadCast(SOCKET from, const char* buf, int len, bool loop_back)
{
    int retval;
    for (SOCKET client : clients)
    {
        if (!loop_back && client == from)
            continue;
        retval = send(client, buf, len, 0);
    }
}

void Server::handleData(uint8_t type, string data)
{
    switch (type)
    {
    case 0x01: // 메시지
        cout << "message received: " << data << endl;
        break;
    case 0x02: // 파일
        cout << "File received" << endl;
        break;
    case 0x03: // 드로잉
        cout << "Drawing command received" << endl;
        break;
    case 0x04:
        cout << "Clear canvas command received" << endl;
        break;
    default:
        cout << "Unknown Type: " << type << endl;
        break;
    }
}

Server::ClientInfo Server::getClientInfo(sockaddr_in6 sockAddr)
{
    ClientInfo info{};

    char addr[INET6_ADDRSTRLEN];
    sockaddr_in6* s = (sockaddr_in6*)&sockAddr;
    if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr))
    {
        struct in_addr ipv4_addr;
        memcpy(&ipv4_addr, &s->sin6_addr.s6_addr[12], sizeof(ipv4_addr));
        inet_ntop(AF_INET, &ipv4_addr, addr, sizeof(addr));
        info.isIPv4 = true;
    }
    else
    {
        inet_ntop(AF_INET6, &s->sin6_addr, addr, sizeof(addr));
        info.isIPv4 = false;
    }

    info.address = addr;  // std::string에 직접 할당
    info.port = ntohs(sockAddr.sin6_port);
    return info;
}