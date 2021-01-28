/*
  Simple example for receiving
  
  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pinMode(3, OUTPUT);
}

void loop() {
  if (mySwitch.available()) {

    unsigned long hodnota = mySwitch.getReceivedValue();

    if (hodnota == 1361) {
      Serial.print("rele zapnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
      digitalWrite(3, HIGH);
    }
    else if (hodnota == 1364) {
      Serial.print("rele vypnuto");
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
      digitalWrite(3, LOW);
      
    }
    else {
      digitalWrite(3, LOW);
      Serial.print("nic se nestalo");
    }
  }
}
