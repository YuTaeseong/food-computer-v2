#include "DHT.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#define humidity 14
#define DHTPIN 5
#define DHTTYPE DHT11
#define air_circulation 16
#define vent_fan 4
#define heater_fan_A 2
#define heater_fan_B 0
#define heater 12
#define water_level_sensor A0

DHT dht(DHTPIN, DHTTYPE);
ESP8266WiFiMulti WiFiMulti;

int water_level = 450;
int temperature = 26;
int humidity_v = 60;

void setup ()
{
  pinMode(air_circulation, OUTPUT);
  pinMode(vent_fan, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(humidity, OUTPUT);
  
  pinMode(heater_fan_A, OUTPUT);
  pinMode(heater_fan_B, OUTPUT);

  pinMode(water_level_sensor,INPUT);
  dht.begin();
  Serial.begin(115200);

  for(unsigned char t=4; t > 0; t--) {
    Serial.printf("setting!, please wait! %d...\n",t);
    Serial.flush();
    delay(1000);
  }

  WiFiMulti.addAP("hogwart", "youarewizard");
  
}

void loop ()
{ 
  float h = 0;
  float t = 0;
  float w = 0;

  for(int i = 0; i < 10; i++){
      delay(200);
      h += dht.readHumidity();
      t += dht.readTemperature();
      w += analogRead(water_level_sensor);
    }

  h = h/10;
  t = t/10;
  w = w/10;

  String w_send = "False";
  if(w > water_level){
    w_send = "False";
    } else {
      w_send = "True";
      }
      
  String data = "{\"temperature\" : " + String(t) + ", \"humidity\" : " + String(h)+ ", \"waterlevel\" : \""+ w_send + "\" }";
  Serial.println(data);
  if(WiFiMulti.run() == WL_CONNECTED){
      HTTPClient http;

      http.begin("http://admin:password@117.123.33.177:5984/food_computer/");
      http.addHeader("Content-Type", "application/json");
      //int httpcode = http.GET();
      int httpcode = http.POST(data);

      if(httpcode == HTTP_CODE_OK){
          String payload = http.getString();
          Serial.println(payload);
        } else {
          Serial.printf("failed, error: %s\n", httpcode);
          }

      http.end();
    }
  
  //print analog value
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  Serial.print("water level: ");
  Serial.println(w);

  //control heater
  if(t > temperature) {
    digitalWrite (heater_fan_A, LOW);
    digitalWrite (heater_fan_B, LOW);
    digitalWrite (heater, HIGH);
    } else {
      digitalWrite (heater_fan_A, LOW);
      digitalWrite (heater_fan_B, HIGH);
      digitalWrite (heater, LOW);
      }

  //control humidity
  if(h < humidity_v) {
    digitalWrite (humidity, LOW);
    digitalWrite (vent_fan, HIGH);
    } else {
      digitalWrite (humidity, HIGH);
      digitalWrite (vent_fan, LOW);
      }
}

