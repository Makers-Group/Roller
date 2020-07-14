# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

import mysql.connector								#Libreria para conectar con la base de datos
import serial										#Libreria para comunicarse con ESP por medio del puerto serial
import numpy as np
import cv2, PIL
from cv2 import aruco
# para raspberry
import time
from picamera.array import PiRGBArray
from picamera import PiCamera

db = mysql.connector.connect(						#Datos para poder accesar a la base de datos Roller
	host="192.168.22.14",							#Direccion de la Camara 1, 
	user="Roller4B2",								#Usuario
	passwd="password",								#contraseña
	database="Roller"								#Nombre de la base de datos
)
cursor=db.cursor()											#Para navegar por las tablas desde Python
comilla = "'"												#Recurso para escribir Querys
retries = 5
cap = PiCamera()
cap.resolution = (640, 480)
cap.framerate = 32
rawCapture = PiRGBArray(cap, size=(640, 480))
time.sleep(0.1)

cursor.execute("SELECT gem FROM gemelo WHERE id = 3")				#Consulta el numero del ciclo actual para sincronizarse con la Camara 1
sup=cursor.fetchone()
ciclo = sup[0]

prtx = []										#Present X, arreglo de apoyo para actualizar los arucos en tiempo real
pastx = []										#Past X, arreglo de apoyo para actualizar los arucos en tiempo real

