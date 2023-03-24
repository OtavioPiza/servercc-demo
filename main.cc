#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::libcc::utils::Status;
using ostp::servercc::client::Client;
using ostp::servercc::client::TcpClient;
using ostp::servercc::client::UdpClient;
using ostp::servercc::connector::Connector;
using ostp::servercc::connector::ConnectorRequest;
using ostp::servercc::server::Request;
using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using ostp::servercc::server::TcpServer;
using ostp::servercc::server::UdpServer;

/// Echoes the tcp request.

/// The main function.
int main(int argc, char *argv[]) {
    /// Interface for multicast.
    const std::string interface = "ztyxaydhop";
    std::string interface_ip = argv[1];

    /// Group for multicast.
    const std::string group = "224.1.1.1";

    /// Port for multicast server UDP and for the TCP server.
    const int port = 7000;

    Connector conn(
        [](const ConnectorRequest request) {
            /// Print the request.
            std::cout << "Connector request: " << request.request << std::endl;
        },
        [](int i) { return; });

    /// Handler for a multicast request.
    std::function<void(const Request)> multicast_request_handler = [](const Request request) {
        /// Find the ip address of the client that sent the request by looking
        /// after the first space in the request.
        int space_index = request.data.find(' ');
        std::string ip_address = request.data.substr(space_index + 1);

        /// Print the request.
        std::cout << "Multicast request from " << ip_address << std::endl;
        std::cout << "Request: " << request.data << std::endl;
    };

    /// Creates a Connector for the TCP server.
    std::function<void(const Request)> tcp_connector_creator = [&](const Request request) {
        /// Find the ip address of the client that sent the request by looking
        /// after the first space in the request.
        int space_index = request.data.find(' ');
        std::string ip_address = request.data.substr(space_index + 1);

        /// Create a new TCP client.
        TcpClient client(ip_address, port);

        /// Create a new connector for the client.
        conn.add_client(TcpClient(request.client_fd.value()));
    };

    /// Create a UDP server for multicast.
    UdpServer m_server(port, group, {interface_ip});
    m_server.set_default_processor(multicast_request_handler);

    /// Create a TCP server.
    TcpServer t_server(port);
    t_server.set_default_processor(tcp_connector_creator);

    /// Run the servers in threads.
    std::thread m_thread(&UdpServer::run, &m_server);
    std::thread t_thread(&TcpServer::run, &t_server);

    /// Wait for 1 second.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /// Create a UDP client for multicast.
    UdpClient m_client(interface, "127.0.0.1", port, 3, group);
    m_client.open_socket();
    m_client.send_message("Hello " + interface_ip);

    /// Read from stdin and broadcast the message.
    std::string message;
    while (std::getline(std::cin, message)) {
        m_client.send_message(message);
    }


    /// Wait for the servers to finish.
    m_thread.join();
    t_thread.join();
}
