#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <DHT.h>
#include <ESP32Servo.h>

// -------- NETWORK CONFIGURATION --------
const char* ssid = "username";
const char* password = "password";
// Update this URL whenever you restart ngrok!
const char* serverUrl = "your_server_url";

// -------- PIN CONFIG --------
int bulbRelayPin   = 26;
int pumpRelayPin   = 27;
int ldrPin         = 5;     
int waterSensorPin = 34;    
int gasSensorPin   = 35;    
int fireSensorPin  = 4;     
int dhtPin         = 19;
int servoPin       = 13;

// -------- OBJECTS & CONSTANTS --------
DHT dht(dhtPin, DHT22);
Servo foodServo;

const int WATER_PUMP_THRESHOLD = 300;
const unsigned long FEED_INTERVAL = 15000; // Every 15 seconds
const unsigned long FEED_DURATION = 2000;  // Stay open for 2 seconds
const unsigned long CLOUD_INTERVAL = 4000; 

unsigned long lastFeedTrigger = 0;
unsigned long lastCloudSync   = 0;
bool isFeeding = false;

String currentBulbStatus = "OFF";
String currentDoorStatus = "CLOSED";
int currentFireStatus    = 0; // 0 for Secure, 1 for Fire

void setup() {
  Serial.begin(115200);

  // --- ESP32 SERVO TIMER ALLOCATION ---
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  
  foodServo.setPeriodHertz(50);
  foodServo.attach(servoPin, 500, 2400);
  foodServo.write(0); 

  pinMode(bulbRelayPin, OUTPUT);
  pinMode(pumpRelayPin, OUTPUT);
  digitalWrite(bulbRelayPin, HIGH); // OFF (Active Low)
  digitalWrite(pumpRelayPin, HIGH); // OFF (Active Low)

  pinMode(ldrPin, INPUT);
  pinMode(fireSensorPin, INPUT_PULLUP); // Using Pullup for Digital Fire Sensor
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\n✅ POULTRY SYSTEM ONLINE (farmer)");
}

void loop() {
  unsigned long currentTime = millis();

  // 1. SENSOR READINGS
  int waterRaw  = analogRead(waterSensorPin);
  int ldrState  = digitalRead(ldrPin);
  int fireRaw   = digitalRead(fireSensorPin);
  int gasValue  = analogRead(gasSensorPin);
  float temp    = dht.readTemperature();
  float hum     = dht.readHumidity();

  // 2. AUTOMATION & LOGIC
  
  // Fire Detection (Active LOW sensor)
  currentFireStatus = (fireRaw == LOW) ? 1 : 0;
  if(currentFireStatus == 1) {
    Serial.println("🔥 ALERT: FIRE DETECTED!");
  }

  // Water Pump Control
  digitalWrite(pumpRelayPin, (waterRaw < WATER_PUMP_THRESHOLD) ? LOW : HIGH);
  
  // Bulb Control (HIGH = Dark on LDR module)
  if (ldrState == HIGH) {
    digitalWrite(bulbRelayPin, LOW); // Relay ON
    currentBulbStatus = "ON";
  } else {
    digitalWrite(bulbRelayPin, HIGH); // Relay OFF
    currentBulbStatus = "OFF";
  }

  // --- CORRECTED FEEDING LOGIC ---
  if (!isFeeding && (currentTime - lastFeedTrigger >= FEED_INTERVAL)) {
    isFeeding = true;
    lastFeedTrigger = currentTime;
    foodServo.write(90);
    currentDoorStatus = "OPEN";
    Serial.println("🍴 Feeding Cycle Started...");
  }

  if (isFeeding && (currentTime - lastFeedTrigger >= FEED_DURATION)) {
    isFeeding = false;
    foodServo.write(0);
    Serial.println("🍴 Feeding Cycle Finished.");
    // currentDoorStatus remains "OPEN" to ensure the 4s Cloud Sync catches it
  }

  // 3. CLOUD SYNC
  if (currentTime - lastCloudSync >= CLOUD_INTERVAL) {
    lastCloudSync = currentTime;
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      NetworkClientSecure client;
      client.setInsecure();
      http.begin(client, serverUrl);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("ngrok-skip-browser-warning", "true");

      // Matching Christo's Email (23iota04) and added Fire_Status
      String payload = "{\"Email\":\"poultryfarmer@example.com\",";
      payload += "\"Temp (°C)\":" + String(isnan(temp) ? 0 : temp) + ",";
      payload += "\"Humidity (%)\":" + String(isnan(hum) ? 0 : hum) + ",";
      payload += "\"Gas_Level (ppm)\":" + String(gasValue) + ",";
      payload += "\"Light_Level\":\"" + String(ldrState == HIGH ? "Dark" : "Bright") + "\",";
      payload += "\"Light_Bulb_Status\":\"" + currentBulbStatus + "\",";
      payload += "\"Door_Status\":\"" + currentDoorStatus + "\",";
      payload += "\"Fire_Status\":" + String(currentFireStatus) + "}";

      int httpResponseCode = http.POST(payload);
      Serial.printf("📡 Sync farmer: %d | Fire: %d | Door: %s\n", 
                    httpResponseCode, currentFireStatus, currentDoorStatus.c_str());

      // Handshake: Reset door status only if sync was successful
      if(httpResponseCode == 200) currentDoorStatus = "CLOSED"; 
      
      http.end();
    }
  }
}