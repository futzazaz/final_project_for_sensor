#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <EEPROM.h>

// Wi-Fi credentials

#define WIFI_SSID "Mi 11 Lite"//I'am Shadow!
#define WIFI_PASSWORD "Futzazaz140815"//


// Firebase setup
#define FIREBASE_HOST "driftking-d5a48-default-rtdb.asia-southeast1.firebasedatabase.app" // Replace with Auto, Monica, モニカ・セニオリス, monicaeverett, NunoiEnter, KOKOMO9462, Monica モニカ, Nunoi Enter  (エンタ一), The man who like Todoroki Hajime, ออโต้ขั้นกว่าของมนุษย์, J O R#moo85, Nunoi Enter, monicafansub, Monica FS, ปวช. ออโต้ ภาคกลาง, 💫𝓜𝓸𝓷𝓲𝓬𝓪💫, Momo Firebase Realtime Database URL
#define FIREBASE_AUTH "1oRXFar73k0U8QRnj9kqcoYAeBRUGfXhzxyEWzcE" // Replace with Auto, Monica, モニカ・セニオリス, monicaeverett, NunoiEnter, KOKOMO9462, Monica モニカ, Nunoi Enter  (エンタ一), The man who like Todoroki Hajime, ออโต้ขั้นกว่าของมนุษย์, J O R#moo85, Nunoi Enter, monicafansub, Monica FS, ปวช. ออโต้ ภาคกลาง, 💫𝓜𝓸𝓷𝓲𝓬𝓪💫, Momo Firebase Secret or Web API Key

// Sensor connections
#define SENSOR_DIGITAL_PIN 16 // GPIO16
#define SENSOR_ANALOG_PIN 4  // GPIO4

// LED pin
#define LED_PIN 2 // GPIO2 (LED on ESP8266 board)

// Timer variables
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerRunning = false;

// Firebase object
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Function prototypes
void connectToWiFi();
void connectToFirebase();
void checkWiFiConnection();
void checkFirebaseConnection();
void indicateError(int errorCode);
unsigned long calculateDuration(unsigned long start, unsigned long end);
void sendDataToFirebase(unsigned long timerDuration);
<<<<<<< HEAD
void sendRaceResultToFirebase(int player, unsigned long lap1, unsigned long lap2, unsigned long lap3, unsigned long totalTime);
String formatTime(unsigned long ms);
bool debounceSensor();

// เพิ่มฟังก์ชัน formatTime
String formatTime(unsigned long ms) {
    unsigned long minutes = ms / 60000;
    unsigned long seconds = (ms % 60000) / 1000;
    unsigned long milliseconds = ms % 1000;

    char buffer[15];
    sprintf(buffer, "%02lu:%02lu:%03lu", minutes, seconds, milliseconds);
    return String(buffer);
}
=======
>>>>>>> parent of 27e1899 (succeed code)

void connectToWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println(".");
        indicateError(2); // Blink LED 2 times
    }
    Serial.println("\nConnected to Wi-Fi");
}

void connectToFirebase() {
    Serial.println("Connecting to Firebase...");
    firebaseConfig.host = FIREBASE_HOST;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&firebaseConfig, &firebaseAuth);

    if (!Firebase.ready()) {
        Serial.println("Failed to connect to Firebase!");
        indicateError(1); // Blink LED 1 times
        while (true); // Stop execution
    }
    Serial.println("Connected to Firebase");
}

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi disconnected. Reconnecting...");
        indicateError(3); // Blink 2 time
        connectToWiFi();
    }
}

void checkFirebaseConnection() {
    if (!Firebase.ready()) {
        Serial.println("Firebase disconnected. Reconnecting...");
        indicateError(4); // Blink 3 times
        connectToFirebase();
    }
}

