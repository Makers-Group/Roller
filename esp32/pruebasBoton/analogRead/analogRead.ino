#include <ESP32Servo.h>
#define botonD 4
#define botonE 15

Servo myservo;  // create servo object to control a servo
int servoPin = 32;

bool flagD=false;
bool flagDD=false;
long tiempoBotonD=0;
long lastTimeD=0;
int contD;

bool flagE=false;
bool flagEE=false;
long tiempoBotonE=0;
long lastTimeE=0;
int contE;

long timeToPrint=0;

void setup() 
{
  	Serial.begin(115200);
  	Serial.println("Cliente iniciado");
    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(servoPin, 900, 2100); // attaches the servo on pin 18 to the servo object
    Serial.println("Desbloqueando...");
    myservo.write(0);
}

void loop() 
{
    myservo.write(0);
    
    if(timeToPrint<millis())
    {
        Serial.print("Last D: ");
        tiempo(millis()-lastTimeD);
        Serial.print("-");
        Serial.print(contD);
        Serial.print("  Last E: ");
        tiempo(millis()-lastTimeE);
        Serial.print("-");
        Serial.print(contE);
        Serial.println("");
        timeToPrint=millis()+3000;
    }
//---------------------------------------------------------------------    
    if(analogRead(botonE)==0)
    {
        if(!flagE)
        {
            tiempoBotonE=millis();
            flagE=true;
        }
        if(millis()-tiempoBotonE>130)
        {
            if(!flagEE)
            {
                contE++;
                Serial.println("ciclo E"+String(contE));
                lastTimeE=millis();
                flagEE=true;
            }
        }
    }
    else
    {
        tiempoBotonE=millis();
        flagE=false;
        flagEE=false;
    } //*/
//---------------------------------------------------------------------
    if(analogRead(botonD)==0)
    {
        if(!flagD)
        {
            tiempoBotonD=millis();
            flagD=true;
        }
        if(millis()-tiempoBotonD>130)
        {
            if(!flagDD)
            {
                contD++;
                Serial.println("ciclo D"+String(contD));
                lastTimeD=millis();
                flagDD=true;
            }
        }
    }
    else
    {
        tiempoBotonD=millis();
        flagD=false;
        flagDD=false;
    } 
}

void tiempo(unsigned long timeNow)
{
  long day = 86400000; // 86400000 milliseconds in a day
  long hour = 3600000; // 3600000 milliseconds in an hour
  long minute = 60000; // 60000 milliseconds in a minute
  long second =  1000; // 1000 milliseconds in a second
  int days = timeNow / day ;                                //number of days
  int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
  int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
  int seconds = (((timeNow % day) % hour) % minute) / second;
  
   // digital clock display of current time
   Serial.print(days,DEC);  
   printDigits(hours);  
   printDigits(minutes);
   printDigits(seconds);
   //Serial.println();    
}
void printDigits(byte digits)
{
 // utility function for digital clock display: prints colon and leading 0
 Serial.print(":");
 if(digits < 10)
   Serial.print('0');
 Serial.print(digits,DEC);  
}
