# -*- coding: utf-8 -*-

"""
Created on Tue Mar 10 14:36:17 2020
@authors: gabo_, rudy<3
"""

import sys
sys.path.append('../')
import arducam_mipicamera as arducam
import v4l2
from isp_lib import *

import mysql.connector                                          #Libreria para conectar con base de datos
import serial                                                   #Libreria para conectarse con ESP32 por serial
import numpy as np
import cv2, PIL
import RPi.GPIO as GPIO                                         #Libreria para entradas y salidas digitales
from cv2 import aruco                                           #Libreria para reconocimiento de arucos
import cv2
from imutils.video import VideoStream
# para raspberry
import time                                                     #Libreria de tiempo para RPi
from picamera.array import PiRGBArray                          
from picamera import PiCamera                                   #Libreria oficial de camara para RPi

db = mysql.connector.connect(                                   #Datos para ingresar a la base de datos Roller
    host="192.168.22.14",
    user="user",
    passwd="password",
    database="Roller"
)

cursor=db.cursor()                                              #Para navegar por las tablas desde python
comilla = "'"                                                   #Recurso para escritura de Query
GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.OUT)                                        #La salida 21 sera para indicarle al Pinblock si debe salir o no
GPIO.setup(20, GPIO.IN)                                         #La entrada 20 sera para saber si el bypass esta activo o no
GPIO.setup(26, GPIO.IN)
maquina = "R4"                                                  #Recurso para el numero de maquina
retries = 5
q = GPIO.input(20)                                              #Seudonimo de la entrada 20
time.sleep(0.1)

areaCuero = 45.00                                               #Area promedio del cuero en sqft
utlMeta = 5.00                                                 #Utilizacion meta, (PROVISIONAL)
utl = 0.00                                                      #Utilizacion inicial (RECURSO PARA CALCULOS)
GPIO.output(21, True)

#arduino.write(b"S1*")

def set_controls(camera):
    try:
        print("Reset the focus...")
        camera.reset_control(v4l2.V4L2_CID_FOCUS_ABSOLUTE)
    except Exception as e:
        print(e)
        print("The camera may not support this control.")
    try:
        time.sleep(2)
        print("Setting the exposure...")
        camera.set_control(v4l2.V4L2_CID_EXPOSURE, 10)
        time.sleep(2)
        print("Setting the exposure...")
        camera.set_control(v4l2.V4L2_CID_EXPOSURE, 100)
        time.sleep(2)
        print("Setting the hflip...")
        camera.set_control(v4l2.V4L2_CID_HFLIP, 1)
        time.sleep(2)
        
    except Exception as e:
        print(e)
        print("The camera may not support this control.")

def resize(frame, dst_width = 1080):          #2336, 1920
    height = frame.shape[0]
    width = frame.shape[1]

    scale = (dst_width * 1.0) / width
    return cv2.resize(frame, (int(scale * width), int(scale * height))) 

##########    
def orden(lista):
    lista2 = []
    for i in range (0, len(lista)):
        lista2.append(lista[i][0])
        o = sorted(lista2)
    return o
##########

if __name__ == "__main__":
    try:
        camera = arducam.mipi_camera()
        print("Open camera...")
        camera.init_camera()
        _isp = isp(camera.camera_instance)
        print("Setting the mode...")
        camera.set_mode(2)
        fmt = camera.get_format()
        #fmt = camera.set_resolution(1920, 1920)
        fmt = (fmt["width"], fmt["height"])
        print("Current resolution is {}".format(fmt))
        set_controls(camera)

        start_time = time.time()
        do_change = True
        
    except Exception as e:
        print(e)

for i in range(retries):
    try:
        cursor.execute("SELECT max(cycle) FROM corte")                                  #Query para tomar el valor mayor en la columna cycle de la tabla corte
        sup=cursor.fetchone()                                                           #Lo que se obtenga, se guardara en un array llamado sup
        db.commit()                                                                     #Asi se ejecutan los Querys
        ciclo = sup[0] + 1
    except:
        time.sleep(1)
        print("No pudo consultar el ciclo maximo de la tabla corte")
        i+=1
        continue
    break                                                                       #Aumenta el ciclo en 1
