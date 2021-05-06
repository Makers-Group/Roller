#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"  // Sustituir con datos de vuestra red
#include "API.hpp"
#include "ESP32_Utils.hpp"
int cont=0;
int cont2=0;
long timeToPrint=0;

void setup() 
{
	Serial.begin(115200);
  
 	ConnectWiFi_STA();
	Serial.println("Cliente iniciado");
	//GetAll();
	//GetItem(1);
	//GetQuery("ABC");
	//Create("New item");
	//ReplaceById(2, "New item");
  //UpdateById(2, "New item");
	//DeleteById(5);
}

void loop() 
{
  if (Serial.available())
  {
    String mensaje=Serial.readStringUntil('*');
    String controlVariable=mensaje.substring(0,1);
    char variable[3];
    controlVariable.toCharArray(variable,2);
    String datos = mensaje.substring(1);
    String ids;
    String value;
    int pos;
    switch (variable[0]) 
    {
      case 'a':     
        GetItem(datos.toInt());
        break;
      case 'b':
        pos=datos.indexOf(",");
        ids=datos.substring(0,pos+1);
        value=datos.substring(pos+1);     
        ReplaceById(ids.toInt(),value);
        break; 
      default:
        Serial.println("variable desconocida");
        break;
   }
  }
  if(timeToPrint<millis())
  {
    GetItem(0);
    cont++;
    cont2++;
    timeToPrint=millis()+1000;
  }
  if(cont2==7)
  {
    ReplaceById(0,String(cont));
    cont2=0;
  }
  if(cont>=9)
  {
    cont=0;
  }
}
