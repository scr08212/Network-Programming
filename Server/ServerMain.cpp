#include "Server.h"
#include <iostream>

int main(int argc, char* argv[])
{
    //TODO: 
    // [v]: dual stack���� IPv4, IPv6 ���� ó��
    // [v]: ��Ƽ������� TCP����
    // [v]: �������� Ŭ���̾�Ʈ�� ���� �޽��� ������ 
    // [ ]: �̹���/�ؽ�Ʈ���� ó��
    // [ ]: �ǽð� ����� ó��
    // [ ]: �̹���/�ؽ�Ʈ���� �� �Է� Ÿ���� ������ �տ� 4����Ʈ type���� ����
    // [ ]: IPv4 ���� �� �ּ� ��½� IPv4 �κи� ��µǰ� ó��

    Server server(9000);
    server.start();


    return 0;
}