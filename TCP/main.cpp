#pragma once
#include "Server.h"

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    //TODO: 
    // [v]: dual stack���� IPv4, IPv6 ���� ó��
    // [v]: ��Ƽ������� TCP����
    // [v]: �������� Ŭ���̾�Ʈ�� ���� �޽��� ������ 
    // [ ]: �̹���/�ؽ�Ʈ���� ó��
    // [ ]: �̹���/�ؽ�Ʈ���� �� �Է� Ÿ���� ������ �տ� 4����Ʈ type���� ����

    Server server;
    server.Start();

    return 0;
}