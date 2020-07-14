import serial 
import time

arduino = serial.Serial('/dev/ttyUSB1', 115200) 

arduino.write(b'A')
time.sleep(1)
arduino.write(b'B')
time.sleep(1)
arduino.write(b'C')

arduino.close()