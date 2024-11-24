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
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define BUFSIZE 2048
#define HEADERSIZE 5
#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03

Client::Client()
    :serverAddr("127.0.0.1"), serverPort(9000), clientSocket(NULL), isIPv6(false), sockAddrStorage({})
{

}

Client::~Client()
{
}

void Client::connectToServer(string addr, int port, bool isIPv6)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

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
        string msg = "";

        if (fgets(buf, BUFSIZE, stdin)) {
            msg.append(buf);

            while (strchr(buf, '\n') == NULL && !feof(stdin)) {
                if (fgets(buf, BUFSIZE, stdin)) {
                    msg.append(buf);
                }
            }
        }

        if (msg.length() == 0)
            continue;
        if (msg[msg.length() - 1] == '\n')
            msg[msg.length() - 1] = '\0';

        uint8_t type = MESSAGE; // 클라이언트 실행 전 테스트할 데이터 타입으로 미리 변경할 것.
        switch (type)
        {
        case MESSAGE:
            sendMessage(msg);
            break;
        case FILE:
            sendFile("D:/Projects/Server/Network-Programming/Client/dummyText.txt"); // 절대경로. 자신에 맞는 파일 위치로 하거나, 실행파일 있는 곳에 파일 두고 상대경로로 사용.
            break;
        case DRAWING:
            sendDrawing(); // 아직 구현 x 그리기가 어떤 방식인지 알아야됨.
            break;
        default:
            break;
        }
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

void Client::sendMessage(string msg)
{
    // 헤더
    uint8_t type = MESSAGE;
    uint32_t length = (uint32_t)strlen(msg.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    if (send(clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
        logError("send()", true);
    
    if (send(clientSocket, msg.c_str(), length, 0) == SOCKET_ERROR)
        logError("send()", true);
}

void Client::sendFile(string filePath)
{
    uint8_t type = FILE;

    ifstream file(filePath, ios::binary);
    if (!file.is_open())
    {
        logError("file()", false);
        return;
    }

    file.seekg(0, ios::end);
    int fileSize = (int)file.tellg();
    file.seekg(0, ios::beg);

    char* buf = (char*)malloc(sizeof(char) * fileSize);
    file.read(buf, fileSize);

    string data(buf, fileSize);
   
    uint32_t length = (uint32_t)strlen(data.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    if (send(clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
        logError("send()", true);
    if (send(clientSocket, data.c_str(), length, 0) == SOCKET_ERROR)
        logError("send()", true);
}

void Client::sendDrawing()
{
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
        char buf[BUFSIZE] = {};
        int received = recv(clientSocket, buf, BUFSIZE, 0);
        if (received <= 0)
        {
            logError("recv()", true);
            break;
        }

        uint8_t type = buf[0];
        uint32_t length;
        memcpy(&length, buf + 1, 4);


        string data;
        if (received > HEADERSIZE)
        {
            data = string(buf + HEADERSIZE, received - HEADERSIZE);
        }

        // 전체 데이터가 더 있다면 반복하여 계속 받기
        uint32_t totalReceived = received - HEADERSIZE;
        while (totalReceived < length)
        {
            int nextReceived = recv(clientSocket, buf, BUFSIZE, 0);
            if (nextReceived <= 0)
            {
                logError("recv()", true);
                break;
            }

            data.append(buf, nextReceived);
            totalReceived += nextReceived;
        }

        data += '\0';
        printf("[받은 데이터] %s\n", data.c_str());

        // 후처리
        switch (type)
        {
        case MESSAGE:
            break;
        case FILE:
            // # send 수행시 바이너리 데이터의 시작부분에 파일명.확장자? 형식으로 문자열 삽입
            // 1. recv 때, data의 앞에서부터 ?가 나올때까지 문자열 읽기. 이후 파일명, 확장자명 확인
            // 2. ofstream으로 파일 생성
            break;
        case DRAWING:
            // 실시간 그리기 형식이 어떤지 확인해봐야함
            break;
        }

    }
    return;
}