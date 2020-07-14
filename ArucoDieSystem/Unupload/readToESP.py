import serial 

arduino = serial.Serial('/dev/ttyUSB1', 115200) 

stringFromArduino = arduino.readLine()
print(stringFromArduino)

arduino.close()