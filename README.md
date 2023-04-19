# SERVERCC Demo

This repository provides a demo for the [SERVERCC](https://github.com/OtavioPiza/servercc) library.

## Prerequisites

This project depends on support for C++20 and GCC 11.0.0 or newer. It also requires CMake 3.18 or newer to build.

## Building

To build the project, run the following commands:

```bash
cmake -S <path-to-repo> -B <path-to-build-dir>
cmake --build <path-to-build-dir>
```

## Running

To run the project, run the following commands:

```bash
./<path-to-build-dir>/servercc-demo <interface> <interface ip> <multicast group> <port> <abilities>...
```

The abilities fields allows you to specify which services the server will provide allowing you to create asymmetric servers. The available abilities are:

- `echo`: Echo service
- `report_mem`: Memory report service
- `report_temp`: Temperature report service (requires `lm-sensors` to be installed)
- `sort`: Sort service

The interface and interface ip fields are used to specify the interface and ip address the server will bind to. The multicast group and port fields are used to specify the multicast group and port the server will listen to. Servers can only communicate with other servers that are listening to the same multicast group and port.

## Usage

After initializing the server, you will be provided with a shell-like interface. The following commands are available:

- `help`: Prints the help message for the shell.
- `peers`: Prints the list of peers connected to the server.
- `services`: Prints the list of services the server can access.
- `clear`: Clears the screen.
- `echo <message>`: Sends a message to all connected peers and prints the responses.
- `block <peer ip>`: Sends a request to the specified peer which will block (never send a response). This is useful for testing the disconnection detection.
- `report_mem`: Sends a request to all connected peers that support the `report_mem` service and prints the responses.
- `report_temp`: Sends a request to all connected peers that support the `report_temp` service and prints the responses.
- `sc`: Sends the connect UDP multicast.
