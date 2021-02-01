//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 4. prints anything it receives to Serial.print
//
//
//************************************************************
#include <painlessMesh.h>
#include <ESP32Servo.h>
#include <esp_task_wdt.h>
// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Servo myservo;  // create servo object to control a servo

// Prototypes
void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);
String pin = "D";
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

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode (boton, INPUT);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION | COMMUNICATION);  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run 
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
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
  //desplazamiento(myservo,0,10,1);
  delay(4000);
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void loop() {
  if(printHour<millis())
  {
    if(pulse)
      pulse=false;
    else
      pulse=true;
    time();
    printHour=millis()+5000;
  }
  esp_task_wdt_reset();
  if(digitalRead(boton))
  {
    x++;
    presionado=true;
  }
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  digitalWrite(LED, !onFlag);
  if(block)
  {
    if(afuera)
    {
      if(pulse)
        myservo.write(180);
      else
        myservo.write(175);
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
        myservo.write(0);
      else
        myservo.write(5);
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

void sendMessage() {
  String msg = "";
  if(presionado)
  {
    msg = "Z" + pin;
    Serial.println("boton presionado");
    presionado=false;
    mesh.sendBroadcast(msg);
    delay(500);
  }
  if (Serial.available()){
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
      case 'C':     //pass
        Serial.println("<<----------------------------");
        Serial.print("C recibida: ");
        Serial.println(valor);
        if(valor=="1"){modoManual=true;Serial.println("Modo manual activado");}
        if(valor=="0"){modoManual=false;Serial.println("Modo manual desactivado");}
        Serial.println();
        msg = "C";
        break;
      case 'D':     
        Serial.println("<<----------------------------");
        Serial.print("D recibida: ");
        Serial.println();
        msg = "D";
        break;
      case 'E':
        Serial.println("<<----------------------------");
        Serial.print("E recibida: ");
        //pwm.reset();
        break;
      case 'F':
        Serial.println("<<----------------------------");
        Serial.print("F recibida: ");
        msg = "ZD"; //simular el boton
        //pwm.wakeup();
        break;  
      default:
        Serial.println("variable desconocida");
      break;
   }
   mesh.sendBroadcast(msg);
}   
  //msg += mesh.getNodeId();
  //msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  //mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.printf("Sending message: %s\n", msg.c_str());
  
  taskSendMessage.setInterval( TASK_SECOND * 1);  // 1 SEGONDO
}


void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  int id = msg.indexOf(pin); //recibe por ejemplo: C1D1F0
  if(id > 0){
    String mensaje = msg.substring(id+1,id+2);
    if (mensaje == "1"){
      block = true;
      Serial.print("bloqueando");
      }
     else if (mensaje == "0"){
      Serial.println("desbloqueando");
      block = false;
      }
    }
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

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
