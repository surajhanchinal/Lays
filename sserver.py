import socket,os
from _thread import *
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
sock.bind(('127.0.0.1', 12345))  
sock.listen(5)  
connection,address = sock.accept()  
while True:  
    buf = connection.recv(1024)  
    print(buf)
    connection.send("wtf is this".encode())    		    
connection.close()
