String texto="0000carlos,0001gabriel,0002vane,0100gaby,0300chuy,";

void setup() 
{
 Serial.begin(115200);
}

void loop() 
{
  if(Serial.available())
  {
    String mensaje=Serial.readStringUntil('*');
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
    int indice=texto.indexOf(mensaje);
    if(indice<0)
    {
      Serial.println(" usuario: no registrado");
    }
    else
    {
    int indiceF=texto.indexOf(",",indice);
    String usuario=texto.substring(indice+4,indiceF);
    Serial.print(" usuario: ");
    Serial.println(usuario);
    }
  }
}
