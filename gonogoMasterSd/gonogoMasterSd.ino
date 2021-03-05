#include <Nextion.h>
#include <MemoryFree.h>
#include <SPI.h>
#include <SD.h>
#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"
/*
 Voltage Divider         <->  (Pin #) <-> Voltage Divider <-> 5V Arduino w/ Atmega328P
                            <-> UART_TX (3.3V TTL) (Pin 1) <->                 <->       RX (pin 4)
  GND <-> 10kOhm <-> 10kOhm <-> UART_RX (3.3V TTL) (Pin 2) <->      10kOhm     <->       TX (pin 5)
          GND               <->        GND         (Pin 3) <->       GND       <->        GND
                            <->    Vin (3.3V~6V)   (Pin 4) <->                 <->        5V 
*/

// set up software serial pins for Arduino's w/ Atmega328P's
// FPS (TX) is connected to pin 4 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 5 (Arduino's Software TX)
//FPS_GT511C3 fps(4, 5); // (Arduino SS_RX = pin 4, Arduino SS_TX = pin 5)

/*If using another Arduino microcontroller, try commenting out line 60 and
uncommenting line 69 due to the limitations listed in the
library's note => https://www.arduino.cc/en/Reference/softwareSerial . Do
not forget to rewire the connection to the Arduino*/

// FPS (TX) is connected to pin 10 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 11 (Arduino's Software TX)
FPS_GT511C3 fps(11, 12); // (Arduino SS_RX = pin 11, Arduino SS_TX = pin 12)
bool finger=false;
String users;
int userId;
bool parametro;
bool fingerFound;
String userName ="";
bool approval;
const char mesas[]={'A','B','C','D','E','F','G','H'};

int interval=100;

// variable para velocidad de perforado
float velocidad [10]; // global para que se guarde la información al salir de la función

String mensaje="";
char variable[2]="";
String valor="";

String ssid ="";//"SaltilloCutting"; // your network SSID (name)
String pass ="";//"S1lt3ll4c";       // your network password
String local_IP="";//"192.168.22.3";
String gateway="";//"192.168.22.1";
String subnet="";//"255.255.255.0";
String machine="";//"3";
String location="";
String machineT="";

unsigned long lastMillis = 0;
unsigned long esperaConexion=0;

// declaración de pines usados
int ssSD = 46;
int supplySD = 48;

// datos de maquina
long cuentaPerpetua = 0;
long tiempo=0;
float golpesPorMin = 0.0;
bool huboGolpe = false;
bool huboReposo = false;
int erroresMqtt=0;
long memoriaGolpes;
long memoriaParo;
int setPoint=100;

bool errorShield=false;
bool errorSensor=false;
bool errorWifi=false;
bool errorMqtt=false;
bool errorSd=false;
bool reinicio=false;

int espWifi=1;
int espMqtt=1;

// variables de tiempo
unsigned long tiempoUltimoGolpe = 0; //tiempo en segundos del ultimo golpe detectado
unsigned long tiempoTranscurrido = 0;
unsigned long tiempoSD = 180000;
unsigned long tiempoParo = 0;
unsigned long tiempoMillis = 0;
unsigned long timeoutEsp=0;

const int timeThreshold = 5000;
long startTime = 0;
volatile bool inicio=false;

long cuentaSensor=0;

//pagina de carga inicial
NexPage             espera = NexPage(0, 0, "page0");
NexProgressBar    progreso = NexProgressBar(0, 1, "j0");

