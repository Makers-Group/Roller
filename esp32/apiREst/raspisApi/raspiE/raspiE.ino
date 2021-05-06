#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"  // Sustituir con datos de vuestra red
#include "API.hpp"
#include "ESP32_Utils.hpp"

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

long timeToPrint=0;

bool pulse=true;
String pres = "Z";
String anterior;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
int counter = 0;

const int requestTime=800;

void setup() 
{
	Serial.begin(115200);

 	ConnectWiFi_STA();
}

void loop() 
{
  if(timeToPrint<millis())
  {
    String valorData=GetItem(be);
    if(valorData.toInt()>=1)
    {
      Serial.println("Z");
      ReplaceById(be,"0");
      timeToPrint=millis()+5000;
    }
    else
    {
      timeToPrint=millis()+ requestTime; 
    }
    
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
        //Serial.println("<<----------------------------");
        //Serial.print("A recibida: ");
        ReplaceById(re,"1");
        ReplaceById(be,"0");
        break;
      case 'B':     //pass
        //Serial.println("<<----------------------------");
        //Serial.print("B recibida: ");
        ReplaceById(re,"0");
        ReplaceById(be,"0");
        break; 
      default:
        //Serial.println("variable desconocida");
      break;
   }
 }
}
