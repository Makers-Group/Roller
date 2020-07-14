# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

#############################LIBRERIAS##################################
import mysql.connector											#Libreria para conectar con base de datos
import serial													#Libreria para conectarse con ESP32 por serial
import numpy as np
import cv2, PIL
import RPi.GPIO as GPIO											#Libreria para entradas y salidas digitales
from cv2 import aruco											#Libreria para reconocimiento de arucos
import cv2
from imutils.video import VideoStream
# para raspberry
import time														#Libreria de tiempo para RPi
from picamera.array import PiRGBArray							
from picamera import PiCamera									#Libreria oficial de camara para RPi
#############################LIBRERIAS##################################

#arduino = serial.Serial('/dev/ttyUSB0', 115200)				#El ESP32 esta conectado en el puerto USB1 y configurado a 115200 baudios

db = mysql.connector.connect(									#Datos para ingresar a la base de datos Roller
	host="localhost",
	user="Rudy",
	passwd="password",
	database="Roller"
)

cursor=db.cursor()												#Para navegar por las tablas desde python
comilla = "'"													#Recurso para escritura de Query
GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.OUT)										#La salida 21 sera para indicarle al Pinblock si debe salir o no
GPIO.setup(20, GPIO.IN)											#La entrada 20 sera para saber si el bypass esta activo o no
maquina = "R4"													#Recurso para el numero de maquina
retries = 5
q = GPIO.input(20)												#Seudonimo de la entrada 20
cap = PiCamera()
cap.resolution = (640, 480)
cap.framerate = 32
rawCapture = PiRGBArray(cap, size=(640, 480))
time.sleep(0.1)

areaCuero = 45.00												#Area promedio del cuero en sqft
utlMeta = 55.00													#Utilizacion meta, (PROVISIONAL)
utl = 0.00														#Utilizacion inicial (RECURSO PARA CALCULOS)

for i in range(retries):
	try:
		cursor.execute("SELECT max(cycle) FROM corte")									#Query para tomar el valor mayor en la columna cycle de la tabla corte
		sup=cursor.fetchone()															#Lo que se obtenga, se guardara en un array llamado sup
		db.commit()																		#Asi se ejecutan los Querys
		ciclo = sup[0] + 1
	except:
		time.sleep(1)
		print("No pudo consultar el ciclo maximo de la tabla corte")
		i+=1
		continue
	break																		#Aumenta el ciclo en 1
cursor.execute("UPDATE gemelo SET gem = "+str(ciclo)+" WHERE id = 3")			#Le decimos a la segunda camara cual es el ciclo actual
db.commit()																		

prtx = []																		#PresenteX, arreglo de apoyo para actualizar los arucos vistos
pastx = []																		#PasadoX, arreglo de apoyo para actualizar los arucos vistos

while(True):																	#Inicia el ciclo "infinito"
    
        for frame in cap.capture_continuous(rawCapture, format="bgr", use_video_port=True):							
			print("Espera...")
			#time.sleep(0.1)
			videoframe = frame.array
			gray = cv2.cvtColor(videoframe, cv2.COLOR_BGR2GRAY)
			aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
			parameters =  aruco.DetectorParameters_create()
			corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
			frame_markers = aruco.drawDetectedMarkers(videoframe.copy(), corners, ids)
			cv2.imshow("frame_markers", frame_markers)
			rawCapture.truncate(0)
                
			if ids is not None:													#ids es un array de los arucos vistos, si tiene un valor hara esto:
				cursor.execute("UPDATE gemelo SET gem = 1 WHERE id = 1")		#Le indico a la segunda camara que la primera esta viendo ids
				db.commit()	
				y = len(ids)													#la variable y toma el valor de la longitud de ids
				x = ids															#x es un array con los valores de ids
				x.astype(int)													#el array x se convierte en un array de enteros, sigue con el mismo nombre
				prtx = sorted(x)												#el array presenteX toma el valor del array x pero ordenado de menor a mayor

				for w in range (0,y):																											#ciclo for de 0 hasta la cantidad de arucos presentes, detectados en ids
					n = int(x[w])																												#n toma el valor de uno de los elementos del arreglo x, el elemento con la posicion del ciclo actual del for
					for i in range(retries):
						try:
							cursor.execute("SELECT * FROM corte WHERE aruco = "+comilla+str(n)+comilla+" AND cycle ="+comilla+str(ciclo)+comilla)		#se toma la fila de la tabla corte en donde la columna aruco tenga el valor de n y la columna cycle tenga el valor de ciclo
							cursor.fetchone()
						except:
							#time.sleep(1)
							print("No realizo la consulta en corte")
							i+=1
							continue
						break																											
							
					if cursor.rowcount >= 1:									#Si esta fila ya habia sido seleccionada previamente, se imprime un espacio en blanco
						print("  ")
									
					else:
						cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(n))		#Si no se habia seleccionado previamente, se selecciona el valor de la columna ft de la tabla ArucoId (Tabla de Ignition)
						visto=cursor.fetchone()												#El valor de ft se guarda en el arreglo llamado visto
						db.commit()
									
						sql_insert = "INSERT INTO corte (aruco, ft, cycle) VALUES (%s, %s, %s)"		#Inserta en la tabla corte 3 valores en las columnas aruco, sqft y cycle
						cursor.execute(sql_insert, (n, visto[0], ciclo))								#En aruco sera el valor n, en sqft sera el primer elemento del arreglo visto y el valor de ciclo
						db.commit()
