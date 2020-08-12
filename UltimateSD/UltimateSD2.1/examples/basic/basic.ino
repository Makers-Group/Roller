
/* Creado y desarrollado por Fernando J. Pinedo y 
   Alejandro Ramírez Vilchis con la ayuda de Carlos Jiménez,
   Gabriel Ramírez, Ricardo Lara, Rene Ariyama, Mariángel 
   Pérez e Ivan Alexander Flores. Todos los derechos reservados.

////////////////// PIN LAYOUT /////////////////////

 *             Sd Card      Arduino       Arduino
 *             Reader/PCD   Uno           Mega
 * Signal      Pin          Pin           Pin
 * --------------------------------------------------
 * SPI SS      SDA(SS)      10            49 (CUSTOM)
 * SPI MOSI    MOSI         11 / ICSP-4   51
 * SPI MISO    MISO         12 / ICSP-1   50
 * SPI SCK     SCK          13 / ICSP-3   52
 */

// incluye la libreria UltimateSD
#include <SPI.h>
#include <UltimateSD.h>

// declaración de pines usados
int chipSelect = 46; 
int supplySD = 48; // por si se esta alimentando el modulo de la SD con un pin digital
				   // para trabajar simultaneamente con dos modulos que utilicen SPI
// cambiar el numero del pin para que concuerde con el de tu sistema
// Arduino Ethernet shield: pin 4


int contador  = 0;

// declaración de objeto de UltimateSD
UltimateSD sd(chipSelect); // necesario para poder utilizar las funciones de la libreria

void setup()
{
  Serial.begin(115200); // Iniciar comunicación serial
  Serial.println("---------- INICIANDO ----------");
  pinMode(supplySD, OUTPUT);		// definir el pin de alimentacion como salida digital
  digitalWrite(supplySD, HIGH);		// y mandar 1 cada que se quieran utilizar sus funciones (solo para )

  String datosAguardar = "funciona";

  if (sd.escribir("nombreArchivo.txt", datosAguardar)) // funcion para escribir datos en un archivo
    Serial.println("Datos escritos en SD");
  else
    Serial.println("Hubo error escribiendo en SD");
}

void loop()
{
  if (sd.escribirEnInicio("contador.txt", String(contador))) // esta funcion sobre-escribe solo en el inicio del archivo
    Serial.println("Datos escritos en SD");
  else
    Serial.println("Hubo error escribiendo en SD");
  
  contador++;
  String datosLeidos = sd.leerContenido("contador.txt");
  Serial.println("Datos leidos del archivo contador.txt: " + datosLeidos);
  delay(1000);
}



  
