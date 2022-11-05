#include "tcpserver.h"

#include <future>
#include <thread>

#include "taskqueue.h"

const int MAX_BUFFER_SIZE = 256;

bool TcpServer::init()
{
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        std::cerr << "Failed to initialize" << std::endl;
        return false;
    }
#endif
    return true;
}

bool TcpServer::createSocket(Protocol protocol, IpVersion ipVersion, int flags, const std::string& port)
{
    m_hints.ai_family = ipVersion == IpVersion::IpV4 ? AF_INET : AF_INET6; // IPv4 address
    m_hints.ai_socktype = protocol == Protocol::Tcp ? SOCK_STREAM : SOCK_DGRAM;
    m_hints.ai_flags = flags;

    getaddrinfo(nullptr, port.c_str(), &m_hints, &m_bindAddress);
    m_socketListen = socket(m_bindAddress->ai_family, m_bindAddress->ai_socktype, m_bindAddress->ai_protocol);
    if (!ISVALIDSOCKET(m_socketListen))
    {
        std::cerr << "socket() failed." << GETSOCKETERRNO() << std::endl;
        return false;
    }
    if (bindSocket())
    {
        listen();
        FD_ZERO(&m_master);
        // first add the listening socket
        FD_SET(m_socketListen, &m_master);
        return true;
    }
    return false;
}

void TcpServer::onClientConnected(SOCKET socket)
{
    auto start = std::chrono::system_clock::now();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);

    char timeMsg[26];
    ctime_s(timeMsg, sizeof timeMsg, &startTime);
    std::cout << "Socket " << socket << ": Client connected on " << timeMsg << std::endl;

    const char* status = "Connected\r\n";
    sendToClient(socket, status, 12);
}

void TcpServer::onClientDisconnected(SOCKET socket)
{
    auto start = std::chrono::system_clock::now();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);

    char timeMsg[26];
    ctime_s(timeMsg, sizeof timeMsg, &startTime);
    std::cout << "Socket " << socket << ": Client disconnected on " << timeMsg << std::endl;
}

void TcpServer::onMessageReceived(SOCKET socket, char* str, int bytes_in)
{
    auto start = std::chrono::system_clock::now();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);
    char timeMsg[26];
    ctime_s(timeMsg, sizeof timeMsg, &startTime);
    std::cout << "Socket " << socket << ": Messaged received on " << timeMsg << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "Message: '" << std::string(str, 0, bytes_in) << "' has been processed" << std::endl;

}

void TcpServer::sendToClient(int clientSocket, const char* msg, int length)
{
    send(clientSocket, msg, length, 0);
}

void TcpServer::broadcastToClients(int excludedClient, const char* msg, int length)
{
    for (int i = 0; i < m_master.fd_count; i++)
    {
        SOCKET outSock = m_master.fd_array[i];
        if (outSock != m_socketListen && outSock != excludedClient)
        {
            sendToClient(outSock, msg, length);
        }
    }
}

bool TcpServer::bindSocket() const
{
    int option = 0;
    // IPV6_V6ONLY is enabled by default, so clear it
    if (setsockopt(m_socketListen, IPPROTO_IPV6, IPV6_V6ONLY,
                   static_cast<const char*>(static_cast<void*>(&option)), sizeof(option)))
    {
        std::cerr << "setsockopt() failed." << GETSOCKETERRNO() << std::endl;
        return false;
    }
    if (bind(m_socketListen, m_bindAddress->ai_addr, m_bindAddress->ai_addrlen) == SOCKET_ERROR) 
    {
        std::cerr << "bind() failed. " << GETSOCKETERRNO() << std::endl;
        return false;
    }
    freeaddrinfo(m_bindAddress);
    return true;
}

bool TcpServer::listen() const
{
    if (::listen(m_socketListen, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed. " << GETSOCKETERRNO() << std::endl;
        return false;
    }
    return true;
}

int TcpServer::accept()
{
    bool running = true;

    TaskQueue tasksQueue;
    while (running)
    {
        fd_set readyToReadSockets = m_master;
        const int socketCount = select(0, &readyToReadSockets, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++)
        {
            SOCKET sock = readyToReadSockets.fd_array[i];

            // Is it an inbound communication?
            if (FD_ISSET(m_socketListen, &readyToReadSockets))
            {
                // Accept a new connection
                SOCKET client = ::accept(m_socketListen, nullptr, nullptr);
                std::cout << "Client socket " << client << " created" << std::endl;

                // Add the new connection to the list of connected clients
                FD_SET(client, &m_master);

                onClientConnected(client);
            }
            else // It's an inbound message
            {
                auto buf = std::make_shared<char[]>(2 << 10);

                // Receive message
                int bytesIn = recv(sock, buf.get(), 2 << 10, 0);
                if (bytesIn <= 0)
                {
                    // Drop the client
                    onClientDisconnected(sock);
                    closesocket(sock);
                    FD_CLR(sock, &m_master);
                }
                else
                {
                    tasksQueue.submit<void>([this, sock, buf, bytesIn]() 
                        {
                            onMessageReceived(sock, buf.get(), bytesIn);
                        });
                    if (buf[0] == 'Q')
                    {
                        running = false;
                    }
                }
            }
        }
    }
    std::cout << "Waiting for completion..." << std::endl;

    tasksQueue.waitForCompletion();

    FD_CLR(m_socketListen, &m_master);
    closesocket(m_socketListen);

    while (m_master.fd_count > 0)
    {
        // Get the socket number
        SOCKET sock = m_master.fd_array[0];

        // Remove it from the master file list and close the socket
        FD_CLR(sock, &m_master);
        closesocket(sock);
    }
    std::cout << "Finished work" << std::endl;

    return 0;
}


TcpServer::~TcpServer()
{
#if defined(_WIN32)
    WSACleanup();
#endif
}
