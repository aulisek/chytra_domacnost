#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <CSV_Parser.h>

//Proměné pro připojení k wifi
const char* ssid = "esp_wifi"; //Napište SSID
const char* password = "keplerprojekt"; //Heslo sítě

//Parametry pro HTTP GET requesty
const char* PARAMETR = "zarizeni"; //Parametr pro čtení HTTP GET requestu
const char* PARAM_MESSAGE = "zprava"; //Parametr pro čtení HTTP GET requestu
const char* PARAM_INPUT_1 = "input1"; //Parametr pro čtení HTTP GET requestu

//Kód pro radiové ovládání
String kod = "1364"; //kód pro zařízení (pro request)

//Deklarace pro CSV parser
char csv_str [512] = {'\0'};  // inicializace pro nacitani souboru po radcich

AsyncWebServer server(80);

RCSwitch mySwitch = RCSwitch();

//funkce pro čtení souboru
void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = SPIFFS.open(path);
  if (!file) {
    Serial.println("prázdný soubor nebo chyba");
    return;
  }
  uint16_t i = 0;
  while (file.available()) {
    csv_str [i] = file.read();
    i++;
  }
  csv_str [i] = '\0';
  Serial.println(csv_str);
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
  CSV_Parser cp(csv_str,/*format*/ "sdd", /*has_header*/ true, /*delimiter*/ ',');
  cp.print();

  char **nazev = (char**)cp["nazev"];
  int16_t *kod_zapnuto = (int16_t*)cp["kod_zapnuto" ];
  int16_t *kod_vypnuto = (int16_t*)cp["kod_vypnuto"];

  //cyklus zajišťující přiřazení tlačítka k prvkům ze souboru
  if (nazev && kod_zapnuto && kod_vypnuto) {
    for (int row = 0; row < cp.getRowsCount(); row++) {
      //debug, zobrazení, co je v souboru
      Serial.print(nazev[row]);               Serial.print(" - ");
      Serial.print(kod_zapnuto[row], DEC);    Serial.print(" - ");
      Serial.print(kod_vypnuto[row], DEC);    Serial.print(" - ");
      //Sestavení HTML pro placeholder
      tlacitka += "<p>";
      tlacitka += (nazev)[row];
      tlacitka += "</p>";
      tlacitka += "<p><a href=\"/get?zprava=";
      tlacitka += (kod_zapnuto[row]);
      tlacitka += "\"><button>ZAPNOUT</button></a> <a href=\"/get?zprava=";
      tlacitka += (kod_vypnuto[row]);
      tlacitka += "\"><button>VYPNOUT</button></a>";
    }
  } else {
    Serial.println("At least 1 of the columns was not found, something went wrong.");
  }
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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/test_file.html", String(), false, processor);
  });


  server.begin();
  Serial.println("Server je spuštěn");
  readFile(SPIFFS, "/zarizeni.csv");

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    //Get input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      addRow(SPIFFS, "/zarizeni.csv", inputMessage.c_str());
      readFile(SPIFFS,"/zarizeni.csv");
    }
    //Odeslani kodu z tlacitka na strace
    // Send a GET request to <IP>/get?zprava=<inputMessage>
    if (request->hasParam(PARAM_MESSAGE)) {
      inputMessage = request->getParam(PARAM_MESSAGE)->value();
      kod = inputMessage;
      mySwitch.switchOn("kod", "11111"); //druhý parametr -> výchozí klíč (nastavení na ovladači a zařízeních)
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    //request->send(200, "text/text", inputMessage);
  });
}

void loop() {
  //  if csv_str
}
