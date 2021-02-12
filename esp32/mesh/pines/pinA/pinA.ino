//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"
#include <ESP32Servo.h>

#define   MESH_SSID       "whateverYouLike2"
#define   MESH_PASSWORD   "somethingSneaky2"
#define   MESH_PORT       5555
#define   botonA           4
#define   botonH           15

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
String raspiA = "raspiA";
String raspiH = "raspiH";
String pres = "Z";

int buttonStateA;             // the current reading from the input pin
int lastButtonStateA = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
int counterA = 0;
int counterH = 0;
int buttonStateH;             // the current reading from the input pin
int lastButtonStateH = LOW;   // the previous reading from the input pin

String nodeName = "pinA"; // Name needs to be unique

void setup() {
  Serial.begin(115200);
  pinMode(botonA, INPUT);
  pinMode(botonH, INPUT);
  //-----------------------------servo-------------------------------------
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
  
  //--------------------------mesh-------------------------------------------------
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    String controlVariable=msg.substring(0,1);
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
     case 'D':
        Serial.println("<<----------------------------");
        Serial.print("D recibida: "); 
        mesh.sendSingle(raspiA, pres);
    case 'E':
        Serial.println("<<----------------------------");
        Serial.print("E recibida: "); 
        mesh.sendSingle(raspiH, pres);
      default:
        //Serial.println("variable desconocida");
      break;
   }
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
     case 'D':
        Serial.println("<<----------------------------");  //SIMULAR BOTON D
        Serial.print("D recibida: "); 
        mesh.sendSingle(raspiA, pres);
     case 'E':
        Serial.println("<<----------------------------");  //SIMULAR BOTON E
        Serial.print("E recibida: "); 
        mesh.sendSingle(raspiH, pres);
      default:
        //Serial.println("variable desconocida");
      break;
   }
 }
//---------------lectura del boton------------------------------
   int readingA = digitalRead(botonA);
  if (readingA != lastButtonStateA)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay)
    if (readingA != buttonStateA)
      buttonStateA = readingA;
  if(readingA == 1)
  {
    if(counterA == 1){
      mesh.sendSingle(raspiA, pres);
      Serial.println(pres);
      counterA = 0;}
  }
  else counterA = 1;
  
  int readingH = digitalRead(botonH);
  if (readingH != lastButtonStateH)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay)
    if (readingH != buttonStateH)
      buttonStateH = readingH;
  if(readingH == 1)
  {
    if(counterH == 1){
      mesh.sendSingle(raspiH, pres);
      Serial.println(pres);
      counterH = 0;}
  }
  else counterH = 1;
//---------------------------------------------------------------
   if(block)
  {
    if(afuera)
      if(pulse) myservo.write(180);
      else myservo.write(175);
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
  mesh.update();
}
