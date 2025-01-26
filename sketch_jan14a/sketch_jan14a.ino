#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h> // Use Firebase ESP8266 Library

// Wi-Fi credentials
#define WIFI_SSID "I'am Shadow!"
#define WIFI_PASSWORD "open2255"

// Firebase setup
#define FIREBASE_HOST "your-database-name.firebaseio.com" // Replace with Auto, Monica, モニカ・セニオリス, monicaeverett, NunoiEnter, KOKOMO9462, Monica モニカ, Nunoi Enter  (エンタ一), The man who like Todoroki Hajime, ออโต้ขั้นกว่าของมนุษย์, J O R#moo85, Nunoi Enter, monicafansub, Monica FS, ปวช. ออโต้ ภาคกลาง, 💫𝓜𝓸𝓷𝓲𝓬𝓪💫, Momo Firebase Realtime Database URL 
#define FIREBASE_AUTH "your-firebase-auth-key"           // Replace with Auto, Monica, モニカ・セニオリス, monicaeverett, NunoiEnter, KOKOMO9462, Monica モニカ, Nunoi Enter  (エンタ一), The man who like Todoroki Hajime, ออโต้ขั้นกว่าของมนุษย์, J O R#moo85, Nunoi Enter, monicafansub, Monica FS, ปวช. ออโต้ ภาคกลาง, 💫𝓜𝓸𝓷𝓲𝓬𝓪💫, Momo Firebase Secret or Web API Key

FirebaseData firebaseData;

// Sensor connections
#define SENSOR_DIGITAL_PIN D0 // GPIO16
#define SENSOR_ANALOG_PIN D2  // GPIO4

// Timer variables
unsigned long startTime = 0;
unsigned long duration = 0;
bool timerRunning = false;

void setup() {
  // Setup Serial for Debug
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Sensor Setup
  pinMode(SENSOR_DIGITAL_PIN, INPUT);
  pinMode(SENSOR_ANALOG_PIN, INPUT);

  Serial.println("MH-Sensor Series Test");
}

void loop() {
  // Read digital and analog sensor values
  int digitalValue = digitalRead(SENSOR_DIGITAL_PIN);
  int analogValue = analogRead(SENSOR_ANALOG_PIN);

  // Start or stop the timer based on the sensor state
  if (digitalValue == LOW) {
    // If the sensor is not detecting, start the timer
    if (!timerRunning) {
      startTime = millis();
      timerRunning = true;
    }
  } else {
    // If the sensor is detecting, stop the timer and calculate duration
    if (timerRunning) {
      duration = millis() - startTime;
      timerRunning = false;

      // Send the duration and sensor data to Firebase
      sendDataToFirebase(duration, digitalValue, analogValue);
    }
  }

  // Log the values to the Serial Monitor
  Serial.print("Digital Value: ");
  Serial.print(digitalValue);
  Serial.print(" | Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" | Timer Duration: ");
  Serial.println(timerRunning ? (millis() - startTime) : duration);

  // Delay for stability
  delay(500);
}

void sendDataToFirebase(unsigned long timerDuration, int digitalValue, int analogValue) {
  // Create a JSON object to store the data
  FirebaseJson json;
  json.set("digital_value", digitalValue);
  json.set("analog_value", analogValue);
  json.set("timer_duration_ms", timerDuration);

  // Send the JSON data to Firebase
  if (Firebase.pushJSON(firebaseData, "/sensor_data", json)) {
    Serial.println("Data sent to Firebase successfully!");
  } else {
    Serial.print("Failed to send data to Firebase: ");
    Serial.println(firebaseData.errorReason());
  }
}
