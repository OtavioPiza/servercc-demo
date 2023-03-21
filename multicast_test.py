import socket

group = "244.1.1.1"
port = 8080
ttl = 2

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
s.setsockopt(socket.SOL_SOCKET, 25, b'ztyxaydhop')
s.sendto(b"Hello", (group, port))
