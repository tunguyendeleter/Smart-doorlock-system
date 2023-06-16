# Importing Libraries
import serial
import time
arduino = serial.Serial(port='COM3', baudrate=9600)
arduino.timeout = 1
while True:
    key = arduino.readline().decode('ascii')
    if key == "face":
        out = "1"
        arduino.write(out.encode())
        time.sleep(1)
arduino.close()

