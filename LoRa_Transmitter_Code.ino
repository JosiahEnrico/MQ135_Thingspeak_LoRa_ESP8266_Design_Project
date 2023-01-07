#include <MQ135.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

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

//LoRa Pin Setup
#define ss 15
#define rst 16
#define dio0 4

//Variables
int counter = 0;
int del = 500;

void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); 
  
  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);
    if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1);
  }
  for (int i = 0; i < numReadings; i++) {
      mqread[i] = 0;
  }
  delay(10);
}
 
void loop() 
{  
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
  
  digitalWrite(LED_BUILTIN, HIGH);  

  // send packet
  LoRa.beginPacket();
  LoRa.print(F("Pkt No:"));
  LoRa.print(counter);
  LoRa.print(",");
  
  LoRa.print(NewPPM);
  LoRa.print(",");
  LoRa.print(air_quality);
  LoRa.print(",");
  LoRa.print(mqavg);
  LoRa.print(",");
  LoRa.print(RZero);
  LoRa.print(",");
  LoRa.print(Res);
  LoRa.endPacket();

  counter++;
 
  delay(del/2);
  digitalWrite(LED_BUILTIN, LOW);
  delay(del/2); 
}
