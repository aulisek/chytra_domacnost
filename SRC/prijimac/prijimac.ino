#include <RCSwitch.h>

// Definování pinů
#define LEDpin 13          // pin s LED ukazující status
#define RELEpin 4          // pin připojené relé
#define PIRpin 5           // pin připojený PIR senzor
#define TLACITKOpin 3      // pin s tlačítkem (musí být #2/#3 kvůli přerušení)
#define FOTOREZISTORpin A0 // pin s fotorezistorem, analogový!    

//"debouncing" po stisku tlačítka
#define PRODLEVA 50        // prodleva pro čtení tlačítka v milisekundách

int PIRstav = LOW;         // inicializujeme počáteční stav jako vypnuto (načítá stav PIR senzoru)
int hodnota = 0;           // proměná pro čtení hodnoty na senzoru PIR

int soucasny_stav = LOW;   // současný stav (mód ve kterém pracujeme) čidlo/(zap/vyp)

// Určování světla/tmy
int fotorezistor_hodnota;  // Sem se načítá hodnota analogového vstupu
#define odchylka 10        // Odchylka měření (aby nepřepínalo při každé změně na senzoru)
#define hranice_svetlo 150 // Napište, při jaké hodnotě na fotorezistoru chcete zapínat PIR (tma)
int tma;                   // Zapíše jestli je tma -> 1

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
  pinMode(FOTOREZISTORpin, INPUT);
  
  mySwitch.enableReceive(0);  // Přijímač je na pinu přerušení 0 -> arduino uno pin #2

// Zahájení komunikace po sériové lince (v monitoru potřeba nastavit správnou rychlost)
  Serial.begin(9600);
}

void loop() {
  // Načtení napětí na vstupu s fotorezistorem
  fotorezistor_hodnota = analogRead(FOTOREZISTORpin);
  // Určuje, jestli je světlo/tma za použití hystereze
  if(fotorezistor_hodnota < (hranice_svetlo - odchylka)){
    tma = 1;
    } 
  else if(fotorezistor_hodnota > (hranice_svetlo + odchylka)){
    tma = 0;
    }
  //Serial.println(fotorezistor_hodnota); // Zahltí konsoli, jen při debug
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
      digitalWrite(LEDpin, HIGH);  // zapne se signalizační dioda
      Serial.println("led zapnuta");
      digitalWrite(RELEpin, LOW);  // zapne relé 
      blokovani = true;            // nastaví se příznak
      mySwitch.resetAvailable();
    }
    else if (hodnota == 1364) {
      Serial.println("rele vypnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      digitalWrite(LEDpin, LOW);    // vypne se signalizační dioda
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
    // Zároveň musí být splněna podmínka tmy (detekováno fotorezistorem)
    if (hodnota == HIGH && tma == 1) {
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