#########################################DEBUG############################################################
				if len(prtx) == len(pastx):
					print("Son del mismo tamaño")
					for i in range(0, len(prtx)):
						try:
							b = pastx.index(prtx[i])
							print("Mismo aruco: "+str(prtx[i][0])+" -- "+str(pastx[b][0]))
						except ValueError:
							print(str(prtx[i][0])+" No esta dentro del pasado") #Hay un dado nuevo
							cursor.execute("select * from corte where aruco = "+str(prtx[i][0])+" and cycle ="+str(ciclo))
							t = cursor.fetchone()
							db.commit()
							if cursor.rowcount >= 1:
								cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i][0]))
								ft = cursor.fetchone()
								db.commit()
								cursor.execute("update corte set ft = "+str(ft[0])+" where aruco ="+str(prtx[i][0])+" and cycle ="+str(ciclo))
								db.commit()
							else:
								cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i][0]))
								ft = cursor.fetchone()
								db.commit()
								cursor.execute("insert into corte (aruco, ft, cycle) values("+str(prtx[i][0])+","+str(ft[0])+","+str(ciclo)+")")
								db.commit()
						try:
							b = prtx.index(pastx[i])
							print("Mismo aruco: "+str(prtx[b])+" -- "+str(pastx[i]))
						except ValueError:
							print(str(pastx[i][0])+" No esta dentro del presente") #Quitaron un dado
							cursor.execute("update corte set ft = 0 where aruco ="+str(pastx[i][0])+" and cycle ="+str(ciclo))
							db.commit()

				elif len(prtx) > len(pastx):
					print("Hay mas dados")
					for i in range (0, len(prtx)):
						try:
							b = pastx.index(prtx[i])
							print("Mismo aruco: "+str(prtx[i][0])+" -- "+str(pastx[b][0]))
						except ValueError:
							print(str(prtx[i][0])+" No esta dentro del pasado")	#Hay un dado nuevo
							cursor.execute("select * from corte where aruco = "+str(prtx[i][0])+" and cycle ="+str(ciclo))
							t = cursor.fetchone()
							db.commit()
							if cursor.rowcount >= 1:
								cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i][0]))
								ft = cursor.fetchone()
								db.commit()
								cursor.execute("update corte set ft = "+str(ft[0])+" where aruco ="+str(prtx[i][0])+" and cycle ="+str(ciclo))
								db.commit()
							else:
								cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i][0]))
								ft = cursor.fetchone()
								db.commit()
								cursor.execute("insert into corte (aruco, ft, cycle) values("+str(prtx[i][0])+","+str(ft[0])+","+str(ciclo)+")")
								db.commit()

				elif len(prtx) < len(pastx):
					print("Hay menos dados")
					for i in range (0, len(pastx)):
						try:
							b = prtx.index(pastx[i])
							print("Mismo aruco: "+str(pastx[i][0])+" -- "+str(prtx[b][0]))
						except ValueError:
							print (str(pastx[i][0])+" No esta en el presente")	#Quitaron un dado
							cursor.execute("update corte set ft = 0 where aruco ="+str(pastx[i][0])+" and cycle ="+str(ciclo))	
							db.commit()					
#########################################DEBUG############################################################
			else:																		#si no encontro ningun aruco, ids = none y hara o siguiente
				for i in range(retries):
					try:
						cursor.execute("UPDATE gemelo SET gem = 0 WHERE id = 1")				#le avisa a la segunda camara que no esta viendo nada
						db.commit()
					except:
						time.sleep(1)
						print("No pudo actualizar en gemelo")
						i+=1
						continue
					break
				
				for i in range(retries):
					try:
						cursor.execute("SELECT gem FROM gemelo WHERE id = 1")
						cam1=cursor.fetchone()
						db.commit()
					except:
						time.sleep(1)
						print("No pudo consultar en gemelo")
						i+=1
						continue
					break
				
				for i in range(retries):
					try:
						cursor.execute("SELECT gem FROM gemelo WHERE id = 2")					#pregunta si la camara 2 esta viendo algo
						cam2=cursor.fetchone()
						db.commit()
					except:
						time.sleep(1)
						print("No pudo consultar en gemelo")
						i+=1
						continue
					break
				
				cam = cam1[0] + cam2[0]													#si el resultado de esta suma es 0, ninguna camara ve algun aruco, si es 1 alguna esta viendo arucos
				
				#time.sleep(2)
				if cam == 0 :																					#si el resultado de la suma es 0 
					pass

			print("Area total de los dados: "+str(piesaje[0]))			
			utl = piesaje[0] / areaCuero								#obtiene el valor de utilizacion dividiendo piesaje entre en el area promedio del cuero
			utl = utl * 100												#obtiene el porcentaje que representa
			print("Utilizacion del cuero: "+str(utl)+" %")
			pastx = prtx												
			time.sleep(0.1)
	
			if cv2.waitKey(1) & 0xFF == ord('q'):
				continue
						
help(aruco.DetectorParameters_create)
