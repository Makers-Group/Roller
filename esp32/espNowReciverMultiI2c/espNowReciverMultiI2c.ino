/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  
  https://www.arduinolibraries.info/libraries/esp32-servo
*/
/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.
 */
#include <esp_wifi.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  225 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  520 // this is the 'maximum' pulse length count (out of 4096)
#define boton 14
// our servo # counter
uint8_t servonum = 0;

//Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 16;

// REPLACE WITH THE MAC Address of your receiver 
//uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xAD, 0xE6, 0xD0};//2
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66};
//uint8_t broadcastAddress[] = {0xFC, 0xF5, 0xC4, 0x0E, 0xE1, 0x88};//1
uint8_t newMACAddress[] = {0xFC, 0xF5, 0xC4, 0x0E, 0xE1, 0x88};

// Define variables to store BME280 readings to be sent
bool block;

bool bypasss=false;
bool adentro=false;
bool afuera=false;
long cycle=0;
long timeToSend=0;
long timeMaster=0;
int countBypass=0;
bool change=false;

String mensaje="";
char variable[2]="";
String valor="";
bool modoManual=false;
bool master=false;
long x;
// Define variables to store incoming readings
bool incomingBlock;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    bool block;
    bool bypass;
    int counter;
    int id;
    long verify;
    long y;
} struct_message;

// Create a struct_message called ESPReadings (para datos que se van a enviar)
struct_message ESPReadings;

// Create a structure to hold the readings
struct_message incomingInfo;

// Create structures to each board
struct_message incomingReadings1;
struct_message incomingReadings2;

// Create an array with all the structures
struct_message boardsStruct[2] = {incomingReadings1, incomingReadings2};

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0)
  {
    success = "Delivery Success :)";
  }
  else
  {
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) 
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingInfo, incomingData, sizeof(incomingInfo));
  Serial.printf("Board ID %u: %u bytes\n", incomingInfo.id, len);
  if(incomingInfo.id==2)
   {
     Serial.println("Master indicacion");
     timeMaster=millis()+6000;
     master=true;
   }
   else
   {
     Serial.println("Rasp indicacion");  
   }
  // Update the structures with the new incoming data
  boardsStruct[incomingInfo.id-1].block = incomingInfo.block;
  boardsStruct[incomingInfo.id-1].y = incomingInfo.y;
  Serial.printf("block value: %d \n", boardsStruct[incomingInfo.id-1].block);
  Serial.printf("y value: %d \n", boardsStruct[incomingInfo.id-1].y);
  //Serial.print(incomingReadings2.block);
  //Serial.println(incomingReadings2.block);
  Serial.println();
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  //incomingBlock = incomingReadings1.block;
  //Serial.println("block :"+String(incomingBlock));
}
 
void setup() 
{
  //myservo.setPeriodHertz(50);    // standard 50 hz servo
  //myservo.attach(servoPin, 900, 2100); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
 // myservo.write(0);
  //delay(500);
  //myservo.write(45);
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(23,INPUT);
  
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  
  WiFi.mode(WIFI_STA);// Set device as a Wi-Fi Station

  Serial.print("[OLD] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  esp_wifi_set_mac(ESP_IF_WIFI_STA, &newMACAddress[0]);
  
  Serial.print("[NEW] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  attachInterrupt(boton, escucharGolpes, RISING);
}
 
void loop() 
{
  
  if(digitalRead(23))
  {
    //Serial.println("1");
    //desplazamiento(myservo,0,10,1);
    bypasss=true;
    block=false;
  }
  else
  {
    //Serial.println("0");
    bypasss=false;
  }

  if(bypasss)
  {
    if(!change)
    {
      countBypass++;
      change=true;
    }
  }
  else
  {
    change=false;  
  }
  
  if(Serial.available())
  {
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
        break;
      case 'D':     
        Serial.println("<<----------------------------");
        Serial.print("D recibida: ");
        Serial.println(x);
        x++;
        Serial.println(x);
        Serial.println();
        break;              
      default:
        Serial.println("variable desconocida");
      break;
    }
    mensaje=""; 
  }
  getReadings();
  if(timeMaster<millis())
  {
    master=false;
    //timeMaster=timeMaster+6000;
  }
  if(timeToSend<millis())
  {
    int yrec=boardsStruct[0].y;
    Serial.println("x: "+String(x)+"y: "+String(yrec));
    //getReadings();
   
    // Set values to send
    ESPReadings.block = block;
    ESPReadings.bypass = bypasss;
    ESPReadings.counter = countBypass;
    ESPReadings.verify= x;
  
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESPReadings, sizeof(ESPReadings));
     
    if (result == ESP_OK) 
    {
      Serial.println("Sent with success");
    }
    else 
    {
      Serial.println("Error sending the data");
    }

    if(block)
    {
      //Serial.println("Bloqueo");
      if(!afuera)
      {
        Serial.println("Bloqueando...");
        //myservo.write(90);    // tell servo to go to position in variable 'pos'
        //delay(1000);             // waits 15ms for the servo to reach the position
        //myservo.write(180);    // tell servo to go to position in variable 'pos'
        //delay(10);             // waits 15ms for the servo to reach the position
        //desplazamiento(myservo,180,10,1);
        for( int angle =0; angle<181; angle +=1)
        {
          delay(10);
          pwm.setPWM(0, 0, angleToPulse(angle));
        }

        afuera=true;
        adentro=false;
      }
    }
    else
    {
      //Serial.println("Desbloqueo");
       if(!adentro)
      {
        Serial.println("Desbloqueando...");
        //myservo.write(90);    // tell servo to go to position in variable 'pos'
        //delay(1000);             // waits 15ms for the servo to reach the position
        //myservo.write(0);    // tell servo to go to position in variable 'pos'
        //delay(10);             // waits 15ms for the servo to reach the position
        //desplazamiento(myservo,0,10,1);
        for( int angle =180; angle>0; angle -=1)
        {
          delay(10);
          pwm.setPWM(0, 0, angleToPulse(angle));
        }
        afuera=false;
        adentro=true;
      }
    }
    timeToSend=timeToSend+2000;
  }
}
void getReadings()
{
  /*if(cycle<millis())
  {
    if (block)
    {
      block=false;
    }
    else
    {
      block=true;
    }
    cycle=cycle+15000;
    Serial.println("ciclo: "+String(cycle)+" block: "+String(block));
  }//*/
  if(modoManual==0&&bypasss==false)
  {
    if(master)
    {
      block=boardsStruct[1].block;
   /*   Serial.print("Lectura de block master:");
      Serial.print(incomingReadings2.block);
      Serial.println("-> block: "+String(block));*/
    }
    else
    {
      //Serial.println("Lectura de block rasp");
      block=boardsStruct[0].block;
     /* Serial.print("Lectura de block rasp:");
      Serial.print(incomingReadings1.block);
      Serial.println("-> block: "+String(block));*/
    }
    //block=incomingBlock;
  }//*/
  //temperature = random(2,80);//bme.readTemperature();
  //humidity = random(50,80);//bme.readHumidity();
  //pressure = random(2,8);//(bme.readPressure() / 100.0F);
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

int angleToPulse(int ang)
{
   int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
   //Serial.print("Angle: ");Serial.print(ang);
   //Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}

void escucharGolpes() 
{
  x++;
}