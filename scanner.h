#pragma once

#if _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <future>
#include <iostream>
#include <vector>

constexpr int BATCH_SIZE = 8;   // Bitwise digits of the port number;

bool is_init = false;


/// <summary>
/// Socket object
/// </summary>
class Scanner {
private:
    int _family, _type, _protocol;
public:
    Scanner(int family, int type, int protocol);
    bool is_open(std::string ip, unsigned short port);
    std::vector<unsigned short> scan_all_port(std::string ip);
};


/// <summary>
/// Create a socket
/// </summary>
/// <param name="family"></param>
/// <param name="type"></param>
/// <param name="protocol"></param>
Scanner::Scanner(int family, int type, int protocol) {
    _family = family;
    _type = type;
    _protocol = protocol;

#ifdef _WIN32
    // Initialize Ws2_32.lib
    if (!is_init) {
        WSADATA wsadata;
        if (int res = WSAStartup(MAKEWORD(2, 2), &wsadata)) {
            std::cout << "WSAStartup failed: " << res << "\n";
        }
        else {
            std::cout << "WSAStartup success\n";
        }
        is_init = true;
    }
#endif
}


/// <summary>
/// Check if the port is open.
/// </summary>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>bool</returns>
bool Scanner::is_open(std::string ip, unsigned short port) {
    // Set connection target
    sockaddr_in target{};
    target.sin_family = _family;
    inet_pton(_family, ip.c_str(), &target.sin_addr);
    target.sin_port = htons(port);

    // Create socket
    SOCKET sfd = socket(_family, _type, _protocol);
    if (sfd == INVALID_SOCKET) std::cout << "Create socket failed.\n";

    // Connect to target
    bool is_port_open = false;
    if (connect(sfd, (sockaddr*)&target, sizeof(target)) != SOCKET_ERROR) is_port_open = true;

    // Close socket
    closesocket(sfd);

    return is_port_open;
}


/// <summary>
/// Scan all port
/// </summary>
/// <param name="ip"></param>
std::vector<unsigned short> Scanner::scan_all_port(std::string ip) {
    std::vector<std::future<void>> futs;
    std::vector<unsigned short> ports;
    unsigned short port;
    for (int i = 0; i < 1 << (16 - BATCH_SIZE); i++) {
        std::cout << "Scanning: " << (i << BATCH_SIZE) << " ~ " << ((i + 1) << BATCH_SIZE) - 1 << "\n";
        for (int j = 0; j < 1 << BATCH_SIZE; j++) {
            port = i << BATCH_SIZE | j;
            futs.push_back(
                std::async([this, ip, port, &ports]() {
                    if (is_open(ip, port)) ports.push_back(port);
                    })
            );
        }
        for (std::future<void>& fut : futs) fut.get();
        futs.clear();
    }
    return ports;
}