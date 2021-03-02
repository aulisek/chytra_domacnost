#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <CSV_Parser.h>

const char* ssid = "esp_wifi"; //Napište SSID
const char* password = "keplerprojekt"; //Heslo sítě
const char* PARAMETR = "zarizeni"; //Parametr pro čtení HTTP GET requestu



AsyncWebServer server(80);

RCSwitch mySwitch = RCSwitch();


//funkce pro čtení souboru
String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("prázdný soubor nebo chyba");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
  file.close();
}
//funkce pro zápis do souboru
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("chyba při přístupu do souboru");
    return;
  }
  if (file.print(message)) {
    Serial.println("zpráva zapsána");
  } else {
    Serial.println("Chyba při zapisování");
  }
  file.close();
}
//funkce pro přidání řádku
void addRow(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Připisuji řádek: %s\r\n", path);
  File fileToAppend = SPIFFS.open(path, FILE_APPEND);
  if (!fileToAppend) {
    Serial.println("chyba při přístupu do souboru");
    return;
  }

  if (fileToAppend.println(message)) {
    Serial.println("zpráva přidána na nový řádek");
  }
  else {
    Serial.println("Chyba při zapisování řádku");
  }
  fileToAppend.close();
}

//Replaces placeholder with input1 values
String processor(const String& var) {
  // if (var == "zarizeni") {
  // String zarizeni[] = {id,cislo}
  // zarizeni = readFile(SPIFFS, "/zarizeni.txt");
  // }
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

  //Spuštění serveru
  server.on("/html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/test_file.html", String(), false, processor);
  });


  server.begin();
  Serial.println("Server je spuštěn");

  /*
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
  */

}



void loop() {
  // Ověření, co je v souboru



}
