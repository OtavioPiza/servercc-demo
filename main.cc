#include <arpa/inet.h>
#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::servercc::distributed::DistributedServer;
using namespace std;

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

    set<string> peers;

    /// Create the distributed server.
    DistributedServer server(
        interface, interface_ip, group, port,
        [](const Request request) {
            std::cout << "Received request: " << request.data << std::endl;
        },
        [&](const std::string ip) { peers.insert(ip); },
        [&](const std::string ip) { peers.erase(ip); });

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

        // For every peer, send a message with an echo request.
        for (const auto &peer : peers) {
            server.log(Status::OK, "Sending echo request to peer: " + peer);
            server.send_message(peer, "echo hi");
        }

        // Close the connection.
        close(request.fd);
    });

    server.add_handler("echo", [&](const Request request) {
        server.log(Status::OK, "Received echo request: " + request.data);
        close(request.fd);
    });

    server.run();

    sleep(2);

    /// Start the shell.
    cout << "\n\n"
            "░██████╗███████╗██████╗░██╗░░░██╗███████╗██████╗░░█████╗░░█████╗░\n"
            "██╔════╝██╔════╝██╔══██╗██║░░░██║██╔════╝██╔══██╗██╔══██╗██╔══██╗\n"
            "╚█████╗░█████╗░░██████╔╝╚██╗░██╔╝█████╗░░██████╔╝██║░░╚═╝██║░░╚═╝\n"
            "░╚═══██╗██╔══╝░░██╔══██╗░╚████╔╝░██╔══╝░░██╔══██╗██║░░██╗██║░░██╗\n"
            "██████╔╝███████╗██║░░██║░░╚██╔╝░░███████╗██║░░██║╚█████╔╝╚█████╔╝\n"
            "╚═════╝░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░╚═╝░╚════╝░░╚════╝░\n"
            "\n\n";

    /// Sleep forever.
    string line = "";
    do {
        if (line == "peers") {
            cout << "== Peers Currently Connected ==" << endl;

            for (const auto &peer : peers) {
                cout << "- \t" << peer << endl;
            }
            if (peers.empty()) {
                cout << "No peers connected." << endl;
            }

            cout << "================================" << endl;
        } else if (line == "exit") {
            break;
        } else if (line == "clear") {
            system("clear");
        } else if (line.starts_with("echo ")) {
            string message = line.substr(5);
            for (const auto &peer : peers) {
                server.log(Status::OK, "Sending echo request to peer: " + peer);
                server.send_message(peer, "echo " + message);
            }
        } else if (line.starts_with("mcast ")) {
            string message = line.substr(6);
            server.log(Status::OK, "Sending multicast request: " + message);
            server.multicast_message(message);
        }

        cout << "sever-demo-shell: ";
    } while (getline(cin, line));
}
