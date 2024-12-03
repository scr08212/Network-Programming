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
#include "resource2.h"
#include <atlconv.h>

using namespace std;
namespace fs = std::filesystem;

#define BUFSIZE 2048
#define HEADERSIZE 5
#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03
#define CLEARCANVAS 0x04

Client::Client()
    :serverAddr("127.0.0.1"), serverPort(9000), clientSocket(NULL), isIPv6(false), isUDP(false), sockAddrStorage({}), stopFlag(true)
{
}

void Client::setDlgHandle(HWND hwnd)
{
    hDlg = hwnd;
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

    stopFlag = false;
    recvThread =  thread(&Client::threadReceive, this);

    EnableWindow(GetDlgItem(hDlg, IDC_SEND_FILE), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_SEND_MESSAGE), TRUE);
}

void Client::disconnect()
{
    // stop recvthread
    if (clientSocket != NULL)
    {
        shutdown(clientSocket, SD_BOTH); // 송수신 중단
        closesocket(clientSocket);      // 소켓 닫기
        clientSocket = NULL;
    }

    stopFlag = true;
    if (recvThread.joinable())
        recvThread.join();
    WSACleanup();
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

void Client::threadReceive()
{
    // TODO: 프로그램 강종할시 thread부터 꺼지게. 지금 코드는 작동 안하는듯.
    while (!stopFlag)
    {
        char buf[BUFSIZE] = {};
        int received = recv(clientSocket, buf, BUFSIZE, 0);

        if (received <= 0)
        {
            if (stopFlag)
                break;
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
            USES_CONVERSION;
            wstring wstr = wstring(A2W(data.c_str()));
            SendMessage(GetDlgItem(hDlg, IDC_LIST_RECEIVED), LB_ADDSTRING, 0, (LPARAM)wstr.c_str());
        }
        else if (type == FILE)
        {
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

            wstring msg(L"File received");
            SendMessage(GetDlgItem(hDlg, IDC_LIST_RECEIVED), LB_ADDSTRING, 0, (LPARAM)msg.c_str());
        }
        else if (type == DRAWING)
        {
            // color 추출
            size_t colorEnd = data.find('?');
            COLORREF color = std::stoi(data.substr(0, colorEnd)); // color 값

            // from 좌표 추출
            size_t fromStart = colorEnd + 1;
            size_t fromSeparator = data.find(':', fromStart);
            int fromX = std::stoi(data.substr(fromStart, fromSeparator - fromStart));
            size_t fromEnd = data.find('?', fromSeparator);
            int fromY = std::stoi(data.substr(fromSeparator + 1, fromEnd - fromSeparator - 1));

            POINT from = { fromX, fromY };

            // to 좌표 추출
            size_t toStart = fromEnd + 1;
            size_t toSeparator = data.find(':', toStart);
            int toX = std::stoi(data.substr(toStart, toSeparator - toStart));
            int toY = std::stoi(data.substr(toSeparator + 1));

            POINT to = { toX, toY };

            onDrawingReceived(hDlg, color, from, to);
        }
        else if (type == CLEARCANVAS)
        {
            onClearCanvasReceived(hDlg);
        }
    }
    
    return;
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

void Client::sendFile(filesystem::path filePath)
{
    uint8_t type = FILE;

    // 파일 열기
    ifstream file(filePath, ios::binary);
    if (!file.is_open())
    {
        logError("file()", false);
        return;
    }

    // 파일 크기 가져오기
    file.seekg(0, ios::end);
    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // 파일 데이터를 읽어서 바이너리로 저장
    vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
    {
        logError("file.read()", false);
        return;
    }

    // 파일명 얻기
    string filename = filePath.filename().string();

    // 파일명과 바이너리 데이터를 합침 (파일명?바이너리 데이터)
    string data = filename + "?" + string(buffer.begin(), buffer.end());

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

void Client::sendDrawing(COLORREF color, POINT from, POINT to)
{
    uint8_t type = DRAWING;
    
    string msg = to_string(color) + "?" + to_string(from.x) + ":" + to_string(from.y) + "?" + to_string(to.x) + ":" + to_string(to.y); // color?x:y?x:y 양식으로 서버에 전달
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

    cout << "Drawing pos sent successfully: " << to.x << " " << to.y << endl;
}

void Client::sendClearCanvas()
{
    // 헤더
    string msg = " ";
    uint8_t type = CLEARCANVAS;
    uint32_t length = (uint32_t)strlen(msg.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    if (send(clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
        logError("send()", true);

    if (send(clientSocket, msg.c_str(), length, 0) == SOCKET_ERROR)
        logError("send()", true);
}