cursor.execute("UPDATE gemelo SET gem = "+str(ciclo)+" WHERE id = 3")           #Le decimos a la segunda camara cual es el ciclo actual
db.commit()                                                                     

prtx = []                                                                       #PresenteX, arreglo de apoyo para actualizar los arucos vistos
pastx = []                                                                      #PasadoX, arreglo de apoyo para actualizar los arucos vistos

while(True):
    while True:
        print("Espera...")
        time.sleep(0.1)
        ##Nueva camara
        data = camera.capture(quality = 80, encoding = 'raw')  #########DEBUG##########
        # Use different variable names to avoid memory being released
        frame = arducam.unpack_raw10_to_raw8(data.buffer_ptr, fmt[0], fmt[1])
        frame = cv2.cvtColor(frame.as_array.reshape((fmt[1], fmt[0])), cv2.COLOR_BAYER_RG2BGR)
        _isp.run_awb(frame)
        _isp.run_ae(frame)
        videoframe = resize(frame)
        ##Nueva camara
        gray = cv2.cvtColor(videoframe, cv2.COLOR_BGR2GRAY)
        aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
        parameters =  aruco.DetectorParameters_create()no 
        corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
        frame_markers = aruco.drawDetectedMarkers(videoframe.copy(), corners, ids)
        cv2.imshow("frame_markers", frame_markers)
        #rawCapture.truncate(0)
        
        if ids is not None:                                                 #ids es un array de los arucos vistos, si tiene un valor hara esto:
            cursor.execute("UPDATE gemelo SET gem = 1 WHERE id = 1")        #Le indico a la segunda camara que la primera esta viendo ids
            db.commit() 
            y = len(ids)                                                    #la variable y toma el valor de la longitud de ids
            x = ids                                                         #x es un array con los valores de ids
            prtx = orden(x)                                                   #el array x se convierte en un array de enteros, sigue con el mismo nombre
            for w in range (0,y):                                                                                                           #ciclo for de 0 hasta la cantidad de arucos presentes, detectados en ids
                n = int(x[w])                                                                                                               #n toma el valor de uno de los elementos del arreglo x, el elemento con la posicion del ciclo actual del for
                for i in range(retries): 
                    try:
                        cursor.execute("SELECT * FROM corte WHERE aruco = "+comilla+str(n)+comilla+" AND cycle ="+comilla+str(ciclo)+comilla)       #se toma la fila de la tabla corte en donde la columna aruco tenga el valor de n y la columna cycle tenga el valor de ciclo
                        cursor.fetchone()
                    except:
                        time.sleep(1)
                        print("No realizo la consulta en corte")
                        i+=1
                        continue
                    break                                                                                                           
                        
                if cursor.rowcount >= 1:                                    #Si esta fila ya habia sido seleccionada previamente, se imprime un espacio en blanco
                    print("  ")
                                
                else:
                    cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(n))      #Si no se habia seleccionado previamente, se selecciona el valor de la columna ft de la tabla ArucoId (Tabla de Ignition)
                    visto=cursor.fetchone()                                             #El valor de ft se guarda en el arreglo llamado visto
                    db.commit()
                                
                    sql_insert = "INSERT INTO corte (aruco, ft, cycle) VALUES (%s, %s, %s)"     #Inserta en la tabla corte 3 valores en las columnas aruco, sqft y cycle
                    cursor.execute(sql_insert, (n, visto[0], ciclo))                                #En aruco sera el valor n, en sqft sera el primer elemento del arreglo visto y el valor de ciclo
                    db.commit()
