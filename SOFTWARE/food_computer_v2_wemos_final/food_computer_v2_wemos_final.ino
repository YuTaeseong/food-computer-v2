/*
author : Yu Taeseong
릴레이의 경우 LOW가 켜는 것입니다!
센서의 정밀도에 따라 소스를 변경해서 사용하세요.
*/

//DHT11 센서를 사용하기 위해 라이브러리 사용
#include "DHT.h" 
#include <Arduino.h>
//ESP8266을 통해 WiFi 기능을 사용하기 위해 라이브러리 포함
#include <ESP8266WiFi.h> 
#include <ESP8266WiFiMulti.h>

//wemos는 아두이노와 핀 배치가 달라서 GPIO핀을 기준으로 설정해야함.
#define humidity 14
#define DHTPIN 5
#define DHTTYPE DHT11
#define air_circulation 16
#define vent_fan 4
#define heater_fan_A 0
#define heater_fan_B 13
#define heater 12
#define water_level_sensor A0

//DHT11 사용하기 위한 준비
DHT dht(DHTPIN, DHTTYPE);
//WiFiMulti 기능을 사용하기 위한 준비
ESP8266WiFiMulti WiFiMulti;

//온도와 습도, 수위를 사용자가 원하는 값으로 설정
int16_t water_level = 780;
int16_t temperature = 26;
int16_t humidity_v = 50;

//와이파이 설정
const int8_t * wifi_id = "hogwart";
const int8_t * wifi_psd = "iamwizard";

//웹 주소와 포트 설정
const int8_t * host = "117.123.33.177";

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
  
  //아두이노와 달리 115200 속도로 통신
  Serial.begin(115200);

  for(uint8_t t=4; t > 0; t--) {
    Serial.printf("setting!, please wait! %d...\n",t);
    Serial.flush();//데이터 전송이 완료될 때까지 기다림
    delay(1000);
  }
  
  //wifimulti 시작
  WiFiMulti.addAP(wifi_id, wifi_psd);
  
}

void loop ()
{ 
  String time_data = "";
  
  if (WiFiMulti.run() == WL_CONNECTED){
    
    const uint16_t port = 5000;
    
    Serial.print("connecting to ");
    Serial.println(host);

    // WiFiClient 사용해서 TCP/IP 통신을 한다.
    WiFiClient clients;
    if (!clients.connect(host, port)) {
        Serial.println("connection failed");
        Serial.println("wait 5 sec...");
        delay(5000);
        return;
    }
    
    // 올린 웹에서 시간 데이터를 얻어온다.
    clients.print("GET /time HTTP/1.1\r\n\r\n");
    uint32_t timeout = millis() + 5000;
    //스트림에 데이터가 들어올 때까지 5000밀리초 기다린다.
    while (clients.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        clients.stop();
        return;
      }
    }
    
    //스트림의 데이터가 다 읽힐 때까지 실행한다.
    while(clients.available()) {
      String line = clients.readStringUntil('\r');
      if ((line.indexOf("\"date\"")) == 1){
        time_data = line;
        Serial.println(line);
        Serial.println("closing connection");
        clients.stop();
      }
    }
  }
  
  Serial.println(time_data);
  
  float h = 0;
  float t = 0;
  float w = 0;

  for(int i = 0; i < 10; i++){
      delay(500);
      h += dht.readHumidity();
      t += dht.readTemperature();
      w += analogRead(water_level_sensor);
  }

  h = h/10;
  t = t/10;
  w = w/10;

  String w_send = "False";

  //습도 센서는 물이 닿으면 값이 내려간다. 즉 값이 낮으면 물이 존재한다.(LOW == True)
  if(High_or_Low(w,'w') == "High"){
    w_send = "False";
    } else if(High_or_Low(w,'w') == "Low"){
    w_send = "True";
    }
      
  //데이터를 json 형태로 바꿈
  String data = "{\"temperature\" : " + String(t) + ", \"humidity\" : " + String(h)+ ", \"waterlevel\" : \""+ w_send + "\" ,"+ time_data + "}";
  Serial.println(data);
  
  if(WiFiMulti.run() == WL_CONNECTED){
     
    const uint16_t port = 5984;

    Serial.print("connecting to ");
    Serial.println(host);
    
    WiFiClient client;
    
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        Serial.println("wait 5 sec...");
        delay(5000);
        return;
    } 
    
    client.print("POST /food_computer HTTP/1.1\r\n");
    client.print("Host: 117.123.33.177:5984\r\n");
    client.print("Content-Type:application/json\r\n");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\r\n\r\n");
    client.print(data);
    
    uint32_t timeout = millis() + 5000;
    //스트림에 데이터가 들어올 때까지 5000밀리초 기다린다.
    while (clients.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        clients.stop();
        return;
      }
    }
    
    //스트림의 데이터가 다 읽힐 때까지 실행한다.
    while(clients.available()) {
      String line = clients.readStringUntil('\r');
      Serial.println(line);
      Serial.println("closing connection");
      clients.stop();
      }
    }
  }
  
  //센싱한 값을 출력한다.
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  Serial.print("water level: ");
  Serial.println(w);

  //온도 데이터를 바탕으로 히터 제어
  if(High_or_Low(t,'t') == "High") {
    heater_off();
    } else if(High_or_Low(t,'t') == "Low"){
    heater_on();
    }

  //습도 데이터를 바탕으로 습도 제어
  if(High_or_Low(h,'h') == "High") {
    humidity_off();
    } else if(High_or_Low(h,'h') == "Low"){
    humidity_on();
    }
}

void air_circulation_fan_on(){
  digitalWrite(air_circulation, LOW);
  }
  
void air_circulation_fan_off(){
  digitalWrite(air_circulation, LOW);
  }
  
void humidity_on(){
  digitalWrite(LOW);
  }
  
void humidity_off(){
  digitalWrite(LOW);
  }
  
void heater_on(){
  digitalWrite (heater_fan_A, LOW);
  digitalWrite (heater_fan_B, HIGH);
  digitalWrite (heater, LOW);
  digitalWrite (vent_fan, HIGH);
  }

void heater_off(){
  digitalWrite (heater_fan_A, LOW);
  digitalWrite (heater_fan_B, LOW);
  digitalWrite (heater, HIGH);
  digitalWrite (vent_fan, LOW);
  }

String High_or_Low(float data, char * data_type){
  int16_t threshold;
  if (data_type == 'h'){
    threshold = humidity_v;
    } else if (data_type == 'w') {
    threshold = water_level;
    } else if (data_type == 't') {
    threshold = temperature;
    }
    
  if (data >= threshold){
    return "High";
    } else if(data < threshold){
    return "Low";  
    }
  }
 

