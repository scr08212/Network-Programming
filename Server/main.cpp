#pragma once
#include "TCPServer.h"
#include "UDPServer.h"
#include <thread>
#include <iostream>

int main(int argc, char* argv[])
{
    TCPServer tcpServer;
    UDPServer udpServer;
    thread tcpThread = thread(&TCPServer::run, tcpServer, 9000);
    thread udpThread = thread(&UDPServer::run, udpServer, 9001);
    
    tcpThread.join();
    udpThread.join();
}