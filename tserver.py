import socket
from _thread import *
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server = '192.168.0.105'
port = 5555

server_ip = socket.gethostbyname(server)

try:
    s.bind((server, port))

except socket.error as e:
    print(str(e))

s.listen(2)
print("Waiting for a connection")

player1 = "p1"
isplayer1 = False
player1data = "hello from 1"
player2 = "p2"
isplayer2 = False
player2data = "randaap"
def threaded_client(conn,addr):
    global player1,player2,isplayer1,isplayer2,player1data,player2data
    myname = "0"
    while True:
        try:
            #print(addr)
            data = conn.recv(2048)
            reply = data.decode('utf-8')
            if not data:
                conn.send(str.encode("Goodbye"))
                if(myname == player1):
                    isplayer1 = False
                if(myname == player2):
                    isplayer2 = False
                break
            else:
                arr = reply.split(":")
                if(myname == "0"):
                    if(arr[0] == player1):
                        myname = player1
                        isplayer1 = True
                    if(arr[0] == player2):
                        isplayer2 = True
                        myname = player2
                if(myname == player1 and not isplayer2):
                    conn.sendall("Waiting for Player 1".encode())
                if(myname == player2 and not isplayer1):
                    conn.sendall("Waiting for Player 2".encode())
                if(isplayer1 and isplayer2):
                    if(myname == player1):
                        conn.sendall(player2data.encode())
                        player1data = arr[1]
                    if(myname == player2):
                        conn.sendall(player1data.encode())
                        player2data = arr[1]
                        #print(player2data)
        except:
            if(myname == player1):
                isplayer1 = False
            if(myname == player2):
                isplayer2 = False
            break
    print("Connection Closed")
    conn.close()

while True:
    conn, addr = s.accept()
    print("Connected to: ", addr)

    start_new_thread(threaded_client, (conn,addr))