//pagina principal 
NexPage          principal = NexPage(1, 0, "page1");
NexText           rollerNo = NexText(1, 12, "roller");
NexDSButton         statSd = NexDSButton(1, 13, "sdStat");
NexDSButton       statMqtt = NexDSButton(1, 14, "mqttStat");
NexDSButton       statWifi = NexDSButton(1, 15, "wifiStat");
NexNumber    disablePinNex = NexNumber(1,17,"disable");
//pagina razon
NexNumber          nexMesa = NexNumber(2, 7, "mesa");
NexNumber        nexMotivo = NexNumber(2, 8, "motivo");
NexNumber      nexSolCorte = NexNumber(2, 9, "send");
NexText        datosConcat = NexText(2,12, "concat"); 
//pagina de huellaId
NexPage       pageHuellaId = NexPage(3, 0, "page3");
//permitiendo corte
NexPage       pagePermitir = NexPage(4, 0, "page4");
//usuario no registrado
NexPage        pageUserNot = NexPage(5, 0, "page5");
//pin
NexPage            pagePin = NexPage(6, 0, "page6");
//verificar id registrar
NexPage       pageVerRegis = NexPage(7, 0, "page7");
NexNumber      nexRegistro = NexNumber(7, 3, "newUser");
//usuario registrado
NexPage      pageUserRegis = NexPage(8, 0, "page8");
//registro nuevo usuario
NexPage       pageRegistro = NexPage(9, 0, "page9");
//poner dedo
NexPage      pagePonerDedo = NexPage(10, 0, "page10");
//quitar dedo
NexPage     pageQuitarDedo = NexPage(11, 0, "page11");
//verificar datos
NexPage      pageVerificar = NexPage(12, 0, "page12");
NexText        nexUserName = NexText(12, 2, "userName");
NexText      nexUserNomina = NexText(12, 3, "userNomina");
NexNumber     nexConfirmar = NexNumber(12, 6, "aceptUser");
//verificar datos
NexPage   pageRegistroExitoso = NexPage(14, 0, "page13");
//error inesperado
NexPage   pageError = NexPage(16, 0, "page14");

char buffer[100] = {0};
uint32_t razon=0;
uint32_t solPermision=0;
uint32_t mesa=0;
uint32_t motivo=0;
uint32_t registro=0;
uint32_t confirmar=0;
uint32_t disablePin=0;

void setup() 
{
  Serial3.begin(115200);
  Serial.begin(115200);
  Serial1.begin(115200);
  nexInit();
  Serial.println("->Iniciando");
  Serial.println("->Configurando salidas y entradas");
  espera.show();
  progreso.setValue(10); 
  pinMode(48,OUTPUT);
  pinMode(49,OUTPUT);
  digitalWrite(48,HIGH);
  digitalWrite(49,HIGH);
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  delay(24);
  if (!SD.begin(ssSD)) 
  {
    Serial.println("Card failed, or not present");
    while (1);
  }
  progreso.setValue(46);
  Serial.println("->Card initialized");
  //Enroll();          //begin enrolling fingerprint
  Serial.println("->Configurando wifi");
    ssid = getData("ssid.txt");
    pass = getData("password.txt");
    local_IP = getData("local_IP.txt");
    gateway = getData("gateway.txt");
    subnet = getData("subnet.txt");
    machine = getData("machineN.txt");
    location = getData("location.txt");
    machineT = getData("machineT.txt");
    cuentaPerpetua = getData("counter.txt").toInt();
    Serial.print(F("Counter: "));
    Serial.println(cuentaPerpetua);
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) 
  {
    errorSd=false;
    dataFile.println("----------------------------------------");
    dataFile.close();
  }
  else 
  {
    Serial.println("error opening datalog.txt");
    errorSd=true;
  }
  Serial.println("->Iniciando ESP");  
  delay(10);
  digitalWrite(10,LOW);
  delay(30);
  digitalWrite(10,HIGH);
  //users = getData("users.txt");  
  fps.Open();         //send serial command to initialize fps
  fps.SetLED(true);   //turn on LED so fps can see fingerprint
  attachInterrupt(0, escucharGolpes, RISING);
  progreso.setValue(70);
  delay(500);
  progreso.setValue(100);
  delay(500);
  Serial.println("->Inicio exitoso");
  principal.show();
}

