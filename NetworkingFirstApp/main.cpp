// NetworkingFirstApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <iostream>

#include "tcpserver.h"


int main()
{
    TcpServer server;
    if (server.init())
    {
        const auto noPrivilegeHttpPort = "8080";

        // When the AI_PASSIVE flag is set and pNodeName is a NULL pointer,
        // the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses
        // and IN6ADDR_ANY_INIT for IPv6 addresses.
        std::cout << "Creating socket..." << std::endl;
        server.createSocket(TcpServer::Protocol::Tcp, TcpServer::IpVersion::IpV6, AI_PASSIVE, noPrivilegeHttpPort);
        server.accept();
    }

    
    return 0;
}