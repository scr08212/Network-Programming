#pragma once
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <thread>

using namespace std;

typedef void (*OnDrawingReceivedListener)(HWND, COLORREF, POINT, POINT);
typedef void (*OnClearCanvasReceivedListener)(HWND);

class Client
{
private:
    SOCKET clientSocket;
    string serverAddr;
    int serverPort;
    bool isIPv6;
    bool isUDP;
    sockaddr_storage sockAddrStorage;
    thread recvThread;
    HWND hDlg;

    void logError(const char* msg, bool fatal = false);
    void initializeSocket();
    void threadReceive();
    sockaddr* getSockAddr();

public:
    Client();
    void setDlgHandle(HWND hwnd);
    void sendMessage(string msg);
    void sendFile(filesystem::path filePath);
    void sendDrawing(COLORREF color, POINT from, POINT to);
    void sendClearCanvas();
    void connectToServer(string addr, int port, bool isIPv6);
    void disconnect();
    OnDrawingReceivedListener onDrawingReceived;
    OnClearCanvasReceivedListener onClearCanvasReceived;
    bool stopFlag;
};