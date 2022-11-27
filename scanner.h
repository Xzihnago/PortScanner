#pragma once

#include <bitset>
#include <future>
#include <iostream>
#include <vector>
#include "socket.h"

constexpr int BATCH_BITSIZE = 16; // Bitwise digits of the port amount, max value is 16
constexpr int BATCH_TIMES = 1 << (16 - BATCH_BITSIZE); // Times of scan batch
constexpr int BATCH_SIZE = 1 << BATCH_BITSIZE; // TImes of scan batch size

class Scanner {
private:
    int _family, _type, _protocol;
    bool _is_nonblock;
public:
    Scanner(int family, int type, int protocol, bool is_nonblock);
    bool is_open(std::string ip, unsigned short port);
    std::vector<unsigned short> scan_all_port(std::string ip);
};


Scanner::Scanner(int family, int type, int protocol, bool is_nonblock) {
    _family = family;
    _type = type;
    _protocol = protocol;
    _is_nonblock = is_nonblock;
}


/// <summary>
/// Check if the port is open.
/// </summary>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>bool</returns>
bool Scanner::is_open(std::string ip, unsigned short port) {
    // Create socket
    SOCKET sfd = create_socket(_family, _type, _protocol, _is_nonblock);

    // Create sockaddr_in
    SOCKADDR_IN target = create_sockaddr_in(_family, ip.c_str(), port);
    
    // Connect to target
    bool is_port_open = false;
    int res = connect(sfd, (SOCKADDR*)&target, sizeof(target));

    if (_is_nonblock) { // TODO
        TIMEVAL tv{};
        tv.tv_sec = 10;

        fd_set fds{};
        FD_SET(sfd, &fds);
        switch (select(sfd + 1, &fds, &fds, &fds, &tv))
        {
        case 0:
            // Timeout
            std::cout << "Timeout\n";
            break;
        case SOCKET_ERROR:
            std::cout << "Connect failed\n";
            break;
        default:
            is_port_open = true;
            //std::cout << port << "\n";
            break;
        }

        // TODO
        //WSAAsyncSelect(sfd);
    }
    else if (res != SOCKET_ERROR) {
        is_port_open = true;
        //std::cout << port << "\n";
    }
    //std::cout << port << "\n";

    // Close socket
    close_socket(sfd);

    return is_port_open;
}


/// <summary>
/// Scan all port.
/// </summary>
/// <param name="ip"></param>
std::vector<unsigned short> Scanner::scan_all_port(std::string ip) {
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
    for (int i = 0; i < 65536; i++) {
        if (ports[i]) res.push_back(i);
    }
    return res;
}