while(True):								#Ciclo infinito
	
    for frame in cap.capture_continuous(rawCapture, format="bgr", use_video_port=True):
		videoframe = frame.array
		gray = cv2.cvtColor(videoframe, cv2.COLOR_BGR2GRAY)
		aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
		parameters =  aruco.DetectorParameters_create()
		print("Espera...")
		time.sleep(0.1)
		corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
		frame_markers = aruco.drawDetectedMarkers(videoframe.copy(), corners, ids)
		cv2.imshow("frame_markers", frame_markers)
		rawCapture.truncate(0)
               
		if ids is not None:                   									#ids es un arreglo donde se guardan los arucos que detecta la camara
			for i in range(retries):
				try:
					cursor.execute("UPDATE gemelo SET gem = 1 WHERE id = 2")			#La camara 2 le informa a la camara 1 que si detecto arucos
					db.commit()
				except:
					time.sleep(1)
					print("no pudo actualizar la tabla gemelo")
					i+=1
					continue
				break
			y = len(ids)														#Y toma el valor del tamaño del arreglo ids
			x = ids																#X es un arreglo igual al arreglo ids
			x.astype(int)														#Transformamos x para que sea un arreglo de enteros
			prtx = sorted(x)													#Ordenamos x de menor a mayor
			
			for w in range (0,y):																												#Ciclo for limitado por la cantidad de arucos detectados, guardados en el array ids
				n = int(x[w])																													#La variable n tomara el valor de cada uno de los valores en el array x
				for i in range(retries):
					try:
						cursor.execute("SELECT * FROM corte WHERE aruco = "+comilla+str(n)+comilla+"AND cycle ="+comilla+str(ciclo)+comilla)			#Consultamos cada uno de los renglones para los arucos dentro del ciclo actual
						cursor.fetchone()
					except:
						time.sleep(1)
						print("No pudo consultar en corte")
						i+=1
						continue
					break
					
				if cursor.rowcount >= 1:							#Si esta fila ya habia sido consultada
					print("Dado "+str(n)+" ya identificado")		#Imprime la indicacion
						
				else:																						#En caso de que no					
					for i in range(retries):
						try:
							cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+comilla+str(n)+comilla)			#Consulta el piesaje relacionado al aruco almacenado en la tabla de ArucoId
							visto=cursor.fetchone()																	#Almacena el valor del piesaje en una variable llamada visto
							db.commit()
						except:
							time.sleep(1)
							print("No pudo consultar en ArucoId")
							i+=1
							continue
						break
					
					for i in range(retries):
						try:
							sql_insert = "INSERT INTO corte (aruco, sqft, cycle) VALUES (%s, %s, %s)"				#Almacena a la tabla de corte en las columnas de aruco, sqft y cycle
							cursor.execute(sql_insert, (n, visto[0], ciclo))										#Guarda el numero del aruco detectado, la cantidad de pies cuadrados consultados y el ciclo actual
							db.commit()
						except:
							time.sleep(1)
							print("No pudo escribir en corte")
							i+=1
							continue
						break
		
			if len(prtx) == len(pastx):													#Si el tamaño del arreglo presente es igual al tamaño del arreglo pasado
				print("Son del mismo tamaño...")										
				time.sleep(1)
				for cn in range (0, len(prtx)):											#Ciclo for del tamaño del arreglo presente
					if prtx[cn][0] == pastx[cn][0]:										#Compara cada elemento del arreglo presente con cada elemento del arreglo pasado
						print ("Mismo aruco...")
						print (str(prtx[cn][0])+" -- "+str(pastx[cn][0]))				
						time.sleep(1)

					else:																										#Si los arreglos son de diferentes tamaños																			
						print("Diferente aruco...")																				
						print(str(prtx[cn][0])+" -- "+str(pastx[cn][0]))
						
						for i in range(retries):
							try:
								cursor.execute("SELECT * FROM corte WHERE aruco = "+str(prtx[cn][0])+" AND cycle = "+str(ciclo))		#Consulta la fila de los arucos que no que no se encuentran en el arreglo pasado
								cursor.fetchone()
							except:
								time.sleep(1)
								print("No pudo consultar en la tabla corte")
								i+=1
								continue
							break
						
						if cursor.rowcount >= 1:																				#Si ya habian sido consultados
							for i in range(retries):
								try:
									cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(prtx[cn][0]))							#Selecciona el piesaje de ArucoId
									ft=cursor.fetchone()																				#Guarda el valor en una variable llamada ft
									db.commit()
								except:
									time.sleep(1)
									print("No pudo consultar en la tabla ArucoId")
									i+=1
									continue
								break
							for i in range(retries):
								try:
									cursor.execute("UPDATE corte SET sqft = "+str(ft[0])+" WHERE aruco = "+str(prtx[cn][0]))			#Actualiza los valores de piesaje en la tabla corte
									db.commit()
								except:
									time.sleep(1)
									print("No pudo actualizar corte")
									i+=1
									continue
								break
							
						else:																					#Si no habian sido consultados
							for i in range(retries):
								try:
									cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(prtx[cn][0]))			#Consulta el piesaje de la tabla ArucoId
									ft=cursor.fetchone()																#Almacena el valor en una variable llamada ft
									db.commit()
								except:
									time.sleep(1)
									print("No pudo consultar de ArucoId")
									i+=1
									continue
								break
							for i in range(retries):
								try:
									sql_insert="INSERT INTO corte (aruco, sqft) VALUES (%s, %s)"						#Guarda el valor dentro de la tabla corte
									cursor.execute(sql_insert, (prtx[cn][0], ft))										#Guarda el aruco que fue detectado en el presente y no se encuentra en el pasado y su valor de piesaje asignado
									db.commit()
								except:
									time.sleep(1)
									print("No pudo escribir en corte")
									i+=1
									continue
								break
							
						for i in range(retries):
							try:
								cursor.execute("UPDATE corte SET sqft = 0 WHERE aruco = "+str(pastx[cn][0]))			#Actualiza el valor de piesaje a 0 para los arucos del arreglo pasado que no se encontraron en el arreglo presente
								db.commit()
							except:
								time.sleep(1)
								print("No pudo actualizar corte")
								i+=1
								continue
							break
						time.sleep(1)
						
			else:																						#si el arreglo pasado y presente son de tamaños diferentes
				print("Son de diferentes tamaños...")
				time.sleep(1)
				if len(prtx)<len(pastx):																#Si el presente es mayor que el pasado
					print("Hay menos dados..."+str(len(prtx))+" -- "+str(len(pastx)))					
					time.sleep(1)
					print("Los dados faltantes son...")
					for dies in range (len(prtx), len(pastx)):													#Se hace un for del tamaño del arreglo presente contra el arreglo pasado
						print(pastx[dies][0])
						
						for i in range(retries):
							try:
								cursor.execute("UPDATE corte SET sqft = 0 WHERE aruco = "+str(pastx[dies][0]))			#Actualiza el valor de piesaje a 0 para los arucos que se encuentran en el arreglo pasado y no se encuentran en el arreglo presente
								db.commit()
							except:
								time.sleep(1)
								print("No pudo actualizar corte")
								i+=1
								continue
							break
						
						time.sleep(1)
						
				else:																					#Si el arreglo pasado es mayor al arreglo presente
					print("Hay mas dados..."+str(len(prtx))+" -- "+str(len(pastx)))
					time.sleep(1)
					print("Los nuevos dados son: ")
					for dies in range (len(pastx), len(prtx)):											#Se hace un for del tamaño del arreglo pasado contra el arreglo presente
						print(prtx[dies][0])
						
						for i in range(retries):
							try:
								cursor.execute("SELECT * FROM corte WHERE aruco = "+str(prtx[dies][0])+" AND cycle = "+str(ciclo))		#Selecciona las filas de los arucos en el arreglo presente
								cursor.fetchone()
							except:
								time.sleep(1)
								print("No pudo consultar de corte")
								i+=1
								continue
							break
						
						if cursor.rowcount >= 1:														#Si las filas ya habian sido consultadas
							
							for i in range(retries):
								try:
									cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+comilla+str(prtx[dies][0])+comilla)		#Consulta el piesaje de ArucoId para los arucos del arreglo presente
									ft=cursor.fetchone()																			#Guarda el piesaje en una variable llamada ft
									db.commit()
								except:
									time.sleep(1)
									print("No pudo consultar de ArucoId")
									i+=1
									continue
								break
							for i in range(retries):
								try:
									cursor.execute("UPDATE corte SET sqft = "+str(ft[0])+" WHERE aruco = "+str(prtx[dies][0]))		#Actualiza el valor de piesaje para los arucos presentes en caso de que se hayan actualizado a 0 en algun momento
									db.commit()
								except:
									time.sleep(1)
									print("No pudo actualizar corte")
									i+=1
									continue
								break
							
						else:																					#Si no habian sido consultados
							for i in range(retries):
								try:
									cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(prtx[dies][0]))			#Selecciona el piesaje de los arucos del presente 
									ft=cursor.fetchone()																#Guarda el piesaje en una variable llamada ft
									db.commit()
								except:
									time.sleep(1)
									print("No pudo consultar ArucoId")
									i+=1
									continue
								break
							for i in range(retries):
								try:
									sql_insert="INSERT INTO corte (aruco, sqft) VALUES (%s, %s)"						#Guarda los valores consultados en la tabla corte
									cursor.execute(sql_insert, (prtx[dies][0], ft[0]))
									db.commit()
								except:
									time.sleep(1)
									print("No pudo escribir en corte")
									i+=1
									continue
								break
							
						time.sleep(1)
		
		else:																		#Si no encuentra arucos
			for i in range(retries):
				try:
					cursor.execute("UPDATE gemelo SET gem = 0 WHERE id = 2")				#La camara 2 le indica a la camara 1 que no encontro ningun aruco
					db.commit()
				except:
					time.sleep(1)
					print("No pudo actualizar gemelo")
					i+=1
					continue
				break
			
			for i in range(retries):
				try:
					cursor.execute("SELECT gem FROM gemelo WHERE id = 3")					#Consulta el ciclo actual, que provee la camara 1
					sup=cursor.fetchone()													#Guarda el ciclo en la variable sup
					db.commit()
					ciclo=sup[0]
				except:
					time.sleep(1)
					print("No pudo consultar de gemelo")
					i+=1
					continue
				break
																				#Actualiza el ciclo en la camara 2
			
		pastx = prtx 												#Asigna el valor del arreglo pasado al arreglo presente
		time.sleep(0.1)
	        
		if cv2.waitKey(1) & 0xFF == ord('q'):
			continue
           
help(aruco.DetectorParameters_create)
