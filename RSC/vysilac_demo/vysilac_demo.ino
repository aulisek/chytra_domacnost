#include <RCSwitch.h>
#include <WiFi.h>

const char* ssid = "****"; //Napište SSID
const char* password = "********"; //Heslo sítě

WiFiServer server(80);

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);
  Serial.println();
  Serial.print("Připojování k:");
  Serial.println(ssid);

  // Zahájení připojení k Wi-Fi
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Připojování");

  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");  
  }

  Serial.println();

  Serial.println("Úspěšně připojeno k WIFI");
  Serial.println("Vaše IP adresa: ");
  Serial.println(WiFi.localIP() );

  server.begin();
  Serial.println("Server je spuštěn");
  
  
  // Vysílač připojen k pinu #23, výchozí protokol 1
  mySwitch.enableTransmit(23);

  // Nastavení počtu opakování vysílání
  mySwitch.setRepeatTransmit(5);


}

void loop() {

 // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("Hello New client");
  
  unsigned long timeout = millis();
  while(!client.available()){
    if (millis() - timeout > 2000) {
      Serial.println("Client Timeout !");
      client.stop();
      return;
    }
  }

   // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);

   // Match the request
 
  int value = LOW;
  if (request.indexOf("/ZAPNUTO") != -1)  {
    mySwitch.send(1361, 24);
    value = HIGH;
  }
  if (request.indexOf("/VYPNUTO") != -1)  {
    mySwitch.send(1364, 24);
    value = LOW;
  }
 
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<body>");
  client.println("<h1>Svetla</h1>");
  client.println("<h2>Svetlo v pokoji</h2>");
 
  client.print("Svetlo je: ");
 
  if(value == HIGH) {
    client.print("ZAPNUTO");
  } else {
    client.print("VYPNUTO");
  }
  client.println("<br><br>");
  client.println("<a href=\"/ZAPNUTO\"\"><button>ZAPNOUT </button></a>");
  client.println("<a href=\"/VYPNUTO\"\"><button>VYPNOUT </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  
}
