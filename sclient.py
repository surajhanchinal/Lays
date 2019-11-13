import socket
import sys
import time
import struct
# Create a TCP/IP socket

# Connect the socket to the port where the server is listening
server_address = ('localhost', 12345)
print("connecting to {} port {}".format(server_address[0],server_address[1]))
t_end = time.time() + 1
count = 0
abcd = [i for i in range(8000000,8000000+16)]
print(len(abcd))
bt = struct.pack("%sf"%len(abcd),*abcd)
#bt = bytes(abcd)
print(bt)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(server_address)
while time.time()<t_end:
    count += 1        
    sock.sendall(bt)
    data = sock.recv(1024)
    print(data)
sock.close()
print(count)