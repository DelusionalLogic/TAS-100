import serial
import time

ser = serial.Serial(
        port = "/dev/ttyACM0",
        baudrate = 9600,
        bytesize = serial.EIGHTBITS
        )

time.sleep(2)
ser.write("\x36\x20\x00")
print(hex(ord(ser.read(1))))
print(hex(ord(ser.read(1))))
print(hex(ord(ser.read(1))))

while(True):
    print(hex(ord(ser.read(1))))
    print(hex(ord(ser.read(1))))
    print(hex(ord(ser.read(1))))
    print(hex(ord(ser.read(1))))
    print(hex(ord(ser.read(1))))
