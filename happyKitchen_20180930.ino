#include <Wire.h>   // i2C 통신을 위한 라이브러리 
#include <LiquidCrystal_I2C.h>  // LCD 1602 I2C용 라이브러리
#include <Servo.h>
#include <SoftwareSerial.h>  //블루투스 라이브러리
#include <DFRobotDFPlayerMini.h>

LiquidCrystal_I2C lcd(0x27,16,2); // 접근주소: 0x3F or 0x27

int servoPin = 9;
int angle = 10; //속도, 현재 angle=92로 두면 정지
Servo servo;

int bluetoothTx = 2;
int bluetoothRx = 3;
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

SoftwareSerial mySoftwareSerial(6,5);  //MP3 시리얼통신용
DFRobotDFPlayerMini myDFPlayer;  

int pin = 8; // 미세먼지
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 5000;      //sampe 30s ;
unsigned long lowpulseoccupancy = 0;      //매 30초마다 결정되는 Low Pusle 시간. 단위:마이크로세컨드 pulseIn 함수를 사용.

float pcsPerCF = 0;
float ugm3 = 0;
float ratio = 0;
float concentration = 0;    //농도


void setup() {
  Serial.begin(9600);
  starttime = millis();       //get the current time;
  
  mySoftwareSerial.begin(9600); // hh
  myDFPlayer.begin(mySoftwareSerial);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);
  
  bluetooth.begin(9600);  
  pinMode(8, INPUT);
  
  lcd.init();                      // LCD 초기화
  lcd.backlight();                // 백라이트 켜기
   
  servo.attach(servoPin);  //attach() 함수를 통해 아두이노에 모터를 장착하도록 한다. 내부인자로는 핀 번호를 대입
  
}

void loop() {

  servo.write(10);
  
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis() - starttime) > sampletime_ms)     //if the sampel time == 30s
  {
    ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    pcsPerCF = concentration * 100;
    ugm3 = pcsPerCF / 13000;
    Serial.print("Dust Density [ug/m3]: ");            // 시리얼 모니터에 미세먼지 값 출력
    Serial.print(ugm3);
    Serial.println("ug/m3");

    lcd.clear();
    lcd.print("Dust:");
    lcd.print(ugm3);
    lcd.print("ug/m3");

    if(ugm3 <= 30.0)
    {       // 대기 중 미세먼지가 좋음 일 때 파란색 출력
      Serial.print("   ");
      Serial.println("good");
      bluetooth.println("good");
     
      lcd.setCursor(0,1);             // 1번째, 2라인
      lcd.print("good");
      myDFPlayer.play(5);
      servo.write(93);  //멈춤
      delay(10);
    }else if(30.0 < ugm3 && ugm3 <= 80.0){      // 대기 중 미세먼지가 보통 일 때 녹색 출력
      Serial.print("   ");
      Serial.println("not bad"); 
      bluetooth.println("not bad");
     
      lcd.setCursor(0,1);             // 1번째, 2라인
      lcd.print("not bad");     
      myDFPlayer.play(4);
      servo.write(87);  //1단계
      delay(10);     
    }else if (80.0 < ugm3 && ugm3 <= 150.0){    // 대기 중 미세먼지가 나쁨 일 때 노란색 출력
      Serial.print("   ");
      Serial.println("not good");  
      bluetooth.println("not good"); 
    
      lcd.setCursor(0,1);             // 1번째, 2라인
      lcd.print("not good");   
      myDFPlayer.play(3);  

      servo.write(85);  //2단계
      delay(10);  
    }else{                                                     // 대기 중 미세먼지가 매우 나쁨 일 때 빨간색 출력
      Serial.print("   ");
      Serial.println("bad");
      bluetooth.println("bad");
    
      lcd.setCursor(0,1);             // 1번째, 2라인
      lcd.print("bad");  
      myDFPlayer.play(2);

      servo.write(10);  //가장 빠름
      delay(10);       
    }
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}
