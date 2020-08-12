#include <SD.h>
#include "Arduino.h"
#include "UltimateSD.h"


UltimateSD::UltimateSD(int ss)
{
  SS_PIN_SD = ss;
}


void UltimateSD::inicializar()
{
  SD.begin(SS_PIN_SD);
  if (!card.init(SPI_HALF_SPEED, SS_PIN_SD)) 
  {
    Serial.println("SD Initialization failed");
    return;
  }
}


void UltimateSD::label(String label){
  Serial.println(label);
}




bool UltimateSD::setValue(String fileName, String dataNameStr, String dataValueStr)
{
  inicializar();
  // Init Opening file ====================================
  String fileStr = "";
  File file = SD.open(fileName, FILE_WRITE);
  
  if (file) 
  {
    while (file.available()) 
    {
      fileStr += char(file.read());
    }

    file.close();
    // End Opening file ====================================

    
    // init Main Function ==================================
    int indexInit = fileStr.indexOf(dataNameStr);
  
    if(indexInit != -1)
    {
      int indexFirstQuote = fileStr.indexOf("\"",indexInit) + 1;
      int indexEnd = fileStr.indexOf("\"", indexFirstQuote) + 1;
      String line = fileStr.substring(indexInit, indexEnd);
      //Serial.print("line: "); Serial.print(line); Serial.println(".end");
      String newLine = dataNameStr + ": \"" + dataValueStr + "\"";

      //label("Will replace " + line + " with " + newLine);
      fileStr.replace(line, newLine);
    }
    else
    {
      label("Error: couldn't find property " + String(dataNameStr));
      return(false);  
    }
    // end Main Function ====================================

    // init Over writing the file =====================================
    SD.remove(fileName);
    //String label = "Opening " + fileName + "..."; Serial.println(label);
    File file = SD.open(fileName, FILE_WRITE);
    if (file)
    {
      file.print(fileStr);
      file.close();
      //String label = "Successfully changed " + String(dataNameStr) + " to " + String(dataValueStr) ; 
      //Serial.println(label);
    }
    else
    {
      label("Error 2: Couldn't open " + String(fileName));
      return(false);
    }
    return(true);
    // end Over writing the file ============================

  } else {
    // if the file didn't open, print an error:
    label("Error: Couldn't open " + String(fileName));
    return(false);
  }  
}



String UltimateSD::getValue(String fileName, String dataNameStr)
{
  inicializar();

  String fileStr = "";
  File file = SD.open(fileName);
  
  if (file) {
    // Serial.println("Reading file..);
    while (file.available()) {
      fileStr += char(file.read());
    }
 
    file.close();

    //  Main Function =====================
    int indexInit = fileStr.indexOf(dataNameStr);
  
    if(indexInit != -1){
      int indexInitValue = fileStr.indexOf("\"",indexInit) + 1;
      int indexEndValue = fileStr.indexOf("\"",indexInitValue);
    
      String dataValue = fileStr.substring(indexInitValue,indexEndValue);

      // testing
      //return(dataNameStr +": "+ dataValue ); // "cuenta: 12"
      return(dataValue);                       // "12"
    }else{
      return("Error: value '" + dataNameStr + "' not found.");  
    }
    // ====================================
    
  } else {
    // if the file didn't open, print an error:
    return("Error: Couldn't opening " + fileName);
  }         
}


String UltimateSD::getFileStr(String fileName){
  inicializar();

  String fileStr = "";
  File file = SD.open(fileName);
  
  if (file) {
    // Serial.println("Reading file..);
    while (file.available()) {
      fileStr += char(file.read());
    }
    file.close();
    String label = "Success reading " + String(fileName);
    Serial.println(label);
    return fileStr;    
  } else {
    // if the file didn't open, print an error:
    return("Error: couldn't open " + fileName);
  }
}

String UltimateSD::leerContenido(String fileName)
{
  inicializar();

  File file = SD.open(fileName);
  String content = "";
  if (file) 
  {
    while (file.available()) 
    {
      char c = file.read();
      content += c;
    }

    file.close();
  } 
  else 
  {
    Serial.println("Error opening " + fileName);
  }
  return content;
}


bool UltimateSD::escribirNumero(String archivo, long numero)
{
  inicializar();

  File file = SD.open(archivo, FILE_WRITE);

  if (file) {
    file.println(numero);
    file.close();
    return true;
  }
  else 
  {
    Serial.println("Error abriendo archivo");
    return false;
  }
}


bool UltimateSD::escribir(String archivo, String texto)
{
  inicializar();

  File file = SD.open(archivo, FILE_WRITE);

  if (file) {
    file.println(texto);
    file.close();	
    return true;
  }
  // if the file isn't open, pop up an error:
  else 
  {
    Serial.println("Error abriendo " + archivo);
    return false;
  }
}


/*
* Sobreescribe texto en la primera posición de un archivo
*/
bool UltimateSD::sobreescribir(String archivo, String texto)
{
  inicializar();

  File file = SD.open(archivo, O_WRITE | O_CREAT | O_TRUNC);

  if (file)
  {
    file.println(texto);
    file.close();
    return true;
  }
  // if the file isn't open, pop up an error:
  else 
  {
    Serial.println("Error abriendo " + archivo);
    return false;
  }
}


/*
* Escribe texto en la primera posición de un archivo
*/
bool UltimateSD::escribirEnInicio(String archivo, String texto)
{
  inicializar();

  File file = SD.open(archivo, FILE_WRITE);

  if (file) 
  {
    file.seek(0);
    file.println(texto);
    file.flush();
    file.close();
    return true;
  }
  else 
  {
    Serial.println("Error abriendo " + archivo);
    return false;
  }
}