#########################################DEBUG############################################################
            if len(prtx) == len(pastx):
                print("Son del mismo tamaño")
                for i in range(0, len(prtx)):
                    try:
                        b = pastx.index(prtx[i])
                        print("Mismo aruco: "+str(prtx[i])+" -- "+str(pastx[b]))
                    except ValueError:
                        print(str(prtx[i])+" No esta dentro del pasado") #Hay un dado nuevo
                        cursor.execute("select * from corte where aruco = "+str(prtx[i])+" and cycle ="+str(ciclo))
                        t = cursor.fetchone()
                        db.commit()
                        if cursor.rowcount >= 1:
                            cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i]))
                            ft = cursor.fetchone()
                            db.commit()
                            cursor.execute("update corte set ft = "+str(ft[0])+" where aruco ="+str(prtx[i])+" and cycle ="+str(ciclo))
                            db.commit()
                        else:
                            cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i]))
                            ft = cursor.fetchone()
                            db.commit()
                            cursor.execute("insert into corte (aruco, ft, cycle) values("+str(prtx[i])+","+str(ft[0])+","+str(ciclo)+")")
                            db.commit()
                    try:
                        b = prtx.index(pastx[i])
                        print("Mismo aruco: "+str(prtx[b])+" -- "+str(pastx[i]))
                    except ValueError:
                        print(str(pastx[i])+" No esta dentro del presente") #Quitaron un dado
                        cursor.execute("update corte set ft = 0 where aruco ="+str(pastx[i])+" and cycle ="+str(ciclo))
                        db.commit()

            elif len(prtx) > len(pastx):
                print("Hay mas dados")
                for i in range (0, len(prtx)):
                    try:
                        b = pastx.index(prtx[i])
                        print("Mismo aruco: "+str(prtx[i])+" -- "+str(pastx[b]))
                    except ValueError:
                        print(str(prtx[i])+" No esta dentro del pasado") #Hay un dado nuevo
                        cursor.execute("select * from corte where aruco = "+str(prtx[i])+" and cycle ="+str(ciclo))
                        t = cursor.fetchone()
                        db.commit()
                        if cursor.rowcount >= 1:
                            cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i]))
                            ft = cursor.fetchone()
                            db.commit()
                            cursor.execute("update corte set ft = "+str(ft[0])+" where aruco ="+str(prtx[i])+" and cycle ="+str(ciclo))
                            db.commit()
                        else:
                            cursor.execute("select ft from ArucoId where aruco ="+str(prtx[i]))
                            ft = cursor.fetchone()
                            db.commit()
                            cursor.execute("insert into corte (aruco, ft, cycle) values("+str(prtx[i])+","+str(ft[0])+","+str(ciclo)+")")
                            db.commit()

            elif len(prtx) < len(pastx):
                print("Hay menos dados")
                for i in range (0, len(pastx)):
                    try:
                        b = prtx.index(pastx[i])
                        print("Mismo aruco: "+str(pastx[i])+" -- "+str(prtx[b]))
                    except ValueError:
                        print (str(pastx[i])+" No esta en el presente")  #Quitaron un dado
                        cursor.execute("update corte set ft = 0 where aruco ="+str(pastx[i])+" and cycle ="+str(ciclo))  
                        db.commit()                 
