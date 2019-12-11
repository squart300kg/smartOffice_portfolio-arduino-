#include "Adafruit_VL53L0X.h" 
//#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;

#define ssid        "teamnova05_2G"  
#define password    "05teamnova8911"  
//WiFi.mode(WIFI_STA);
//WiFiMulti.addAP(ssid, password);

// address we will assign if dual sensor is present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_LOX1 D2
#define SHT_LOX2 D8

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

int distance1 = 0;
int distance2 = 0;

//중복 입출입을 방지하기 위한 변수 
//0 - 사람 감지 가능
//1 - 사람 감지 불가능
int count_identifier = 0;
/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */
void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);    
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);

  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1);
  }
  delay(10);

  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1);
  }
}

void read_dual_sensors() {
  
  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
  Serial.print("1: ");
  if(measure1.RangeStatus != 4) {     // if not out of range
    distance1 = measure1.RangeMilliMeter;
    if(distance1 == 8191)
    {
      distance1 = 0;
    }
    Serial.print(distance1);
    Serial.print("          ");
  } else {
//    Serial.print("Out of range");
    Serial.print(distance1);
  }
  
  Serial.print("                ");

  // print sensor two reading
  Serial.print("2: ");
  if(measure2.RangeStatus != 4) {
    distance2 = measure2.RangeMilliMeter;
    if(distance2 == 8191)
    {
      distance2 = 0;
    }
    Serial.print(distance2);
  } else {
//    Serial.print("Out of range");
    Serial.print(distance2);
  }
  
  Serial.println();

  //레이저 센서 1번과 2번을 모두 측정했다. 이제 레이저센서의 상태에 따라 사람이 들어왔는지, 아님 나갔는지를 정한다.

  //사람감지를 다시 하기 위해, 입출입 인원수 변수를 0으로 정한다.
  if(distance1 == 0 && distance2 == 0){
    count_identifier = 0;
  }
  
  //사람이 들어왔다.
  if(distance1 > distance2 && distance2 == 0 && count_identifier == 0){
    //사람이 들어왔다는 정보를 DB로 전송
      if((WiFiMulti.run() == WL_CONNECTED)){
      HTTPClient http;//HTTP통신 준비

      //HTTP통신 시작
      http.begin("http://squart300kg.cafe24.com/iot/officeInOut.php?type="+String("in")); //HTTP 
      //통신 결과값 수신
      int httpCode = http.GET();

      //통신을 했다면!
      if(httpCode > 0){
        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          //한사림이 들어갔다.
          count_identifier = 1;
          }
      }else{ 
        //통신 에러
        Serial.printf("[HTTP] 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end(); 
      
      } 
  }

  //사람이 나갔다.
  if(distance1 < distance2 && distance1 == 0 && count_identifier == 0){
    //사람이 나갔다는 정보를 DB로 전송
    if((WiFiMulti.run() == WL_CONNECTED)){
      HTTPClient http;//HTTP통신 준비

      //HTTP통신 시작 
      http.begin("http://squart300kg.cafe24.com/iot/officeInOut.php?type="+String("out")); //HTTP 
      //통신 결과값 수신
      int httpCode = http.GET();

      //통신을 했다면!
      if(httpCode > 0){
        Serial.printf("[HTTP] 통신코드(GET): %d\n", httpCode);
        //HTTP 응답을 받음!
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
          //한사림이 들어갔다.
          count_identifier = 1;
          }
      }else{ 
        //통신 에러
        Serial.printf("[HTTP] 통신에러(GET): %s\n", http.errorToString(httpCode).c_str());
      }
      //HTTP통신을 종료한다.
      http.end(); 
    }  
  }
  distance1 = 0;
  distance2 = 0;
}

void setup() {
  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) { delay(1); }

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);

  Serial.println("Shutdown pins inited...");

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);

  Serial.println("Both in reset mode...(pins are low)");
  
  
  Serial.println("Starting...");
  
  setID();

  //와이파이 접속 설정
//  ESP8266WiFiMulti WiFiMulti;
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
}

void loop() {
  read_dual_sensors();
//  delay(100);
}
