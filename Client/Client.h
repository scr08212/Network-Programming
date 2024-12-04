#pragma once
#include <WinSock2.h>
#include <string>
#include <filesystem>
#include <thread>

using namespace std;

enum class IPVersion : uint8_t
{
    IPv4 = 0x01,
    IPv6 = 0x02
};

enum class Protocol :uint8_t
{
    TCP = 0X01,
    UDP = 0X02
};

struct ServerInfo
{
    string address;
    int port;
};

typedef void (*OnConnectedListener)(HWND);
typedef void (*OnDisconnectedListener)(HWND);
typedef void (*OnMessageReceivedListener)(HWND, wstring);
typedef void (*OnFileReceivedListener)(HWND, wstring);
typedef void (*OnDrawingReceivedListener)(HWND, COLORREF, POINT, POINT);
typedef void (*onClearCanvasRequestedListener)(HWND);

class Client
{
private:
    SOCKET _clientSocket;
    ServerInfo _serverInfo;
    IPVersion _ipVersion;
    Protocol _protocol;
    bool _stopFlag;
    thread _recvThread;
    HWND hDlg;
    sockaddr_storage addrStorage;

    void logError(const char* msg, bool fatal = false);
    void receiveThread();
    void sendData(string header, string data);
    sockaddr* getServerSockAddr();

public:
    OnConnectedListener onConnected;
    OnDisconnectedListener onDisconnected;
    OnMessageReceivedListener onMesssageReceived;
    OnFileReceivedListener onFileReceived;
    OnDrawingReceivedListener onDrawingReceived;
    onClearCanvasRequestedListener onClearCanvasRequested;

    void connectToServer(string serverAddr, int port, IPVersion ipVersion, Protocol protocol);
    void disconnect();
    void sendMessage(string msg);
    void sendFile(filesystem::path filePath);
    void sendDrawing(COLORREF color, POINT from, POINT to);
    void sendClearCanvas();
    void setHDlg(HWND hDlg);
    bool getStopFlag();
};