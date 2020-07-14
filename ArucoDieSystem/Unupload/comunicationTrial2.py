import serial
import time

arduino = serial.Serial('/dev/ttyUSB0', 115200)
arduino.flush()

arduino.write(b"S1*")
time.sleep(2)
arduino.write(b"S0*")

while True:
    time.sleep(0.5)
    arduino.write(b"E*")
    print("Enviado")
    line = arduino.readline().decode('utf-8').rstrip()
    print(line)
    
    if line == "a":
	    print("Boton presionado")
        time.sleep(2)
        arduino.write('B')
	elif line == "b":
	    print("No presionado")
	elif line == "c":
	    print("Error, el de arriba es mayor")
    else:
	print("No se reconoce la recepcion de line")


# a - presionaron
#b - no presionaron
#c - Error