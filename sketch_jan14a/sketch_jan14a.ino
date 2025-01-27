#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <EEPROM.h>

// Wi-Fi credentials
#define WIFI_SSID "I'am Shadow!"
#define WIFI_PASSWORD "open2255"

// Firebase setup
#define FIREBASE_HOST "your-database-name.firebaseio.com" // Replace with your Firebase URL
#define FIREBASE_AUTH "your-firebase-auth-key"           // Replace with your Firebase Secret

// Sensor connections
#define SENSOR_DIGITAL_PIN D0 // GPIO16
#define SENSOR_ANALOG_PIN D2  // GPIO4

// LED pin
#define LED_PIN D4 // GPIO2 (LED บนบอร์ด ESP8266)

// Timer variables
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerRunning = false;

// Send data variables
int sendCount = 0; // Counter for sending data

// Firebase object
FirebaseData firebaseData;

// Moving average variables for analog sensor
const int numReadings = 10;
int readings[numReadings];
int index = 0;
int total = 0;

// Function prototypes
void connectToWiFi();
void connectToFirebase();
void checkWiFiConnection();
void checkFirebaseConnection();
void checkSensorConnection();
void indicateError(int errorCode);
unsigned long calculateDuration(unsigned long start, unsigned long end);
int readAnalogSensor();
void sendDataToFirebase(unsigned long timerDuration, int digitalValue, int analogValue);

// ฟังก์ชันเชื่อมต่อ Wi-Fi
void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
}

// ฟังก์ชันเชื่อมต่อ Firebase
void connectToFirebase() {
  Serial.println("Connecting to Firebase...");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if (!Firebase.ready()) {
    Serial.println("Failed to connect to Firebase!");
    indicateError(2); // กระพริบ 2 ครั้ง
    while (true); // หยุดการทำงาน
  }
  Serial.println("Connected to Firebase");
}

// ฟังก์ชันตรวจสอบการเชื่อมต่อ Wi-Fi
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    indicateError(1); // กระพริบ 1 ครั้ง
    connectToWiFi();
  }
}

// ฟังก์ชันตรวจสอบการเชื่อมต่อ Firebase
void checkFirebaseConnection() {
  if (!Firebase.ready()) {
    Serial.println("Firebase disconnected. Reconnecting...");
    indicateError(2); // กระพริบ 2 ครั้ง
    connectToFirebase();
  }
}

// ฟังก์ชันตรวจสอบเซ็นเซอร์
void checkSensorConnection() {
  if (digitalRead(SENSOR_DIGITAL_PIN) == HIGH && analogRead(SENSOR_ANALOG_PIN) == 0) {
    Serial.println("Sensor not connected!");
    indicateError(3); // กระพริบ 3 ครั้ง
    while (true); // หยุดการทำงาน
  }
}

// ฟังก์ชันแจ้งเตือนด้วย LED
void indicateError(int errorCode) {
  for (int i = 0; i < errorCode; i++) {
    digitalWrite(LED_PIN, LOW); // LED ติด (Active Low)
    delay(500); // กระพริบทุก 500 มิลลิวินาที
    digitalWrite(LED_PIN, HIGH); // LED ดับ
    delay(500);
  }
}

// ฟังก์ชันคำนวณระยะเวลา (จัดการการล้นของ millis())
unsigned long calculateDuration(unsigned long start, unsigned long end) {
  if (end < start) {
    // Handle overflow
    return (ULONG_MAX - start) + end;
  }
  return end - start;
}

// ฟังก์ชันอ่านค่าเซ็นเซอร์แอนะล็อก (ใช้ moving average)
int readAnalogSensor() {
  total = total - readings[index];
  readings[index] = analogRead(SENSOR_ANALOG_PIN);
  total = total + readings[index];
  index = (index + 1) % numReadings;
  return total / numReadings;
}

// ฟังก์ชันส่งข้อมูลไปยัง Firebase
void sendDataToFirebase(unsigned long timerDuration, int digitalValue, int analogValue) {
  // Create a JSON object to store the data
  FirebaseJson json;
  json.set("digital_value", digitalValue);
  json.set("analog_value", analogValue);
  json.set("timer_duration_ms", timerDuration);

  // Send the JSON data to Firebase
  String deviceID = "ESP8266_01";
  String firebasePath = "/sensor_data/" + deviceID;
  if (Firebase.pushJSON(firebaseData, firebasePath, json)) {
    Serial.println("Data sent to Firebase successfully!");
  } else {
    Serial.print("Failed to send data to Firebase: ");
    Serial.println(firebaseData.errorReason());
  }
}

void setup() {
  // ตั้งค่า Serial และ EEPROM
  Serial.begin(115200);
  EEPROM.begin(512);

  // ตั้งค่า LED_PIN เป็น OUTPUT
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // ปิด LED เริ่มต้น

  // ตั้งค่าเซ็นเซอร์
  pinMode(SENSOR_DIGITAL_PIN, INPUT);
  pinMode(SENSOR_ANALOG_PIN, INPUT);

  // เชื่อมต่อ Wi-Fi และ Firebase
  connectToWiFi();
  connectToFirebase();

  // ตรวจสอบเซ็นเซอร์
  checkSensorConnection();

  Serial.println("System initialized successfully!");
}

void loop() {
  // ตรวจสอบการเชื่อมต่อ Wi-Fi และ Firebase
  checkWiFiConnection();
  checkFirebaseConnection();

  // อ่านค่าจากเซ็นเซอร์
  int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);
  int analogValue = readAnalogSensor();

  // จับเวลาและส่งข้อมูลไปยัง Firebase
  if (digitalValue == LOW) {
    if (!timerRunning) {
      startTime = millis();
      timerRunning = true;
      sendCount = 0; // รีเซ็ตตัวนับเมื่อเริ่มจับเวลาใหม่
    }
  } else {
    if (timerRunning) {
      duration = calculateDuration(startTime, millis());
      timerRunning = false;

      if (sendCount < 3) {
        sendDataToFirebase(duration, digitalValue, analogValue);
        sendCount++;
      }
    }
  }

  // ดีเลย์เพื่อความเสถียร
  delay(500);
}