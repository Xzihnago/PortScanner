#pragma once

#if _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <fcntl.h>
#endif
#include <bitset>
#include <future>
#include <iostream>
#include <vector>

constexpr int BATCH_BITSIZE = 16;                        // Bitwise digits of the port amount, max value is 16
constexpr int BATCH_TIMES = 1 << (16 - BATCH_BITSIZE);  // Times of scan batch
constexpr int BATCH_SIZE = 1 << BATCH_BITSIZE;          // TImes of scan batch size

bool is_init = false;

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
/// Create a socket.
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
/// Create a sockaddr_in.
/// </summary>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>sockaddr_in</returns>
sockaddr_in Scanner::create_sockaddr_in(std::string ip, int port) {
    sockaddr_in addr{};
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
    // Create sockaddr_in
    sockaddr_in target = create_sockaddr_in(ip, port);

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
/// Scan all port.
/// </summary>
/// <param name="ip"></param>
std::vector<unsigned short> Scanner::scan_all_port(const std::string ip) {
    std::bitset<65536> ports;

    std::cout << "Scan all port of " << ip << "\n";
    std::vector<std::future<void>> futs(BATCH_SIZE);
    for (int t = 0; t < BATCH_TIMES; t++) {
        int pstart = t << BATCH_BITSIZE;
        std::cout << "Scan batch " << t + 1 << " of " << BATCH_TIMES << ": " << pstart << " ~ " << pstart + BATCH_SIZE - 1 << "\n";
        for (int i = 0; i < BATCH_SIZE; i++) {
            int port = pstart | i;
            futs[i] = std::async([&, port]() { ports[port] = is_open(ip, port); });
        }
        for (auto& fut : futs) fut.get();
    }
    std::cout << "Scan finish\n";

    // Summon open ports list
    std::vector<unsigned short> res;
    for (size_t i = 0; i < ports.size(); i++) {
        if (ports[i]) res.push_back(i);
    }
    return res;
}