#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <EEPROM.h>

// Wi-Fi credentials
#define WIFI_SSID "Mi 11 Lite"
#define WIFI_PASSWORD "Futzazaz140815"

// Firebase setup - updated to match your database
#define FIREBASE_HOST "driftking-d5a48-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "1oRXFar73k0U8QRnj9kqcoYAeBRUGfXhzxyEWzcE"  // Using your database secret token here

// Sensor connections
#define SENSOR_DIGITAL_PIN 16 // GPIO16
#define SENSOR_ANALOG_PIN 4  // GPIO4

// LED pin
#define LED_PIN 2 // GPIO2 (LED on ESP8266 board)

// Timer variables
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerRunning = false;

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Player tracking
int currentPlayer = 1;
int currentLap = 1;
String lap1Time = "";
String lap2Time = "";
String lap3Time = "";

// Function prototypes
void connectToWiFi();
void connectToFirebase();
void checkWiFiConnection();
void checkFirebaseConnection();
void indicateError(int errorCode);
String formatTime(unsigned long duration);
void sendLapTimeToFirebase(int player, int lap, String formattedTime);
void updateTotalTime(int player);

void connectToWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        indicateError(2); // Blink LED 2 times
    }
    Serial.println("\nConnected to Wi-Fi");
}

void connectToFirebase() {
    Serial.println("Connecting to Firebase...");
    
    // Remove "https://" from the beginning and "/" from the end if present
    String host = FIREBASE_HOST;
    if (host.startsWith("https://")) {
        host = host.substring(8);
    }
    if (host.endsWith("/")) {
        host = host.substring(0, host.length() - 1);
    }
    
    // Configure Firebase with legacy token auth
    firebaseConfig.database_url = host;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    
    Firebase.begin(&firebaseConfig, &firebaseAuth);
    Firebase.reconnectWiFi(true);
    
    // Wait for connection and report status
    delay(1000);
    if (Firebase.ready()) {
        Serial.println("Connected to Firebase successfully!");
    } else {
        Serial.println("Failed to connect to Firebase!");
        Serial.print("Error reason: ");
        Serial.println(Firebase.getErrorReason());
        indicateError(3); // Blink LED 3 times
        delay(1000);
        ESP.restart(); // Restart the ESP if failed to connect
    }
}

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi disconnected. Reconnecting...");
        indicateError(3); // Blink 3 times
        connectToWiFi();
    }
}

void checkFirebaseConnection() {
    if (!Firebase.ready()) {
        Serial.println("Firebase disconnected. Reconnecting...");
        indicateError(4); // Blink 4 times
        connectToFirebase();
    }
}

void indicateError(int errorCode) {
    for (int i = 0; i < errorCode; i++) {
        digitalWrite(LED_PIN, LOW); // LED ON (Active Low for most ESP8266 boards)
        delay(200);
        digitalWrite(LED_PIN, HIGH); // LED OFF
        delay(200);
    }
}

String formatTime(unsigned long duration) {
    // Format time as MM:SS:mmm (minutes:seconds:milliseconds)
    unsigned long milliseconds = duration % 1000;
    unsigned long seconds = (duration / 1000) % 60;
    unsigned long minutes = (duration / 60000) % 60;
    
    char formattedTime[10];
    sprintf(formattedTime, "%02lu:%02lu:%03lu", minutes, seconds, milliseconds);
    return String(formattedTime);
}

void sendLapTimeToFirebase(int player, int lap, String formattedTime) {
    String playerKey = "player" + String(player);
    String lapKey = "lap" + String(lap);
    String path = "race_results/" + playerKey + "/" + lapKey;
    
    if (Firebase.setString(firebaseData, path, formattedTime)) {
        Serial.print("Lap time sent to Firebase: Player ");
        Serial.print(player);
        Serial.print(", Lap ");
        Serial.print(lap);
        Serial.print(" = ");
        Serial.println(formattedTime);
    } else {
        Serial.print("Failed to send lap time: ");
        Serial.println(firebaseData.errorReason());
        indicateError(5);
    }
}

