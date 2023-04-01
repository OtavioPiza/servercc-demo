#include <bits/stdc++.h>

#include <functional>
#include <arpa/inet.h>

#include "servercc.h"

using ostp::servercc::distributed::DistributedServer;

/// Echoes the tcp request.

/// The main function.
int main(int argc, char *argv[]) {
    /// Interface for multicast.
    const std::string interface = "ztyxaydhop";
    std::string interface_ip = argv[1];

    /// Group for multicast.
    const std::string group = "224.1.1.1";

    /// Port for multicast server UDP and for the TCP server.
    const int port = 8000;

    /// Create the distributed server.
    DistributedServer server(interface, interface_ip, group, port);
    server.run();

    /// Sleep forever.
    while (true) {
        sleep(1);
    }
}
