#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::libcc::utils::Status;
using ostp::servercc::Request;
using ostp::servercc::client::Client;
using ostp::servercc::client::MulticastClient;
using ostp::servercc::client::TcpClient;
using ostp::servercc::connector::Connector;
using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using ostp::servercc::server::TcpServer;
using ostp::servercc::server::UdpServer;

/// Echoes the tcp request.

/// The main function.
int main(int argc, char *argv[]) {
    /// Interface for multicast.
    const std::string interface = "ztyxaydhop";
    std::string interface_ip = "172.24.100.137";

    /// Group for multicast.
    const std::string group = "224.1.1.1";

    /// Port for multicast server UDP and for the TCP server.
    const int port = 8080;

    Connector conn(
        [](const Request request) {
            /// Print the request.
            std::cout << "Connector request: " << request.data << std::endl;
        },
        [](int i) { return; });

    /// Handler for a multicast request.
    std::function<void(const Request)> multicast_request_handler = [&](const Request request) {
        // If the request is from self then ignore it.

        /// Find the ip address of the client that sent the request by looking
        /// after the first space in the request.
        int space_index = request.data.find(' ');
        std::string port_string = request.data.substr(space_index + 1);

        /// Get the ip from the multicast request.
        struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(request.addr.get());
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

        // If the ip address is the same as the interface ip then ignore the
        // request.
        std::cout << "Multicast request from '" << ip << "'" << std::endl;
        std::cout << "Request: " << request.data << std::endl;
        if (ip == interface_ip) {
            std::cout << "Ignoring request from self." << std::endl;
            return;
        }

        /// Create a new connector for the client.
        int fd = conn.add_client(TcpClient(ip, std::stoi(port_string)));
    };

    /// Creates a Connector for the TCP server.
    std::function<void(const Request)> tcp_connector_creator = [&](const Request request) {
        /// Find the ip address of the client that sent the request by looking
        /// after the first space in the request.
        struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(request.addr.get());
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
        int port = ntohs(addr->sin_port);

        std::cout << "TCP request from '" << ip << "'" << std::endl;
        std::cout << "Request: " << request.data << std::endl;

        /// Create a new connector for the client.
        int f = conn.add_client(TcpClient(request.fd, ip, port));
        std::cout << "Added client with fd: " << f << std::endl;
        conn.send_message(f, "Connected to server.");
    };

    /// Create a handler that echoes the request.
    std::function<void(const Request)> echo_request_handler = [&](const Request request) {
        /// Print the request.
        std::cout << "Request: " << request.data << std::endl;

        close(request.fd);
    };

    /// Create a UDP server for multicast.
    UdpServer m_server(port, group, {interface_ip});
    m_server.set_default_processor(multicast_request_handler);

    /// Create a TCP server.
    TcpServer t_server(port);
    t_server.set_default_processor(echo_request_handler);
    t_server.set_processor("connect", tcp_connector_creator);

    /// Run the servers in threads.
    std::thread m_thread(&UdpServer::run, &m_server);
    std::thread t_thread(&TcpServer::run, &t_server);

    /// Wait for 1 second.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /// Create a UDP client for multicast.
    // MulticastClient m_client(interface, group, port);
    // m_client.open_socket();
    // m_client.send_message("connect " + std::to_string(port));

    /// Read from stdin and broadcast the message.
    // std::string message;
    // while (std::getline(std::cin, message)) {
    //     m_client.send_message(message);
    // }

    TcpClient t_client("localhost", port);
    t_client.open_socket();
    int cfd = conn.add_client(t_client);
    std::cout << "Added client with fd cfd: " << cfd << std::endl;
    conn.send_message(cfd, "connect " + std::to_string(port));
    conn.send_message(cfd, "Hello world!");

    /// Wait for the servers to finish.
    m_thread.join();
    t_thread.join();
}
