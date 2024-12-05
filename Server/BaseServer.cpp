#pragma once
#include "BaseServer.h"
#include <ws2tcpip.h>
#include <iostream>

using namespace std;

void BaseServer::logError(const char* msg, bool fatal)
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
        stop();
    }
    else
    {
        printf("%s\n", msg);
        LocalFree(lpMsgBuf);
    }
}

void BaseServer::stop()
{
    if (_sock != NULL)
    {
        shutdown(_sock, SD_BOTH); // 송수신 중단
        closesocket(_sock);      // 소켓 닫기
        _sock = NULL;
    }

    closesocket(_sock);
    WSACleanup();
}