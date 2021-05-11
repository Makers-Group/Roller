AsyncWebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(

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
  <h2 style="color:Lime;">Servidor de roller 1</h2>
  <p style="color:MintCream;">Este es el servicio de la api rest que conecta a la roller 1 mesa CDEF</p>
</body>
</html>

)rawliteral";//*/


void homeRequest(AsyncWebServerRequest *request) {
  request->send(200, "text/html", index_html);//"Roller 1 server"
}

void notFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
}

void InitServer()
{
	server.on("/", HTTP_GET, homeRequest);
	server.on("/item", HTTP_GET, getRequest);
	server.on("/item", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, postRequest);
	server.on("/item", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL, putRequest);
	server.on("/item", HTTP_PATCH, [](AsyncWebServerRequest * request){}, NULL, patchRequest);
	server.on("/item", HTTP_DELETE, deleteRequest);
	
	server.onNotFound(notFound);

	server.begin();
    Serial.println("HTTP server started");
}
