import socket
import sys

if __name__ == '__main__':
    # Get interface, port, group and ttl from command line arguments. If not provided,
    # print usage and exit.
    if len(sys.argv) != 5:
        print("Usage: python3 multicast_test.py <interface> <port> <group> <ttl>")
        sys.exit(1)
       
    interface = sys.argv[1] 
    port = int(sys.argv[2])
    group = sys.argv[3]
    ttl = int(sys.argv[4])
    
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
    sock.setsockopt(socket.SOL_SOCKET, 25, interface.encode())
    
    # Read input from stdin and send it to the multicast group
    message: str
    while message := sys.stdin.readline():
        sock.sendto(message.encode(), (group, port))
    