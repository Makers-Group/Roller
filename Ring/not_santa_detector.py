# USAGE
# python not_santa_detector.py 

# import the necessary packages
from keras.preprocessing.image import img_to_array
from keras.models import load_model
from imutils.video import VideoStream
from threading import Thread
import numpy as np
import imutils
import time
import cv2
import os
import RPi.GPIO as GPIO
import automationhat

x = 0

def light_tree(tree, sleep=5):
	# loop over all LEDs in the tree and randomly blink them with
	# varying intensities
	for led in tree:
		led.source_delay = 0.1
		led.source = random_values()

	# sleep for a bit to let the tree show its Christmas spirit for
	# santa clause
	time.sleep(sleep)

	# loop voer the LEDs again, this time turning them off
	for led in tree:
		led.source = None
		led.value = 0

def play_christmas_music(p):
	# construct the command to play the music, then execute the
	# command
	command = "aplay -q {}".format(p)
	os.system(command)

# define the paths to the Not Santa Keras deep learning model and
# audio file
MODEL_PATH = "modelSaltillov5.model"
AUDIO_PATH = "jolly_laugh.wav"

# initialize the total number of frames that *consecutively* contain
# santa along with threshold required to trigger the santa alarm
TOTAL_CONSEC = 0
TOTAL_THRESH = 20

# initialize is the santa alarm has been triggered
LEATHER = False

# load the model
print("[INFO] loading model...")
model = load_model(MODEL_PATH)



# initialize the video stream and allow the camera sensor to warm up
print("[INFO] starting video stream...")
vs = VideoStream(src=0).start()
#vs = VideoStream(usePiCamera=True).start()
time.sleep(2.0)

# ocnfiguration for GPIO
GPIO.setmode(GPIO.BCM)
#GPIO.setup(33, GPIO.OUT)

# loop over the frames from the video stream
while True:
	# grab the frame from the threaded video stream and resize it
	# to have a maximum width of 400 pixels
	frame = vs.read()
	frame2 = vs.read()
	frame = imutils.resize(frame, width=400)

	#prepare the image to be classified by our deep learning network
	image = cv2.resize(frame, (28, 28))

	####TEST####
	#Guardar frames para contar los contornos
	z = x % 1200
	if(z==0):
		cv2.imwrite("/home/pi/Desktop/Timelapse/Frames/frame%s.jpg" %x , frame2)
	x = x+1
	####TEST####

	image = image.astype("float") / 255.0
	image = img_to_array(image)
	image = np.expand_dims(image, axis=0)

	# classify the input image and initialize the label and
	# probability of the prediction
	(NoLeather, Leather) = model.predict(image)[0]
	label = "Not leather"
	proba = NoLeather
	
	# check to see if santa was detected using our convolutional
	# neural network
	if Leather > NoLeather:
		# update the label and prediction probability
		label = "Leather"
		proba = Leather

		# increment the total number of consecutive frames that
		# contain santa
		TOTAL_CONSEC += 1

		#output to arduino
		#GPIO.output(33, GPIO.LOW)
		automationhat.relay.one.off()

	# otherwise, reset the total number of consecutive frames and the
	# santa alarm
	else:
		TOTAL_CONSEC = 0
		LEATHER = False
		#output to arduino
		#GPIO.output(33, GPIO.HIGH)
		automationhat.relay.one.on()

	# build the label and draw it on the frame
	label = "{}: {:.2f}%".format(label, proba * 100)
	frame = cv2.putText(frame, label, (10, 25),
		cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

	# show the output frame
	cv2.imshow("Frame", frame)
	key = cv2.waitKey(1) & 0xFF
 
	# if the `q` key was pressed, break from the loop
	if key == ord("q"):
		break

# do a bit of cleanup
print("[INFO] cleaning up...")
cv2.destroyAllWindows()
vs.stop()
