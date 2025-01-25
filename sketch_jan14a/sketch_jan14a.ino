#include <ESP8266Wifi.h>
#include <FirebaseClient.h>

#define



// กำหนดขาของเซนเซอร์
#define SENSOR_DIGITAL_PIN D0 // GPIO16
#define SENSOR_ANALOG_PIN D2 // GPIO4 (เปลี่ยนตามบอร์ด)
#define D0 16
#define D2 4
void setup() {
  // ตั้งค่า Serial สำหรับ Debug
  Serial.begin(115200);

  // ตั้งค่าขา SENSOR_DIGITAL_PIN เป็น input
  pinMode(SENSOR_DIGITAL_PIN, INPUT);

  // ข้อความเริ่มต้น
  Serial.println("MH-Sensor Series Test");
}

void loop() {
  // อ่านค่าดิจิทัลจาก SENSOR_DIGITAL_PIN
  int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);

  // อ่านค่าอะนาล็อกจาก SENSOR_ANALOG_PIN
  int analogValue = analogRead(SENSOR_ANALOG_PIN);

  // แสดงค่าบน Serial Monitor
  Serial.print("Digital Value: ");
  Serial.print(digitalValue);
  Serial.print(" | Analog Value: ");
  Serial.println(analogValue);

  // หน่วงเวลา 500 มิลลิวินาที
  delay(100);
}

