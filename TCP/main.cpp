#pragma once
#include "Server.h"

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    //TODO: 
    // [v]: dual stack으로 IPv4, IPv6 동시 처리
    // [v]: 멀티쓰레드로 TCP연결
    // [v]: 접속중인 클라이언트에 받은 메시지 보내기 
    // [ ]: 이미지/텍스트파일 처리
    // [ ]: 이미지/텍스트파일 등 입력 타입은 데이터 앞에 4바이트 type으로 결정

    Server server;
    server.Start();

    return 0;
}