//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"
#include <ESP32Servo.h>

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   boton           4

Scheduler     userScheduler; // to control your personal task
Servo myservo;  // create servo object to control a servo
namedMesh  mesh;

bool block=false;
String mensaje;
bool adentro=true;
bool afuera=false;
int pos = 0;    // variable to store the servo position, Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 32;
bool pulse=true;
char variable[2]="";
String raspi = "raspiC";
String pres = "Z";
String anterior;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
int counter = 0;

String nodeName = "master"; // Name needs to be unique

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.println("iniciando master");
  //pinMode(boton, INPUT);
  //-----------------------------servo-------------------------------------
  //myservo.setPeriodHertz(50);    // standard 50 hz servo
  //myservo.attach(servoPin, 900, 2100); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  /*Serial.println("Desbloqueando...");
  for( int angle =180; angle>0; angle -=1)
  {
    delay(10);
    myservo.write(angle);
  }*/
  
  //--------------------------mesh-------------------------------------------------
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    /*String controlVariable=msg.substring(0,1);
    controlVariable.toCharArray(variable,2);
    switch (variable[0]) 
    {
      case 'A':     
        if(from == anterior)
        {
          Serial.println("<<----------------------------");
          Serial.print("A recibida: ");
          block=true;
          digitalWrite(27,HIGH);
          Serial.println(block);
        }
        else Serial.println("no me voy a bajar y mis webotes alv");
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("B recibida: ");
        block=false;
        digitalWrite(27,LOW);
        Serial.println(block);
        anterior = from;
        break; 
     case 'C':
        Serial.println("<<----------------------------");
        Serial.print("C recibida: "); 
        mesh.sendSingle(raspi, pres);
      default:
        //Serial.println("variable desconocida");
      break;
   }*/
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
}

void loop() {
  if (Serial.available()){
    mensaje=Serial.readStringUntil('*');
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    switch (variable[0]) 
    {
      case 'A':     
        Serial.println("<<----------------------------");
        Serial.print("A recibida: ");
        block=true;
        digitalWrite(27,HIGH);
        Serial.println(block);
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("B recibida: ");
        block=false;
        digitalWrite(27,LOW);
        Serial.println(block);
        break; 
     case 'C':
        Serial.println("<<----------------------------");
        Serial.print("C recibida: "); 
        mesh.sendSingle(raspi, pres);
      default:
        //Serial.println("variable desconocida");
      break;
   }
 }
 if (Serial2.available()){
    mensaje=Serial2.readStringUntil('*');
    Serial.println("mensaje: "+mensaje);
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    String motivo=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'C':     
        Serial.println("<<----------------------------");
        Serial.print("C recibida: ");
        Serial.print("Mesa C aprobada, razon ");
        Serial.println(motivo);
        raspi="raspiC";
        mesh.sendSingle(raspi, pres);
        break;
      case 'D':     
        Serial.println("<<----------------------------");
        Serial.print("D recibida: ");
        Serial.print("Mesa D aprobada, razon ");
        Serial.println(motivo);
        raspi="raspiD";
        mesh.sendSingle(raspi, pres);
        break;
      case 'E':
        Serial.println("<<----------------------------");
        Serial.print("E recibida: ");
        Serial.print("Mesa E aprobada, razon ");
        Serial.println(motivo);
        raspi="raspiE";
        mesh.sendSingle(raspi, pres);
      case 'F':
        Serial.println("<<----------------------------");
        Serial.print("F recibida: ");
        Serial.print("Mesa F aprobada, razon ");
        Serial.println(motivo);
        raspi="raspiF";
        mesh.sendSingle(raspi, pres);  
      default:
        Serial.println("variable desconocida");
        break;
   }
 }
  mesh.update();
}
