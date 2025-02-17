#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <EEPROM.h>

// Wi-Fi credentials
#define WIFI_SSID "Mi 11 Lite" // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "Futzazaz140815" // Replace with your Wi-Fi password

// Firebase setup
#define FIREBASE_HOST "driftking-d5a48-default-rtdb.asia-southeast1.firebasedatabase.app" // Replace with your Firebase Realtime Database URL
#define FIREBASE_AUTH "1oRXFar73k0U8QRnj9kqcoYAeBRUGfXhzxyEWzcE" // Replace with your Firebase Secret or Web API Key

// Sensor connections
#define SENSOR_DIGITAL_PIN 16 // GPIO16
#define SENSOR_ANALOG_PIN 4  // GPIO4

// LED pin
#define LED_PIN 2 // GPIO2 (LED on ESP8266 board)

// Timer variables
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerRunning = false;

// Lap tracking variables
bool carAtSensor = false;
unsigned long lapStartTime = 0;
unsigned long lapEndTime = 0;
unsigned long lapTimes[3] = {0, 0, 0};  
int lapCount = 0;
int playerCount = 1;

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
void sendRaceResultToFirebase(int player, unsigned long lap1, unsigned long lap2, unsigned long lap3, unsigned long totalTime);
String formatTime(unsigned long ms);

// เพิ่มฟังก์ชัน formatTime
String formatTime(unsigned long ms) {
    unsigned long minutes = ms / 60000;
    unsigned long seconds = (ms % 60000) / 1000;
    unsigned long milliseconds = ms % 1000;

    char buffer[15];
    sprintf(buffer, "%02lu:%02lu:%03lu", minutes, seconds, milliseconds);
    return String(buffer);
}

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

// ฟังก์ชันส่งข้อมูลผลการแข่งไปยัง Firebase
void sendRaceResultToFirebase(int player, unsigned long lap1, unsigned long lap2, unsigned long lap3, unsigned long totalTime) {
    String path = "/race_results/player" + String(player);
    FirebaseJson json;
    json.set("lap1", formatTime(lap1));
    json.set("lap2", formatTime(lap2));
    json.set("lap3", formatTime(lap3));
    json.set("totalTime", formatTime(totalTime));

    if (Firebase.setJSON(firebaseData, path, json)) {
        Serial.println("Race result sent to Firebase successfully!");
    } else {
        Serial.print("Failed to send race result to Firebase: ");
        Serial.println(firebaseData.errorReason());
        indicateError(5);
    }
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

    bool currentSensorState = digitalRead(SENSOR_DIGITAL_PIN) == HIGH;  // Detects car presence

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
                playerCount++;  
            }
        }
        carAtSensor = true;  
    } else if (carAtSensor && !currentSensorState) {
        lapStartTime = millis();
        carAtSensor = false;
    }

    delay(500); // Avoid excessive processing
}