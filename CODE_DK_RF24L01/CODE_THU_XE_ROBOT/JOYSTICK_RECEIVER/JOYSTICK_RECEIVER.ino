#include <SPI.h> // thư viện chuẩn giao tiếp SPI
#include <nRF24L01.h>
#include <RF24.h> // Thư viện nrf24l01
#define enA 3   // chân ena nối với chân số 3 của arduino
#define in1 2  // chân in1 nối với chân số 2 của arduino
#define in2 4  // chân in2 nối với chân số 4 của arduino
#define enB 5  // chân enb nối với chân số 5 của arduino
#define in3 6  // chân in3 nối với chân số 6 của arduino
#define in4 7  // chân in4 nối với chân số 7 của arduino
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001"; // địa chỉ rf
char receivedData[32] = "";
int  xAxis, yAxis;
int motorSpeedA = 0;
int motorSpeedB = 0;
void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}
void loop() {
  if (radio.available()) {   
    radio.read(&receivedData, sizeof(receivedData)); // đọc dữ liệu mà mạch phát gửi tới
    xAxis = atoi(&receivedData[0]); // chuyển đổi dữ liệu sang dạng số nguyên (trục x)
    delay(10);
    radio.read(&receivedData, sizeof(receivedData));
    yAxis = atoi(&receivedData[0]);// chuyển đổi dữ liệu sang dạng số nguyên (trục y)
    delay(10);
  }
  
 
  if (yAxis < 460) {
    // xe chạy lùi khi y < 460
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    motorSpeedA = map(yAxis, 460, 0, 0, 255);// chuyển đổi giá trị analog trục y của joy gửi về tương ứng với tốc độ độ của động cơ ( 460 -- 0 ; 0---- 255)
    motorSpeedB = map(yAxis, 460, 0, 0, 255);
  }
  else if (yAxis > 550) {
    // xe chạy tiến
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    motorSpeedA = map(yAxis, 550, 1023, 0, 255);
    motorSpeedB = map(yAxis, 550, 1023, 0, 255);
  }
   else {
    motorSpeedA = 0;
    motorSpeedB = 0;
  }
  if (xAxis < 460) {
// chuyển đổi giá trị analog trục x của joy gửi về tương ứng với tốc độ độ của động cơ ( 470 -- 0 ; 0---- 255)
    int xMapped = map(xAxis, 460, 0, 0, 255);
    // xe rẻ trái
    motorSpeedA = motorSpeedA + xMapped;
    motorSpeedB = motorSpeedB - xMapped;
     if (motorSpeedA < 0) {
      motorSpeedA = 0;
    }
    if (motorSpeedB > 255) {
      motorSpeedB = 255;
    }
  }
  if (xAxis > 550) {
  // chuyển đổi giá trị analog trục x của joy gửi về tương ứng với tốc độ độ của động cơ ( 550-----1023 ; 0---- 255)
    int xMapped = map(xAxis, 550, 1023, 0, 255);
    // xe rẻ phải
    motorSpeedA = motorSpeedA - xMapped;
    motorSpeedB = motorSpeedB + xMapped;
     if (motorSpeedA > 255) {
      motorSpeedA = 255;
    }
    if (motorSpeedB < 0) {
      motorSpeedB = 0;
    }
  }
  if (motorSpeedA < 70) {
    motorSpeedA = 0;
  }
  if (motorSpeedB < 70) {
    motorSpeedB = 0;
  }
  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB); 
}
