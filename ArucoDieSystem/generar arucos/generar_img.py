import numpy as np
import cv2, PIL
from cv2 import aruco
import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd

aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_1000)

fig = plt.figure()
nx = 4
ny = 3
j = 0
for i in range(1, 13):
    ax = fig.add_subplot(ny,nx, i, autoscale_on=1)
    img = aruco.drawMarker(aruco_dict,j, 1400)
    plt.imshow(img, cmap = mpl.cm.gray, interpolation = "nearest", aspect=1)
    plt.text(nx, ny-100, j, bbox=dict(fill=False, edgecolor='none', linewidth=2),fontsize=5)
    ax.axis("off")
    j = j + 1

plt.savefig("set10_Longhorn.pdf")    
plt.show()