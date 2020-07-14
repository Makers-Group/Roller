import RPi.GPIO as GPIO
import time

servoPIN = 3
GPIO.setmode(GPIO.BCM)
GPIO.setup(servoPIN, GPIO.OUT)

p = GPIO.PWM(servoPIN, 50) # GPIO 3 for PWM with 50Hz
p.start(2.5) # Initialization

try:
  while True:
        dutyCycle = int(input("Insert dutyCycle: "))
        p.ChangeDutyCycle(dutyCycle)
        time.sleep(0.5)

except KeyboardInterrupt:
  p.stop()
  GPIO.cleanup()