void loop() 
{
  solPermision=0;
  registro=0;
  confirmar=0;
  disablePin=0;
  // sets huboReposo to true, to avoid sending hits when the machine is stoped
  long microSeconds = micros() / 1000;
  if( (tiempoUltimoGolpe + 90000) < microSeconds)
  {
    huboReposo=true;
  }
  // send a message roughly every second to esp32.
  if (millis() - lastMillis > 1000) 
  {
    if(espWifi==0)
    {
      statWifi.setValue(0);
      if(espMqtt==0)
      {
        statMqtt.setValue(0);
        Serial.println(">>>>Conectado<<<<<");
        lastMillis = millis();
        sendFloatEsp('a',golpesPorMin);
        delay(interval);
        sendLongEsp('b',cuentaPerpetua);
        delay(interval);
        sendLongEsp('c',tiempo);
        delay(interval);
        sendLongEsp('d',razon);
        delay(interval);
        //sendLongEsp('e',errorSensor);
        //delay(interval);
        sendLongEsp('f',errorSd);
        delay(interval);
        sendLongEsp('g',reinicio);
        delay(interval);
        //sendStringEsp('h',modelo);
        //delay(interval);
        //sendStringEsp('i',pines);
        //delay(interval);
        //sendStringEsp('j',plates);
        //delay(interval);
        sendLongEsp('k',memoriaGolpes);
        delay(interval);
        //sendLongEsp('l',problema);
        //delay(interval);
        //sendLongEsp('m',accion);
        //delay(interval);
        sendLongEsp('n',tiempoTranscurrido);
        delay(interval);
      }
      else 
      {
        Serial.println("Waiting for mqtt conection");
        statMqtt.setValue(1);  
      }
    }
    else 
    {
      Serial.println("waiting for wifi conection");
      statWifi.setValue(1);
    }
    tiempo = millis(); 
  }
  if (millis() - timeoutEsp > 180000) 
  {
    espWifi=1;
    espMqtt=1;
    Serial.println("->Reiniciando ESP");  
    delay(10);
    digitalWrite(10,LOW);
    delay(30);
    digitalWrite(10,HIGH);
    timeoutEsp=millis(); 
  }
  if (huboGolpe) 
  { //únicamente si se detecta un golpe las lecturas serán grabadas
    Serial.println("<<<..................................................................>>>"); 
    Serial.print(F("Cuenta perpetua:  "));
    Serial.println(cuentaPerpetua);
    Serial.print(F(""));

    golpesPorMin = getVelocidad(tiempoTranscurrido);
    Serial.print("Ggpm: ");
    Serial.println(String(golpesPorMin));

    huboReposo = false;
    huboGolpe = false; // para solo hacer estas acciones cada que un golpe es detectado
    tiempoMillis=millis();
    Serial.println("<<<..................................................................>>>");
  } 

  int paroActivo=digitalRead(3);

  if(paroActivo)
  {
    Serial.println("Paro activado");
    Serial.println("Maquina en mantenimiento");
    razon=2;
  }
  else
  {
    if(huboReposo)
    {
      razon=1;
    }
    else
    {
      razon=0;        
    }
  }
      
  long cuentaCabezal=cuentaPerpetua-memoriaGolpes;
  Serial.println("cuenta perpetua: "+String(cuentaPerpetua));
  Serial.println("cuenta sensor:   "+String(cuentaSensor));
  Serial.println("inicio:          "+String(inicio));
  Serial.println("........................................................................");

  if (esperaConexion < millis())
  {
    guardarSD();
    esperaConexion = millis() + 60000; //tiempo de 1min para guardar datos
  }
  if(errorSd)statSd.setValue(1);
  else statSd.setValue(0);
  //Bloque de comandos seriales, realiza la lectura de comandos seriales
  //---------------------------------------------------------------------------------------------------------------------------
  if(Serial.available())
  {
    mensaje=Serial.readStringUntil('*');
    Serial.println("");
    Serial.println("datos recibidos: "+String(mensaje));
    if(mensaje.length()>1)
    {
      Serial.println("Con parametro");
      parametro=true;
    }
    else
    {
      Serial.println("sin parametro");
      parametro=false;
    }
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    valor=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'A':     
        Serial.println("<<----------------------------");
        Serial.print("Verify: ");
        while(!finger)
        {
          identify();
        }
        finger=false;
        break;
      case 'B':     
        Serial.println("<<----------------------------");
        Serial.print("Enroll: ");
        userId=valor.toInt();
        Serial.println(userId);
        if(!parametro)
        {
          while(!finger)
          {
            fingerFound=identify();
          }
          finger=false;
        }
        else
        {
          fingerFound=false;
        }
        if(!fingerFound)
        {
          Serial.println("ingrese nombre");
          while(Serial.available()==0)
          {
            Serial.println("esperando serial");
          }
          if(Serial.available())
          {
             userName =Serial.readStringUntil('*');
              Serial.println("");
              Serial.println("datos recibidos: "+String(userName));
          }
          int userEnrolled = Enroll(parametro,userId);
          overWrite(fillZero(String (userEnrolled))+".txt",userName);
        } 
        break;  
      default:
        Serial.println("variable desconocida");
      break;
    }
  }
  //Bloque de comandos seriales, realiza la lectura de comandos seriales del ESP32 mqtt
  //---------------------------------------------------------------------------------------------------------------------------
  if(Serial3.available())
  {
    mensaje=Serial3.readStringUntil('*');
    Serial.println("");
    Serial.println("datos recibidos: "+String(mensaje));
    String controlVariable=mensaje.substring(0,1);
    controlVariable.toCharArray(variable,2);
    valor=mensaje.substring(1);
    switch (variable[0]) 
    {
      case 'A':     //ssid
        Serial.println("<<----------------------------");
        Serial.print("Red enviada: ");
        Serial.println(ssid);
        sendStringEsp('A',ssid);
        delay(interval);
        break;
      case 'B':     //pass
        Serial.println("<<----------------------------");
        Serial.print("Contrasena enviada: ");
        Serial.println(pass);
        sendStringEsp('B',pass);
        delay(interval);
        break;
      case 'C':     //localIp
        Serial.println("<<----------------------------");
        Serial.println("Ip enviada: "+local_IP);
        sendStringEsp('C',local_IP);
        delay(interval);
        break;
      case 'D':     //gateway
        Serial.println("<<----------------------------");
        Serial.println("Gateway enviada: "+gateway);
        sendStringEsp('D',gateway);
        delay(interval);
        break;
      case 'E':     //subnet
        Serial.println("<<----------------------------");
        Serial.println("Subnet enviada: "+subnet);
        sendStringEsp('E',subnet);
        delay(interval);
        break; 
      case 'F':     //machine
        Serial.println("<<----------------------------");
        Serial.println("Maquina enviada: "+machine);
        sendStringEsp('F',machine);
        delay(interval);
        break;
      case 'G':     //statWifi
        Serial.println("<<----------------------------");
        Serial.print("statWifi recibido: ");
        espWifi=valor.toInt();
        Serial.println(espWifi);
        timeoutEsp= millis();
        break;
      case 'H':     //statMqtt
        Serial.println("<<----------------------------");
        Serial.print("statMqtt recibido: ");
        espMqtt=valor.toInt();
        Serial.println(espMqtt);
        timeoutEsp=millis();
        break;
      case 'I':
        Serial.println("<<----------------------------");
        Serial.print("location enviada: ");
        Serial.println(location);
        sendStringEsp('I',location);
        delay(interval);
        break;        
      default:
        Serial.println("variable desconocida");
      break;
    }
  }
  disablePinNex.getValue(&disablePin);
  if(disablePin==2)
  {
    disablePinNex.setValue(0);
  }
  //Bloque de solicitud de corte, realiza la verificacion de id para permitir un corte fuera de utilizacion 
  //---------------------------------------------------------------------------------------------------------------------------
  nexSolCorte.getValue(&solPermision);
  if(solPermision==1)
  {
    Serial.print("solicitud de corte ");
    String stringConcat=obtenerTexto(datosConcat);
    String mesa1=stringConcat.substring(0,1);
    String motivo1=stringConcat.substring(1,2);
    Serial.println("mesa1:"+mesa1+" motivo1:"+motivo1);
    //enviarDatosTexto("","",datosConcat);
    nexMesa.getValue(&mesa);
    nexMotivo.getValue(&motivo);
    Serial.println("mesa:"+String(mesa)+" motivo:"+String(motivo));
    nexSolCorte.setValue(0);
    pageHuellaId.show();
    while(!finger)
    {
       approval=identify();
       Serial.println(String(approval));
    }
    finger=false;
    if(approval==1)
    {
      Serial.println("Aprobado");
      pagePermitir.show();
      //String buss=String(mesa)+","+String(motivo);
      Serial.println(String(mesa)+String(motivo));
      //sendStringEspMesh(mesas[mesa-1],String(motivo));
      sendStringEspMesh(mesa1,motivo1);
    }
    if(approval==0)
    {
      Serial.println("Denegado");
      pageUserNot.show();
    }
    delay(2000);
    principal.show();
  }
  //Bloque de registro de nuevo usuario, realiza el registro de un nuevo usuario en la DB del lector de huellas, registrandolo ademas en la memoria sd  
  //----------------------------------------------------------------------------------------------------------------------------------------------------  
  nexRegistro.getValue(&registro);
  if(registro==2)
  {
    nexRegistro.setValue(0);      
    while(!finger)
    {
      fingerFound=identify();
    }
    finger=false;   
    if(!fingerFound)
    {
      pageRegistro.show();
      Serial.println("ingrese nombre");
      while(confirmar==0)
      {
        Serial.println("Esperando datos");
        nexConfirmar.getValue(&confirmar);
      }
      userName=obtenerTexto(nexUserName);
      Serial.println("datos recibidos: "+String(userName));
      int userEnrolled = Enroll(0,userId);
      if(userEnrolled>0)
      {
      overWrite(fillZero(String (userEnrolled))+".txt",userName);
      pageRegistroExitoso.show();
      }
    }
    else
    {
      pageUserRegis.show();
    }
  }  
}

