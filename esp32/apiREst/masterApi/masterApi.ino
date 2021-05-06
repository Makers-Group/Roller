#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "config.h"  // Sustituir con datos de vuestra red
#include "API.hpp"
#include "Server.hpp"
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

#define WDT_TIMEOUT 3

long timeToPrint=5000;

void setup() 
{
	Serial.begin(115200);
  Serial2.begin(115200);

	//ConnectWiFi_STA();
  ConnectWiFi_AP();

	InitServer();
 esp_task_wdt_init(WDT_TIMEOUT,true);
 esp_task_wdt_add(NULL);
 Serial.println("--------------------------------------------------------------");
}

void loop() 
{
  esp_task_wdt_reset();
  if(roller1[rc]==1&&roller1[rd]==1&&roller1[re]==1)  roller1[pc]=1;
  else roller1[pc]=0;
  
  if(roller1[rc]==1&&roller1[rd]==1&&roller1[re]==1&&roller1[rf]==1)  roller1[pd]=1;
  else roller1[pd]=0;
  
  if(roller1[rd]==1&&roller1[re]==1&&roller1[rf]==1)  roller1[pf]=1;
  else roller1[pf]=0;
 
  if(timeToPrint<millis())
  {
      String stringRoller="";
      String clientsState="";
      Serial.print("estado:");
      for(int i=0;i<22;i++)
      {
        stringRoller=stringRoller+String(roller1[i]);
        if(clientTime[i]+35000<millis())
        {
          //Serial.println(nodos[i]+ " muerto");
          clientState[i]=false;
        }
        else
        {
          //Serial.println(nodos[i]+ " vivo");
          clientState[i]=true;
        }
        clientsState=clientsState+String(clientState[i]);
      }
      Serial.print(clientsState);
      //sendComand('A',stringRoller);
      Serial.print(" |clientes:");
      Serial.print(WiFi.softAPgetStationNum());
      Serial.print(" |memoria:");
      Serial.println(ESP.getFreeHeap());
      sendComand('A',clientsState);
      timeToPrint=millis()+10000;
  }
  if (Serial2.available())
  {
      String mensaje=Serial2.readStringUntil('*');
      char variable[4];
      Serial.println("mensaje S2: "+mensaje);
      mensaje.toCharArray(variable,3);
      switch (variable[0]) 
      {
        case 'C':     
          Serial.println("<<----------------------------");
          Serial.print("C recibida: ");
          break;
        case 'D':     
          Serial.println("<<----------------------------");
          Serial.print("D recibida: ");
          break;
        case 'E':
          Serial.println("<<----------------------------");
          Serial.print("E recibida: ");
        case 'F':
          Serial.println("<<----------------------------");
          Serial.print("F recibida: ");
        case 'X':
          Serial.println("<<----------------------------");
          Serial.println("System command "+variable[1]);
          switch (variable[1]) 
          {
            case 'A':     
              Serial.println("subiendo");
              roller1[rc]=1;
              roller1[rd]=1;
              roller1[re]=1;
              roller1[rf]=1;
              break;
            case 'B':     
              Serial.println("bajando");
              roller1[rc]=0;
              roller1[rd]=0;
              roller1[re]=0;
              roller1[rf]=0;
              break;    
            default:
              Serial.println("variable desconocida command");
              break;
          } 
      }
  }
}

void sendComand(char label,String var)
{
  String buss="";
  char buff[30];
  buss=String(label)+var+"*";
  buss.toCharArray(buff,30);
  Serial2.write(buff);
}
