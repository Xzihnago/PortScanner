#pragma once

#if _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <fcntl.h>
#endif
#include <future>
#include <iostream>
#include <vector>

constexpr int BATCH_SIZE = 16;   // Bitwise digits of the port amount, max value is 16

bool is_init = false;


/// <summary>
/// Socket object
/// </summary>
class Scanner {
private:
    int _family, _type, _protocol;
public:
    Scanner(int family, int type, int protocol);
    SOCKET create_socket(bool is_nonblock);
    sockaddr_in create_sockaddr_in(std::string ip, int port);
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
        if (int res = WSAStartup(MAKEWORD(2, 2), &wsadata) != NO_ERROR) {
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
/// Create a socket
/// </summary>
/// <returns>SOCKET</returns>
SOCKET Scanner::create_socket(bool is_nonblock) {
    SOCKET sfd = socket(_family, _type, _protocol);
    if (sfd == INVALID_SOCKET) std::cout << "Create socket failed.\n";
    else {
        // Setup socket block mode
#ifdef _WIN32
        unsigned long sockmode = is_nonblock;
        if (int res = ioctlsocket(sfd, FIONBIO, &sockmode) != NO_ERROR) std::cout << "ioctlsocket failed: " << res << "\n";
#else
        fcntl(sfd, F_SETFL, O_NONBLOCK);
#endif
    }

    return sfd;
}


/// <summary>
/// Create a sockaddr of in
/// </summary>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>sockaddr</returns>
sockaddr_in Scanner::create_sockaddr_in(std::string ip, int port) {
    sockaddr_in addr;
    addr.sin_family = _family;
    addr.sin_port = htons(port);
    inet_pton(_family, ip.c_str(), &addr.sin_addr);

    return addr;
}


/// <summary>
/// Check if the port is open.
/// </summary>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>bool</returns>
bool Scanner::is_open(std::string ip, unsigned short port) {
    // Set connection target
    //sockaddr_in target = create_sockaddr_in(ip, port);
    sockaddr_in target;
    target.sin_family = _family;
    target.sin_port = htons(port);
    inet_pton(_family, ip.c_str(), &target.sin_addr);

    // Create socket
    SOCKET sfd = create_socket(false);

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
    std::vector<bool> port_stat(65536, false);

    std::vector<std::future<void>> futs(1 << BATCH_SIZE);
    for (int t = 0; t < 1 << (16 - BATCH_SIZE); t++) {
        std::cout << "Scanning: " << (t << BATCH_SIZE) << " ~ " << ((t + 1) << BATCH_SIZE) - 1 << "\n";
        for (int i = 0; i < 1 << BATCH_SIZE; i++) {
            unsigned short port = t << BATCH_SIZE | i;
            futs[i] = std::async([this, ip, port, &port_stat]() {
                std::cout << port << "\n";
                if (is_open(ip, port)) port_stat[port] = true;
                });
        }
        //for (std::future<void>& fut : futs) fut.get();
    }

    // Calculate open port
    std::vector<unsigned short> res;
    for (int i = 0; i < port_stat.size(); i++) {
        if (port_stat[i]) res.push_back(i);
    }
    return res;
}