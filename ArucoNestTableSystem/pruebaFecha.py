# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

import mysql.connector											#Libreria para conectar con base de datos

db = mysql.connector.connect(									#Datos para ingresar a la base de datos Roller
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"
)
cursor=db.cursor()												#Para navegar por las tablas desde python
retries = 5

cursor.execute("SELECT * FROM NestTable WHERE aruco = "+str(166))						#se selecciona el ft del aruco del arreglo presenteX
datos=cursor.fetchone()																		#se guarda en una variable local llamada ft
db.commit()

print("los datos extraidos son: ", datos)
print("fecha", datos[4])