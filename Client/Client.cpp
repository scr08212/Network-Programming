#pragma once
#include "Client.h"
#include <ws2tcpip.h>
#include <atlconv.h>
#include <fstream>

#define BUFSIZE 512
#define HEADERSIZE 5
#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03
#define CLEARCANVAS 0x04

void Client::logError(const char* msg, bool fatal)
{
    LPVOID lpMsgBuf = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL);

    auto icon = fatal ? MB_ICONERROR : MB_ICONINFORMATION;
    MessageBoxA(NULL, (const char*)lpMsgBuf, msg, icon);
    LocalFree(lpMsgBuf);
    if (fatal)
        disconnect();
}

void Client::receiveThread()
{
    sockaddr peerAddr;
    int addrlen;

    while (!_stopFlag)
    {
        char buf[BUFSIZE] = {};
        int received;
        if (_protocol == Protocol::TCP)
            received = recv(_clientSocket, buf, BUFSIZE, 0);
        else
            received = recvfrom(_clientSocket, buf, BUFSIZE, 0, &peerAddr, &addrlen);

        if (_stopFlag)
            break;
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
            int nextReceived;
            if (_protocol == Protocol::TCP)
                nextReceived = recv(_clientSocket, buf, BUFSIZE, 0);
            else
                nextReceived = recvfrom(_clientSocket, buf, BUFSIZE, 0, &peerAddr, &addrlen);

            if (nextReceived <= 0)
            {
                logError("recv()", true);
                break;
            }

            data.append(buf, nextReceived);
            totalReceived += nextReceived;
        }
        data += '\0';

        // 후처리
        if (type == MESSAGE)
        {
            USES_CONVERSION;
            wstring wstr = wstring(A2W(data.c_str()));
            onMesssageReceived(hDlg, wstr);
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

            ofstream outputFile(outputDirectory, ios::binary);
            if (!outputFile.is_open())
                break;

            outputFile.write(binaryData.c_str(), binaryData.size());
            outputFile.close();

            wstring wstr(L"File received");
            onFileReceived(hDlg, wstr);
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
            onClearCanvasRequested(hDlg);
        }
    }
}

void Client::connectToServer(string serverAddr, int port, IPVersion ipVersion, Protocol protocol)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    _serverInfo.address = serverAddr;
    _serverInfo.port = port;
    _ipVersion = ipVersion;
    _protocol = protocol;

    memset(&addrStorage, 0, sizeof(addrStorage));

    if (_ipVersion == IPVersion::IPv4)
    {
        sockaddr_in* addr = (sockaddr_in*)&addrStorage;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(_serverInfo.port);
        inet_pton(AF_INET, _serverInfo.address.c_str(), &addr->sin_addr);
    }
    else
    {
        sockaddr_in6* addr = (sockaddr_in6*)&addrStorage;
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(_serverInfo.port);
        inet_pton(AF_INET6, _serverInfo.address.c_str(), &addr->sin6_addr);
    }

    auto af = _ipVersion == IPVersion::IPv6 ? AF_INET6 : AF_INET;
    auto type = _protocol == Protocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
    _clientSocket = socket(af, type, 0);
    if (_clientSocket == INVALID_SOCKET)
    {
        logError("socket()", true);
        return;
    }

    if (_protocol == Protocol::TCP)
    {
        int retval = connect(_clientSocket, getServerSockAddr(), sizeof(serverAddr));
        if (retval == SOCKET_ERROR)
        {
            logError("connect()", true);
            return;
        }
    }

    _stopFlag = false;
    _recvThread = thread(&Client::receiveThread, this);

    onConnected(hDlg);
}

void Client::disconnect()
{
    if (_clientSocket != NULL)
    {
        shutdown(_clientSocket, SD_BOTH); // 송수신 중단
        closesocket(_clientSocket);      // 소켓 닫기
        _clientSocket = NULL;
    }

    _stopFlag = true;
    if (_recvThread.joinable())
        _recvThread.join();

    WSACleanup();
    onDisconnected(hDlg);
}

void Client::sendData(string header, string data)
{
    if (_protocol == Protocol::TCP)
    {
        if (send(_clientSocket, header.c_str(), HEADERSIZE, 0) == SOCKET_ERROR)
        {
            logError("send()", true);
            return;
        }

        if (send(_clientSocket, data.c_str(), strlen(data.c_str()), 0) == SOCKET_ERROR)
        {
            logError("send()", true);
            return;
        }
    }
    else
    {
        string str(header, HEADERSIZE);
        str += data;
        auto serveraddr = getServerSockAddr();
        if (sendto(_clientSocket, str.c_str(), strlen(str.c_str()), 0, serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        {
            logError("send()", true);
            return;
        }
    }
}

void Client::sendMessage(string msg)
{
    // 헤더
    uint8_t type = MESSAGE;
    uint32_t length = (uint32_t)strlen(msg.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    sendData(string(header), msg);
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
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    sendData(string(header), data);
}

void Client::sendDrawing(COLORREF color, POINT from, POINT to)
{
    uint8_t type = DRAWING;

    string data = to_string(color) + "?" + to_string(from.x) + ":" + to_string(from.y) + "?" + to_string(to.x) + ":" + to_string(to.y); // color?x:y?x:y 양식으로 서버에 전달
    uint32_t length = strlen(data.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    sendData(string(header), data);
}

void Client::sendClearCanvas()
{
    string data = " ";
    uint8_t type = CLEARCANVAS;
    uint32_t length = (uint32_t)strlen(data.c_str());

    char header[HEADERSIZE] = {};
    header[0] = type;
    memcpy(header + 1, &length, sizeof(length));

    sendData(string(header), data);
}

void Client::setHDlg(HWND hDlg)
{
    this->hDlg = hDlg;
}

bool Client::getStopFlag()
{
    return _stopFlag;
}

sockaddr* Client::getServerSockAddr()
{
    return (sockaddr*)&addrStorage;
}