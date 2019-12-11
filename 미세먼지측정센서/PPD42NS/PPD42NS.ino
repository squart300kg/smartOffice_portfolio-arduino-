#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;

int pin = 4;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float ugm3 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(pin,INPUT);
  starttime = millis();

  //와이파이 접속 설정
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("teamnova05_2G", "05teamnova8911");
}

void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    ugm3= concentration * 100 /13000;
    Serial.println(" ");
    Serial.print("미세먼지량(ugm3) = " );
    Serial.print(ugm3);
    Serial.println("ug/m3" );

    lowpulseoccupancy = 0;
    starttime = millis();
    if((WiFiMulti.run() == WL_CONNECTED)){
      HTTPClient http;//HTTP통신 준비

      //HTTP통신 시작
      http.begin("http://54.180.2.52/PPD42NS.php?ugm3="+String(ugm3)); //HTTP 
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
        Serial.printf("[HTTP] 통신코드(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end();  
    }
  }
} 
