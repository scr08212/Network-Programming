#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0601

#pragma once
#include "UDPServer.h"
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm> 
#include <map>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 2048
#define HEADERSIZE 13

#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03
#define CLEARCANVAS 0x04

#define MULTICASTIP "::ffff:224.0.0.1" // 듀얼스택을 사용중이니 IPv4는 IPv6에 매핑해야됨.
#define MULTICASTIP_V6 "ff02::1"
#define REMOTEPORT 9002 // 멀티캐스팅 포트

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
    sockaddr_in6 remoteaddr_in6;
    sockaddr_in6 remoteaddr_in;
    int addrlen;
    char buf[BUFSIZE];
    int recvBytes;

    memset(&remoteaddr_in6, 0, sizeof(remoteaddr_in6));
    remoteaddr_in6.sin6_family = AF_INET6;
    remoteaddr_in6.sin6_port = htons(REMOTEPORT);
    inet_pton(AF_INET6, MULTICASTIP_V6, &remoteaddr_in6.sin6_addr);

    memset(&remoteaddr_in, 0, sizeof(remoteaddr_in));
    remoteaddr_in.sin6_family = AF_INET6;
    remoteaddr_in.sin6_port = htons(REMOTEPORT);
    inet_pton(AF_INET6, MULTICASTIP, &remoteaddr_in.sin6_addr);

    map<string, vector<pair<uint32_t, string>>> chunksMap{};

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
        clientInfo.address = addr;
        clientInfo.port = ntohs(clientAddr.sin6_port);

        uint8_t type = buf[0];
        uint32_t length, sequence, totalChunks;
        memcpy(&length, buf + 1, 4);
        memcpy(&sequence, buf + 5, 4);
        memcpy(&totalChunks, buf + 9, 4);
        string data(buf + HEADERSIZE, recvBytes - HEADERSIZE);

        if (sequence > totalChunks)
            continue;

        string key = clientInfo.address + to_string(clientInfo.port);
        auto& chunks = chunksMap[key];
        chunks.push_back({ sequence, data });

        // ack 송신
        sendto(_sock, (char*)&sequence, sizeof(sequence), 0, (sockaddr*)&clientAddr, addrlen);

        // sendto가 실패 -> 청크가 부족함 -> 무한루프 가능성
        // 아직까지 문제된 적은 없음. 잘 작동함. 예외처리는 해야할듯.

        if (chunks.size() == totalChunks)
        {
            // 청크 순서별로 정렬
            sort(chunks.begin(), chunks.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
                return a.first < b.first;
                });

            // 클라이언트로 다시 보냄.
            // loopback 못하게 하는법은 아직 모르겠음.
            uint32_t totalChunks = chunks.size();
            for (auto& chunk : chunks)
            {
                uint32_t sequence = chunk.first;
                string data = chunk.second;
                uint32_t dataSize = data.size();

                vector<char> buffer(HEADERSIZE + dataSize);
                memcpy(buffer.data(), &type, sizeof(type));
                memcpy(buffer.data() + 1, &dataSize, sizeof(dataSize));
                memcpy(buffer.data() + 5, &sequence, sizeof(sequence));
                memcpy(buffer.data() + 9, &totalChunks, sizeof(totalChunks));
                memcpy(buffer.data() + 13, data.c_str(), dataSize);

                int sendBytes = sendto(_sock, buffer.data(), buffer.size(), 0, (sockaddr*)&remoteaddr_in6, sizeof(remoteaddr_in6));
                if (sendBytes == SOCKET_ERROR)
                {
                    logError("sendto() failed");
                }

                sendBytes = sendto(_sock, buffer.data(), buffer.size(), 0, (sockaddr*)&remoteaddr_in, sizeof(remoteaddr_in));
                if (sendBytes == SOCKET_ERROR)
                {
                    logError("sendto() failed");
                }

            }

            std::cout << "received all " << totalChunks << " chunks" << endl;
            chunks.clear();
        }
    }
}