#include "DHT.h"

#define humidity 3
#define water_pump_A 4
#define water_pump_B 5
#define light 6
#define DHTPIN 7
#define DHTTYPE DHT11
#define air_circulation 8
#define vent_fan 9
#define heater_fan_A 10
#define heater_fan_B 11
#define heater 12
#define water_level_sensor A0

DHT dht(DHTPIN, DHTTYPE);

int water_level = 450;
int temperature = 26;
int humidity_v = 60;

float h = 0;
float t = 0;
float w = 0;

void setup ()
{
  pinMode(light, OUTPUT);
  pinMode(air_circulation, OUTPUT);
  pinMode(vent_fan, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(humidity, OUTPUT);
  
  pinMode(heater_fan_A, OUTPUT);
  pinMode(heater_fan_B, OUTPUT);

  pinMode(water_level_sensor,INPUT);

  Serial.begin(9600);
  dht.begin();
}

void loop ()
{ 
  for(int i = 0; i < 10; i++){
      delay(200);
      h += dht.readHumidity();
      t += dht.readTemperature();
      w += analogRead(water_level_sensor);
    }

  h = h/10;
  t = t/10;
  w = w/10;
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
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

  //control water
  if(w  ==  water_level){
      digitalWrite(water_pump_A, LOW);
      digitalWrite(water_pump_B, HIGH);

      delay(200);

      digitalWrite(water_pump_A, LOW);
      digitalWrite(water_pump_B, LOW); 
    } else {
      digitalWrite(water_pump_A, LOW);
      digitalWrite(water_pump_B, LOW); 
      }

  //control light
  unsigned long current_time = millis();

  if(current_time % 86400000 <= 36000000){
    digitalWrite (light, LOW); 
    } else {
      digitalWrite (light, HIGH);
    }

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
