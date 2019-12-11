#include <SoftwareSerial.h>
#include <TinyGPS.h>

//#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;

#define ssid        "teamnova04_2G"  
#define password    "04teamnova8911"  

#define RXPIN D3
#define TXPIN D2

#define GPSBAUD 9600
  
TinyGPS gps;
 
SoftwareSerial uart_gps(RXPIN, TXPIN);
  
void getgps(TinyGPS &gps);
  
void setup()
{
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  
  Serial.begin(9600);
  //Sets baud rate of your GPS
  uart_gps.begin(GPSBAUD);
  
  Serial.println("");
  Serial.println("GPS Shield QuickStart Example Sketch v12");
  Serial.println("       ...waiting for lock...           ");
  Serial.println("");

  
}
  
void loop()
{
  while(uart_gps.available())     // While there is data on the RX pin...
  {
      int c = uart_gps.read();    // load the data into a variable...
      if(gps.encode(c))      // if there is a new valid sentence...
      {
        getgps(gps);         // then grab the data.
      }   
  }
}
 
// The getgps function will get and print the values we want.
void getgps(TinyGPS &gps)
{
   
  float latitude, longitude; 
  gps.f_get_position(&latitude, &longitude); 

  Serial.print("Latitude : ");
  Serial.println(latitude, 5);
  Serial.print("Longtitude : ");
  Serial.println(longitude, 5);
  Serial.println();
  
//  
//  if((WiFiMulti.run() == WL_CONNECTED)){
//      HTTPClient http;//HTTP통신 준비
//
//      //HTTP통신 시작 
//      http.begin("http://squart300kg.cafe24.com/iot/officeInOut.php?type="+String("out")); //HTTP 
//      //통신 결과값 수신
//      int httpCode = http.GET();
//
//      //통신을 했다면!
//      if(httpCode > 0){
//        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
//        //HTTP 응답을 받음!
//        if(httpCode == HTTP_CODE_OK) {
//          String payload = http.getString();
//          Serial.println(payload); 
//          }
//      }else{ 
//        //통신 에러
//        Serial.printf("[HTTP] 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
//      }
//      //HTTP통신을 종료한다.
//      http.end(); 
//    }  
    
  delay(10000);
}
 
