# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

import mysql.connector											#Libreria para conectar con base de datos
import numpy as np
import cv2, PIL
from cv2 import aruco											#Libreria para reconocimiento de arucos
import cv2
from imutils.video import VideoStream
import time														#Libreria de tiempo para RPi
from picamera.array import PiRGBArray
from picamera import PiCamera									#Libreria oficial de camara para RPi

db = mysql.connector.connect(									#Datos para ingresar a la base de datos Roller
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"ruptura
)
cursor=db.cursor()												#Para navegar por las tablas desde python
retries = 5
cap = PiCamera()
cap.resolution = (640, 480)
cap.framerate = 32
rawCapture = PiRGBArray(cap, size=(640, 480))
time.sleep(0.1)
stab = 0

while(True):																	#Inicia el ciclo "infinito"
    
        for frame in cap.capture_continuous(rawCapture, format="bgr", use_video_port=True):
			print("Espera...")
			time.sleep(0.1)
			videoframe = frame.array
			gray = cv2.cvtColor(videoframe, cv2.COLOR_BGR2GRAY)
			aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
			parameters =  aruco.DetectorParameters_create()
			corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
			frame_markers = aruco.drawDetectedMarkers(videoframe.copy(), corners, ids)
			cv2.imshow("frame_markers", frame_markers)
			rawCapture.truncate(0)

			if ids is not None:	
				y = len(ids)													#la variable y toma el valor de la longitud de ids
				x = ids															#x es un array con los valores de ids
				x.astype(int)													#el array x se convierte en un array de enteros, sigue con el mismo nombre
				prtx = sorted(x)												#el array presenteX toma el valor del array x pero ordenado de menor a mayor

				for w in range (0,y):											#ciclo for de 0 hasta la cantidad de arucos presentes, detectados en ids
					n = int(x[w])												#n toma el valor de uno de los elementos del arreglo x, el elemento con la posicion del ciclo actual del for
					if n == 166:
						for i in range(retries):
							try:
								cursor.execute("SELECT * FROM NestTable WHERE aruco = "+str(166))						#se selecciona el ft del aruco del arreglo presenteX
								mesa=cursor.fetchone()																		#se guarda en una variable local llamada ft
								db.commit()
							except:
								time.sleep(1)
								print("No pudo consultar en NestTable")
								i+=1
								continue
							break
						for i in range(retries):
							try:
								cursor.execute("UPDATE NestTable SET status = "+str(1)+" WHERE aruco = "+str(166))
								db.commit()
							except:
								time.sleep(1)
								print("No pudo actualizar en NestTable")
								i+=1
								continue
							break
						
					else:
						for i in range(retries):
							try:
								cursor.execute("UPDATE NestTable SET status = "+str(0)+" WHERE aruco = "+str(166))
								db.commit()
								print("La tabla esta fuera de la mesa")

							except:
								time.sleep(1)
								print("No pudo actualizar en NestTable")
								i+=1
								continue
							break
					
					cursor.execute("select status from NestTable where aruco = 166")
					sta = cursor.fetchone()
					db.commit()
					
					if sta[0] == stab:
						print("Es el mismo status")
					else:
						for i in range(retries):
							try:
								cursor.execute("select ciclos from NestTable where aruco = 166")
								c = cursor.fetchone()
								db.commit()
								c2 = c[0] + 1
								cursor.execute("update NestTable set ciclos ="+str(c2))
								db.commit()
								cursor.execute("select ciclos from NestTable where aruco = 166")
								c = cursor.fetchone()
								db.commit()
							except:
								time.sleep(1)
								print("No se pudo conectar con la base de datos")
								i += 1
								continue
							break

						if c[0] % 2 == 0:
							for i in range(retries):
								try:
									cursor.execute("select realCiclos from NestTable where aruco = 166")
									rc = cursor.fetchone()
									db.commit()
									rc2 = rc[0] + 1
									cursor.execute("update NestTable set realCiclos ="+str(rc2))
									db.commit()
								except:
									time.sleep(1)
									print("No se pudo conectar con la base de datos")
									i += 1
									continue
								break
						else:
							print("es inpar")
					
					stab = sta[0]
				
			if cv2.waitKey(1) & 0xFF == ord('q'):
				continue

help(aruco.DetectorParameters_create)