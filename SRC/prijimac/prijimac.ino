#include <RCSwitch.h>

#define LEDpin 13          // pin s LED ukazující status
#define RELEpin 4          // pin připojené relé
#define PIRpin 5           // pin připojený PIR senzor
#define TLACITKOpin 3      // pin s tlačítkem (musí být #2/#3 kvůli přerušení)

//"debouncing" po stisku tlačítka
#define PRODLEVA 50        // prodleva pro čtení tlačítka v milisekundách

int PIRstav = LOW;         // inicializujeme počáteční stav jako vypnuto
int hodnota = 0;           // proměná pro čtení hodnoty na senzoru

int soucasny_stav = LOW;   // současný stav (mód ve kterém pracujeme) čidlo/(zap/vyp)

// Inicializujeme, jestli je aktivní čtení z čidla pohybu PIR (trvale zapnuto = true)
volatile boolean blokovani = false;       

//Uložení času změny stavu tlačítka
unsigned long cas_zmeny = 0;

RCSwitch mySwitch = RCSwitch();

// Funkce, která přepíná stav trvale zapnuto/čtení ze senzoru
void blokovani_pohybu() {
  // nereaguje na podněty PIR čidla, blokovani
  if (blokovani) {
    digitalWrite(LEDpin, LOW);    // vypne se signalizační dioda
    Serial.println("LED vypnuta");
    digitalWrite(RELEpin, HIGH);  // Vypne se relé (obrácená logika -> 0 = zapnuto)
    blokovani = false;            // nastaví se příznak
  }
  else {
    digitalWrite(LEDpin, HIGH);  // zapne se signalizační dioda
    Serial.println("led zapnuta");
    digitalWrite(RELEpin, LOW);  // zapne relé 
    blokovani = true;            // nastaví se příznak
  }
}

void setup() {
  pinMode(RELEpin, OUTPUT);   // Nastavení relé jako výstup
  pinMode(PIRpin, INPUT);     // Nastavení PIR senzoru jako výstup
  pinMode(TLACITKOpin, INPUT);// Nastavení tlačítka jako vstup (zapojeno přes pull-down rezistor -> log1 = zapnuto)
  
  mySwitch.enableReceive(0);  // Přijímač je na pinu přerušení 0 -> arduino uno pin #2

// Zahájení komunikace po sériové lince (v monitoru potřeba nastavit správnou rychlost)
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(TLACITKOpin) == HIGH) {
    if (soucasny_stav == LOW && (millis() - cas_zmeny) > PRODLEVA) {
      soucasny_stav = HIGH;
      blokovani_pohybu();
      Serial.println("tlačítko stisknuto");
    }
  }
  else {
    cas_zmeny = millis();
    soucasny_stav = LOW;
  }
  if (mySwitch.available()) {

    int hodnota = mySwitch.getReceivedValue();

    if (hodnota == 1361) {
      Serial.println("rele zapnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      soucasny_stav = HIGH;
      blokovani_pohybu();
      mySwitch.resetAvailable();
    }
    else if (hodnota == 1364) {
      Serial.println("rele vypnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      soucasny_stav = LOW;
      blokovani_pohybu();
      mySwitch.resetAvailable();
    }
    else {
      Serial.println("nevysíláme");
      mySwitch.resetAvailable();
    }
  }

// Pracuje pouze pokud nebylo stisknuto tlačíko nebo příkaz po rádiu
// Zajišťuje zapíná na základě pohybu (PIR senzor)
  if (!blokovani && (analogRead(A0) < 150)) {
    hodnota = digitalRead(PIRpin);  // Přečte výstup PIR senzoru
    // Kontrola, jestli je detekován pohyb
    if (hodnota == HIGH) {
      digitalWrite(RELEpin, LOW);  // zapne relé
      
      if (PIRstav == LOW) {
       Serial.println("Pohyb detekován");
        // Přepisujeme pouze stav pinu, nikoliv soucasny_stav (blokovani...)
        PIRstav = HIGH;
      }
    }
    else {
      digitalWrite(RELEpin, HIGH);
      if (PIRstav == HIGH) {
        Serial.println("Pohyb skončil");
        // Přepisujeme pouze stav pinu, nikoliv soucasny_stav (blokovani...)
        PIRstav = LOW;
      }
    }
  }
}
