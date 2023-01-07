#include <MQ135.h>
#include <ESP8266WiFi.h>

//Parameter from the library
#define PARA 116.6020682
#define PARB 2.769034857

//Smoothing Variables
const int numReadings = 30;
float mqread[numReadings];
float mqavg = 0;
float mqtotal = 0;
int mqindex = 0;
int totalindex = 0;

//Variables
int counter = 0;
int del = 500;

const char *ssid = "ssid";
const char *password = "password";

float AQISValue = 0;
float AQICValue = 0;        
float RZeroSValue = 0;        
float RZeroValue = 0;         
float ResValue = 0;      

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  
  for (int i = 0; i < numReadings; i++) {
      mqread[i] = 0;
  }

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  delay(10);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  
  
  //Smoothing Algorithm
  MQ135 gasSensor = MQ135(A0);
  mqtotal = mqtotal - mqread[mqindex];
  mqread[mqindex] = gasSensor.getRZero();
  mqtotal = mqtotal + mqread[mqindex];
  mqindex = mqindex + 1;

  if (mqindex >= numReadings) {
    mqindex = 0;
  }
  
  mqavg = mqtotal / numReadings;

  float air_quality = gasSensor.getPPM();
  float RZero = gasSensor.getRZero();
  float Res = gasSensor.getResistance();
  float NewPPM;
  if (totalindex <= numReadings) {
    NewPPM = PARA * pow((Res/RZero), -PARB);
  } else{
    NewPPM = PARA * pow((Res/mqavg), -PARB);
  }

  totalindex = totalindex + 1;

  Serial.print("AQI (Smoothed): ");
  Serial.print(NewPPM);
  Serial.println("  PPM");
  Serial.print("AQI (Calibrated): ");
  Serial.print(air_quality);
  Serial.println("  PPM");
  Serial.print("RZero (Smoothed): ");
  Serial.println(mqavg);
  Serial.print("RZero: ");
  Serial.println(RZero);
  Serial.print("Res: ");
  Serial.println(Res);
  Serial.println();
  
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  AQISValue = NewPPM*100;
  AQICValue = air_quality*100;        
  RZeroSValue = mqavg*100;        
  RZeroValue = RZero*100;         
  ResValue = Res*100; 

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.4.1";            //default IP address
  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request. Something like /data/?sensor_reading=123
  
  String url = "/data/";;
  url += "?sensor_reading=";
  url +=  "{\"sensor0_reading\":\"sensor0_value\",\"sensor1_reading\":\"sensor1_value\",\"sensor2_reading\":\"sensor2_value\",\"sensor3_reading\":\"sensor3_value\",\"sensor4_reading\":\"sensor4_value\"}";

  url.replace("sensor0_value", String(AQISValue));
  url.replace("sensor1_value", String(AQICValue));
  url.replace("sensor2_value", String(RZeroSValue));
  url.replace("sensor3_value", String(RZeroValue));
  url.replace("sensor4_value", String(ResValue));
    
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  counter++;
  delay(del/2);
  digitalWrite(LED_BUILTIN, LOW);  
  delay(del/2);
}
