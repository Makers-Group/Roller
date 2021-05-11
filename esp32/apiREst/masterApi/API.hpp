#include "ESP32_Utils_APIREST.hpp"
//----------------------------------------1-1-1-1-1-1-1-1-1-1-2-2
//--------------------0-1-2-3-4-5-6-7-8-9-0-1-2-3-4-5-6-7-8-9-0-1
//--------------------a-b-c-d-e-f-g-h-a-b-c-d-f-g-a-b-c-d-e-f-g-h
int      roller1[22]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0};
long  clientTime[22]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool clientState[22]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const char* PARAM_FILTER = "filter";

const char index2_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML><html>
<head>
  <title>PTS rol 1</title>
  <meta charset="utf-8" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
  </style>
</head>
<body style="background-color:black;">
  <h2 style="color:Lime;">Estado</h2>
  <p style="color:MintCream;">Estado de roller CDEF</p>
  <p style="color:MintCream;">

)rawliteral";

const char medio[] PROGMEM = R"rawliteral(
</p>
<p style="color:MintCream;">Estado de clientes CDEF</p>
<p style="color:MintCream;">

)rawliteral";
const char cierre[] PROGMEM = R"rawliteral(
</p>
</body>
</html>

)rawliteral";

void getAll(AsyncWebServerRequest *request)
{
  String message = "";
  String message2="";
  for(int i=0;i<22;i++)
  {
    message=message+String(roller1[i]);
    message2=message2+String(clientState[i]);
  }
  Serial.println(message);
  //request->send(200, "text/plain", message);
  request->send(200, "text/html",index2_html+message+medio+message2+cierre );
}

void getFiltered(AsyncWebServerRequest *request)
{
  String message = "Get filtered by " + request->getParam(PARAM_FILTER)->value();
  Serial.println(message);
  request->send(200, "text/plain", message);
}

void getById(AsyncWebServerRequest *request)
{
  int id = GetIdFromURL(request, "/item/");

  //String message = String("Get by Id ") + id;
  String message = String(roller1[id]);
  clientTime[id]=millis();
  Serial.print("solicitud id"+String(id)+":");
  Serial.println(message);
  request->send(200, "text/plain", message);
}

void getRequest(AsyncWebServerRequest *request) {
  
  if (request->hasParam(PARAM_FILTER)) {
    getFiltered(request);
  }
  else if(request->url().indexOf("/item/") != -1)
  {
    getById(request);
  }
  else {
    getAll(request);
  }
}

void postRequest(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{ 
  String bodyContent = GetBodyContent(data, len);
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) { request->send(400); return;}

  String string_data = doc["data"];
  String message = "Create " + string_data;
  Serial.println(message);
  request->send(200, "text/plain", message);
}

void patchRequest(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
  int id = GetIdFromURL(request, "/item/");
  String bodyContent = GetBodyContent(data, len);
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) { request->send(400); return;}

  String string_data = doc["data"];
  String message = String("Update ") + id + " with " + string_data;
  Serial.println(message);
  request->send(200, "text/plain", message);
}

void putRequest(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
  int id = GetIdFromURL(request, "/item/");
  String bodyContent = GetBodyContent(data, len);
  Serial.print("[put]id:");
  Serial.print(id);
  Serial.print(" bodyContent:");
  Serial.println(bodyContent); 
  roller1[id]=bodyContent.toInt();
  /*StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, bodyContent);
  if (error) { request->send(400); return;}

  String string_data = doc["data"];
  String message = String("Replace ") + id + " with " + string_data;*/
  //Serial.println(message);
  String message = "reemplazado";
  request->send(200, "text/plain", message);
}

void deleteRequest(AsyncWebServerRequest *request) {
  int id = GetIdFromURL(request, "/item/");

  String message = String("Delete ") + id;
  Serial.println(message);
  request->send(200, "text/plain", message);
}
