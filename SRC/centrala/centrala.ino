#include <RCSwitch.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <CSV_Parser.h>
#include <ESPmDNS.h>


//Parametry pro připojení k síti
const char* ssid = "esp_wifi";          //Do uvozovek napište SSID Vaší wifi sítě
const char* password = "keplerprojekt"; //Mezi uvozovky napište heslo sítě

//Heslo pro administracni rozhrani
const char* http_username = "admin"; //Vyplňte jméno uživatele pro přístup do administračního rozhraní
const char* http_password = "admin"; //Vyplňte své heslo

//Parametry pro HTTP GET requesty
const char* KOD_ZAPNUTO = "zapnuto";  
const char* KOD_VYPNUTO = "vypnuto"; 
const char* NAZEV = "nazev"; 
const char* AKCE = "akce";
const char* KOD = "kod";

//Deklarace pro CSV parser, funkce readFile sem načte soubor
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
  char **kod_zapnuto = (char**)cp["kod_zapnuto" ];
  char **kod_vypnuto = (char**)cp["kod_vypnuto"];

  //cyklus zajišťující přiřazení tlačítka k prvkům ze souboru
  if (nazev && kod_zapnuto && kod_vypnuto) {
    for (int row = 0; row < cp.getRowsCount(); row++) {
      //debug, zobrazení, co je v souboru
      Serial.print(nazev[row]);          Serial.print(" - ");
      Serial.print(kod_zapnuto[row]);    Serial.print(" - ");
      Serial.print(kod_vypnuto[row]);   Serial.print(" - ");
      
      //Sestavení HTML pro placeholder <ESP_IP>/get?
      //Nadpis pro tlačítko (název zařízení)
      tlacitka += "<h3>";
      tlacitka += (nazev)[row];
      tlacitka += "</h3>";
      
      //Zapnout tlačítko
      tlacitka += "<p><button class='button button1' name='kod' type='submit' value='";
      tlacitka += (kod_zapnuto[row]);
      tlacitka += "'>ZAPNOUT</button>";
      
      //Vypnout tlačítko
      tlacitka += "<button class ='button button2'name='kod' type='submit' value='";
      tlacitka += (kod_vypnuto[row]);
      tlacitka += "'>VYPNOUT</button>";
    }
  } else {
    Serial.println("Alespoň jeden sloupec nenalezen, chyba");
  }
  //Vrací string s tlačítko pro vypsání na stránku
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
  Serial.begin(115200);
  Serial.println();

  //připojení souborového svazku
  if (!SPIFFS.begin()) {
    Serial.println("Stala se chyba při připojování SPIFFS");
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
    Serial.println("Chyba při spouštění mDNS");
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

  //Načte obsah souboru do csv_str
  readFile(SPIFFS, "/zarizeni.csv");
  
  // Odešle HTTP GET request na <ESP_IP>/get
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputNazev;
    String inputZapnuto;
    String inputVypnuto;
    String inputAkce;
    String inputKod;
    //Zápis do souboru z uživatelského rozhraní, komerční zařízení
    //Získej hodnoty z <ESP_IP>/get?nazev=<inputNazev>&ovladac=<inputOvladac>&zarizeni=<inputZarizeni>
    if (request->hasParam(KOD_ZAPNUTO) && request->hasParam(KOD_VYPNUTO) && request->hasParam(NAZEV)) {
      inputNazev = request->getParam(NAZEV)->value();
      inputZapnuto = request->getParam(KOD_ZAPNUTO)->value();
      inputVypnuto = request->getParam(KOD_VYPNUTO)->value();
      inputMessage += inputNazev + "," + inputZapnuto + "," + inputVypnuto;
      addRow(SPIFFS, "/zarizeni.csv", inputMessage.c_str());
      readFile(SPIFFS, "/zarizeni.csv");                                     
    }

    //Zpracování requestu pro ovládání zařízení
    //Získej hodnoty z <ESP_IP>/get?kod=<kod_zapnuto/kod_vypnuto>
    if (request->hasParam(KOD)) {
      inputKod = request->getParam(KOD)->value();
      int kod = inputKod.toInt();
      Serial.println("Odeslán kód");
      Serial.print(kod);
      mySwitch.send(kod, 24);
    }

    else {
      inputMessage = "Nebyla odeslána žádná zpráva";
    }
    Serial.println(inputMessage);
    request->send(200);
  });
}

void loop() {
}
