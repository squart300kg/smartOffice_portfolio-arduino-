
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h> 
#include <json/json.h>  
#include "FirebaseESP8266.h" 
#include <SoftwareSerial.h>

//#define RX_pin 5
//#define TX_pin 6
//
//SoftwareSerial esp8266_Serial (RX_pin, TX_pin);
FirebaseData firebaseData; 
String projectId = "https://smartoffice-c4768.firebaseio.com";
String dbPassword = "hKQicTXqEQJYZp1toN0ujTRHNi2vYBvNbxp5y220";

String door_state = "lock";

ESP8266WiFiMulti WiFiMulti;

#define ssid        "teamnova05_2G"  
#define password    "05teamnova8911"  

int speakerPin = D1;
int switchPin = D2;
 
WiFiServer server(80);
WiFiClient client;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
//  esp8266_Serial.begin(9600);
//  
//  pinMode(RX_pin, INPUT);
//  pinMode(TX_pin, OUTPUT);
  
  pinMode(speakerPin, OUTPUT);
  pinMode(switchPin, OUTPUT); 

//  WiFi.mode(WIFI_STA);
//  WiFiMulti.addAP(ssid, password);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    Serial.print(".");
  }
  server.begin();
 
}

void loop() {
// String message = "unlock";
  
//  esp8266_Serial.print(10);
//  esp8266_Serial.print("\n");
  
  int readValue = digitalRead(switchPin);
 

  if(readValue == HIGH){ 
    //초인종을 누름
    analogWrite(speakerPin, 32);

    //노티피케이션 전송
    if((WiFiMulti.run() == WL_CONNECTED)){
      Serial.println("WiFi 연결됨");
 
       sendDataToFirebase();
 
      } else {
//        Serial.println("WIFI 연결 안됨");
      }
    delay(2000);
  } else { 
    //초인종을 안누름
    analogWrite(speakerPin, 0);
  }
}
void sendDataToFirebase() {
    //내꺼 서버키
//    String device_token = "f3gnDbqUf1Y:APA91bGoIDV87MJGUdPiJGVzve_ES2cZunIuqh-Fo_zPijWO2MhvD7sdloOALUFGQOPdG5cQ58YRdQpNRmhnZTq0bJUj-mFOfheDKRuHVipIOvStn_2GFaT2Cnxh-T1-EG1kD6zNfbm6";
//    String server_key = "AAAA3a7MQ2c:APA91bE9GwnMCpTdAsbb2NK-IcVTiguYOzAQ_J-eYSNVJzbnBzSAz1_cXjBa7UsUBjOA2Q_vBvxSJVDFpLjgCe3C3-lQR66C0bF6Ba53mvvgGO7pT0XhF6WUnWH0yNx23s2-TL7NYorP";
    
    //성훈이꺼 서버키
    String device_token = "cNUaKfZRp0g:APA91bEAcMOCPMs2bFtrkORjLNs_WJOKe5aqq0KTI595eMc7OgIvGdlNkh6AiCQ-FtCBYN3SrBEgVAFhESvWXCZS6jO2VM8C5R5JgJIxIVvnBG-RO-QoG0v1URdDvL_fiB_Doc1sxcjR";
    String server_key = "AAAAD5KXSAU:APA91bFyHKufpY3K1GvjnpTbRpgARuhUrgbeOCuW0MlygWkiUdKL5jlna6p3JSTlQytVwtzlW5v_3xVdoKgcSHejq9l7Cyfbf8rTxfJAhBK5MDDuAJUPkdgtBSry5OOEzJeLqfjz2HgI";
    
    String body = "noti_body";
    String title = "noti_title";
    String fcmServer = "fcm.googleapis.com";
// 
//    Serial.println("Send data...");
//    
firebaseData.fcm.begin(server_key);
firebaseData.fcm.addDeviceToken(device_token);
firebaseData.fcm.setPriority("high");
firebaseData.fcm.setTimeToLive(5000);
firebaseData.fcm.setNotifyMessage("초인종", "누군가 방문하였습니다.");
//Set the custom message data
firebaseData.fcm.setDataMessage("{\"myData\":\"bell\"}");
 
if (Firebase.sendMessage(firebaseData, 0))
{
  //Success, print the result returned from server
//  Serial.print("성공");
//  Serial.println(firebaseData.fcm.getSendResult());
  
}
else
{
  //Failed, print the error reason
//  Serial.print("에러");
//  Serial.println(firebaseData.errorReason());
  
}  

}
void door(){
  String payload = "";
  HTTPClient http;//HTTP통신 준비
  if((WiFiMulti.run() == WL_CONNECTED)){
      

      //HTTP통신 시작
      http.begin("http://54.180.2.52/door_state.php"); //HTTP 
      //통신 결과값 수신
      int httpCode = http.GET();

      //통신을 했다면!
      if(httpCode > 0){
//        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          payload = http.getString();
//          Serial.println(payload);
          door_state = payload;
          }
          
      }else{ 
        //통신 에러
//        Serial.printf("[HTTP] 통신코드(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
        
    }
     
}
