import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 5555))
data, addr = sock.recvfrom(20480)
try:
    data = data.decode()
    print data
    print addr
    sock.sendto(data.encode(), addr)
except:
    print 'error!'
