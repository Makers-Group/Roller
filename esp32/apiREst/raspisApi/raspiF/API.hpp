//String ApiHost = "http://192.168.22.75:80";
//String ApiHost = "http://192.168.22.38:5000";
String ApiHost = "http://192.168.4.1:80";
int requestFail=0;
String processResponse(int httpCode, HTTPClient& http)
{
  String payload;
	if (httpCode > 0) {
		//Serial.printf("Response code: %d\t", httpCode);

		if (httpCode == HTTP_CODE_OK) {
			payload = http.getString();
			//Serial.println(payload);
		}
	}
	else {
		Serial.printf("Request failed, error: %s\n", http.errorToString(httpCode).c_str());
	  requestFail++;
	}
	http.end();
  return payload;
}

void GetAll()
{
	HTTPClient http;
	http.begin(ApiHost + "/item");
  //http.begin(ApiHost + "/roller1");
	int httpCode = http.GET();
	processResponse(httpCode, http);
}

String GetItem(int id)
{
  
	HTTPClient http;
	http.begin(ApiHost + "/item/" + id);
	int httpCode = http.GET();
	String getData=processResponse(httpCode, http);
  return getData;
}

void GetQuery(String filter)
{
	HTTPClient http;
	http.begin(ApiHost + "/item?filter=" + filter);
	int httpCode = http.GET();
	processResponse(httpCode, http);
}

void Create(String newData)
{
	HTTPClient http;
	http.begin(ApiHost + "/item");
	http.addHeader("Content-Type", "application/json");
	
	String message = "";
	StaticJsonDocument<300> jsonDoc;
	jsonDoc["data"] = newData;
	serializeJson(jsonDoc, message);
	
	int httpCode = http.POST(message);
	processResponse(httpCode, http);
}

void ReplaceById(int id, String newData)
{
	HTTPClient http;
	http.begin(ApiHost + "/item/" + id);
  //http.begin(ApiHost + "/roller1" );
	//http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Type", "text/plain");
  Serial.println("id: "+String(id)+" newData:"+newData);
	String message = newData;
	/*StaticJsonDocument<300> jsonDoc;
	jsonDoc["data"] = newData;
  //jsonDoc["mesa"] = "mesaA";
  //jsonDoc["pin"] = "1";
  //jsonDoc["presionado"] = "1";
	serializeJson(jsonDoc, message);*/

	int httpCode = http.PUT(message);
	processResponse(httpCode, http);
}

void UpdateById(int id, String newData)
{
	HTTPClient http;
	http.begin(ApiHost + "/item/" + id);
	http.addHeader("Content-Type", "application/json");
	
	String message = "";
	StaticJsonDocument<300> jsonDoc;
	jsonDoc["data"] = newData;
	serializeJson(jsonDoc, message);
	
	int httpCode = http.PATCH(message);
	processResponse(httpCode, http);
}

void DeleteById(int id)
{
	HTTPClient http;
	http.begin(ApiHost + "/item/" + id);
	int httpCode = http.sendRequest("DELETE");
	processResponse(httpCode, http);
}
