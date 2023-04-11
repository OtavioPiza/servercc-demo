#include <arpa/inet.h>
#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::servercc::distributed::DistributedServer;

/// Echoes the tcp request.

/// The main function.
int main(int argc, char *argv[]) {
    /// Interface for multicast.
    const std::string interface = "ztyxaydhop";
    std::string interface_ip;
    if (argc != 1) {
        interface_ip = argv[1];
    } else {
        interface_ip = "172.24.202.75";
    }

    /// Group for multicast.
    const std::string group = "224.1.1.1";

    /// Port for multicast server UDP and for the TCP server.
    const int port = 8000;

    /// Create the distributed server.
    DistributedServer server(
        interface, interface_ip, group, port,
        [](const Request request) {
            std::cout << "Received request: " << request.data << std::endl;
        },
        [](const std::string ip) { std::cout << "Peer connected: " << ip << std::endl; },
        [](const std::string ip) { std::cout << "Peer disconnected: " << ip << std::endl; });

    server.add_handler("get", [&](const Request request) {
        server.log(Status::OK, "Received get request: " + request.data);

        /// Send the response.
        string response = "Hello from server!";
        string http_response =
            "HTTP/1.1 200 OK\r"
            "Content-Type: text/plain\r"
            "Content-Length: " +
            std::to_string(response.size()) +
            "\r"
            "\r" +
            response;

        // Write response to request.fd.
        write(request.fd, http_response.c_str(), http_response.size());

        // Close the connection.
        close(request.fd);
    });

    server.run();

    /// Sleep forever.
    while (true) {
        sleep(1);
    }
}