void escucharGolpes() 
{
  cuentaSensor++;
  if(millis() - startTime > timeThreshold)
  {
    if(inicio==true)
    {
      tiempoTranscurrido = (micros() / 1000) - tiempoUltimoGolpe;
      //if(tiempoTranscurrido>2000)
      //{
        cuentaPerpetua++;
        huboGolpe = true;
        huboReposo = false;
        inicio=false;
      //}
      tiempoUltimoGolpe = micros() / 1000;
    }
    else
    {
      inicio=true;
    }
    startTime = millis();
  }
}

int Enroll(bool param,int id)
{
   int enrollid = 0;
   bool usedid = true;
  // find open enroll id
  if(!param)
  {
    enrollid = 0;
    usedid = true;
    while (usedid == true)  //identifica el siguiente id vacio
    {
      usedid = fps.CheckEnrolled(enrollid);
      if (usedid==true) enrollid++;
    }
  }
  else
  {
    enrollid=id;
  }
  fps.EnrollStart(enrollid);

  // enroll
  Serial.print("Press finger to Enroll #");
  Serial.println(enrollid);
  pagePonerDedo.show();
  while(fps.IsPressFinger() == false) delay(100);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;
  if (bret != false)
  {
    Serial.println("Remove finger");
    pageQuitarDedo.show();
    fps.Enroll1(); 
    while(fps.IsPressFinger() == true) delay(100);
    Serial.println("Press same finger again");
    pagePonerDedo.show();
    while(fps.IsPressFinger() == false) delay(100);
    bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      Serial.println("Remove finger");
      pageQuitarDedo.show();
      fps.Enroll2();
      while(fps.IsPressFinger() == true) delay(100);
      Serial.println("Press same finger yet again");
      pagePonerDedo.show();
      while(fps.IsPressFinger() == false) delay(100);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        Serial.println("Remove finger");
        pageQuitarDedo.show();
        iret = fps.Enroll3();
        if (iret == 0)
        {
          Serial.println("Enrolling Successful");
          return enrollid;
        }
        else
        {
          Serial.print("Enrolling Failed with error code:");
          pageError.show();
          Serial.println(iret);
         return -1;
        }
      }
      else Serial.println("Failed to capture third finger");
      pageError.show();
      return -1;
    }
    else Serial.println("Failed to capture second finger");
   pageError.show();
    return -1;
  }
  else Serial.println("Failed to capture first finger");
  pageError.show();
  return -1;
}

