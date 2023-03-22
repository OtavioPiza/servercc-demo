#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::libcc::utils::Status;
using ostp::severcc::server::Request;
using ostp::severcc::server::ServerMode;
using namespace ostp::severcc::server;

std::function<void(const Request)> handler = [](const Request req) {
    std::cout << "Received request: " << req.data << std::endl;
};



int main(int argc, char *argv[]) {

    // Verify there is at least three arguments.
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <port> <group> <interfaces>" << std::endl;
        return 1;
    }

    // Extract the port, group and interfaces.
    int port = atoi(argv[1]);
    std::string group = argv[2];
    std::vector<std::string> interfaces;
    for (int i = 3; i < argc; i++) {
        interfaces.push_back(argv[i]);
    }

    // Create a UDP server.
    UdpServer udp_server(port, group, interfaces);
    Server &server = udp_server;
    server.register_default_processor(handler);
    server.run();
    return 0;
}
