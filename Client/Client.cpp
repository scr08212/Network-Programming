#pragma once
#include "Client.h"
#include <ws2tcpip.h>
#include <atlconv.h>
#include <fstream>
#include <cmath>
#include "resource2.h"
#include <map>
#include <set>

#define BUFSIZE 2048
#define HEADERSIZE 5
#define EXTRAHEADERSIZE 8

#define MESSAGE 0X01
#define FILE 0x02
#define DRAWING 0X03
#define CLEARCANVAS 0x04

#define MULTICASTIP "235.7.8.9"
#define MULTICASTIP_V6 "FF12::1:2:3:4"
#define LOCALPORT 9002

Client::Client()
    :_stopFlag(true)
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

    auto icon = fatal ? MB_ICONERROR : MB_ICONINFORMATION;
    MessageBoxA(NULL, (const char*)lpMsgBuf, msg, icon);
    LocalFree(lpMsgBuf);
    if (fatal)
        disconnect();
}

void Client::receiveThread()
{
    char buf[BUFSIZE];
    int recvBytes;
    vector<pair<uint32_t, string>> chunks;

    while (true)
    {
        uint8_t type = 0x00;
        string data;

        if (_protocol == Protocol::TCP)
        {
            recvBytes = recv(_clientSocket, buf, BUFSIZE, 0);

            if (recvBytes <= 0)
            {
                if (_stopFlag)
                    break;
                logError("recv()", true);
                break;
            }

            type = buf[0];
            uint32_t length;
            memcpy(&length, buf + 1, 4);

            if (recvBytes > HEADERSIZE)
            {
                data = string(buf + HEADERSIZE, recvBytes - HEADERSIZE);
            }

            // 전체 데이터가 더 있다면 반복하여 계속 받기
            uint32_t totalReceived = recvBytes - HEADERSIZE;
            while (totalReceived < length)
            {
                int nextReceived;
                nextReceived = recv(_clientSocket, buf, BUFSIZE, 0);

                if (nextReceived <= 0)
                {
                    logError("recv()", true);
                    break;
                }

                data.append(buf, nextReceived);
                totalReceived += nextReceived;
            }
            data += '\0';
            handleReceivedData(type, data);
        }
        else
        {
            sockaddr_in6 addr;
            int addrlen = sizeof(addr);
            recvBytes = recvfrom(_clientSocket, buf, BUFSIZE, 0, (sockaddr*)&addr, &addrlen);

            if (recvBytes <= 0)
            {
                if (_stopFlag)
                    break;
                logError("recvfrom()", true);
                break;
            }

            uint8_t type = buf[0];
            uint32_t length, sequence, totalChunks;
            memcpy(&length, buf + 1, 4);
            memcpy(&sequence, buf + 5, 4);
            memcpy(&totalChunks, buf + 9, 4);
            string data(buf + HEADERSIZE + EXTRAHEADERSIZE, recvBytes - HEADERSIZE - EXTRAHEADERSIZE);
            
            if (sequence > totalChunks)
                continue;

            chunks.push_back({ sequence, data });

            if (chunks.size() == totalChunks)
            {
                sort(chunks.begin(), chunks.end(), [](const pair<int, string>& a, const pair<int, string>& b) {
                    return a.first < b.first;
                    });

                string data;
                for (auto& value : chunks)
                {
                    data += value.second;
                }

                handleReceivedData(type, data);

                chunks.clear();
            }
        }

    }
}

