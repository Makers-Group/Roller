/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  
  https://www.arduinolibraries.info/libraries/esp32-servo
*/
#include <esp_wifi.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// REPLACE WITH THE MAC Address of your receiver 

uint8_t newMACAddress[] = {0x24, 0x6F, 0x28, 0xAD, 0xE6, 0xD0};
uint8_t broadcastAddress[] = {0xFC, 0xF5, 0xC4, 0x0E, 0xE1, 0x88};//1 24:6F:28:AD:E6:D0
//uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xAD, 0xE6, 0xD0};//3
// Define variables to store BME280 readings to be sent
bool block=false;
bool serial=false;
bool question=false;

long cycle=0;
long timeToSend=0;

String mensaje="";
char variable[2]="";
String valor="";

// Define variables to store incoming readings
bool incomingBlock;
bool incomingBypass;
int incomingCounter;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    bool block;
    bool bypass;
    int counter;
} struct_message;

// Create a struct_message called ESPReadings to hold sensor readings
struct_message ESPReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(serial)Serial.print("\r\nLast Packet Send Status:\t");
  if(serial)Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  incomingBlock = incomingReadings.block;
  incomingBypass = incomingReadings.bypass;
  incomingCounter = incomingReadings.counter;
  //incomingTemp = incomingReadings.temp;
  //incomingHum = incomingReadings.hum;
  //incomingPres = incomingReadings.pres;
  if(serial)Serial.println("block :"+String(incomingBlock)+" Bypass: "+String(incomingBypass)+" Counter: "+String(incomingCounter));
}
 
void setup() {
  pinMode(27,OUTPUT);
  pinMode(26,INPUT);
  
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Serial.print("[OLD] ESP32 Board MAC Address:  ");
  //Serial.println(WiFi.macAddress());
  
  esp_wifi_set_mac(ESP_IF_WIFI_STA, &newMACAddress[0]);
  
  //Serial.print("[NEW] ESP32 Board MAC Address:  ");
  //Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
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
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() 
{
  if(Serial.available())
  {
    mensaje=Serial.readStringUntil('*');
    if(serial)Serial.println("datos recibidos: "+String(mensaje));
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    valor=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'A':     
        if(serial)Serial.println("<<----------------------------");
        if(serial)Serial.print("A recibida: ");
        block=true;
        //digitalWrite(27,HIGH);
        if(serial)Serial.println(block);
        break;
      case 'B':     //pass
        if(serial)Serial.println("<<----------------------------");
        if(serial)Serial.print("B recibida: ");
        block=false;
        //digitalWrite(27,LOW);
        if(serial)Serial.println(block);
        break;
      case 'Q':     //pass
        if(serial)Serial.println("<<----------------------------");
        if(serial)Serial.print("Q recibida: ");
        question=true;
        break; 
      case 'S':     //pass
        Serial.println("<<----------------------------");
        Serial.print("B recibida: ");
        if(valor=="1"){serial=true;Serial.println("Impresion serial activado");}
        if(valor=="0"){serial=false;Serial.println("Impresion serial desactivada");}
        break;       
      default:
        Serial.println("variable desconocida");
      break;
    }
    mensaje=""; 
  }
  if(question)
  {
    char b;
    if(incomingBypass)
    {
      b='a';
    }
    else 
    {
      b='b';
    }
    Serial.println(b);
    question=false;
  }
  
  if(incomingBypass)
  {
    digitalWrite(27,HIGH);
  }
  else 
  {
    digitalWrite(27,LOW);
  }
/*
  if(digitalRead(26))
  {
    block=true;
  }
  else
  {
    block=false;
  }
  //*/
  if(timeToSend<millis())
  {
    //getReadings();
    //Serial.println("Block: "+String(block));
    // Set values to send
    ESPReadings.block = block;
  
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESPReadings, sizeof(ESPReadings));
     
    if (result == ESP_OK) 
    {
      if(serial)Serial.println("Sent with success");
    }
    else 
    {
      if(serial)Serial.println("Error sending the data");
    }
    if(block==incomingBlock)
    {
      if(serial)Serial.println("Sincronizacion completa");
    }
    else
    {
      if(serial)Serial.println("Sincronizacion en curso");
    }
    timeToSend=timeToSend+2000;
  }
}
void getReadings()
{
  if(cycle<millis())
  {
    if (block)
    {
      block=false;
    }
    else
    {
      block=true;
    }
    cycle=cycle+10000;
    Serial.println("ciclo: "+String(cycle)+" block: "+String(block));
  }//*/
  //block=digitalRead(27);
  //temperature = random(2,80);//bme.readTemperature();
  //humidity = random(50,80);//bme.readHumidity();
  //pressure = random(2,8);//(bme.readPressure() / 100.0F);
}
