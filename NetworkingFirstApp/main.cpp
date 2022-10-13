// NetworkingFirstApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
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

void print_adapter(PIP_ADAPTER_ADDRESSES aa)
{
    char buf[BUFSIZ]{};
    WideCharToMultiByte(CP_ACP, 0, aa->FriendlyName, wcslen(aa->FriendlyName), buf, BUFSIZ, NULL, NULL);
    std::cout <<"adapter name: " <<  buf << std::endl;
}

void print_addr(PIP_ADAPTER_UNICAST_ADDRESS ua)
{
    char buf[BUFSIZ];

    int family = ua->Address.lpSockaddr->sa_family;
    std::cout << "\t" << (family == AF_INET ? "IPv4 " : "IPv6 ");

    memset(buf, 0, BUFSIZ);
    getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);
    std::cout << buf << std::endl;

}

int main()
{
#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) 
    {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    DWORD asize = 20000;
    PIP_ADAPTER_ADDRESSES adapters{};
    do {
        adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(new char[asize]);
        if (!adapters) {
            printf("Couldn't allocate %ld bytes for adapters.\n", asize);
            WSACleanup();
            return -1;
        }
        int r = GetAdaptersAddresses(AF_UNSPEC, //we want both IPv4 and IPv6 addresses
            GAA_FLAG_INCLUDE_PREFIX, nullptr, adapters, &asize);
        if (r == ERROR_BUFFER_OVERFLOW) {
            printf("GetAdaptersAddresses wants %ld bytes.\n", asize);
            delete[] adapters;

        }
        else if (r == ERROR_SUCCESS) {
            PIP_ADAPTER_ADDRESSES adapter = adapters;
            while (adapter) {
                print_adapter(adapters);
                PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
                while (address) {
                    print_addr(address);
                    address = address->Next;
                }
                adapter = adapter->Next;
            }
        }
        else {
            printf("Error from GetAdaptersAddresses: %d\n", r);
            delete[] adapters;
            WSACleanup();
            return -1;
        }
    } while (!adapters);
#endif
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
