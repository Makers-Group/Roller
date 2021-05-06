import numpy as np
import cv2
import cv2.aruco as aruco
import random
from fpdf import FPDF

aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_1000)
def gen_arucos(inicio, fin):
    for i in range(inicio,fin):
        img = aruco.drawMarker(aruco_dict,i, 1400)
        cv2.imwrite("id_" + str(i) + ".png", img)

def GENERARPDF(inicio, fin, nombre):
    arucosx = 0
    arucosy = 0
    pdf = FPDF('P', 'mm', (210.02,297.01)) #tamaño hoja custom para impresora
    imagelist = []
    pdf.set_font('arial', 'B', 13.0)
    x = 15
    y = 15
    pdf.add_page()
    pdf.line(8.4,0,8.4,279.4)
    pdf.line(0,8.4,215.9 ,8.4)
    for j in range(inicio, fin):
        img = 'id_' + str(j) + '.png'
        imagelist.append(img)
#-------------------------------escibir imagenes en el pdf---------------------------------------
    for image in imagelist:
        pdf.set_xy(x, y-5)
        pdf.cell(ln=0, h=5.0, align='L', w=0, txt='id: ' + str(inicio), border=0)
        inicio += 1
        pdf.image(image,x,y ,w = 45, h = 45)
        x+=58.2
        pdf.line(x-6.6,0,x-6.6,295)
        arucosx += 1
        if arucosx == 3:
            arucosx = 0
            x = 15
            y += 58.2
            pdf.line(0, y-6.6, 250, y-6.6)
            arucosy += 1
        if arucosy == 5:
            arucosy = 0
            pdf.add_page(orientation = 'P', format = 'Letter', same = True)
            x = 15
            y = 15
            pdf.line(8.4,0,8.4,279.4)
            pdf.line(0,8.4,215.9 ,8.4)

    pdf.output(nombre + ".pdf", "F")

gen_arucos(300,701)
#GENERARPDF(0,301, "final")