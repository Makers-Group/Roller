#import mysql.connector 
import time
#import RPi.GPIO as GPIO
"""
db = mysql.connector.connect(									
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"
)
cursor = db.cursor()

GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.OUT)
GPIO.setup(20, GPIO.IN)

maquina = "R4"
q = GPIO.input(20)

if (q == True):
    print "Llave activa"                        
    cursor.execute("SELECT max(bypass) FROM Pinblock")
    acceso = cursor.fetchone() 
    db.commit()

    llave = acceso[0] + 1
    sql_insert = "INSERT INTO Pinblock (bypass, roller) VALUES (%s, %s)"		
	cursor.execute(sql_insert, (llave, maquina))								
	db.commit()

else:
    ValorUtl = input("Utilizacion: ")
    if int(ValorUtl) >= 50:
        GPIO.output(21, False)
    else: 
        GPIO.output(21, True)
"""

###################
#Comunicacion serial
import serial
import mysql.connector 

arduino = serial.Serial('/dev/ttyUSB0', 115200) 
arduino.flsuh()
maquina = "R4"
db = mysql.connector.connect(									
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"cd ..
)
cursor = db.cursor()

arduino.write(b'Q')
bit = arduino.readline()
if (bit[0] == "a"):
    cursor.execute("SELECT max(bypass) FROM Pinblock")
    acceso = cursor.fetchone()
    db.commit()
    acceso[0] = acceso[0] + 1
    sql_insert = "INSERT INTO Pinblock (bypass, roller) VALUES (%s, %s)"
	cursor.execute(sql_insert, (acceso[0], maquina))
	db.commit()

elif (bit[0] == "b"):
    ValorUtl = input("Utilizacion: ")
    if int(ValorUtl) >= 50:
        arduino.write(b'B')
    else: 
        arduino.write(b'A')

else:
    print "Error"