#pragma once

#if _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#endif
#include <iostream>

std::once_flag flag_init;

/// <summary>
/// Create a socket.
/// </summary>
/// <param name="family"></param>
/// <param name="type"></param>
/// <param name="protocol"></param>
/// <param name="is_nonblock"></param>
/// <returns>SOCKET</returns>
SOCKET create_socket(int family, int type, int protocol, bool is_nonblock) {
#ifdef _WIN32
    // Initialize Ws2_32.lib
    std::call_once(flag_init, []() {
        WSADATA wsadata;
        if (WSAStartup(MAKEWORD(2, 2), &wsadata) != NO_ERROR) std::cout << "WSAStartup failed (" << WSAGetLastError() << ")\n";
        else std::cout << "WSAStartup success\n";

        atexit([]() {
            if (WSACleanup() != NO_ERROR) std::cout << "WSACleanup failed (" << WSAGetLastError() << ")\n";
            else std::cout << "WSACleanup success\n";
            });
        });
#endif

    // Create socket
    SOCKET sfd = socket(family, type, protocol);

    // Check if socket is valid
    if (sfd == INVALID_SOCKET) {
#ifdef _WIN32
        std::cout << "Create socket failed (" << WSAGetLastError() << ")\n";
#else
        std::cout << "Create socket failed\n";
#endif
    }
    else {
        // Setup socket block mode
#ifdef _WIN32
        unsigned long sockmode = is_nonblock;
        int res = ioctlsocket(sfd, FIONBIO, &sockmode);
        if (res != NO_ERROR) res = WSAGetLastError();
#else
        int flag = fcntl(sfd, F_GETFL, 0);
        int res = fcntl(sfd, F_SETFL, is_nonblock ? flag | O_NONBLOCK : flag & ~O_NONBLOCK);
#endif
        if (res != NO_ERROR) std::cout << "ioctlsocket failed (" << res << ")\n";
    }

    return sfd;
}

/// <summary>
/// Close a socket.
/// </summary>
/// <param name="sfd"></param>
void close_socket(SOCKET sfd) {
    if (int res = closesocket(sfd) != NO_ERROR) {
#ifdef _WIN32
        res = WSAGetLastError();
#endif
        std::cout << "closesocket failed (" << res << ")\n";
    }
}

/// <summary>
/// Create a sockaddr_in.
/// </summary>
/// <param name="family"></param>
/// <param name="ip"></param>
/// <param name="port"></param>
/// <returns>sockaddr_in</returns>
SOCKADDR_IN create_sockaddr_in(int family, const char* ip, int port) {
    SOCKADDR_IN addr{};
    addr.sin_family = family;
    addr.sin_port = htons(port);
    inet_pton(family, ip, &addr.sin_addr);

    return addr;
}