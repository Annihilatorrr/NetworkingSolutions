#pragma once
#include <chrono>
#include <iostream>
#include <ostream>
#include <WS2tcpip.h>

#include "defines.h"

class TcpServer
{
    addrinfo* m_bindAddress{};
    addrinfo m_hints{};

    SOCKET m_socketListen;
    SOCKET m_socketClient;
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

    int start()
    {
#if defined(_WIN32)
        WSADATA d;
        if (WSAStartup(MAKEWORD(2, 2), &d))
        {
            std::cerr << "Failed to initialize" << std::endl;
            return -1;
        }
#endif
        return 0;
    }
    void configure(Protocol protocol, IpVersion ipVersion, int flags, std::string port)
    {
        m_hints.ai_family = ipVersion == IpVersion::IpV4 ? AF_INET: AF_INET6; // IPv4 address
        m_hints.ai_socktype = protocol == Protocol::Tcp ? SOCK_STREAM:SOCK_DGRAM;
        m_hints.ai_flags = flags;

        getaddrinfo(nullptr, port.c_str(), &m_hints, &m_bindAddress);
    }

    int createSocket()
    {

        m_socketListen = socket(m_bindAddress->ai_family, m_bindAddress->ai_socktype, m_bindAddress->ai_protocol);
        if (!ISVALIDSOCKET(m_socketListen))
        {
            std::cerr << "socket() failed." << GETSOCKETERRNO() << std::endl;
            return 1;
        }
        return 0;
    }

    int bindSocket() const
    {
        int option = 0;
        // IPV6_V6ONLY is enabled by default, so clear it
        if (setsockopt(m_socketListen, IPPROTO_IPV6, IPV6_V6ONLY,
            static_cast<const char*>(static_cast<void*>(&option)), sizeof(option))) 
        {
            std::cerr << "setsockopt() failed." << GETSOCKETERRNO() << std::endl;
            return 1;
        }
        if (bind(m_socketListen,
            m_bindAddress->ai_addr, m_bindAddress->ai_addrlen)) {
            std::cerr << "bind() failed. " << GETSOCKETERRNO() << std::endl;
            return 1;
        }
        freeaddrinfo(m_bindAddress);
        return 0;
    }

    int listen() const
    {
        if (::listen(m_socketListen, 10) < 0)
        {
            std::cerr << "listen() failed. " << GETSOCKETERRNO() << std::endl;
            return 1;
        }
        return 0;
    }

    int accept()
    {
        sockaddr_storage client_address;
        socklen_t clientLength = sizeof(client_address);
        m_socketClient = ::accept(m_socketListen, reinterpret_cast<sockaddr*>(&client_address), &clientLength);
        if (!ISVALIDSOCKET(m_socketClient))
        {
            std::cerr << "accept() failed. " << GETSOCKETERRNO() << std::endl;
            return 1;
        }

        std::cout << "Client is connected... " << std::endl;
        char address_buffer[100];
        getnameinfo(reinterpret_cast<sockaddr*>(&client_address),
            clientLength,
            address_buffer,
            sizeof(address_buffer),
            nullptr,
            0,
            NI_NUMERICHOST);

        std::cout << address_buffer << std::endl;

        std::cout << "Reading request..." << std::endl;
        char request[1024];
        int bytesReceived = recv(m_socketClient, request, 1024, 0);
        std::cout << "Received " << bytesReceived << " bytes." << std::endl;
        std::cout << std::string(request, request+ bytesReceived) << std::endl;
        std::cout << "Sending response..." << std::endl;
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Local time is: ";
        int bytes_sent = send(m_socketClient, response, strlen(response), 0);

        auto start = std::chrono::system_clock::now();
        // Some computation here

        std::time_t startTime = std::chrono::system_clock::to_time_t(start);

        char time_msg[26];
        ctime_s(time_msg, sizeof time_msg, &startTime);


        bytes_sent = send(m_socketClient, time_msg, strlen(time_msg), 0);
        std::cout << "Sent " << bytes_sent << " of " << static_cast<int>(strlen(time_msg)) << " bytes" << std::endl;
        std::cout << "Sent " << bytes_sent << " of " << static_cast<int>(strlen(response)) << " bytes" << std::endl;

        return 0;
    }

    void close()
    {
        CLOSESOCKET(m_socketClient);
#if defined(_WIN32)
        WSACleanup();
#endif
    }
};

