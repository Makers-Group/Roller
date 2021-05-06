#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"  // Sustituir con datos de vuestra red
#include "API.hpp"
#include "ESP32_Utils.hpp"

#include <ESP32Servo.h>


#define botonF 36//39
#define ba 0
#define bb 1
#define bc 2
#define bd 3
#define be 4
#define bf 5
#define bg 6
#define bh 7
#define pa 8
#define pb 9
#define pc 10
#define pd 11
#define pf 12
#define pg 13
#define ra 14
#define rb 15
#define rc 16
#define rd 17
#define re 18
#define rf 19
#define rg 20
#define rh 21

Servo myservo;  // create servo object to control a servo

long timeToPrint=0;
long timeToPrint2=0;

bool block=false;
bool adentro=true;
bool afuera=false;
int pos = 0;    // variable to store the servo position, Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 32;
bool pulse=true;
String pres = "Z";
String anterior;
/*
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
int counter = 0;*/

const int requestTime=800;

//long startTime=0;
//const int timeThreshold=150;

bool flagF=false;
bool flagFF=false;
long tiempoBotonF=0;
long lastTimeF=0;
int contF;

void setup() 
{
	Serial.begin(115200);
  //pinMode(botonF, INPUT);
 	ConnectWiFi_STA();
	Serial.println("Cliente iniciado");
  //-----------------------------servo-------------------------------------
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 900, 2100); // attaches the servo on pin 18 to the servo object
  Serial.println("Desbloqueando...");
  //for( int angle =180; angle>0; angle -=1)
  //{
    //delay(10);
    myservo.write(0);
  //}
}

void loop() 
{
  if(timeToPrint2<millis())
  {
      Serial.print("Last F: ");
      tiempo(millis()-lastTimeF);
      Serial.print("-");
      Serial.print(contF);
      Serial.println("");
      timeToPrint2=millis()+5000;
  }
  if(timeToPrint<millis())
  {
    String valorData=GetItem(pf);
    if(valorData.toInt()>=1)
    {
      block=true;
    }
    else
    {
      block=false;
    }
    timeToPrint=millis()+ requestTime;
  }

  if(requestFail>5)
  {
    ESP.restart();
  }
  
  if (Serial.available())
  {
    String mensaje=Serial.readStringUntil('*');
    String controlVariable=mensaje.substring(0,1);
    char variable[4];
    controlVariable.toCharArray(variable,2);
    switch (variable[0]) 
    {
      case 'A':     
        Serial.println("<<----------------------------");
        Serial.print("A recibida: ");
        block=true;
        Serial.println(block);
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("B recibida: ");
        block=false;
        Serial.println(block);
        break; 
     case 'C':
        Serial.println("<<----------------------------");
        Serial.print("C recibida: "); 
        ReplaceById(bf,"1");
      default:
        Serial.println("variable desconocida");
      break;
   }
 }
 //---------------lectura del boton------------------------------
 /*  int reading = digitalRead(boton);
  if (reading != lastButtonState)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay)
    if (reading != buttonState)
      buttonState = reading;
  if(reading == 1)
  {
    if(counter == 1){
      ReplaceById(bf,"1");
      Serial.println(pres);
      counter = 0;}
  }
  else counter = 1;
  lastButtonState = reading;
  if(!digitalRead(boton))
  {
    if (millis() - startTime > timeThreshold)
     {
        ReplaceById(bf,"1");
        Serial.println(pres);
        startTime = millis();
     }
  }*/

  if(analogRead(botonF)==0)
  {
        if(!flagF)
        {
            tiempoBotonF=millis();
            flagF=true;
        }
        if(millis()-tiempoBotonF>130)
        {
            if(!flagFF)
            {
                contF++;
                Serial.println("ciclo F"+String(contF)+"------------------------------------------------------------");
                ReplaceById(bf,"1");
                Serial.println(pres);
                lastTimeF=millis();
                flagFF=true;
            }
        }
    }
    else
    {
        tiempoBotonF=millis();
        flagF=false;
        flagFF=false;
    }
//---------------------------------------------------------------
  if(block)
  {
    if(afuera)
    {
      if(pulse) 
      {
        //myservo.write(180);
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
        //myservo.write(angle);
      }
      afuera=true;
      adentro=false;
    }
  }
  else
  {
    if(adentro)
      if(pulse) myservo.write(0);
      else myservo.write(5);
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

void tiempo(unsigned long timeNow)
{
  long day = 86400000; // 86400000 milliseconds in a day
  long hour = 3600000; // 3600000 milliseconds in an hour
  long minute = 60000; // 60000 milliseconds in a minute
  long second =  1000; // 1000 milliseconds in a second
  int days = timeNow / day ;                                //number of days
  int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
  int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
  int seconds = (((timeNow % day) % hour) % minute) / second;
  
   // digital clock display of current time
   Serial.print(days,DEC);  
   printDigits(hours);  
   printDigits(minutes);
   printDigits(seconds);
   //Serial.println();    
}
void printDigits(byte digits)
{
 // utility function for digital clock display: prints colon and leading 0
 Serial.print(":");
 if(digits < 10)
   Serial.print('0');
 Serial.print(digits,DEC);  
}
