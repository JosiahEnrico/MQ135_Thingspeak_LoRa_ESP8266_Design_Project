#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

DynamicJsonBuffer jsonBuffer;

const char *ssid      = "ssid";
const char *password  = "password";
int del = 300;

float AQISValue = 0;
float AQICValue = 0;        
float RZeroSValue = 0;        
float RZeroValue = 0;         
float ResValue = 0;        
String sensor_values;

ESP8266WebServer server(80);

void handleSentVar() {
  if (server.hasArg("sensor_reading"))
  {
    sensor_values = server.arg("sensor_reading");
    //Serial.println(sensor_values);
  }
  JsonObject& root = jsonBuffer.parseObject(sensor_values);
  if (root.success()){
    AQISValue           = root["sensor0_reading"].as<float>();
    AQICValue           = root["sensor1_reading"].as<float>();
    RZeroSValue         = root["sensor2_reading"].as<float>();
    RZeroValue          = root["sensor3_reading"].as<float>();
    ResValue            = root["sensor4_reading"].as<float>();
  } else {
    Serial.println("parseObject() failed");
    return;
  }

  Serial.print("ValueRead: ");
  Serial.print(",");
  Serial.print(AQISValue/100);
  Serial.print(",");
  Serial.print(AQICValue/100);
  Serial.print(",");
  Serial.print(RZeroSValue/100);
  Serial.print(",");
  Serial.print(RZeroValue/100);
  Serial.print(",");
  Serial.println(ResValue/100);

  server.send(200, "text/html", "Data received");
}

void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();


  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
  Serial.print("Test Start");
}

void loop() {
  server.handleClient();
  //delay(del);
}
