import serial
import time

ser = serial.Serial(
        port = "/dev/ttyACM0",
        baudrate = 9600,
        bytesize = serial.EIGHTBITS
        )

time.sleep(2)
while(True):
    print(hex(ord(ser.read(1))))
