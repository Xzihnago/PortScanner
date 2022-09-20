#include <iostream>
#include "scanner.h"

using namespace std;

int main() {
    Scanner scanner(AF_INET, SOCK_STREAM, IPPROTO_IP);
    std::vector<unsigned short> ports = scanner.scan_all_port("127.0.0.1");

    cout << "Open ports: " << ports.size() << " in total\n";
    for (unsigned short& port : ports) cout << port << "\n";
}