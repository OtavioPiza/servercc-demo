#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::libcc::utils::Status;
using ostp::severcc::server::Request;
using ostp::severcc::server::ServerMode;
using namespace ostp::severcc::server;

std::function<void(const Request)> handler = [](const Request req) {
    const std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";
    send(req.client_fd, response.c_str(), response.size(), 0);
    close(req.client_fd);
};

int main() {
    TcpServer tcp_server = TcpServer(8080);
    Server &server = tcp_server;
    server.register_default_processor(handler);
    server.run();
    return 0;
}