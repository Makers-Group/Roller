#include <Nextion.h>
#include <MemoryFree.h>
#include <SPI.h>
#include <SD.h>
#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"
/*
 Voltage Divider         <-> Fingerprint Scanner(Pin #) <-> Voltage Divider <-> 5V Arduino w/ Atmega328P
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
FPS_GT511C3 fps(10, 11); // (Arduino SS_RX = pin 10, Arduino SS_TX = pin 11)
bool errorSd=false;
int ssSD = 46;
long cuentaPerpetua = 0;
String mensaje="";
char variable[2]="";
String valor="";
bool finger=false;
String users;
int userId;
bool parametro;
bool fingerFound;
String userName ="";
bool approval;

//pagina de carga inicial
NexPage             espera = NexPage(0, 0, "page0");
NexProgressBar    progreso = NexProgressBar(0, 1, "j0");

//pagina principal 
NexPage          principal = NexPage(1, 0, "page1");
NexText           rollerNo = NexText(1, 12, "roller");
NexDSButton         statSd = NexDSButton(1, 13, "sdStat");
NexDSButton       statMqtt = NexDSButton(1, 14, "mqttStat");
NexDSButton       statWifi = NexDSButton(1, 15, "wifiStat");
//pagina razon
NexNumber          nexMesa = NexNumber(2, 7, "mesa");
NexNumber        nexMotivo = NexNumber(2, 8, "motivo");
NexNumber      nexSolCorte = NexNumber(2, 9, "send");
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
uint32_t solPermision=0;
uint32_t mesa=0;
uint32_t motivo=0;
uint32_t registro=0;
uint32_t confirmar=0;
void setup() 
{
  Serial3.begin(115200);
  Serial.begin(115200);
  nexInit();
  Serial.println(F("---------- INICIANDO... ----------"));
  espera.show();
  progreso.setValue(10);
	pinMode(ssSD, OUTPUT); 
  delay(24);
  if (!SD.begin(ssSD)) 
  {
    Serial.println("Card failed, or not present");
    while (1);
  }
  progreso.setValue(46);
  Serial.println("->Card initialized");
	//Enroll();          //begin enrolling fingerprint
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
  users = getData("users.txt");  
  /*dataFile = SD.open("users.txt", FILE_WRITE);
  if (dataFile) 
  {
    errorSd=false;
    dataFile.println("0000Gabriel_Ramirez,");
    dataFile.close();
  }
  else 
  {
    Serial.println("error opening datalog.txt");
    errorSd=true;
  }*/
  fps.Open();         //send serial command to initialize fps
  fps.SetLED(true);   //turn on LED so fps can see fingerprint
  progreso.setValue(70);
  delay(500);
  progreso.setValue(100);
  delay(500);
  principal.show();
}

void loop() 
{
  solPermision=0;
  registro=0;
  confirmar=0;
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
      case 'C':     
        Serial.println("<<----------------------------");
        Serial.println("Ip enviada: ");
        break;      
      default:
        Serial.println("variable desconocida");
      break;
    }
  }
//---------------------------------------------------------------------------------------------------------------------------  
  nexSolCorte.getValue(&solPermision);
  if(solPermision==1)
  {
    Serial.print("solicitud de corte ");
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
    }
    if(approval==0)
    {
      Serial.println("Denegado");
      pageUserNot.show();
    }
    delay(2000);
    principal.show();
  }
//-------------------------------------------------------------------------------------------------------------------------------  
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
    //
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
  String tupla;// = getCsvMaquina();
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
