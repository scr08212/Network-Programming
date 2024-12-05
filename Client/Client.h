#pragma once
#include <WinSock2.h>
#include <string>
#include <filesystem>
#include <thread>
#include <WS2tcpip.h>

using namespace std;

enum class IPVersion: uint8_t
{
    IPv4 = 0x01,
    IPv6 = 0x02
};

enum class Protocol: uint8_t
{
    TCP = 0X01,
    UDP = 0X02
};

struct ServerInfo
{
    string address;
    int port;
};

typedef union {
    struct ip_mreq ipv4;   // IPv4 멀티캐스트 요청
    struct ipv6_mreq ipv6; // IPv6 멀티캐스트 요청
} IpMreqUnion;

#pragma pack(1)
struct Packet 
{
    uint8_t type;           // 데이터 타입
    uint32_t dataSize;      // 데이터 크기
    string data;            // 실제 데이터
 
    Packet(uint8_t type, uint32_t dataSize, string data)
    {
        this->type = type;
        this->dataSize = dataSize;
        this->data = data;
    }
}; 
#pragma pack()

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
    sockaddr_storage _serverAddrStorage;
    sockaddr_storage _multicastAddrStorage;
    HWND _hDlg;


    void logError(const char* msg, bool fatal = false);
    void receiveThread();
    void handleReceivedData(uint8_t type, string data);
    void sendData(Packet sendPacket);
    sockaddr* getServerSockAddr();

public:
    Client();

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