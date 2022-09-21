#include <chrono>
#include <iostream>
#include "scanner.h"

using namespace std;

int main() {
    Scanner scanner(AF_INET, SOCK_STREAM, IPPROTO_IP);
    auto tstart = chrono::steady_clock::now();
    std::vector<unsigned short> ports = scanner.scan_all_port("127.0.0.1");
    auto tend = chrono::steady_clock::now();
    cout << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(tend - tstart).count() << "ms\n";

    cout << "Open ports (" << ports.size() << " in total):\n";
    for (auto& port : ports) cout << port << "\n";
}