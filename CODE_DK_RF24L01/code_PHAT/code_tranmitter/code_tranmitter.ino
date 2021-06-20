#include <SPI.h> // thư viện chuẩn SPI
#include <nRF24L01.h> 
#include <RF24.h>  // Thu viện RF24L01
RF24 radio(8,9); // CE, CSN
const byte address[6] = "00001";// địa chỉ
char xyData[32] = "";
String xAxis, yAxis;
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
void loop() {
  
 xAxis = analogRead(A1); // đọc giá trị analoag trục x của joystick
 yAxis = analogRead(A0); // đọc giá trị analoag trục y của joystick
// gửi giá trị analog của trục x
 xAxis.toCharArray(xyData, 5); 
 radio.write(&xyData, sizeof(xyData)); 
  // gửi giá trị analog của trục y
  yAxis.toCharArray(xyData, 5);
  radio.write(&xyData, sizeof(xyData));
  delay(20);
}
