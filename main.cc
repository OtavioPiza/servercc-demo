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

/// The main function.
int main(int argc, char *argv[]) {
    // Get the interface name, ip, group, and port from the command line.
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <interface> <ip> <group> <port> [echo|report_temp]..."
             << endl;
        return 0;
    }

    /// The abilities of the server.
    set<string> abilities = {"announce_services"};
    for (int i = 5; i < argc; i++) {
        // Check if the ability is valid.
        if (string(argv[i]) != "echo" && string(argv[i]) != "report_temp") {
            cout << "Invalid ability: " << argv[i] << endl;
            continue;
        }

        // Add the ability.
        abilities.insert(argv[i]);
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

    /// A map of peers to the services they provide.
    map<string, set<string>> peers;

    /// A map of services to the peers that provide them.
    map<string, set<string>> services;

    /// Callback function for when a peer connects.
    function<void(const string)> on_peer_connect = [&](const string ip) {
        peers.insert({ip, set<string>()});
    };

    /// Callback function for when a peer disconnects.
    function<void(const string)> on_peer_disconnect = [&](const string ip) {
        for (const auto &service : peers[ip]) {
            services[service].erase(ip);
        }
        peers.erase(ip);
    };

    // Setup default handlers.

    /// Default request handler.
    function<void(const Request)> default_request_handler = [](const Request request) {
        // Send HTTP OK response.
        const string http_ok = "HTTP/1.1 200 OK\r\n\r\n";
        write(request.fd, http_ok.c_str(), http_ok.size());
        close(request.fd);
    };

    /// Create the distributed server.
    DistributedServer server(interface, interface_ip, group, port, default_request_handler,
                             on_peer_connect, on_peer_disconnect);

    // Add custom handlers.

    /// Handler for the announce_services request.
    server.add_handler("announce_services", [&](const Request request) {
        // Get the ip address of the peer from request.addr.
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

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

    /// Handler for the echo request.
    server.add_handler("echo", [&](const Request request) {
        // Get the ip address of the peer from request.addr.
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

        // Log the request.
        server.log(Status::OK, "Received echo request: '" + request.data + "'");
        write(request.fd, request.data.c_str(), request.data.size());

        // Log the response.
        server.log(Status::OK, "Sent echo response to: '" + string(ip, INET_ADDRSTRLEN) + "'");

        // Close the connection.
        close(request.fd);
    });

    /// Handler for the report_temp request.
    server.add_handler("report_temp", [&](const Request request) {
        // Get the ip address of the peer from request.addr.
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

        // Log the request.
        server.log(Status::OK, "Received report_temp request: '" + request.data + "'");

        // Send the temperature by running the sensors command.
        FILE *fp = popen("sensors | grep 'Package id 0:' | awk '{print $4}'", "r");
        if (fp == nullptr) {
            server.log(Status::ERROR, "Could not run sensors command");
            close(request.fd);
            return;
        }

        // Read the output into the request.fd.
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            write(request.fd, buffer, strlen(buffer));
        }

        // Close the pipe.
        pclose(fp);

        // Close the connection.
        close(request.fd);
    });

    // Start the server.
    server.run();

    sleep(1);

    // Start the shell.
    cout << letterhead << endl;
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

        // Handle the services command.
        else if (line == "services") {
            cout << "== Services Currently Available ==" << endl;
            for (const auto &service : services) {
                cout << "- \t" << service.first << " (" << service.second.size() << ")" << endl;
            }
            cout << "==================================" << endl;
        }

        // Handle the clear command.
        else if (line == "clear") {
            system("clear");
            cout << letterhead << endl;
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
                string services_string = "";
                while (true) {
                    StatusOr<string> response = server.receive_message(id.result);
                    if (!response.ok()) {
                        break;
                    }
                    services_string += response.result;
                }

                // Split the services by space and add to the services map.
                int i = 0, j = 0;
                for (; j < services_string.size(); j++) {
                    if (services_string[j] == ' ') {
                        // Extract the service.
                        string service = services_string.substr(i, j - i);
                        i = j + 1;

                        // Add the peer to the service.
                        peers[peer.first].insert(service);

                        // Create the service if it does not exist.
                        if (services.find(service) == services.end()) {
                            services[service] = set<string>();
                        }

                        // Add the peer to the service.
                        services[service].insert(peer.first);
                    }
                }
                // Print the services from the peer.
                cout << peer.first << ": " << services_string << endl;
            }
            cout << "=================================" << endl;
        }

        // Handle echo command.
        else if (line.starts_with("echo ")) {
            // Get the message.
            string message = line.substr(5);

            // Send the echo request to peers who support the service.
            auto peers_it = services.find("echo");
            if (peers_it == services.end() || peers_it->second.empty()) {
                server.log(Status::ERROR, "No peers support the echo service");
                continue;
            }

            for (const auto &peer : peers_it->second) {
                auto id = server.send_message(peer, "echo " + message);

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

        // Handle the report_temp command.
        else if (line == "report_temp") {
            // Check if we have any peers that support the service.
            auto peers_it = services.find("report_temp");
            if (peers_it == services.end() || peers_it->second.empty()) {
                server.log(Status::ERROR, "No peers support the report_temp service");
                continue;
            }

            // Send the report_temp request to peers who support the service.
            cout << "== Temperature Reported by Peers ==" << endl;
            for (const auto &peer : peers_it->second) {
                auto id = server.send_message(peer, "report_temp");

                // If we could not send the message, continue.
                if (!id.ok()) {
                    server.log(id.status, std::move(id.status_message));
                    continue;
                }

                // Read from response.
                string temp = "";
                while (true) {
                    StatusOr<string> response = server.receive_message(id.result);
                    if (!response.ok()) {
                        break;
                    }
                    temp += response.result;
                }

                // Print the temperature from the peer.
                cout << peer << ": " << temp << endl;
            }
            cout << "==================================" << endl;
        }
    }
}
