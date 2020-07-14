# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

import mysql.connector											#Libreria para conectar con base de datos
import serial													#Libreria para conectarse con ESP32 por serial
import time														#Libreria de tiempo para RPi

arduino = serial.Serial('/dev/ttyUSB0', 115200)				#El ESP32 esta conectado en el puerto USB1 y configurado a 115200 baudios
db = mysql.connector.connect(									#Datos para ingresar a la base de datos Roller
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"
)
cursor=db.cursor()												#Para navegar por las tablas desde python
retries = 5
ciclo = 1

arduino.flush()
arduino.write(b"S1*")
time.sleep(1)
arduino.write(b"S0*")
time.sleep(1)

while True:
    line = arduino.readline()
    #boton = int(input("¿Boton presionado? (y/n)"))
    if line == "b":
    #if boton == "n":
        for i in range(0, 3):
            aruco = int(input("Aruco: "))
            cursor.execute("insert into borrador1 (aruco, ciclo) values("+str(aruco)+","+str(ciclo)+")")
            db.commit()
        ciclo += 1
    elif line == "a":
    #elif boton == "y":
        for i in range(0, 3):
            aruco = int(input("Aruco: "))
            cursor.execute("insert into borrador2 (aruco, ciclo) values("+str(aruco)+","+str(ciclo)+")")
            db.commit()