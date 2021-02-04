#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char* ssid = ""; //Napište SSID
const char* password = ""; //Heslo sítě

const char* PARAM_MESSAGE = "message";
String kod = "1364";

AsyncWebServer server(80);

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);
  Serial.println();

//připojení souborového svazku
if(!SPIFFS.begin()){
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
}

  Serial.print("Připojování k:");
  Serial.println(ssid);

  // Zahájení připojení k Wi-Fi
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Připojování");

  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");  
  }

  Serial.println();

  Serial.println("Úspěšně připojeno k WIFI");
  Serial.println("Vaše IP adresa: ");
  Serial.println(WiFi.localIP() );

server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/test_file.html", "text/html");
});
 
  
  // Vysílač připojen k pinu #23, výchozí protokol 1
  mySwitch.enableTransmit(23);

  // Nastavení počtu opakování vysílání
  mySwitch.setRepeatTransmit(15);

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
      if (request->hasParam(PARAM_MESSAGE)) {
        message = request->getParam(PARAM_MESSAGE)->value();
        kod = message;
        mySwitch.switchOn("kod","11111");
      } 
      else {
        message = "No message sent";
      }
      Serial.println(message);
   });

  server.begin();
  Serial.println("Server je spuštěn");
}
  
void loop() {
}