void updateTotalTime(int player) {
    // Get all lap times for this player
    String playerKey = "player" + String(player);
    FirebaseData getLap1;
    FirebaseData getLap2;
    FirebaseData getLap3;
    
    if (!Firebase.getString(getLap1, "race_results/" + playerKey + "/lap1") ||
        !Firebase.getString(getLap2, "race_results/" + playerKey + "/lap2") ||
        !Firebase.getString(getLap3, "race_results/" + playerKey + "/lap3")) {
        Serial.println("Failed to get lap times");
        return;
    }
    
    String lap1Str = getLap1.stringData();
    String lap2Str = getLap2.stringData();
    String lap3Str = getLap3.stringData();
    
    // Simple string validation
    if (lap1Str.length() < 8 || lap2Str.length() < 8 || lap3Str.length() < 8) {
        Serial.println("Invalid lap time format");
        return;
    }
    
    // Calculate total time
    unsigned long totalMs = 0;
    
    // Parse lap1
    unsigned long min1 = lap1Str.substring(0, 2).toInt();
    unsigned long sec1 = lap1Str.substring(3, 5).toInt();
    unsigned long ms1 = lap1Str.substring(6).toInt();
    totalMs += (min1 * 60000) + (sec1 * 1000) + ms1;
    
    // Parse lap2
    unsigned long min2 = lap2Str.substring(0, 2).toInt();
    unsigned long sec2 = lap2Str.substring(3, 5).toInt();
    unsigned long ms2 = lap2Str.substring(6).toInt();
    totalMs += (min2 * 60000) + (sec2 * 1000) + ms2;
    
    // Parse lap3
    unsigned long min3 = lap3Str.substring(0, 2).toInt();
    unsigned long sec3 = lap3Str.substring(3, 5).toInt();
    unsigned long ms3 = lap3Str.substring(6).toInt();
    totalMs += (min3 * 60000) + (sec3 * 1000) + ms3;
    
    // Format and send total time
    String formattedTotal = formatTime(totalMs);
    
    if (Firebase.setString(firebaseData, "race_results/" + playerKey + "/totalTime", formattedTotal)) {
        Serial.print("Total time updated for player ");
        Serial.print(player);
        Serial.print(": ");
        Serial.println(formattedTotal);
    } else {
        Serial.print("Failed to update total time: ");
        Serial.println(firebaseData.errorReason());
        indicateError(5);
    }
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Turn off LED initially (assuming active LOW)
    
    pinMode(SENSOR_DIGITAL_PIN, INPUT);
    
    // Initialize with a pause to stabilize
    delay(1000);
    
    connectToWiFi();
    connectToFirebase();
    
    Serial.println("System initialized successfully!");
    Serial.println("Ready for player 1, lap 1");
}

void loop() {
    checkWiFiConnection();
    checkFirebaseConnection();
    
    int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);
    
    if (digitalValue == HIGH) { // Sensor is active (car passing)
        if (!timerRunning) {
            startTime = millis();
            timerRunning = true;
            Serial.print("Timing started for player ");
            Serial.print(currentPlayer);
            Serial.print(", lap ");
            Serial.println(currentLap);
            digitalWrite(LED_PIN, LOW); // Turn on LED while timing
        }
    } else { // Sensor is inactive
        if (timerRunning) {
            duration = millis() - startTime;
            timerRunning = false;
            digitalWrite(LED_PIN, HIGH); // Turn off LED
            
            String formattedTime = formatTime(duration);
            
            // Store and send the lap time to Firebase
            sendLapTimeToFirebase(currentPlayer, currentLap, formattedTime);
            
            // Store lap time for current player
            if (currentLap == 1) {
                lap1Time = formattedTime;
                currentLap = 2;
            } else if (currentLap == 2) {
                lap2Time = formattedTime;
                currentLap = 3;
            } else if (currentLap == 3) {
                lap3Time = formattedTime;
                // Update total time after completing all 3 laps
                updateTotalTime(currentPlayer);
                
                // Move to next player
                currentPlayer++;
                currentLap = 1;
                lap1Time = "";
                lap2Time = "";
                lap3Time = "";
                
                if (currentPlayer > 4) {
                    currentPlayer = 1; // Reset for next race if needed
                }
            }
            
            Serial.print("Ready for player ");
            Serial.print(currentPlayer);
            Serial.print(", lap ");
            Serial.println(currentLap);
        }
    }
    
    delay(100); // Small delay for stability
}