bool identify()
{
  // Identify fingerprint test
  if (fps.IsPressFinger())
  {
    finger=true;
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();
    
       /*Note:  GT-521F52 can hold 3000 fingerprint templates
                GT-521F32 can hold 200 fingerprint templates
                 GT-511C3 can hold 200 fingerprint templates. 
                GT-511C1R can hold 20 fingerprint templates.
       Make sure to change the id depending on what
       model you are using */
    if (id <200) //<- change id value depending model you are using
    {//if the fingerprint matches, provide the matching template ID
      Serial.print("Verified ID:");
      Serial.println(id);
      //getUserName(id,users);
      getData(fillZero(String(id))+".txt");
      return true;
    }
    else
    {//if unable to recognize
      Serial.println("Finger not found");
     return false;
    }
  }
  else
  {
    Serial.println("Please press finger");
  }
  delay(100);
}

String getData(String fileName)
{
  String archivo;
  File dataFile = SD.open(fileName);
  //Serial.println("tamano: "+String(dataFile.size()));
  long sizeFile=dataFile.size();
  if (dataFile) 
  {
    for(int i=0;i<sizeFile;i++)
    {
      archivo = archivo+char(dataFile.read());
    }
    dataFile.close();
    Serial.println("Datos de "+fileName+": "+ archivo);
    return archivo;
  }
  else 
  {
    Serial.println("error opening "+fileName);
    return "not found";
  }
}

