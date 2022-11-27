#include <chrono>
#include <iostream>
#include "scanner.h"
#include "util.h"

#include <algorithm>
#include <set>
using namespace std;

int main() {
    Scanner scanner(AF_INET, SOCK_STREAM, IPPROTO_IP, false);

    auto tstart = chrono::steady_clock::now();
    auto ports = scanner.scan_all_port("127.0.0.1");
    auto tend = chrono::steady_clock::now();
    cout << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(tend - tstart).count() << "ms\n";

    cout << "Open ports (" << ports.size() << " in total):\n";
    cout << "[" << join(ports, ", ") << "]\n";
}