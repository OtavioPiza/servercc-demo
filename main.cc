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
    set<string> peers;

    /// Callback function for when a peer connects.
    function<void(const string)> on_peer_connect = [&](const string ip) { peers.insert(ip); };

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
        server.log(Status::OK, "Received echo request: " + request.data);
        write(request.fd, request.data.c_str(), request.data.size());
        close(request.fd);
    });

    server.run();

    sleep(2);

    // Start the shell.
    cout << letterhead << endl;

    /// Sleep forever.
    string line = "";
    do {
        // Process the input.
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

        }
        // Handle echo command.
        else if (line.starts_with("echo ")) {
            // Get the message.
            string message = line.substr(5);

            // Send the echo request to all peers.
            for (const auto &peer : peers) {
                auto id = server.send_message(peer, "echo " + message);
                while (id.ok()) {
                    StatusOr<string> response = server.receive_message(id.result);

                    // If we get a response, print it.
                    if (response.ok()) {
                        server.log(Status::OK, "Received echo response: " + response.result);

                    } else {
                        break;
                    }
                }
            }
        } else if (line.starts_with("mcast ")) {
            string message = line.substr(6);
            server.log(Status::OK, "Sending multicast request: " + message);
            server.multicast_message(message);
        }

        // Print the prompt.
        cout << "sever-demo-shell: ";
    } while (getline(cin, line));
}
