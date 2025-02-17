
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>

<<<<<<< HEAD
#define FIREBASE_HOST "your-project.firebaseio.com"  // Replace with your Firebase project URL
#define FIREBASE_AUTH "your-database-secret"  // Replace with your Firebase authentication key
#define WIFI_SSID "your-SSID"  // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "your-PASSWORD"  // Replace with your Wi-Fi password

FirebaseData firebaseData;
int sensorPin = D1;
bool raceActive = false;
unsigned long startTime;
unsigned long lapTimes[3];
int lapCount = 0;
int playerNumber = 1;

void setup() {
    Serial.begin(115200);
    pinMode(sensorPin, INPUT_PULLUP);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi");
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

void loop() {
    if (digitalRead(sensorPin) == LOW) {  // Sensor triggered
        if (!raceActive) {
            startRace();
        } else {
            recordLap();
=======
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

    String deviceID = "ESP8266_01";
    String firebasePath = "/sensor_data/" + deviceID;
    if (Firebase.pushJSON(firebaseData, firebasePath, json)) {
        Serial.println("Data sent to Firebase successfully!");
    } else {
        Serial.print("Failed to send data to Firebase: ");
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
>>>>>>> parent of 4a16877 (add some feature)
        }
        delay(500); // Debounce delay
    }
<<<<<<< HEAD
}

void startRace() {
    Serial.println("Race Started!");
    raceActive = true;
    startTime = millis();
    lapCount = 0;
}

void recordLap() {
    unsigned long lapTime = millis() - startTime;
    lapTimes[lapCount] = lapTime;
    Serial.print("Lap "); Serial.print(lapCount + 1);
    Serial.print(" time: "); Serial.println(lapTime);
    lapCount++;
    
    if (lapCount == 3) {
        finishRace();
    }
}

void finishRace() {
    raceActive = false;
    unsigned long totalTime = lapTimes[0] + lapTimes[1] + lapTimes[2];
    Serial.print("Total Time: "); Serial.println(totalTime);
    uploadData(playerNumber, lapTimes, totalTime);
    playerNumber++;  // Switch to next player
}

void uploadData(int player, unsigned long laps[], unsigned long total) {
    String path = "/race/player" + String(player);
    Firebase.setInt(firebaseData, path + "/lap1", laps[0]);
    Firebase.setInt(firebaseData, path + "/lap2", laps[1]);
    Firebase.setInt(firebaseData, path + "/lap3", laps[2]);
    Firebase.setInt(firebaseData, path + "/totalTime", total);
    Serial.println("Data uploaded to Firebase");
=======

    delay(500); // Avoid excessive processing
>>>>>>> parent of 4a16877 (add some feature)
}
