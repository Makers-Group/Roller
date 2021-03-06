//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"

#define   MESH_SSID       "whateverYouLike2"
#define   MESH_PASSWORD   "somethingSneaky2"
#define   MESH_PORT       5555

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "raspiG"; // Name needs to be unique
String mensaje;
char variable[2]="";
bool block=false;
String pinB = "pinB";
String pinA = "pinA";
String pinG = "pinG";
String bajar = String("B");
String subir = String("A");

void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    //Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    Serial.println(msg);
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
        mesh.sendSingle(pinA, subir);
        mesh.sendSingle(pinG, subir);
        break;
      case 'B':     //pass
        Serial.print("B recibida: ");
        block=false;
        digitalWrite(27,LOW);
        Serial.println(block);
        mesh.sendSingle(pinA, bajar);
        mesh.sendSingle(pinG, bajar);
        Serial.println("<<----------------------------");
        mesh.sendSingle(pinG, bajar);
        break;  
      default:
        Serial.println("variable desconocida");
        break;
   }
  }
  mesh.update();
}
