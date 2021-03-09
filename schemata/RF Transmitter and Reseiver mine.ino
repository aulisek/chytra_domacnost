
/*
   Transmitter 
*/
#include <Manchester.h>

  #define TX_PIN 0  //(5 attiny85)pin where your transmitter is connected
  #define led_PIN 1 //(6 attiny85)
  const int button_PIN = A1; //(7 attiny85)
  const int threshold = 800;
  
  #define button_PIN1 A3 //(2 attiny85)
  //const int threshold1 = 800;
  uint16_t transmit_data = 2761;
  uint16_t transmit_data1 = 0000;
  void setup() {
    pinMode(led_PIN, OUTPUT);
    pinMode(button_PIN, INPUT);
    digitalWrite(button_PIN,LOW);
    pinMode(button_PIN1, INPUT);
    digitalWrite(button_PIN1,LOW);
  man.setupTransmit(TX_PIN, MAN_1200);
  
  } 
void loop() {
 int analogValue = analogRead(button_PIN);
  
  if (analogValue > threshold) {
    digitalWrite(led_PIN, HIGH);
    man.transmit(transmit_data);
  } 
  else {
    digitalWrite(led_PIN,LOW); 
  }
//  int analogValue = analogRead(button_PIN1);
  
  if (analogValue < threshold) {
    digitalWrite(led_PIN, HIGH);
    man.transmit(transmit_data1);
  } 
  else {
    digitalWrite(led_PIN,LOW);}  
 
  
}

/*....................................................................




/*
   Receiver 
*/
#include <Manchester.h>

#define RX_PIN 4 
#define led_PIN 7
#define led1_PIN 8
#define Relay_PIN 10
uint8_t moo = 1;
uint8_t data1;
#define BUFFER_SIZE 10
uint8_t buffer[BUFFER_SIZE];

int incomingData = 0;   // for incoming serial data

void setup()


{
 
pinMode(RX_PIN, INPUT);  
pinMode(led_PIN, OUTPUT);
pinMode(led1_PIN, OUTPUT);
digitalWrite(led_PIN, moo);
pinMode(Relay_PIN, OUTPUT);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE, buffer);
  man.beginReceive();
  
    Serial.begin(9600);  
   //Serial.println( "Receiver is ON" );
  
}

void loop() {
  if (man.receiveComplete()) {
    uint8_t receivedSize = 0;
    
    digitalWrite(Relay_PIN,LOW);
    //digitalWrite(led_PIN,LOW);
    
    incomingData = Serial.read();
    //Serial.print("I received: ");
    Serial.println(incomingData, DEC);
    
    
    man.beginReceiveArray(BUFFER_SIZE, buffer); //start listening for next message 
    moo = ++moo % 2;
    digitalWrite(led_PIN, moo);
    
    digitalWrite(Relay_PIN,HIGH);
    //incomingData = Serial.read();
    //Serial.println(incomingData, DEC);
    Serial.println("data1");
     }
     if (man.receiveComplete()) {
    uint16_t m = man.getMessage();
    digitalWrite(Relay_PIN,LOW);
    man.beginReceive(); //start listening for next message right after you retrieve the message
    moo = ++moo % 2;
    digitalWrite(led1_PIN, moo);
    Serial.println(2761);
    data1 = ++data1 % 2;
   digitalWrite(Relay_PIN,HIGH);
     }  

}