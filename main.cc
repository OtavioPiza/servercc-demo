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

int main() {
    // TcpServer tcp_server = TcpServer(8080);
    // Server &server = tcp_server;
    UdpServer udp_server = UdpServer(8080, ServerMode::SYNC);
    Server &server = udp_server;
    server.register_default_processor(handler);
    server.run();
    return 0;
}
