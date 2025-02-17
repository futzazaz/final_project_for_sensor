#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <EEPROM.h>

// Wi-Fi credentials
#define WIFI_SSID "Mi 11 Lite"
#define WIFI_PASSWORD "Futzazaz140815"

// Firebase setup
#define FIREBASE_HOST "driftking-d5a48-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "1oRXFar73k0U8QRnj9kqcoYAeBRUGfXhzxyEWzcE"

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
void testFirebaseConnection();

void connectToWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        indicateError(2); // Blink LED 2 times
    }
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void connectToFirebase() {
    Serial.println("Connecting to Firebase...");
    
    // Configure Firebase with legacy token auth
    firebaseConfig.database_url = FIREBASE_HOST;
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

void testFirebaseConnection() {
    if (Firebase.setString(firebaseData, "/test", "hello")) {
        Serial.println("Firebase test write successful!");
    } else {
        Serial.println("Firebase test write failed!");
        Serial.println(firebaseData.errorReason());
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
    
    // Test Firebase connection
    testFirebaseConnection();
    
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