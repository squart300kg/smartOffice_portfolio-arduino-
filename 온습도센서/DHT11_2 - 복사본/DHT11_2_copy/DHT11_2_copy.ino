#include "DHT.h"
#include <Arduino.h>
//#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <string.h>    // strcmp 함수가 선언된 헤더 파일
ESP8266WiFiMulti WiFiMulti;

#define ssid        "teamnova05_2G"  
#define password    "05teamnova8911"  
#define HOST_PORT   7777

#define DHTTYPE DHT11   // DHT 11, DHT시리즈중 11을 선택합니다.
#define DHTPIN D2
 
DHT dht(DHTPIN, DHTTYPE);

int humi_limit = 50;
int temp_limit = 27;

String temp_controll = "off";
String humid_controll = "off";

#define relay_humi D6//가습기를 작동할 릴레이 모듈 핀
#define relay_temp D7//선풍기를 작동할 릴레이 모듈 핀

#define UpdateTime 2000
byte data = '0';
 
void setup() {
  Serial.begin(115200);
  Serial.println("DHTxx test!");

  dht.begin();
  
  pinMode(relay_humi, OUTPUT);
  pinMode(relay_temp, OUTPUT);
 
  //와이파이 접속 설정
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop() {
  delay(UpdateTime);  
  
  float humi = dht.readHumidity();// 습도를 측정합니다.
  float temp = dht.readTemperature();// 온도를 측정합니다.
  float f = dht.readTemperature(true);// 화씨 온도를 측정합니다.

  // 값 읽기에 오류가 있으면 오류를 출력합니다.
  if (isnan(humi) || isnan(temp) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  //온습도값을 DB에 저장
  if((WiFiMulti.run() == WL_CONNECTED)){
    HTTPClient http;//HTTP통신 준비
    //HTTP통신 시작
      http.begin("http://squart300kg.cafe24.com/iot/insert_humid_temp.php?humid="+String(humi)+"&temp="+String(temp));
      //통신 결과값 수신
      int httpCode = http.GET();
      
      //통신을 했다면!
      if(httpCode > 0){
        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          
          }
      }else{ 
        //통신 에러
        Serial.printf("[HTTP] 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end(); 
  }

  //선풍기 상태값을 DB에서 조회
  if((WiFiMulti.run() == WL_CONNECTED)){
    HTTPClient http;//HTTP통신 준비
    //HTTP통신 시작
      http.begin("http://squart300kg.cafe24.com/iot/select_humid_temp_controll.php?application=fan");
      //통신 결과값 수신
      int httpCode = http.GET();
      
      //통신을 했다면!
      if(httpCode > 0){
        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          temp_controll = payload;
          }
      }else{ 
        //통신 에러
        Serial.printf("[HTTP] 선풍기 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end(); 
  }

  //가습기 상태값을 DB에서 조회
  if((WiFiMulti.run() == WL_CONNECTED)){
    HTTPClient http;//HTTP통신 준비
    //HTTP통신 시작
      http.begin("http://squart300kg.cafe24.com/iot/select_humid_temp_controll.php?application=humid");
      //통신 결과값 수신
      int httpCode = http.GET();
      
      //통신을 했다면!
      if(httpCode > 0){
        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          humid_controll = payload; 
          }
      }else{ 
        //통신 에러
        Serial.printf("[HTTP] 가습기 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end(); 
  }

//  // 보정된 화씨 값을 가져옵니다.
//  float hif = dht.computeHeatIndex(f, h);
//  // 보정된 섭씨 값을 가져옵니다.
//  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.println(" *F\t");

///////////////////////////////////////////////////////////////////////
//             HIGH - 기기 OFF          LOW - 기기 ON
///////////////////////////////////////////////////////////////////////
 
//습도가 임계치 미만이다. => 가습기 킨다. 
  if( humi < humi_limit ){
    if( humid_controll.compareTo("off") ){
      //앱으로부터 가습기를 끄라고 지시받았다면 이에 따른다.
      digitalWrite(relay_humi, LOW);
    } else {
      //앱으로부터 가습기를 끄라고 안했다면 정상적으로 켜준다.
      digitalWrite(relay_humi, HIGH);
    }
//    delay(1000);
  }
  //습도가 임계치 이상이다. => 가습기를 끈다.
  if( humi >= humi_limit ){
    if( humid_controll.compareTo("on") ){
    //앱으로부터 가습기를 키라고 지시받았다면 이에 빠른다.
    digitalWrite(relay_humi, HIGH);
    } else {
      //앱으로부터 가습기를 끄라고 지시받았다면 정상적으로 꺼준다.
      digitalWrite(relay_humi, LOW);
    }
//    delay(1000);
  } 

  //온도가 임계치를 넘었다. => 선풍기를 킨다.
  if( temp > temp_limit ){
    if( temp_controll.compareTo("off") ){
      //앱으로부터 선풍기를 끄라고 지시받았다면 이에 따른다.
      digitalWrite(relay_temp, LOW);
      Serial.print("선풍기 제어 값 : ");
      Serial.println(temp_controll);
    } else {
      //앱으로부터 선풍기를 끄라고 지시받았다면 정상적으로 켜준다.
      digitalWrite(relay_temp, HIGH);
    } 
//    delay(1000);
  }
  //온도가 임계치를 미만이다. => 선풍기를 끈다.
  if( temp <= temp_limit ){
    if( temp_controll.compareTo("on") ){
      //앱으로부터 선풍기를 키라고 지시받았다면 이에 따른다.
      digitalWrite(relay_temp, HIGH);
    } else {
      //앱으로부터 성풍기를 끄라고 지시받았다면 정상적으로 꺼준다.
      Serial.print("선풍기 제어 값 : ");
      Serial.println(temp_controll);
      digitalWrite(relay_temp, LOW);
    } 
//    delay(1000);
  }
  delay(2000);
  
}
