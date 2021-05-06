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
#define   boton           4

Scheduler     userScheduler; // to control your personal task
Servo myservo;  // create servo object to control a servo
namedMesh  mesh;

String mensaje;
char variable[3]="";

String raspi = "raspiC";
String pinC = "pinC";
String pinD = "pinD";
String pinF = "pinF";
String bajar = String("B");
String subir = String("A");
String pres = "Z";

                 //0----1----2----3----4----5----6----7----8----9----10---11---12---13
String nodos[14]={"ra","rb","rc","rd","re","rf","rg","rh","pa","pb","pc","pd","pf","pg"};//array auxiliar para saber que ID corresponde a que nodo
long tiempos[14];   //almacena el millis del ultimo mensaje por nodo
bool nodeState[14]; //almacena el estado booleano de cada nodo 1=vivo 0=muerto
String nodesState="";

long timeToPrint=0;

String nodeName = "master"; // Name needs to be unique

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.println("iniciando master");
  //--------------------------mesh-------------------------------------------------
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    int nodeSender=msg.toInt();
    //Serial.println(nodos[nodeSender]);
    tiempos[nodeSender]=millis();
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
}

void loop() {
  if(timeToPrint<millis())
  {
    Serial.print("estado: ");
    for(int i=0;i<14;i++)
    {
      Serial.print(nodos[i]+"-"+String(nodeState[i])+",");
      nodesState=nodesState+String(nodeState[i]);
    }
    Serial.println("");
    Serial.println("nodesState: "+nodesState);
    //sendComand('A',nodesState);
    nodesState="";
    bool nodec=mesh.sendSingle(pinC, subir);
    Serial.println("eviado"+String(nodec));
    uint32_t nodeci=mesh.getNodeId();
    Serial.println("eviado"+String(nodeci));
    mesh.sendSingle(pinD, subir);
    timeToPrint=millis()+3000;
  }
  for(int i=0;i<14;i++)
  {
    if(tiempos[i]+35000<millis())
    {
      //Serial.println(nodos[i]+ " muerto");
      nodeState[i]=false;
    }
    else
    {
      //Serial.println(nodos[i]+ " vivo");
      nodeState[i]=true;
    }
  }
  /*
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
 }*/
/* if (Serial2.available())
 {
    mensaje=Serial2.readStringUntil('*');
    Serial.println("mensaje S2: "+mensaje);
    String controlVariable=mensaje.substring(0,1);
    String motivo=mensaje.substring(1);
    mensaje.toCharArray(variable,3);
    //controlVariable.toCharArray(variable,2);
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
      case 'X':
        Serial.println("<<----------------------------");
        Serial.println("System command "+motivo);
        switch (variable[1]) 
        {
          case 'A':     
            Serial.println("subiendo");
            mesh.sendSingle(pinC, subir);
            mesh.sendSingle(pinD, subir);
            mesh.sendSingle(pinF, subir);
            break;
          case 'B':     
            Serial.println("bajando");
            mesh.sendSingle(pinC, bajar);
            mesh.sendSingle(pinD, bajar);
            mesh.sendSingle(pinF, bajar);
            break;    
          default:
            Serial.println("variable desconocida command");
            break;
        }   
      default:
        Serial.println("variable desconocida");
        break;
   }
 }*/
  mesh.update();
}

void sendComand(char label,String var)
{
  String buss="";
  //String messageCS="";
  char buff[30];
  //messageCS=String(label)+var;
  //buss=checksum(messageCS)+"*";
  //Serial.println(buss);
  buss=String(label)+var+"*";
  buss.toCharArray(buff,30);
  Serial2.write(buff);
}
