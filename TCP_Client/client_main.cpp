#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define _WIN32_WINNT 0x0601

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 9000
#define BUFSIZE 2048

char* SERVERIPv4 = (char*)"127.0.0.1";
char* SERVERIPv6 = (char*)"::1";

int IPv4(int argc, char* argv[]);
int IPv6(int argc, char* argv[]);

void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);
    printf("[%s] %s\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

DWORD WINAPI ReceiveMsg(LPVOID arg)
{
    printf("[TCP 클라이언트] recv 시작\n");
    SOCKET sock = (SOCKET)arg;
    char buf[BUFSIZE];
    int retval = 0;

    while (true)
    {
        retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
        {
            continue;
        }
        else if(retval > 0)
        {
            buf[retval] = '\0';
        }

        printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
        printf("[받은 데이터] %s\n", buf);
    }

    printf("[TCP 클라이언트] recv 종료\n");
    return 0;
}

int main(int argc, char* argv[])
{
    return IPv6(argc, argv);
    //return IPv4(argc, argv);
}

int IPv4(int argc, char* argv[])
{
    int retval;
    char buf[BUFSIZE];
    int len;

    if (argc > 1)
        SERVERIPv4 = argv[1];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        err_quit("socket()");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIPv4, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
        err_quit("connect()");

    printf("[TCP 클라이언트] send 시작\n");

    std::thread recv_thread(ReceiveMsg, (LPVOID)sock);
    while (true)
    {
        ZeroMemory(buf, sizeof(buf));
        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        retval = send(sock, (char*)&len, sizeof(int), 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }

        retval = send(sock, buf, len, 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }
        printf("[TCP 클라이언트] %d+%d바이트를 보냈습니다.\n", sizeof(int), retval);
    }
    recv_thread.join();

    closesocket(sock);

    WSACleanup();
    return 0;
}
int IPv6(int argc, char* argv[])
{
    int retval;
    char buf[BUFSIZE + 1];
    int len;

    if (argc > 1)
        SERVERIPv6 = argv[1];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        err_quit("socket()");

    struct sockaddr_in6 serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, SERVERIPv6, &serveraddr.sin6_addr);
    serveraddr.sin6_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
        err_quit("connect()");


    printf("[TCP 클라이언트] send 시작\n");

    std::thread recv_thread(ReceiveMsg, (LPVOID)sock);
    while (1)
    {
        ZeroMemory(buf, sizeof(buf));
        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        retval = send(sock, (char*)&len, sizeof(int), 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }

        retval = send(sock, buf, len, 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }
        printf("[TCP 클라이언트] %d+%d바이트를 보냈습니다.\n", sizeof(int), retval);
    }

    recv_thread.join();

    closesocket(sock);

    WSACleanup();
    return 0;
}