#########################################DEBUG############################################################
        else:                                                                       #si no encontro ningun aruco, ids = none y hara o siguiente
            for i in range(retries):
                try:
                    cursor.execute("UPDATE gemelo SET gem = 0 WHERE id = 1")                #le avisa a la segunda camara que no esta viendo nada
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
                    cursor.execute("SELECT gem FROM gemelo WHERE id = 2")                   #pregunta si la camara 2 esta viendo algo
                    cam2=cursor.fetchone()
                    db.commit()
                except:
                    time.sleep(1)
                    print("No pudo consultar en gemelo")
                    i+=1
                    continue
                break
            
            cam = cam1[0] + cam2[0]                                                 #si el resultado de esta suma es 0, ninguna camara ve algun aruco, si es 1 alguna esta viendo arucos
            if cam == 0 :  
                cursor.execute("select max(cycle) from corte")
                cicloNuevo = cursor.fetchone()
                ciclo = cicloNuevo[0] + 1                                                                                 #si el resultado de la suma es 0 
        ##############################################DEBUG-GPIO##############################
        #Validacion con boton
        def record():
            print("Espera...")
            time.sleep(0.1)
            data = camera.capture(quality = 80, encoding = 'raw')
            frame = arducam.unpack_raw10_to_raw8(data.buffer_ptr, fmt[0], fmt[1])
            frame = cv2.cvtColor(frame.as_array.reshape((fmt[1], fmt[0])), cv2.COLOR_BAYER_RG2BGR)
            _isp.run_awb(frame)
            _isp.run_ae(frame)
            videoframe = resize(frame)
            gray = cv2.cvtColor(videoframe, cv2.COLOR_BGR2GRAY)
            aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
            parameters =  aruco.DetectorParameters_create()
            corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
            frame_markers = aruco.drawDetectedMarkers(videoframe.copy(), corners, ids)
            cv2.imshow("frame_markers", frame_markers)
            
            if ids is not None:                                                 #ids es un array de los arucos vistos, si tiene un valor hara esto:
                cursor.execute("UPDATE gemelo SET gem = 1 WHERE id = 1")        #Le indico a la segunda camara que la primera esta viendo ids
                db.commit() 
                y = len(ids)                                                    #la variable y toma el valor de la longitud de ids
                x = ids                                                         #x es un array con los valores de ids
                prtx = orden(x)                                                   #el array x se convierte en un array de enteros, sigue con el mismo nombre

                for w in range (0,y):                                                                                                           #ciclo for de 0 hasta la cantidad de arucos presentes, detectados en ids
                    n = int(x[w])                                 #n toma el valor de uno de los elementos del arreglo x, el elemento con la posicion del ciclo actual del for
                    for i in range(retries): 
                        try:
                            cursor.execute("SELECT * FROM record WHERE aruco = "+comilla+str(n)+comilla+" AND cycle ="+comilla+str(ciclo)+comilla)       #se toma la fila de la tabla corte en donde la columna aruco tenga el valor de n y la columna cycle tenga el valor de ciclo
                            cursor.fetchone()
                        except:
                            time.sleep(1)
                            print("No realizo la consulta en corte")
                            i+=1
                            continue
                        break
                    if cursor.rowcount >= 1:                                    #Si esta fila ya habia sido seleccionada previamente, se imprime un espacio en blanco
                        print("  ")
                                    
                    else:
                        cursor.execute("SELECT ft FROM ArucoId WHERE aruco = "+str(n))      #Si no se habia seleccionado previamente, se selecciona el valor de la columna ft de la tabla ArucoId (Tabla de Ignition)
                        visto=cursor.fetchone()                                             #El valor de ft se guarda en el arreglo llamado visto
                        db.commit()
                                    
                        sql_insert = "INSERT INTO record (aruco, sqft, cycle) VALUES (%s, %s, %s)"     #Inserta en la tabla corte 3 valores en las columnas aruco, sqft y cycle
                        cursor.execute(sql_insert, (n, visto[0], ciclo))                                #En aruco sera el valor n, en sqft sera el primer elemento del arreglo visto y el valor de ciclo
                        db.commit()
                
                cursor.execute("select sum(sqft) from record where cycle = "+str(ciclo))
                piesaje = cursor.fetchone()
                db.commit()

                return piesaje[0]
        
        try:        
            if GPIO.input(26):
                utl = record()
                print(utl)
                if utl > 5.0:
                    time.sleep(2)
                    print("Meta superada")
                    GPIO.output(21, False)
                    time.sleep(10)
                else:
                    print("No es la meta!")
                    time.sleep(2)
                    GPIO.output(21, False)
                GPIO.output(21, True)
        except:
            print("Ningun dado")
        else:
            print("Boton sin presionar")
            
        ##############################################DEBUG-GPIO##############################
        pastx = prtx
        if cv2.waitKey(1) & 0xFF == ord('q'):
            continue
                        
help(aruco.DetectorParameters_create)
