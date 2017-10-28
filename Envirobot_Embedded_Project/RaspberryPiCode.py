import socket
import serial
import sys
from time import sleep

# Client
HOST = '192.168.1.148'
PORT = 5005

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.setblocking(True)

ser = serial.Serial('/dev/ttyACM0', 57600, timeout = 0.1)

while not ser: # Wait to connect to Arduino, 
    sleep(0.01)
    s.send("waiting")

sleep(3)

while True:
    sleep(.02)
    # Read in keyboard data from PC
    data = str(s.recv(1024).decode())
    
    # Send keyboard data to Arduino
    ser.write(data + "\r")

    # Read in sensor data from Arduino
    ret = str(ser.readline())

    # Send sensor data to PC
    s.send(ret)
    if data == "Q":
        break
    
s.close()
ser.close()
sys.exit()