// Knihovna pro ovládání po rádiu
#include <RCSwitch.h>

// Definování pinů
#define LEDpin 13          // pin s interní LED ukazující status
#define LEDPIRpin 7        // pin s LED ukazující status v krabicce s PIR senzorem
#define RELEpin 4          // pin připojené relé
#define PIRpin 5           // pin připojený PIR senzor
#define TLACITKOpin 3      // pin s tlačítkem (musí být #2/#3 kvůli přerušení)
#define FOTOMODULpin 6     // pin s modulem na hladinu světla    

//"debouncing" po stisku tlačítka
#define PRODLEVA 50        // prodleva pro čtení tlačítka v milisekundách

int PIRstav = LOW;         // inicializujeme počáteční stav jako vypnuto (načítá stav PIR senzoru)
int hodnota = 0;           // proměná pro čtení hodnoty na senzoru PIR

int soucasny_stav = LOW;   // současný stav (mód ve kterém pracujeme) čidlo/(zap/vyp)

// Inicializujeme, jestli je aktivní čtení z čidla pohybu PIR (trvale zapnuto = true)
volatile boolean blokovani = false;       

//Uložení času změny stavu tlačítka
unsigned long cas_zmeny = 0;

RCSwitch mySwitch = RCSwitch();

// Funkce, která přepíná stav trvale zapnuto/čtení ze senzoru (invertuje stav)
void blokovani_pohybu() {
  // nereaguje na podněty PIR čidla, blokovani
  if (blokovani) {
    digitalWrite(LEDpin, LOW);    // vypne se signalizační dioda
    digitalWrite(LEDPIRpin, LOW);
    Serial.println("LED vypnuta");
    digitalWrite(RELEpin, HIGH);  // Vypne se relé (obrácená logika -> 0 = zapnuto)
    blokovani = false;            // nastaví se příznak
  }
  else {
    digitalWrite(LEDpin, HIGH);  // zapne se signalizační dioda
    digitalWrite(LEDPIRpin, HIGH);
    Serial.println("led zapnuta");
    digitalWrite(RELEpin, LOW);  // zapne relé 
    blokovani = true;            // nastaví se příznak
  }
}

void setup() {
  pinMode(RELEpin, OUTPUT);       // Nastavení relé jako výstup
  pinMode(PIRpin, INPUT);         // Nastavení PIR senzoru jako výstup
  pinMode(TLACITKOpin, INPUT);    // Nastavení tlačítka jako vstup (zapojeno přes pull-down rezistor -> log1 = zapnuto)
  pinMode(FOTOMODULpin, INPUT);   // Nastavení fotomodulu jako vstup
  pinMode(LEDPIRpin, OUTPUT);     // Nastavení LED u PIR senzoru jako výstup   
  
  mySwitch.enableReceive(0);      // Přijímač je na pinu přerušení 0 -> arduino uno pin #2 

// Zahájení komunikace po sériové lince, rychlost 9 600 baudů (v monitoru potřeba nastavit správnou rychlost)
  Serial.begin(9600);
}

void loop() {
  // Chování při stisku tlačítka
  if (digitalRead(TLACITKOpin) == HIGH) {
    // Porovná stavy a cas (z důvodu kolísání log1 kvůli stavbě tlačítka)
    if (soucasny_stav == LOW && (millis() - cas_zmeny) > PRODLEVA) {
      soucasny_stav = HIGH;
      // Provede funkci (převrátí stav neblokovaného na blokovaný)
      blokovani_pohybu();
      Serial.println("tlačítko stisknuto");
    }
  }
  else {
    // načte do proměnné cas_zmeny aktuální čas od startu programu
    cas_zmeny = millis();
    soucasny_stav = LOW;
  }
  if (mySwitch.available()) {

    int hodnota = mySwitch.getReceivedValue();

    // Hodnota z tlačítka ON v decimálním zápisu
    if (hodnota == 1361) {
      Serial.println("rele zapnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      // Převrátí stav na blokovaný (z funkce blokovani_pohybu)
      digitalWrite(LEDpin, HIGH);  // zapne se signalizační dioda
      digitalWrite(LEDPIRpin, HIGH);
      Serial.println("led zapnuta");
      digitalWrite(RELEpin, LOW);  // zapne relé 
      blokovani = true;            // nastaví se příznak
      mySwitch.resetAvailable();
    }
    // Hodnota z tlačítka OFF v decimálním zápisu
    else if (hodnota == 1364) {
      Serial.println("rele vypnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      // Převrátí stav na blokovaný (z funkce blokovani_pohybu)
      digitalWrite(LEDpin, LOW);    // vypne se signalizační dioda
      digitalWrite(LEDPIRpin, LOW);
      Serial.println("LED vypnuta");
      digitalWrite(RELEpin, HIGH);  // Vypne se relé (obrácená logika -> 0 = zapnuto)
      blokovani = false;            // nastaví se příznak
      mySwitch.resetAvailable();
    }
    else {
      Serial.println("nevysíláme");
      mySwitch.resetAvailable();
    }
  }

// Pracuje pouze pokud nebylo stisknuto tlačíko nebo příkaz po rádiu
// Zajišťuje zapíná na základě pohybu (PIR senzor)
  if (!blokovani) {
    hodnota = digitalRead(PIRpin);  // Přečte výstup PIR senzoru
    // Kontrola, jestli je detekován pohyb
    // Zároveň musí být splněna podmínka tmy (detekováno modulem, log 0 je tma)
    if (hodnota == HIGH && (digitalRead(FOTOMODULpin) == HIGH)) {
      digitalWrite(RELEpin, LOW);  // zapne relé
      //Mění stav čidla v proměnné PIRstav z LOW na HIGH
      if (PIRstav == LOW) {
       Serial.println("Pohyb detekován");
        // Přepisujeme pouze stav pinu, nikoliv soucasny_stav (blokovani...)
        PIRstav = HIGH;
      }
    }
    else {
      digitalWrite(RELEpin, HIGH); // Vypne relé
      // Změní stav proměnné PIRstav na LOW
      if (PIRstav == HIGH) {
        Serial.println("Pohyb skončil");
        // Přepisujeme pouze stav pinu, nikoliv soucasny_stav (blokovani...)
        PIRstav = LOW;
      }
    }
  }
}
