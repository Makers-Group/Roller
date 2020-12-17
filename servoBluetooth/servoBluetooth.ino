#include <esp_wifi.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include "BluetoothSerial.h"
#include <ESP32Servo.h>
#include <Adafruit_PWMServoDriver.h>
#include <esp_task_wdt.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
Servo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position, Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 16;
#define boton 23
#define WDT_TIMEOUT 3
uint8_t servonum = 0;

bool block=false;

bool bypasss=false;
bool adentro=true;
bool afuera=false;
bool pulse=true;
long cycle=0;
long timeToSend=0;
long timeMaster=0;
int countBypass=0;
bool change=false;
volatile bool presionado=false;
String mensaje="";
char variable[2]="";
String valor="";
bool modoManual=false;
bool master=false;
long x;
const int timeThreshold = 200;
long startTime = 0;
long day = 86400000; // 86400000 milliseconds in a day
long hour = 3600000; // 3600000 milliseconds in an hour
long minute = 60000; // 60000 milliseconds in a minute
long second =  1000; // 1000 milliseconds in a second
long printHour=0;

void setup() 
{
  Serial.begin(115200);
  SerialBT.begin("pin1c"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(boton,INPUT);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 900, 2100); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  Serial.println("Desbloqueando...");
  for( int angle =180; angle>0; angle -=1)
  {
    delay(10);
    myservo.write(angle);
  }
  //desplazamiento(myservo,0,10,1);
  delay(4000);
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void loop() 
{
  if(printHour<millis())
  {
    if(pulse)
    {
      pulse=false;
    }
    else
    {
      pulse=true;
    }
    time();
    printHour=millis()+5000;
  }
  esp_task_wdt_reset();
  if(digitalRead(boton))
  {
    x++;
    presionado=true;
  }
  if(presionado)
  {
    SerialBT.println("Z");
    Serial.println("boton presionado");
    Serial.println(x);
    presionado=false;
    delay(500);
  }
  if (Serial.available()) 
  {
    mensaje=Serial.readStringUntil('*');
    Serial.println("datos recibidos: "+String(mensaje));
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    valor=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'A':     
        Serial.println("<<----------------------------");
        Serial.print("A recibida: ");
        SerialBT.println("A*");
        block=true;
        digitalWrite(27,HIGH);
        Serial.println(block);
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("B recibida: ");
        SerialBT.println("B*");
        block=false;
        digitalWrite(27,LOW);
        Serial.println(block);
        break;
      case 'C':     //pass
        Serial.println("<<----------------------------");
        Serial.print("C recibida: ");
        Serial.println(valor);
        if(valor=="1"){modoManual=true;Serial.println("Modo manual activado");}
        if(valor=="0"){modoManual=false;Serial.println("Modo manual desactivado");}
        Serial.println();
        break;
      case 'D':     
        Serial.println("<<----------------------------");
        Serial.print("D recibida: ");
        Serial.println(x);
        x++;
        Serial.println(x);
        Serial.println();
        break;
      case 'E':
        Serial.println("<<----------------------------");
        Serial.print("E recibida: ");
        //pwm.reset();
        break;
      case 'F':
        Serial.println("<<----------------------------");
        Serial.print("F recibida: ");
        //pwm.wakeup();
        break;  
      default:
        Serial.println("variable desconocida");
      break;
   }
  }
  if (SerialBT.available())
  {
    mensaje=SerialBT.readStringUntil('*');
    Serial.println("datos recibidos: "+String(mensaje));
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    valor=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'A':     
        Serial.println("<<----------------------------");
        Serial.print("BTA recibida: ");
        block=true;
        digitalWrite(27,HIGH);
        Serial.println(block);
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("BTB recibida: ");
        block=false;
        digitalWrite(27,LOW);
        Serial.println(block);
        break;
      case 'C':     //pass
        Serial.println("<<----------------------------");
        Serial.print("BTC recibida: ");
        Serial.println(valor);
        if(valor=="1"){modoManual=true;Serial.println("Modo manual activado");}
        if(valor=="0"){modoManual=false;Serial.println("Modo manual desactivado");}
        Serial.println();
        break;
      case 'D':     
        Serial.println("<<----------------------------");
        Serial.print("BTD recibida: ");
        Serial.println(x);
        x++;
        Serial.println(x);
        Serial.println();
        break;              
      default:
        Serial.println("BTvariable desconocida");
      break;
  }
  }
  if(block)
  {
    if(afuera)
    {
      if(pulse)
      {
        myservo.write(180);
      }
      else
      {
        myservo.write(175);
      }
    }
    else
    {
      Serial.println("Bloqueo controlado");
      for( int angle =0; angle<181; angle +=1)
      {
        delay(5);
        myservo.write(angle);
      }
      afuera=true;
      adentro=false;
    }
  }
  else
  {
    if(adentro)
    {
      if(pulse)
      {
        myservo.write(0);
      }
      else
      {
        myservo.write(5);
      }
    }
    else
    {
      Serial.println("Desbloqueo controlado");
      for( int angle =180; angle>0; angle -=1)
      {
        //delay(10);
        myservo.write(angle);
      }
      afuera=false;
      adentro=true;
    }
  }
}
/*  
int angleToPulse(int ang)
{
   int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
   return pulse;
}*/

void escucharGolpes() 
{
  if(millis() - startTime > timeThreshold)
  {
    x++;
    presionado=true;
    startTime = millis();
  } 
}

void time()
{
long timeNow = millis();
 
int days = timeNow / day ;                                //number of days
int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
int seconds = (((timeNow % day) % hour) % minute) / second;

 // digital clock display of current time
 Serial.print(days,DEC);  
 printDigits(hours);  
 printDigits(minutes);
 printDigits(seconds);
 Serial.println();  
 
}

void printDigits(byte digits)
{
 // utility function for digital clock display: prints colon and leading 0
 Serial.print(":");
 if(digits < 10)
   Serial.print('0');
 Serial.print(digits,DEC);  
}

void desplazamiento(Servo &myServo,int pos,int paso,bool serial)
{
  int posActual=myServo.read(); // leemos el angulo que tiene el servo 
  int sentido= posActual-pos;   // calculamos el sentido de giro
  if(serial)Serial.println("Posicion actual   : "+ String(posActual));
  if(serial)Serial.println("Posicion siguiente: "+ String(pos));
  if(sentido>0)                // cuando vamos a un angulo menor
  {
    if(serial)Serial.println("Rotacion hacia angulo menor");
    for (;posActual>pos; posActual-=1) 
    { 
      //Serial.print(posActual);
      myServo.write(posActual);              
      delay(paso);                       
    }
  }
  else if(sentido<0)                // cuando vamos a un angulo mayor
  {
    if(serial)Serial.println("Rotacion hacia angulo mayor");
    for (;posActual<pos; posActual+=1) 
    { 
      //Serial.print(posActual);
      myServo.write(posActual);              
      delay(paso);                       
    }
  }
  else{ if(serial)Serial.println("sin movimiento");}//*/
  if(serial)Serial.println("Posicion alcanzada: "+ String(myServo.read()));  
}
