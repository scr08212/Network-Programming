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
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

namespace fs = std::filesystem;

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
            sendFile("D:/Projects/Server/Network-Programming/Client/cat.png"); // 절대경로. 자신에 맞는 파일 위치로 하거나, 실행파일 있는 곳에 파일 두고 상대경로로 사용.
            break;
        case DRAWING:
            sendDrawing({ 1,1 }); // x:1 y:1 좌표로 그렸다고 가정.
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

void Client::sendFile(fs::path filePath)
{
    uint8_t type = FILE;

    // 파일 열기
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        logError("file()", false);
        return;
    }

    // 파일 크기 가져오기
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 파일 데이터를 읽어서 바이너리로 저장
    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
    {
        logError("file.read()", false);
        return;
    }

    // 파일명 얻기
    std::string filename = filePath.filename().string();

    // 파일명과 바이너리 데이터를 합침 (파일명?바이너리 데이터)
    std::string data = filename + "?" + std::string(buffer.begin(), buffer.end());

    // 데이터 길이 계산
    uint32_t length = static_cast<uint32_t>(data.size());

    // 헤더 생성
    char header[HEADERSIZE] = {};
    header[0] = type; // 타입 설정
    memcpy(header + 1, &length, sizeof(length));

    // 헤더 전송
    if (send(clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
    {
        logError("send(header)", true);
        return;
    }

    // 데이터 전송
    if (send(clientSocket, data.c_str(), length, 0) == SOCKET_ERROR)
    {
        logError("send(data)", true);
        return;
    }

    cout << "File sent successfully: " << filename << endl;
}

void Client::sendDrawing(POINT mousePos)
{
    uint8_t type = DRAWING;
    string msg = to_string(mousePos.x) + "?" + to_string(mousePos.y); // x?y 양식으로 서버에 전달
    uint32_t length = strlen(msg.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type; // 타입 설정
    memcpy(header + 1, &length, sizeof(length));

    if (send(clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
    {
        logError("send(header)", true);
        return;
    }
    
    if (send(clientSocket, msg.c_str(), strlen(msg.c_str()), 0) == SOCKET_ERROR)
    {
        logError("send(Drawing)", true);
        return; 
    }

    cout << "Drawing pos sent successfully: " << mousePos.x << " " << mousePos.y << endl;
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
        printf("[Received Data] %s\n", data.c_str());

        // 후처리
        if (type == MESSAGE)
        {
            
        }
        else if (type == FILE)
        {
           // # send 수행시 바이너리 데이터의 시작부분에 파일명.확장자? 형식으로 문자열 삽입
           // 1. recv 때, data의 앞에서부터 ?가 나올때까지 문자열 읽기. 이후 파일명, 확장자명 확인
           // 2. ofstream으로 파일 생성

            size_t delimiterPos = data.find('?');
            if (delimiterPos == string::npos)
                break;

            string outputDirectory = "output/";
            if (!filesystem::exists(outputDirectory))
                filesystem::create_directories(outputDirectory);

            string fileName = data.substr(0, delimiterPos);
            string binaryData = data.substr(delimiterPos + 1);

            outputDirectory += fileName;
            // 결과 출력
            cout << "File Name: " << fileName << endl;
            cout << "Binary Data: " << binaryData << endl;

            ofstream outputFile(outputDirectory, ios::binary);
            if (!outputFile.is_open())
                break;

            outputFile.write(binaryData.c_str(), binaryData.size());
            outputFile.close();

            cout << "File saved!" << endl;
        }
        else if (type == DRAWING)
        {
            size_t delimiterPos = data.find('?');
            string x = data.substr(0, delimiterPos);
            string y = data.substr(delimiterPos + 1);
            POINT point = { stoi(x), stoi(y) };

            cout << "Drawing position received! x:" << point.x << " y: " << point.y << endl;
        }

    }
    return;
}
