#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <CSV_Parser.h>

const char* ssid = "esp_wifi"; //Napište SSID
const char* password = "keplerprojekt"; //Heslo sítě

const char* PARAMETR = "zarizeni"; //Parametr pro čtení HTTP GET requestu
const char* PARAM_MESSAGE = "zprava"; //Parametr pro čtení HTTP GET requestu
const char* PARAM_INPUT_1 = "input1"; //Parametr pro čtení HTTP GET requestu

String kod = "1364"; //kód pro zapnutí zařízení (pro request)

char csv_str [512] = {'\0'};  // inicializace pro nacitani souboru po radcich

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
//Vytvoření Stringu pro odeslání seznamu zařízení na stránku (placeholder) - výplň
String vypln() {
  String tlacitka = "";
  String nazev = "koupelna";
  int zapnout = 1364;
  int vypnout = 1360;
  tlacitka += "<p>" + String(nazev) + "</p>";
  tlacitka += "<p><a href=\"/get?message=";
  tlacitka += zapnout;
  tlacitka += "\"><button>ZAPNOUT</button> <a href=\"/get?message=";
  tlacitka += vypnout;
  tlacitka += "\"><button>VYPNOUT</button>";
  return String(tlacitka);
}

//Nahrazení placeholderu na stránce (tlacitkasem) vyplní s tlačítky
String processor(const String& var) {
  if (var == "tlacitkasem") {
    return vypln();
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
    request->send(SPIFFS, "/test_file.html", String(), false, processor);
  });


  server.begin();
  Serial.println("Server je spuštěn");

  File file = SPIFFS.open("/zarizeni.csv");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  uint16_t i = 0;
  while (file.available()) {
    csv_str [i] = file.read();
    // Serial.print (csv_str [i]); //use for debug
    i++;
  }
  csv_str [i] = '\0';
  // Serial.print (csv_str); //use for debug

  // String csv_str = readFile(SPIFFS, "/zarizeni.txt");
  //csv_str.c_str ();
  CSV_Parser cp(csv_str,/*format*/ "sd", /*has_header*/ true, /*delimiter*/ ',');
  // cp.readSDfile("zarizeni.csv");
  cp.print();


  int16_t *nazev = (int16_t*)cp["nazev"];
  String *id = (String*)cp["id"];

  if (nazev && id) {
    for (int row = 0; row < cp.getRowsCount(); row++) {
      // client.println(zarizeni[row], DEC);
      //client.println(<p><a href = "/get?message=id"><button>ZAPNOUT < / button > <a  href = "/get?message=id"><button>VYPNOUT < / button > < / a > );
    }
  } else {
    Serial.println("At least 1 of the columns was not found, something went wrong.");
    file.close();



  }

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
}