void indicateError(int errorCode) {
    for (int i = 0; i < errorCode; i++) {
        digitalWrite(LED_PIN, HIGH); // LED ON (Active Low)
        delay(500);
        digitalWrite(LED_PIN, LOW); // LED OFF
        delay(500);
    }
}

unsigned long calculateDuration(unsigned long start, unsigned long end) {
    if (end < start) {
        return (ULONG_MAX - start) + end;
    }
    return end - start;
}

void sendDataToFirebase(unsigned long timerDuration) {
    FirebaseJson json;
    json.set("timer_duration_ms", timerDuration);

<<<<<<< HEAD
    int retryCount = 0;
    while (retryCount < 3) { // พยายามส่งข้อมูล 3 ครั้ง
        if (Firebase.setJSON(firebaseData, path, json)) {
            Serial.println("Race result sent to Firebase successfully!");
            return;
        } else {
            Serial.print("Failed to send race result to Firebase: ");
            Serial.println(firebaseData.errorReason());
            indicateError(5);
            retryCount++;
            delay(1000); // รอ 1 วินาทีก่อนพยายามใหม่
        }
=======
    String deviceID = "ESP8266_01";
    String firebasePath = "/sensor_data/" + deviceID;
    if (Firebase.pushJSON(firebaseData, firebasePath, json)) {
        Serial.println("Data sent to Firebase successfully!");
    } else {
        Serial.print("Failed to send data to Firebase: ");
        Serial.println(firebaseData.errorReason());
        indicateError(5);
>>>>>>> parent of 27e1899 (succeed code)
    }
    Serial.println("Failed to send data after 3 attempts.");
}

bool debounceSensor() {
    static unsigned long lastDebounceTime = 0;
    static bool lastSensorState = LOW;
    bool currentSensorState = digitalRead(SENSOR_DIGITAL_PIN) == HIGH;

    if (currentSensorState != lastSensorState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) { // Debounce time 50ms
        lastSensorState = currentSensorState;
        return currentSensorState;
    }

    return lastSensorState;
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(SENSOR_DIGITAL_PIN, INPUT);
    connectToWiFi();
    connectToFirebase();

    Serial.println("System initialized successfully!");
}

void loop() {
    checkWiFiConnection();
    checkFirebaseConnection();

<<<<<<< HEAD
    bool currentSensorState = debounceSensor();  // ใช้ฟังก์ชัน Debounce

    if (!carAtSensor && currentSensorState) {
        if (lapStartTime > 0) {
            lapEndTime = millis();
            unsigned long lapTime = lapEndTime - lapStartTime;

            if (lapCount < 3) {
                lapTimes[lapCount] = lapTime;
                lapCount++;
            }

            if (lapCount == 3) {
                unsigned long totalTime = lapTimes[0] + lapTimes[1] + lapTimes[2];
                sendRaceResultToFirebase(playerCount, lapTimes[0], lapTimes[1], lapTimes[2], totalTime);

                Serial.println("Race completed, waiting for reset...");
                delay(2000);  
                lapCount = 0;
                lapStartTime = 0;
                memset(lapTimes, 0, sizeof(lapTimes)); // เคลียร์อาเรย์ lapTimes
                playerCount++;  
            }
=======
    int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);

    if (digitalValue == HIGH) { // Sensor is active
        if (!timerRunning) {
            startTime = millis();
            timerRunning = true;
            Serial.println("Sensor activated. Timer started.");
        }
    } else { // Sensor is inactive
        if (timerRunning) {
            duration = calculateDuration(startTime, millis());
            timerRunning = false;
            Serial.print("Sensor deactivated. Timer duration: ");
            Serial.println(duration);
            sendDataToFirebase(duration);
>>>>>>> parent of 27e1899 (succeed code)
        }
    }

<<<<<<< HEAD
    delay(100); // ลด delay เพื่อเพิ่มความเร็วในการตรวจจับ
}
=======
    delay(500); // Avoid excessive processing
}
>>>>>>> parent of 27e1899 (succeed code)
