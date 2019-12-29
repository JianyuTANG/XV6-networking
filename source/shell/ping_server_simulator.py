# python3
# -*- coding:utf-8 -*-


import random 
from socket import * 
serverSocket = socket(AF_INET, SOCK_DGRAM)
#建立udp协议的socket连接 
serverSocket.bind(('', 12000)) 
print("start listening")
while True: 
    message, address = serverSocket.recvfrom(1024)#接收客户端发送的信息，应该传送ip地址比较好 
    #print(message,address)
    print("From "+address[0]+": "+message.decode())
    serverSocket.sendto(message, address)