void Client::handleReceivedData(uint8_t type, string data)
{
    if (type == MESSAGE)
    {
        USES_CONVERSION;
        wstring wstr = wstring(A2W(data.c_str()));
        onMesssageReceived(_hDlg, wstr);
    }
    else if (type == FILE)
    {
        size_t delimiterPos = data.find('?');
        if (delimiterPos == string::npos)
            return;

        string outputDirectory = "output/";
        if (!filesystem::exists(outputDirectory))
            filesystem::create_directories(outputDirectory);

        string fileName = data.substr(0, delimiterPos);
        string binaryData = data.substr(delimiterPos + 1);

        outputDirectory += fileName;

        ofstream outputFile(outputDirectory, ios::binary);
        if (!outputFile.is_open())
            return;

        outputFile.write(binaryData.c_str(), binaryData.size());
        outputFile.close();

        wstring wstr(L"File received");
        onFileReceived(_hDlg, wstr);
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

        onDrawingReceived(_hDlg, color, from, to);
    }
    else if (type == CLEARCANVAS)
    {
        onClearCanvasRequested(_hDlg);
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

    memset(&_serverAddrStorage, 0, sizeof(_serverAddrStorage));
    memset(&_multicastAddrStorage, 0, sizeof(_multicastAddrStorage));

    if (_ipVersion == IPVersion::IPv4)
    {
        // serverAddr
        sockaddr_in* addr = (sockaddr_in*)&_serverAddrStorage;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(_serverInfo.port);
        inet_pton(AF_INET, _serverInfo.address.c_str(), &addr->sin_addr);
    }
    else // IPv6
    {
        // serverAddr
        sockaddr_in6* addr = (sockaddr_in6*)&_serverAddrStorage;
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

    int retval;
    if (_protocol == Protocol::TCP)
    {
         retval = connect(_clientSocket, getServerSockAddr(), sizeof(serverAddr));
        if (retval == SOCKET_ERROR)
        {
            logError("connect()", true);
            return;
        }
    }


    IpMreqUnion mreq;
    if (_protocol == Protocol::UDP)
    {
        _sendSocket = socket(af, type, 0);

        DWORD optval = 1;
        if(setsockopt(_clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        {
            logError("setsockopt()", true);
            return;
        }
        if (setsockopt(_sendSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        {
            logError("setsockopt()", true);
            return;
        }

        if (_ipVersion == IPVersion::IPv4)
        {
            struct sockaddr_in localaddr;
            memset(&localaddr, 0, sizeof(localaddr));
            localaddr.sin_family = AF_INET;
            localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
            localaddr.sin_port = htons(LOCALPORT);

            retval = bind(_clientSocket, (struct sockaddr*)&localaddr, sizeof(localaddr));
            if(retval == SOCKET_ERROR)
            {
                logError("setsockopt()", true);
                return;
            }

            inet_pton(AF_INET, MULTICASTIP, &mreq.ipv4.imr_multiaddr);
            mreq.ipv4.imr_interface.s_addr = htonl(INADDR_ANY);
            retval = setsockopt(_clientSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq.ipv4, sizeof(mreq.ipv4));
            if (retval == SOCKET_ERROR)
            {
                logError("setsockopt()", true);
                return;
            }
        }
        else
        {
            struct sockaddr_in6 localaddr;
            memset(&localaddr, 0, sizeof(localaddr));
            localaddr.sin6_family = AF_INET6;
            localaddr.sin6_addr = in6addr_any;
            localaddr.sin6_port = htons(LOCALPORT);

            retval = bind(_clientSocket, (struct sockaddr*)&localaddr, sizeof(localaddr));
            if (retval == SOCKET_ERROR)
            {
                logError("setsockopt()", true);
                return;
            }

            inet_pton(AF_INET6, MULTICASTIP_V6, &mreq.ipv6.ipv6mr_multiaddr);
            mreq.ipv6.ipv6mr_interface = 0;
            retval = setsockopt(_clientSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (const char*)&mreq.ipv6, sizeof(mreq.ipv6));
            if (retval == SOCKET_ERROR)
            {
                logError("setsockopt()", true);
                return;
            }   
        }
    }

    _stopFlag = false;
    _recvThread = thread(&Client::receiveThread, this);

    onConnected(_hDlg);
}

void Client::disconnect()
{
    if (_clientSocket != NULL)
    {
        shutdown(_clientSocket, SD_BOTH);
        closesocket(_clientSocket);
        _clientSocket = NULL;
    }

    _stopFlag = true;
    if (_recvThread.joinable())
        _recvThread.join();

    WSACleanup();
    onDisconnected(_hDlg);
}

void Client::sendData(Packet sendPacket)
{
    char header[HEADERSIZE]{};
    header[0] = sendPacket.type;
    memcpy(header + 1, &sendPacket.dataSize, sizeof(sendPacket.dataSize));

    if (_protocol == Protocol::TCP)
    {
        if (send(_clientSocket, header, HEADERSIZE, 0) == SOCKET_ERROR)
        {
            logError("send()");
            return;
        }
        if (send(_clientSocket, sendPacket.data.c_str(), sendPacket.dataSize, 0) == SOCKET_ERROR)
        {
            logError("send()");
            return;
        }
    }
    else
    {
        uint32_t sequence = 1;
        uint32_t totalChunks = sendPacket.dataSize / (BUFSIZE - HEADERSIZE - EXTRAHEADERSIZE);
        if (sendPacket.dataSize % (BUFSIZE - HEADERSIZE - EXTRAHEADERSIZE) != 0)
            totalChunks++;

        int maxRetries = 5;
        int sent = 0;
        while (sent < sendPacket.dataSize)
        {
            uint32_t size = min(BUFSIZE - HEADERSIZE - EXTRAHEADERSIZE, sendPacket.dataSize - sent);

            vector<uint8_t> buffer(HEADERSIZE + EXTRAHEADERSIZE + size);
            buffer[0] = sendPacket.type;
            memcpy(buffer.data() + 1, &size, sizeof(size));
            memcpy(buffer.data() + 1 + 4, &sequence, sizeof(sequence));
            memcpy(buffer.data() + 1 + 4 + 4, &totalChunks, sizeof(totalChunks));
            memcpy(buffer.data() + HEADERSIZE + EXTRAHEADERSIZE, sendPacket.data.c_str() + sent, size);

            int retryCount = 0;
            bool ackReceived = false;

            while(!ackReceived && retryCount < maxRetries)
            {
                if (sendto(_sendSocket, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0, getServerSockAddr(), sizeof(_serverAddrStorage)) == SOCKET_ERROR)
                {
                    logError("sendto()");
                    return;
                }

                sockaddr_in6 addr;
                int addrlen = sizeof(addr);

                char ackBuffer[8];
                int retval = recvfrom(_sendSocket, ackBuffer, sizeof(ackBuffer), 0, (sockaddr*)&addr, &addrlen);
                if (retval > 0)
                {
                    uint32_t ackSeq;
                    memcpy(&ackSeq, ackBuffer, sizeof(uint32_t));

                    if (ackSeq == sequence) // 올바른 ACK 확인
                    {
                        ackReceived = true;
                    }
                }
                retryCount++;
            }

            if (!ackReceived)
            {
                logError("recvfrom()");
                return;
            }

            sequence++;
            sent += size;
        }
    }
}

void Client::sendMessage(string msg)
{
    Packet packet(MESSAGE, msg.size(), msg);

    sendData(packet);
}

void Client::sendFile(filesystem::path filePath)
{
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
    string filename = filePath.filename().string();
    // 파일명과 바이너리 데이터를 합침 (파일명?바이너리 데이터)
    string data = filename + "?" + string(buffer.begin(), buffer.end());

    Packet packet(FILE, data.size(), data);

    sendData(packet); 
}

void Client::sendDrawing(COLORREF color, POINT from, POINT to)
{
    string data = to_string(color) + "?" + to_string(from.x) + ":" + to_string(from.y) + "?" + to_string(to.x) + ":" + to_string(to.y); // color?x:y?x:y 양식으로 서버에 전달

    Packet packet(DRAWING, data.size(), data);

    sendData(packet);
}

void Client::sendClearCanvas()
{
    Packet packet(CLEARCANVAS, 1, " ");

    sendData(packet);
}

void Client::setHDlg(HWND hDlg)
{
    this->_hDlg = hDlg;
}

bool Client::getStopFlag()
{
    return _stopFlag;
}

sockaddr* Client::getServerSockAddr()
{
    return (sockaddr*)&_serverAddrStorage;
}