// NetworkingFirstApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <ctime> 
#include "defines.h"
#include <iostream>

#include "tcpserver.h"


//void print_adapter(PIP_ADAPTER_ADDRESSES aa)
//{
//    char buf[BUFSIZ]{};
//    WideCharToMultiByte(CP_ACP, 0, aa->FriendlyName, wcslen(aa->FriendlyName), buf, BUFSIZ, NULL, NULL);
//    std::cout <<"adapter name: " <<  buf << std::endl;
//}
//
//void print_addr(PIP_ADAPTER_UNICAST_ADDRESS ua)
//{
//    char buf[BUFSIZ];
//
//    int family = ua->Address.lpSockaddr->sa_family;
//    std::cout << "\t" << (family == AF_INET ? "IPv4 " : "IPv6 ");
//
//    memset(buf, 0, BUFSIZ);
//    getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);
//    std::cout << buf << std::endl;
//
//}

int main()
{
    TcpServer server;
    server.start();
    const auto noPrivilegeHttpPort = "8080";
    // When the AI_PASSIVE flag is set and pNodeName is a NULL pointer,
    // the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses
    // and IN6ADDR_ANY_INIT for IPv6 addresses.
    server.configure(TcpServer::Protocol::Tcp, TcpServer::IpVersion::IpV6, AI_PASSIVE, noPrivilegeHttpPort);
    std::cout << "Creating socket..." << std::endl;

    server.createSocket();
    std::cout << "Binding socket to local address..." << std::endl;

    server.bindSocket();

    std::cout << "Listening..." << std::endl;
    server.listen();

    server.accept();
    std::cout << "Closing connection..." << std::endl;
    server.close();
    return 0;
	//DWORD asize = 20000;
 //   PIP_ADAPTER_ADDRESSES adapters{};
 //   do 
 //   {
 //       adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(new char[asize]);
 //       if (!adapters) {
 //           printf("Couldn't allocate %ld bytes for adapters.\n", asize);
 //           WSACleanup();
 //           return -1;
 //       }
 //       int r = GetAdaptersAddresses(AF_UNSPEC, //we want both IPv4 and IPv6 addresses
 //           GAA_FLAG_INCLUDE_PREFIX, nullptr, adapters, &asize);
 //       if (r == ERROR_BUFFER_OVERFLOW) {
 //           printf("GetAdaptersAddresses wants %ld bytes.\n", asize);
 //           delete[] adapters;

 //       }
 //       else if (r == ERROR_SUCCESS) {
 //           PIP_ADAPTER_ADDRESSES adapter = adapters;
 //           while (adapter) {
 //               print_adapter(adapters);
 //               PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
 //               while (address) {
 //                   print_addr(address);
 //                   address = address->Next;
 //               }
 //               adapter = adapter->Next;
 //           }
 //       }
 //       else {
 //           printf("Error from GetAdaptersAddresses: %d\n", r);
 //           delete[] adapters;
 //           WSACleanup();
 //           return -1;
 //       }
 //   } while (!adapters);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
