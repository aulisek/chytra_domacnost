#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char* ssid = "esp_wifi"; //Napište SSID
const char* password = "keplerprojekt"; //Heslo sítě

const char* PARAM_MESSAGE = "zprava"; //Parametr pro čtení HTTP GET requestu
const char* PARAM_INPUT_1 = "input1"; //Parametr pro čtení HTTP GET requestu

String kod = "1364";

AsyncWebServer server(80);

RCSwitch mySwitch = RCSwitch();


//funkce pro čtení souboru
String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}
//funkce pro zápis do souboru
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with input1 values
String processor(const String& var) {
  if (var == "input1") {
    return readFile(SPIFFS, "/input1.txt");
  }
  return String();
}
void setup() {
  //Zahájení sériové komunikace
  Serial.begin(9600);
  Serial.println();

  //připojení souborového svazku
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Serial.print("Připojování k:");
  Serial.println(ssid);

  // Zahájení připojení k Wi-Fi
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Připojování");

  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("Úspěšně připojeno k WIFI");
  Serial.println("Vaše IP adresa: ");
  Serial.println(WiFi.localIP() );

  // Vysílač připojen k pinu #23, výchozí protokol 1
  mySwitch.enableTransmit(23);

  // Nastavení počtu opakování vysílání
  mySwitch.setRepeatTransmit(15);

  //Spuštění serveru
  server.on("/html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/test_file.html", "text/html", processor);
  });


  /*  // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
      String zprava;
      if (request->hasParam(PARAM_MESSAGE)) {
        zprava = request->getParam(PARAM_MESSAGE)->value();
        kod = zprava;
        mySwitch.switchOn("kod", "11111");
      }
      else {
        zprava = "No message sent";
      }
      Serial.println(zprava);
    });
  */

  server.begin();
  Serial.println("Server je spuštěn");

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;

    //Get input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      writeFile(SPIFFS, "/input1.txt", inputMessage.c_str());
    }
    //Odeslani kodu z tlacitka na strace
    // Send a GET request to <IP>/get?zprava=<inputMessage>
    else if (request->hasParam(PARAM_MESSAGE)) {
      inputMessage = request->getParam(PARAM_MESSAGE)->value();
      kod = inputMessage;
      mySwitch.switchOn("kod", "11111");
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    //request->send(200, "text/text", inputMessage);
  });
}


void loop() {
  // Ověření, co je v souboru
  String cojevsouboru = readFile(SPIFFS, "/input1.txt");
  Serial.println(cojevsouboru);
  delay(10000);

}
