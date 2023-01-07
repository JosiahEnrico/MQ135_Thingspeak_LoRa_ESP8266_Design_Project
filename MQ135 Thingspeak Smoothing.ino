#include <MQ135.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>

#define PARA 116.6020682
#define PARB 2.769034857

String apiKey = "9S0G453GEETBMM30"; // Enter your Write API key from ThingSpeak
//const char *ssid = "Samsung Bulldodger";     // replace with your wifi ssid and wpa2 key
//const char *pass = "kammu seperti jelly";
const char *ssid = "CCIT Group Indonesia";     // replace with your wifi ssid and wpa2 key
const char *pass = "itemitemputih";
const char* server = "api.thingspeak.com";

const int numReadings = 30;
float mqread[numReadings];
float mqavg = 0;
float mqtotal = 0;
int mqindex = 0;
int totalindex = 0;

WiFiClient client;
 
void setup()
{
  Serial.begin(115200);
  delay(10);
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");

  for (int i = 0; i < numReadings; i++) {
      mqread[i] = 0;
  }
}
 
void loop()  {
  //Smoothing Algorithm
  MQ135 gasSensor = MQ135(A0);
  mqtotal = mqtotal - mqread[mqindex];
  mqread[mqindex] = gasSensor.getRZero();
  mqtotal = mqtotal + mqread[mqindex];
  mqindex = mqindex + 1;
  totalindex = totalindex + 1;

  if (mqindex >= numReadings) {
    mqindex = 0;
  }

  mqavg = mqtotal / numReadings;

  //MQ1345 Variable Function
  float air_quality = gasSensor.getPPM();
  float RZero = gasSensor.getRZero();
  float Res = gasSensor.getResistance();
    float NewPPM;
    if (totalindex <= numReadings) {
      NewPPM = PARA * pow((Res/RZero), -PARB);
    } else{
      NewPPM = PARA * pow((Res/mqavg), -PARB);
    }

  Serial.print("Air Quality: ");
  Serial.print(NewPPM);
  Serial.println("  PPM");
  Serial.print("Smoothed RZero: ");
  Serial.println(mqavg);
  Serial.print("RZero: ");
  Serial.println(RZero);
  Serial.println();

  if (client.connect(server, 80)) {// "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(NewPPM);
    postStr +="&field2=";
    postStr += String(mqavg);
    postStr +="&field3=";
    postStr += String(RZero);
    postStr += "\r\n\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    
    Serial.println("Data Send to Thingspeak");
  }

  client.stop();
  Serial.println("Waiting...");
 
  delay(2000);      // thingspeak needs minimum 15 sec delay between updates.
}
