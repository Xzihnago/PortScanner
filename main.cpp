#include <iostream>
#include "socket.h"

using namespace std;

int main() {
    Scanner sockfd(AF_INET, SOCK_STREAM, IPPROTO_IP);
    std::vector<unsigned short> ports = sockfd.scan_all_port("127.0.0.1");

    cout << "Open ports:";
    for (unsigned short& port : ports) cout << port << "\n";
}