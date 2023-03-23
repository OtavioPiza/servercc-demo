#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::libcc::utils::Status;
using ostp::severcc::server::Request;
using ostp::severcc::server::ServerMode;
using namespace ostp::severcc::server;

std::function<void(const Request)> handler = [](const Request req) {
    std::cout << "Received request: " << req.data << std::endl;

    if (req.client_fd.has_value()) {
        std::string response =
            "HTTP/1.1 200 OK\r"

            "Content-Type: text/html\r"

            "Content-Length: 12\r"

            "\r"

            "Hello World!";
        send(req.client_fd.value(), response.c_str(), response.size(), 0);

        close(req.client_fd.value());
    }
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
    server.set_default_processor(handler);

    // Print the server information.

    // Start the server on a new thread.
    std::thread server_thread([&server]() {
        std::cout << "Server listening on port " << server.get_port() << std::endl;
        server.run();
    });

    // Create tcp server on the same port.
    TcpServer tcp_server(port);
    Server &tcp_server_ref = tcp_server;
    tcp_server_ref.set_default_processor(handler);

    // Print the server information.

    // Start the server on a new thread.
    std::thread tcp_server_thread([&tcp_server_ref]() {
        std::cout << "Server listening on port " << tcp_server_ref.get_port() << std::endl;
        tcp_server_ref.run();
    });

    // Wait for the server to finish.
    server_thread.join();
    tcp_server_thread.join();

    return 0;
}
