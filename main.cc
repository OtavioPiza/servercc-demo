#include <arpa/inet.h>
#include <bits/stdc++.h>

#include <functional>

#include "servercc.h"

using ostp::servercc::distributed::DistributedServer;
using namespace std;

/// The letterhead for the shell.
const string letterhead = R"(
░██████╗███████╗██████╗░██╗░░░██╗███████╗██████╗░░█████╗░░█████╗░
██╔════╝██╔════╝██╔══██╗██║░░░██║██╔════╝██╔══██╗██╔══██╗██╔══██╗
╚█████╗░█████╗░░██████╔╝╚██╗░██╔╝█████╗░░██████╔╝██║░░╚═╝██║░░╚═╝
░╚═══██╗██╔══╝░░██╔══██╗░╚████╔╝░██╔══╝░░██╔══██╗██║░░██╗██║░░██╗
██████╔╝███████╗██║░░██║░░╚██╔╝░░███████╗██║░░██║╚█████╔╝╚█████╔╝
╚═════╝░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░╚═╝░╚════╝░░╚════╝░
)";

/// The shell prompt.
const string prompt = "servercc> ";

/// An HTTP 200 OK response.
const string http_ok = "HTTP/1.1 200 OK\r\n\r\n";

/// The main function.
int main(int argc, char *argv[]) {
    // Get the interface name, ip, group, and port from the command line.
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <interface> <ip> <group> <port>" << endl;
        return 0;
    }

    // Abilities of the server.
    const set<string> abilities = {"echo", "announce_services", "report_temp"};

    /// The interface name.
    const string interface = argv[1];

    /// The ip address of the interface.
    const string interface_ip = argv[2];

    /// The multicast group.
    const string group = argv[3];

    /// The port to listen on.
    const int port = stoi(argv[4]);

    // Setup the callback functions.

    /// Set of peers currently connected.
    unordered_map<string, vector<string>> peers;

    /// Callback function for when a peer connects.
    function<void(const string)> on_peer_connect = [&](const string ip) {
        peers.insert({ip, vector<string>()});
    };

    /// Callback function for when a peer disconnects.
    function<void(const string)> on_peer_disconnect = [&](const string ip) { peers.erase(ip); };

    // Setup default handlers.

    /// Default request handler.
    function<void(const Request)> default_request_handler = [](const Request request) {
        // Send HTTP OK response.
        write(request.fd, http_ok.c_str(), http_ok.size());
        close(request.fd);
    };

    /// Create the distributed server.
    DistributedServer server(interface, interface_ip, group, port, default_request_handler,
                             on_peer_connect, on_peer_disconnect);

    // Add custom handlers.

    /// Handler for the echo request.
    server.add_handler("echo", [&](const Request request) {
        // Get the ip address of the peer from request.addr.
        shared_ptr<struct sockaddr_in> addr =
            std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

        // Log the request.
        server.log(Status::OK, "Received echo request: '" + request.data + "'");
        write(request.fd, request.data.c_str(), request.data.size());

        // Log the response.
        server.log(Status::OK, "Sent echo response to: '" + string(ip, INET_ADDRSTRLEN) + "'");

        // Close the connection.
        close(request.fd);
    });

    /// Handler for the announce_services request.
    server.add_handler("announce_services", [&](const Request request) {
        // Get the ip address of the peer from request.addr.
        shared_ptr<struct sockaddr_in> addr =
            std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

        // Log the request.
        server.log(Status::OK, "Received announce_services request: '" + request.data + "'");

        // Go through each service and send a response separated by a space.
        for (const auto &service : abilities) {
            write(request.fd, service.c_str(), service.size());
            write(request.fd, " ", 1);
        }

        // Close the connection.
        close(request.fd);
    });

    server.run();

    sleep(1);

    // Start the shell.
    cout << letterhead << endl;

    /// Sleep forever.
    string line = "";
    while (cout << "sever-demo-shell: " && getline(cin, line)) {
        // Process the input.
        if (line == "peers") {
            cout << "== Peers Currently Connected ==" << endl;
            for (const auto &peer : peers) {
                cout << "- \t" << peer.first << endl;
            }
            cout << "================================" << endl;
        }

        // Handle the clear command.
        else if (line == "clear") {
            system("clear");
        }

        // Handle echo command.
        else if (line.starts_with("echo ")) {
            // Get the message.
            string message = line.substr(5);

            // Send the echo request to all peers.
            for (const auto &peer : peers) {
                auto id = server.send_message(peer.first, "echo " + message);

                // If we could not send the message, continue.
                if (!id.ok()) {
                    server.log(id.status, std::move(id.status_message));
                    continue;
                }

                // Read all available responses.
                while (true) {
                    StatusOr<string> response = server.receive_message(id.result);

                    // If we get a response, print it.
                    if (response.ok()) {
                        server.log(Status::OK, "Received echo response: " + response.result);
                    } else {
                        break;
                    }
                }
            }
        }

        // Handle the announce_services command.
        else if (line == "announce_services") {
            cout << "== Services Announced by Peers ==" << endl;
            for (const auto &peer : peers) {
                auto id = server.send_message(peer.first, "announce_services");

                // If we could not send the message, continue.
                if (!id.ok()) {
                    server.log(id.status, std::move(id.status_message));
                    continue;
                }

                // Read from response.
                string services = "";
                while (true) {
                    StatusOr<string> response = server.receive_message(id.result);
                    if (!response.ok()) {
                        break;
                    }
                    services += response.result;
                }
                
                // Print the services from the peer.
                cout << peer.first << ": " << services << endl;
            }
            cout << "=================================" << endl;
        }
    }
}
