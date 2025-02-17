#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>

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
        }
        delay(500); // Debounce delay
    }
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
}
