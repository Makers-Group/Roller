"""
import RPi.GPIO as GPIO
import time

servoPIN = 2
GPIO.setmode(GPIO.BCM)
GPIO.setup(servoPIN, GPIO.OUT)

p = GPIO.PWM(servoPIN, 50) # GPIO 3 for PWM with 50Hz
p.start(2.5) # Initialization

dutyCyclePas = 0.0

try:
  while True:
    dutyCycle = float(input("Insert dutyCycle: "))
    while dutyCyclePas < dutyCycle:
      p.ChangeDutyCycle(dutyCycle)
      dutyCycle += 0.1
      print("Cycle: "+str(dutyCycle))
    while dutyCyclePas > dutyCycle:
      p.ChangeDutyCycle(dutyCycle)
      dutyCycle -= 0.1
      print("Cycle: "+str(dutyCycle))
    dutyCyclePas = dutyCycle

except KeyboardInterrupt:
  p.stop()
  GPIO.cleanup()
"""
import time
x = int(input("numero: "))
xpass = 0
while xpass < x:
  print (str(x)+" -- "+str(xpass))
  print("Ciclo: "+str(xpass))
  xpass += 1
  time.sleep(1)