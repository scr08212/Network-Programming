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

// 둘중 하나만 쓰니까 union 사용
typedef union {
    struct ip_mreq ipv4;   // IPv4 멀티캐스트 요청
    struct ipv6_mreq ipv6; // IPv6 멀티캐스트 요청
} IpMreqUnion;


// 패킷 구조체.
// 솔직히 없어도 됨. 일단 잘 작동하니 유지
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

// 이벤트
typedef void (*OnConnectedListener)(HWND);
typedef void (*OnDisconnectedListener)(HWND);
typedef void (*OnMessageReceivedListener)(HWND, wstring);
typedef void (*OnFileReceivedListener)(HWND, wstring);
typedef void (*OnDrawingReceivedListener)(HWND, COLORREF, POINT, POINT);
typedef void (*onClearCanvasRequestedListener)(HWND);

class Client
{
private:
    SOCKET _clientSocket; // UDP, TCP 공용.
    SOCKET _sendSocket; // UDP 전용. UDP에선 send랑 recv소켓(_clientSock)을 따로 뒀음.
    ServerInfo _serverInfo;
    IPVersion _ipVersion;
    Protocol _protocol;
    bool _stopFlag; // disconnect시 true로 바뀜
    thread _recvThread;
    sockaddr_storage _serverAddrStorage;
    sockaddr_storage _multicastAddrStorage;
    HWND _hDlg; // 이벤트 인수로 넘겨주는 용도

    void logError(const char* msg, bool fatal = false); // err_quit, err_display 통합
    void receiveThread();  // 데이터 수신 관련은 해당 쓰레드에서 처리
    void handleReceivedData(uint8_t type, string data); // receiveThread에서 받은 후 Type에 따라 후처리
    void sendData(Packet sendPacket); // 송신 관련 함수들(sendMessage, sendFile 등) 중복되는 코드 모아둠. 
    sockaddr* getServerSockAddr(); // _serverAddrStorage 타입 변환

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