#pragma once
#include <chrono>
#include <iostream>
#include <ostream>
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <WinSock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "defines.h"

class TcpServer
{

    addrinfo* m_bindAddress{};
    addrinfo m_hints{};

    SOCKET m_socketListen;
    fd_set m_master;		// Master file descriptor set
    //SOCKET m_socketClient;
    bool bindSocket() const;

    bool listen() const;

public:

    enum class Protocol
    {
        Tcp,
        Udp
    };

    enum class IpVersion
    {
        IpV4,
        IpV6
    };

    bool init();

    bool createSocket(Protocol protocol, IpVersion ipVersion, int flags, const std::string& port);

    void onClientConnected(SOCKET uint);
    void onClientDisconnected(SOCKET uint);
    void onMessageReceived(SOCKET uint, char* str, int bytes_in);
    void sendToClient(int clientSocket, const char* msg, int length);
    void broadcastToClients(int excludedClient, const char* msg, int length);
    int accept();


    ~TcpServer();
};

