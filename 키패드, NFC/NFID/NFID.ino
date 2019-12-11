// 라이브러리 해더 
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>  // 서보 라이브러리
#include <Keypad.h>
Servo myservo;  

// SS(Chip Select)과 RST(Reset) 핀 설정
// 나머지 PIN은 SPI 라이브러리를 사용하기에 별도의 설정이 필요없다.
#define SS_PIN D4
#define RST_PIN D8
#define SERVO_PIN D2

 /* 등록된 RF CARD ID */
#define PICC_0 0x76
#define PICC_1 0x97
#define PICC_2 0xD6
#define PICC_3 0x1A


// 라이브러리 생성
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

//이전 ID와 비교하기위한 변수
byte nuidPICC[4];

char* secretCode = "1234";  // 비밀번호를 설정(여기선 1234)
int position = 0; 
int wrong = 0;
// 비밀번호 비교시 쓸 변수 선언(맞는 경우와 틀린경우 2가지)

#define rows 4
#define cols 4
// 키패드의 행, 열의 갯수

char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// 키패드 버튼 위치 설정

bytebytebyte rowPins[rows] = {D15, D14, D13, D12};
byte colPins[cols] = {D11, D10, D9};
// 키패드에 연결된 핀번호 설정(데이터 시트 참고)

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);









void setup() { 
  Serial.begin(115200);
  SPI.begin(); // SPI 시작
  rfid.PCD_Init(); // RFID 시작

  myservo.attach(SERVO_PIN); //서보 시작
  myservo.write(0); //초기 서보 모터를 0도로 위치 시킴

  //초기 키 ID 초기화
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() { 







  
  char key = keypad.getKey(); // 키패드에서 입력된 값을 가져옵니다.

  if((key >= '0' && key <= '9') || (key >= 'A' && key <='D') || (key == '*' || key == '#')){
  // 키패드에서 입력된 값을 조사하여 맞게 입력된 값일 경우(키패드에 있는 버튼이 맞을경우) 비교

    if(key == '*' || key == '#'){ // *, # 버튼을 눌렀을 경우
      position = 0; 
      wrong = 0; // 입력 초기화
      setLocked(true); // 잠금상태로 세팅
    } 
 
    else if(key == secretCode[position]){ // 해당 자리에 맞는 비밀번호가 입력됬을 경우
      position++; // 다음 자리로 넘어 감
      wrong = 0; // 비밀번호 오류 값을 0으로 만듬
    }
    
    else if(key != secretCode[position]){ // 해당 자리에 맞지 않는 비밀번호가 입력됬을 경우
      position = 0; // 비밀번호를 맞았을 경우를 0으로 만듬
      setLocked(true); // 잠금상태로 세팅
      wrong++; // 비밀번호 오류 값을 늘려준다
    }
  
    if(position == 4){ // 4자리 비밀번호가 모두 맞았을 경우
      setLocked(false); // 잠금상태를 해제 함
    }
    
    if(wrong == 4){ // 비밀번호 오류를 4번 했을 경우
//      blink(); // Red LED를 깜빡여 준다.
      wrong = 0; // 비밀번호 오류 값을 0으로 만들어 준다.
    }
  }








  

  // 카드가 인식되었다면 다음으로 넘어가고 아니면 더이상 
  // 실행 안하고 리턴
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // ID가 읽혀졌다면 다음으로 넘어가고 아니면 더이상 
  // 실행 안하고 리턴
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));

  //카드의 타입을 읽어온다.
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  //모니터에 출력
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // MIFARE 방식인지 확인하고 아니면 리턴
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  // 만약 바로 전에 인식한 RF 카드와 다르다면..
  if (rfid.uid.uidByte[0] == PICC_0 || 
      rfid.uid.uidByte[1] == PICC_1 || 
      rfid.uid.uidByte[2] == PICC_2 || 
      rfid.uid.uidByte[3] == PICC_3 ) {
    Serial.println(F("This is a confirmed Card."));   
    Serial.println(F("Motor On!!"));   

    //서보 90도로 이동
    myservo.write(90);
    //1초 대기
    delay(1000);
    //서보 0도로 되돌림
    myservo.write(0);
    
    Serial.println(F("A new card has been detected."));

//    // ID를 저장해둔다.    
//    for (byte i = 0; i < 4; i++) {
//      nuidPICC[i] = rfid.uid.uidByte[i];
//    }
//   
//    //모니터 출력
//    Serial.println(F("The NUID tag is:"));
//
//    Serial.print(F("In hex: "));
//    //16진수로 변환해서 출력
//    printHex(rfid.uid.uidByte, rfid.uid.size);
//    Serial.println();
//
//
//    Serial.print(F("In dec: "));
//    //10진수로 출력
//    printDec(rfid.uid.uidByte, rfid.uid.size);
//    Serial.println();
  }
  else{
    //등록된 카드가 아니라면 시리얼 모니터로 ID 표시
    Serial.println(F("This is an unconfirmed Card."));
    Serial.print(F("In hex: "));
    Serial.println();
    Serial.println(rfid.uid.uidByte[0]);
    Serial.println(rfid.uid.uidByte[1]);
    Serial.println(rfid.uid.uidByte[2]);
    Serial.println(rfid.uid.uidByte[3]);
    Serial.println(); 
    Serial.println(PICC_0);
    Serial.println(PICC_1);
    Serial.println(PICC_2);
    Serial.println(PICC_3);
//    printHex(rfid.uid.uidByte, rfid.uid.size);  
//    Serial.println(F("Card read previously.")); //바로 전에 인식한 것과 동일하다면 
  }

  // PICC 종료
  rfid.PICC_HaltA();

  // 암호화 종료(?)
  rfid.PCD_StopCrypto1();

  //다시 처음으로 돌아감.
}

void setLocked(int locked){ // 잠금시와 해제시에 맞는 LED를 세팅해 주는 함수
  if(locked) { // 잠금
    //서보 0도로 되돌림
    myservo.write(0);
    
    Serial.println(F("문닫힘"));
  }

  else{ // 해제시 Red LED를 꺼주고, Green LED를 켜준다.
    myservo.write(90);
    
    Serial.println(F("문열림"));
  } 
}

//16진수로 변환하는 함수
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//10진수로 변환하는 함수
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
