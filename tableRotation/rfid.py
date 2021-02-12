import serial
from serial import Serial
import mysql.connector
import time

ser = 				serial.Serial()
ser.baudrate = 		115200
ser.port = 			"/dev/ttyUSB0"
ser.timeout = 		1
ser.open()
primer = 			0
tags = 				[]
roller =			""
mesa =				""
tiempo = 			0
ciclos_verde =		0
ciclos_azul =		0
excedido = 			0
rotaciona =			0
rotacionv = 		0
bien =				False
alrevez = 			False
azul =				False
verde =				False
db = mysql.connector.connect(
	host = "192.168.22.14",
	user = "user",
	passwd = "password",
	database = "Roller"
)
cursor = db.cursor(buffered = True) 

try:
	while True:
		if ser.inWaiting():
			mensaje = str(ser.readline().decode("ascii").rstrip())
			print(mensaje)
			if mensaje == "pito":
				print("comenzando a leer")
				start = time.time()
				while True:
					if ser.inWaiting():
						mensaje2 = str(ser.readline().rstrip())
						print(mensaje2)
						if mensaje2 != '':
							tags.append(mensaje2)
						print(tags)
						if len(tags) == 3:
							Ap, Bp, Cp = tags
							break
					else:
						fin = time.time()
						tiempo = fin - start
						if tiempo >= 5.5:
							if len(tags) == 2:
								Ap, Bp = tags
							elif len(tags) == 1:
								print("ciclo incorrecto")
								mal = True
								tags = []
							ser.write(b'E')
							break
		else:
			if tags:
				query = "select * from Boards where %s in (A,B,C,D,E,F)"
				cursor.execute(query, (tags[0],))
				p = cursor.fetchone()
				db.commit()
				p = [str(valor) for valor in p]
				letras = p[4:]
				roller, mesa , lado = p[1:4]
				query = "select * from Ciclos where roller = %s and Mesa = %s"
				cursor.execute(query, (roller, mesa))
				ciclos = cursor.fetchone()
				db.commit()
				ciclos = ciclos[3:]
				ciclos = [int(valor) for valor in ciclos]
				ciclos_verde, ciclos_azul, ciclos_totales = ciclos	
				pos = letras.index(tags[0])
				if pos >= 0 and pos <= 2:
					print("leyendo lado izquierdo")
					A,B,C = letras[0:3]
				elif pos >= 3 and pos <= 5:
					print("leyendo lado derecho")
					A,B,C = letras[3:]
				if lado == "azul":
					A,B,C = C,B,A
					rotacionv = 0
					azul = True
					verde = False
				elif lado == "verde":
					verde = True
					azul = False
					rotaciona = 0
				if len(tags) == 3:
					if Ap == A and Bp == B and Cp == C:
						print("tabla correcta")
						bien = True
					elif Ap == C and Bp == B and Cp == A:
						print("tabla alrevez")
						alrevez = True
					else:
						print("tabla erronea")
				elif len(tags) == 2:
					if Ap == A and Bp == B:
						print("tabla chida")
						alrevez = False
						bien = True
					elif Ap == A and Bp == C:
						print("tabla chida")
						alrevez = False
						bien = True
					elif Ap == B and Bp == C:
						print("tabla chida")
						alrevez = False
						bien = True
					elif Ap == C and Bp == B:
						print("tabla alrevez")
						alrevez = True
						bien = False
					elif Ap == C and Bp == A:
						print("tabla alrevez")
						alrevez = True
						bien = False
					elif Ap == B and Bp == A:
						print("tabla alrevez")
						alrevez = True
						bien = False
					else:
						print("tabla erronea")
				if bien and verde:
					excedido = 0
					rotacionv += 1
					if rotacionv <= 9:
						print("se puede seguir usando la tabla")
						ciclos_verde += 1
						ciclos_totales += 1
					elif rotacionv == 10:
						print("favor de rotar la tabla")
						ciclos_verde += 1
						ciclos_totales += 1
						query = "update Boards set lado = 'azul' where Mesa = %s"
						cursor.execute(query, (mesa,))
						db.commit()
						rotacionv = 0
				elif bien and azul:
					excedido = 0
					rotaciona += 1
					if rotaciona <= 9:
						print("se puede seguir usando la tabla")
						ciclos_azul += 1
						ciclos_totales += 1
					elif rotaciona == 10:
						print("favor de rotar la tabla")
						ciclos_azul += 1
						ciclos_totales += 1
						query = "update Boards set lado = 'verde' where Mesa = %s"
						cursor.execute(query, (mesa,))
						db.commit()
						rotaciona = 0
				elif azul and alrevez:
					excedido += 1
					if excedido == 1:
						print("VOLTEA LA TABLA MALDITA SEA")
						ciclos_verde += 1
						ciclos_totales += 1
					elif excedido == 2:
						print("AL SIGUIENTE CICLO SE BLOQUEARA EL SISTEMA")
						ciclos_verde += 1
						ciclos_totales += 1
					elif excedido >= 3:
						print("ya mamaste")
				elif verde and alrevez:
					excedido += 1
					if excedido == 1:
						print("VOLTEA LA TABLA MALDITA SEA")
						ciclos_azul += 1
						ciclos_totales += 1
					elif excedido == 2:
						print("AL SIGUIENTE CICLO SE BLOQUEARA EL SISTEMA")
						ciclos_azul += 1
						ciclos_totales += 1
					elif excedido >= 3:
						print("ya mamaste")
				query = "update Ciclos set ciclos_verde = %s where Mesa = %s"
				cursor.execute(query, (ciclos_verde, mesa))
				db.commit()
				query = "update Ciclos set ciclos_azul = %s where Mesa = %s"
				cursor.execute(query, (ciclos_azul, mesa))
				db.commit()
				query = "update Ciclos set ciclos_totales = %s where Mesa = %s"
				cursor.execute(query, (ciclos_totales, mesa))
				db.commit()
				print("rotacionv :" + str(rotacionv))
				print("rotaciona :" + str(rotaciona))
				tags = []
			continue
except KeyboardInterrupt:
	ser.close()
	
