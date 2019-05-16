# encoding: utf8

from socket import AF_INET
from socket import SOCK_DGRAM
from socket import SO_BROADCAST
from socket import SOL_SOCKET
from socket import socket


s = socket(AF_INET, SOCK_DGRAM)
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
s.sendto('broadcast msg'.encode('utf-8'), ('255.255.255.255', 12345))