bool overWrite(String fileName, String file)
{
  SD.remove(fileName);
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) 
  {
    dataFile.print(file);
    dataFile.close();
    Serial.println(file);
    return true;
  }
  else
  {
    Serial.println("Error sobreescribiendo "+ file);
    return false;
  }
}

void guardarSD() 
{
  String tupla = getCsvMaquina();
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) 
  {
    errorSd=false;
    dataFile.println(tupla);
    dataFile.close();
    Serial.println(tupla);
  }
  else 
  {
    Serial.println("error opening datalog.txt");
    errorSd=true;
  }
  
  if(overWrite("counter.txt",String(cuentaPerpetua)))
  {
    errorSd=true;
  }
  else
  {
    errorSd=false;
  }
  /*if(overWrite("counterM.txt",String(memoriaGolpes)))
  {
    errorSd=true;
  }
  else
  {
    errorSd=false;
  }*/
}

String getUserName(int userId,String usersDb)
{
    String mensaje=String(userId);
    Serial.print("ID: "+String(mensaje));
    int longitud = mensaje.length();
    if(longitud<4)
    {
      int lugares=4-longitud;
      for(int i=0;i<lugares;i++)
      {
        mensaje="0"+mensaje;
      }
    }
    int indice=usersDb.indexOf(mensaje);
    if(indice<0)
    {
      Serial.println(" usuario: no registrado");
    }
    else
    {
      int indiceF=usersDb.indexOf(",",indice);
      String usuario=usersDb.substring(indice+4,indiceF);
      Serial.print(" usuario: ");
      Serial.println(usuario);
      return usuario;
    }
}

String fillZero(String userId)
{
  //Serial.print("ID: "+userId);
  int longitud = userId.length();
  if(longitud<4)
  {
    int lugares=4-longitud;
    for(int i=0;i<lugares;i++)
    {
      userId="0"+userId;
    }
  }
  return userId;  
}

