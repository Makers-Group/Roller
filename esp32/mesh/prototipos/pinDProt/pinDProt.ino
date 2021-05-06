//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"
#include <ESP32Servo.h>

#define   MESH_SSID       "whateverYouLike3"
#define   MESH_PASSWORD   "somethingSneaky3"
#define   MESH_PORT       5555
#define   botonD           4
#define   botonE           15

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
String raspiD = "raspiD";
String raspiE = "raspiE";
String pres = "Z";
String anterior ;

String raspiMaster="master";
String nodeId="11";
long timeToSend;

int buttonStateD;             // the current reading from the input pin
int lastButtonStateD = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
int counterD = 0;
int counterE = 0;
int buttonStateE;             // the current reading from the input pin
int lastButtonStateE = LOW;   // the previous reading from the input pin

String nodeName = "pinD"; // Name needs to be unique

void setup() {
  Serial.begin(115200);
  pinMode(botonD, INPUT);
  pinMode(botonE, INPUT);
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
     case 'D':
        Serial.println("<<----------------------------");
        Serial.print("D recibida: "); 
        mesh.sendSingle(raspiD, pres);
    case 'E':
        Serial.println("<<----------------------------");
        Serial.print("E recibida: "); 
        mesh.sendSingle(raspiE, pres);
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
  if(timeToSend<millis())
  {
    mesh.sendSingle(raspiMaster, nodeId);
    timeToSend+=2000;
  }
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
        mesh.sendSingle(raspiD, pres);
     case 'E':
        Serial.println("<<----------------------------");  //SIMULAR BOTON E
        Serial.print("E recibida: "); 
        mesh.sendSingle(raspiE, pres);
      default:
        //Serial.println("variable desconocida");
      break;
   }
 }
//---------------lectura del boton------------------------------
   int readingD = digitalRead(botonD);
  if (readingD != lastButtonStateD)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay)
    if (readingD != buttonStateD)
      buttonStateD = readingD;
  if(readingD == 1)
  {
    if(counterD == 1){
      mesh.sendSingle(raspiD, pres);
      Serial.println(pres);
      counterD = 0;}
  }
  else counterD = 1;
  
  int readingE = digitalRead(botonE);
  if (readingE != lastButtonStateE)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay)
    if (readingE != buttonStateE)
      buttonStateE = readingE;
  if(readingE == 1)
  {
    if(counterE == 1){
      mesh.sendSingle(raspiE, pres);
      Serial.println(pres);
      counterE = 0;}
  }
  else counterE = 1;
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
