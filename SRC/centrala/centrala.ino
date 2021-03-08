#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <CSV_Parser.h>
#include <ESPmDNS.h>

//Proměné pro připojení k wifi
const char* ssid = "esp_wifi"; //Napište SSID
const char* password = "keplerprojekt"; //Heslo sítě

//Heslo pro administracni rozhrani
const char* http_username = "admin";
const char* http_password = "admin";

//Parametry pro HTTP GET requesty
const char* KOMERCNI_OVLADAC = "ovladac"; //Parametr pro čtení HTTP GET requestu
const char* KOMERCNI_ZARIZENI = "zarizeni"; //Parametr pro čtení HTTP GET requestu
const char* NAZEV = "nazev"; //Parametr pro čtení HTTP GET requestu
const char* AKCE = "akce";

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
  readFile(SPIFFS, "/zarizeni.csv");
  String tlacitka = "";
  CSV_Parser cp(csv_str,/*format*/ "sss", /*has_header*/ true, /*delimiter*/ ',');
  cp.print();

  char **nazev = (char**)cp["nazev"];
  char **kod_ovladac = (char**)cp["kod_ovladac" ];
  char **kod_zarizeni = (char**)cp["kod_zarizeni"];

  //cyklus zajišťující přiřazení tlačítka k prvkům ze souboru
  if (nazev && kod_ovladac && kod_zarizeni) {
    for (int row = 0; row < cp.getRowsCount(); row++) {
      //debug, zobrazení, co je v souboru
      Serial.print(nazev[row]);          Serial.print(" - ");
      Serial.print(kod_ovladac[row]);    Serial.print(" - ");
      Serial.print(kod_zarizeni[row]);   Serial.print(" - ");
      
      //Sestavení HTML pro placeholder <ESP_IP>/get?ovladac= &zarizeni= &akce=ON/OFF
      //Nadpis pro tlačítko (název zařízení)
      tlacitka += "<h3>";
      tlacitka += (nazev)[row];
      tlacitka += "</h3>";
      
      //Zapnout tlačítko
      tlacitka += "<p><a href='/get?";
      tlacitka += KOMERCNI_OVLADAC;
      tlacitka += "=";
      tlacitka += (kod_ovladac[row]);
      tlacitka += "&";
      tlacitka += KOMERCNI_ZARIZENI;
      tlacitka += "=";
      tlacitka += (kod_zarizeni[row]);
      tlacitka += "&akce=ON";
      tlacitka += "'><button class='button button1'>VYPNOUT</button></a>";
      
      //Vypnout tlačítko
      tlacitka += "<a href='/get?";
      tlacitka += KOMERCNI_OVLADAC;
      tlacitka += "=";
      tlacitka += (kod_ovladac[row]);
      tlacitka += "&";
      tlacitka += KOMERCNI_ZARIZENI;
      tlacitka += "=";
      tlacitka += (kod_zarizeni[row]);
      tlacitka += "&akce=OFF";
      tlacitka += "'><button class='button button2'>VYPNOUT</button></a>";
    }
  } else {
    Serial.println("At least 1 of the columns was not found, something went wrong.");
  }
  return String(tlacitka);
}

//processor -> nahrazuje placeholder na stránce
String processor(const String& var) {
  if (var == "tlacitkasem") {           //Nahrazení placeholderu na stránce (tlacitkasem) vyplní s tlačítky
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

  //nastavení mDNS adresy (dostupné na adrese: http://esp32.local)
  if (!MDNS.begin("esp32")) {
    Serial.println("Error starting mDNS");
    return;
  }
  // Vysílač připojen k pinu #23, výchozí protokol 1
  mySwitch.enableTransmit(23);

  // Nastavení počtu opakování vysílání
  mySwitch.setRepeatTransmit(15);

  //Spuštění serveru
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //stránka pro administraci
  server.on("/admin", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send(SPIFFS, "/admin.html", String(), false);
  });

  //Odhlášení
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(401);
  });

  //Stránka zobrazená po odhlášení
  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/logout.html", String(), false);
  });
  
  server.begin();
  Serial.println("Server je spuštěn");
  
  readFile(SPIFFS, "/zarizeni.csv");
  
  // Odešle HTTP GET request na <ESP_IP>/get
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputNazev;
    String inputOvladac;
    String inputZarizeni;
    String inputAkce;
    //Zápis do souboru z uživatelského rozhraní
    //Získej hodnoty z <ESP_IP>/get?nazev=<inputNazev>&ovladac=<inputOvladac>&zarizeni=<inputZarizeni>
    if (request->hasParam(KOMERCNI_OVLADAC) && request->hasParam(KOMERCNI_ZARIZENI) && request->hasParam(NAZEV)) {
      inputNazev = request->getParam(NAZEV)->value();
      inputOvladac = request->getParam(KOMERCNI_OVLADAC)->value();
      inputZarizeni = request->getParam(KOMERCNI_ZARIZENI)->value();
      inputMessage += inputNazev + "," + inputOvladac + "," + inputZarizeni;
      addRow(SPIFFS, "/zarizeni.csv", inputMessage.c_str());
      readFile(SPIFFS, "/zarizeni.csv");                                     
    }
    //Zpracování requestu pro ovládání komerčních zásuvek
    //Získej hodnoty z <ESP_IP>/get?ovladac=<inputOvladac>&zarizeni=<inputZarizeni>&akce=<ON/OFF>
    if (request->hasParam(KOMERCNI_OVLADAC) && request->hasParam(KOMERCNI_ZARIZENI) && request->hasParam(AKCE)) {
      inputOvladac = request->getParam(KOMERCNI_OVLADAC)->value();
      inputZarizeni = request->getParam(KOMERCNI_ZARIZENI)->value();
      inputAkce = request->getParam(AKCE)->value();
      Serial.println("z tlačítka");
      if (inputOvladac == "x") {
        Serial.print("moje zařízení");
      }
      if (inputAkce == "ON" && inputOvladac != "x") {
        mySwitch.switchOn("inputOvladac", "inputZarizeni");
        Serial.println("odeslán kód zapnuto");
        Serial.println(inputOvladac);
        Serial.println(inputZarizeni);
      }
      if (inputAkce == "OFF" && inputOvladac != "x") {
        mySwitch.switchOff("inputOvladac", "inputZarizeni");
        Serial.println("odeslán kód vypnuto"); 
      }
      request->redirect("/");
   }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200);
  });
}

void loop() {
}
