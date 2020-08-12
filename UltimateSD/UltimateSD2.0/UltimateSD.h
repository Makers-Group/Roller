
#ifndef UltimateSD_h
#define UltimateSD_h


#include "Arduino.h"
#include <SD.h>

class UltimateSD
{
public:
	UltimateSD(int ss);
	void inicializar();
	bool setValue(String fileName, String dataNameStr, String dataValueStr);
	String getValue(String fileName, String dataNameStr);
	String getFileStr(String fileName);
	String leerContenido(String fileName);
	bool escribirNumero(String archivo, long numero);
	bool escribir(String archivo, String texto);
	bool escribirEnInicio(String archivo, String texto);
private:
	int SS_PIN_SD;
	Sd2Card card;
	File lastCountFile;
	void label(String label);
};

#endif