void enviarDatosNex(char arrayAux[10], char caracter[5], NexText etiqueta)
{
  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, arrayAux);
  strcat(buffer, caracter);
  etiqueta.setText(buffer);
}

void enviarDatosTexto(String texto, char caracter[5], NexText etiqueta)
{
  String aux;
  aux = texto;
  char arraux[25] = {0};
  aux.toCharArray(arraux, 25);
  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, arraux);
  strcat(buffer, caracter);
  etiqueta.setText(buffer);
}

String obtenerTexto(NexText nexEtiqueta)
{
  char buffer[100] = {0};
  char arraux[25] = {0};
  memset(buffer, 0, sizeof(buffer));
  nexEtiqueta.getText(buffer, sizeof(buffer));
  strcpy(arraux, buffer);
  String texto = (char*)buffer;
  return texto;
}

String getCsvMaquina()
{
  String tupla = getFecha()+"|"+","+
                 String(tiempo)+","+
                 String(golpesPorMin)+"," +
                 String(cuentaPerpetua)+","+
                 String(tiempoTranscurrido);
  return tupla;
}

void reiniciarMillis()
{
  if (micros() >= 3900000000/*capacidad maxima de micros*/)
  {
     Serial.println("Reiniciando millis()");
     // reiniciar micros y millis 
     extern volatile unsigned long timer0_millis, timer0_overflow_count;
     noInterrupts();
     timer0_millis = timer0_overflow_count = 0;
     interrupts();  
    
    tiempoTranscurrido = 0;
    tiempoUltimoGolpe = 0;
  }
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//Interval is in secs
void restartArduino(int interval)
{
  
  if (micros() >= (interval * 10000000)/*3900000000/*capacidad maxima de micros*/)
  {
    Serial.println("Han pasado 10 minutos, Reiniciando Arduino");
    //publicar(publishTopicTemplate + "reinicio", "1");
    delay(100);
    resetFunc();
   }
}

void restartArduino() 
{
  resetFunc();
}

float getVelocidad(unsigned long tiempoEntreGolpes)
{
  // Serial.print("Tiempo entre golpes: ");
  // Serial.println(String(tiempoEntreGolpes));
  
  if (tiempoEntreGolpes > 1000)
  {
    return 0.0;
  }

  // recorriendo los valores del arreglo
  for (int i = 8; i >= 0; i--)
  {
    velocidad[i] = velocidad[i + 1];
  }
  velocidad[9] = 60000.0 / float(tiempoEntreGolpes);

  // obteniendo el promedio
  float suma = 0.0;
  int muestras = 0;
  for (int i = 0; i < 10; i++)
  {
    if (velocidad[i] != 0)
    {
      suma = suma + velocidad[i];
      muestras++;
    }
  }

  return suma / muestras;
}

void sendFloatEsp(char label,float var)
{
  String buss="";
  char buff[20];
  buss=String(label)+String(var)+"*";
  //Serial.println("enviando: "+buss);
  buss.toCharArray(buff,20);
  Serial3.write(buff);
}

void sendLongEsp(char label,long var)
{
  String buss="";
  char buff[20];
  buss=String(label)+String(var)+"*";
  //Serial.println("enviando: "+buss);
  buss.toCharArray(buff,20);
  Serial3.write(buff);
}

void sendStringEsp(char label,String var)
{
  String buss="";
  char buff[20];
  buss=String(label)+var+"*";
  //Serial.println("enviando: "+buss);
  buss.toCharArray(buff,20);
  Serial3.write(buff);
}
void sendStringEspMesh(String label,String var)
{
  String buss="";
  char buff[20];
  buss=label+var+"*";
  Serial.println("enviando: "+buss);
  buss.toCharArray(buff,20);
  Serial1.write(buff);
}
String getFecha()
{
  NexRtc  rtc;
  uint8_t time_buf[30] = {0};
  String str;
  rtc.read_rtc_time(time_buf, 30);
  str = (char*)time_buf;
  return str;
}
