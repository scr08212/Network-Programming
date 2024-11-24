#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma once
#include "Client.h"
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define BUFSIZE 2048

Client::Client()
    :serverAddr("127.0.0.1"), serverPort(9000), clientSocket(NULL), isIPv6(false), sockAddrStorage({})
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;
}

Client::~Client()
{
}

void Client::connectToServer(string addr, int port, bool isIPv6)
{
    serverAddr = addr;
    serverPort = port;
    this->isIPv6 = isIPv6;

    initializeSocket();

    sockaddr* sockAddr = getSockAddr();
    int retval = connect(clientSocket, sockAddr, sizeof(addr));
    if (retval == SOCKET_ERROR)
        logError("connect()", true);

    char* buf = (char*)malloc(sizeof(char) * BUFSIZE);
    int len;

    thread recvThread(&Client::receive, this);
    while (true)
    {
        ZeroMemory(buf, sizeof(buf));
        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            continue;

        // 헤더
        uint8_t type = 0x01;
        uint32_t length = (uint32_t)len;

        char header[5] = {};
        header[0] = type;
        memcpy(header + 1, &length, sizeof(length));

        send(clientSocket, header, 5, 0);
        send(clientSocket, buf, len, 0);

        printf("%d + %d 바이트를 보냈습니다.\n", 5, length);
    }
    recvThread.join();

    closesocket(clientSocket);
    WSACleanup();
}

sockaddr* Client::getSockAddr()
{
    memset(&sockAddrStorage, 0, sizeof(sockAddrStorage));

    if (!isIPv6)
    {
        sockaddr_in* addr = (sockaddr_in*)&sockAddrStorage;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(serverPort);
        inet_pton(AF_INET, serverAddr.c_str(), &addr->sin_addr);
    }
    else
    {
        sockaddr_in6* addr = (sockaddr_in6*)&sockAddrStorage;
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(serverPort);
        inet_pton(AF_INET6, serverAddr.c_str(), &addr->sin6_addr);
    }

    return (sockaddr*)&sockAddrStorage;
}

void Client::logError(const char* msg, bool fatal)
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

void Client::initializeSocket()
{
    int af = isIPv6 ? AF_INET6 : AF_INET;
    clientSocket = socket(af, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
        logError("socket()", true);
}

void Client::receive()
{
    while (true)
    {
        char buf[BUFSIZE]; // 0: type 1~4: dataSize
        int received = recv(clientSocket, buf, BUFSIZE, 0);
        if (received <= 0)
        {
            logError("recv()", true);
            break;
        }

        uint8_t type = buf[0];
        uint32_t length;
        memcpy(&length, buf + 1, 4);

        string data(buf + 5, length);
        data += '\0';

        printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", received);
        printf("[받은 데이터] %s\n", data.c_str());
    }
    return;
}