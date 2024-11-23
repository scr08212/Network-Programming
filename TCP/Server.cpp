#pragma once
#include "Server.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <thread>

#define SERVERPORT 9000 
#define BUFSIZE 2048

Server::Server()
{
    client_sockets = {};
}

Server::~Server()
{
}

void Server::LogError(const char* msg, bool fatal)
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
        printf("[%s] %s\n", msg, (char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
}


void Server::Start()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    SOCKET listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
        LogError("socket()", true);

    int optval = 0;
    if (setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        LogError("setsockopt()", true);

    sockaddr_in6 server_addr = {};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(SERVERPORT);

    if (bind(listen_sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        LogError("bind()", true);

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR)
        LogError("listen()", true);

    printf("Server is running\n");

    while (AcceptClient(listen_sock));

    closesocket(listen_sock);
    WSACleanup();

    return;
}

bool Server::AcceptClient(SOCKET listen_sock)
{
    SOCKET client_sock;
    struct sockaddr_in6 client_addr = {};
    int addrlen;

    addrlen = sizeof(client_addr);
    client_sock = accept(listen_sock, (sockaddr*)&client_addr, &addrlen);
    if (client_sock == INVALID_SOCKET)
    {
        LogError("accept()");
        return false;
    }

    char addr[INET6_ADDRSTRLEN] = { 0 };
    sockaddr_in6* s = (sockaddr_in6*)&client_addr;
    inet_ntop(AF_INET6, &s->sin6_addr, addr, sizeof(addr));
    printf("\n[TCP/IPv6 서버] 클라이언트 접속: IP 주소 = %s, 포트번호 = %d\n", addr, ntohs(client_addr.sin6_port));

    client_sockets.push_back(client_sock);
    std::thread client_thread(&Server::HandleClient, this, (LPVOID)client_sock);
    client_thread.detach();

    return true;
}


DWORD WINAPI Server::HandleClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in6 client_addr;
    char addr[INET6_ADDRSTRLEN];
    int addrlen;
    int len;
    char buf[BUFSIZE + 1];
    int retval;

    addrlen = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr*)&client_addr, &addrlen);
    inet_ntop(AF_INET6, &client_addr.sin6_addr, addr, sizeof(addr));

    while (1)
    {
        // 고정길이
        retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
        if (retval == SOCKET_ERROR)
        {
            LogError("recv()");
            break;
        }
        else if (retval == 0)
            break;

        // 가변길이
        retval = recv(client_sock, buf, len, MSG_WAITALL);
        if (retval == SOCKET_ERROR)
        {
            LogError("recv()");
            break;
        }
        else if (retval == 0)
            break;

        // 메시지 처리
        buf[retval] = '\0';
        printf("[TCP/%s:%d] %s\n", addr, ntohs(client_addr.sin6_port), buf);

        // 모든 클라이언트에 받은 메시지 전송
        SendMsg(client_sock, buf, retval, true);
    }

    client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_sock), client_sockets.end());
    closesocket(client_sock);
    printf("[TCP 서버] 클라이언트 종료: IP주소 = %s, 포트 번호 = %d\n", addr, client_addr.sin6_port);
    return 0;
}

void Server::SendMsg(SOCKET from, char* buf, int len, bool loop_back)
{
    int retval;
    for (SOCKET client : client_sockets)
    {
        if (!loop_back && client == from)
            continue;
        printf("Sending length: %d to client... : %s\n", len, buf);
        retval = send(client, buf, len, 0);
    }
}