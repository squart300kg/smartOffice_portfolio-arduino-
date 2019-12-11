// Include required libraries
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "ESP8266.h"

// Create instances
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(10, 9); // MFRC522 mfrc522(SS_PIN, RST_PIN)
Servo sg90;

#define SSID       "TEAMNOVA_2G"    
#define PASSWORD   "03NOVA8911"   

//#define RX_pin 0
//#define TX_pin 1
//SoftwareSerial Uno_Serial(RX_pin, TX_pin); 
//ESP8266 wifi(ESP_wifi);

#define servoPin 8 
#define magnet_pin 4
#define PIR_pin 6
#define LED 7
#define speaker_pin 5


char initial_password[4] = {'1', '2', '3', '4'};  // Variable to store initial password
String tagUID = "76 97 D6 1A";  // String to store UID of tag. Change it with your tag's UID
char password[4];   // Variable to store users password
boolean RFIDMode = true; // boolean to change modes
char key_pressed = 0; // Variable to store incoming keys
uint8_t i = 0;  // Variable used for counter
  
const byte ROWS = 4;    // 행(rows) 개수
const byte COLS = 4;    // 열(columns) 개수
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
 
byte rowPins[ROWS] = {A0, A1, A2, A3};   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
byte colPins[COLS] = {A4, 3, 2};   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호
 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

boolean normal_visit = false;
boolean isExist = false;

int door_count = 0;
void setup() {
  Serial.begin(115200); 
//  Uno_Serial.begin(9600);  


    
  
  pinMode(LED, OUTPUT);
  pinMode(speaker_pin, OUTPUT);
  pinMode(magnet_pin, INPUT);
  pinMode(PIR_pin, INPUT);
  sg90.attach(servoPin);  //Declare pin 8 for servo
  sg90.write(0); // Set initial position at 90 degrees
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
}
void loop() {

//  while(Uno_Serial.available() > 0)
//  {
//    float val = Uno_Serial.parseFloat();
//    if(Uno_Serial.read() == '\n')
//    {
//      Serial.println(val);
//    }
//  }
  int isDetected = digitalRead(PIR_pin); 
  
  if (RFIDMode == true) {
    char key_pressed = keypad.getKey(); // Storing keys
    Serial.println(key_pressed);
    if (key_pressed)
    {
      password[i++] = key_pressed; // Storing in password variable

    }
    if (i == 4) // If 4 keys are completed
    {
      delay(200);
      if (!(strncmp(password, initial_password, 4))) // If password is matched
      {
 
        Serial.println("문열림");
        sg90.write(90); // Door Opened
        delay(3000);
        sg90.write(0); // Door Closed
        i = 0;
        RFIDMode = true; // Make RFID mode true
        normal_visit = true; //정상적인 경로로 들어왔다.
      }
      else    // If password is not matched
      {
        i = 0;
        RFIDMode = true;  // Make RFID mode true
        
      }
    }
  }
   
  //문이 열렸다.
  if(digitalRead(magnet_pin) == LOW){
    Serial.println("문열림");
    
    if(normal_visit){
      //정상적인 경로로 들어왔다.

      //1. 움직임 감지? => 불을 켜준다.
      //2. 경보음 ON
      Serial.println("=============정상출입====================");
      Serial.println("1. 움직임 감지? => 불을 켜준다."); 
      Serial.println("2. 경보음 OFF");
      delay(3000);

      if (isDetected == HIGH) {
        //움직임을 감지했다.
        Serial.println("움직임 감지");
        //LCD불을 밝혀주자.
        digitalWrite(LED, HIGH);
        delay(5000); 
        digitalWrite(LED, LOW);
        
        
      }
      else {
        //움직임을 감지하지 않았다.
        Serial.println("감지안함"); 
     }
      //문을 닫는다.
      if(digitalRead(magnet_pin) == HIGH){
        //문이 닫혔으니까 정상경로 유입 변수를 false로
        normal_visit = false;
        Serial.println("문이 닫혔으니까 정상경로 유입 변수를 false로");
        delay(3000);
      }
    } else {
      //비정상적인 경로로 들어왔다.

      //1. 움직임 감지? => 불을 켜주지 않는다.
      //2. 경보음 OFF
      Serial.println("=============비정상출입====================");
      Serial.println("1. 움직임 감지? => 불을 켜주지 않는다.");
      Serial.println("2. 경보음 ON");
      analogWrite(speaker_pin, 32);
      delay(3000);
      analogWrite(speaker_pin, 0);
       
    } 
  } else {
//    Serial.println("문이 닫혀있는 상태");
    
  }
  if (RFIDMode == true) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++)
    {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
      
    }
    tag.toUpperCase();
    Serial.print("카드 ID : ");
    Serial.println(tag);
    Serial.print("등록된 카드 ID : ");
    Serial.println(tagUID);
    //Checking the card
    if (tag.substring(1) == tagUID)
    {


      Serial.println("문열림");
      sg90.write(90); // Door Opened
      delay(3000);
      sg90.write(0); // Door Closed

      
      RFIDMode = true; // Make RFID mode false
      normal_visit = true; //정상적인 경로로 들어왔다.
    }
    else
    {
      // If UID of tag is not matched.

      delay(3000);

    }
  }
  
}
 
