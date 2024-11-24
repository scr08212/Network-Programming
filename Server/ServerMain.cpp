#include "Server.h"

int main(int argc, char* argv[])
{
    //TODO: 
    // [v]: dual stack으로 IPv4, IPv6 동시 처리
    // [v]: 멀티쓰레드로 TCP연결
    // [v]: 접속중인 클라이언트에 받은 메시지 보내기 
    // [ ]: 이미지/텍스트파일 처리
    // [ ]: 실시간 드로잉 처리
    // [ ]: 이미지/텍스트파일 등 입력 타입은 데이터 앞에 4바이트 type으로 결정
    // [ ]: IPv4 감지 후 주소 출력시 IPv4 부분만 출력되게 처리

    Server server(9000);
    server.start();


    return 0;
}