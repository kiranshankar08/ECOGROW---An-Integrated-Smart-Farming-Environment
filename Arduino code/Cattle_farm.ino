#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <NetworkClientSecure.h>

// -------- NETWORK CONFIGURATION --------
const char* ssid = "USERNAME_WIFI";
const char* password = "PASSWORD_WIFI";
const char* serverUrl = "YOUR_SERVER_URL"; //NGROK BASED

// -------- PIN DEFINITIONS --------
#define RELAY_PIN 26 
#define SERVO_PIN 27
#define LASER_PIN 14
#define LDR_PIN 34
#define BUZZER_PIN 25 
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 35
#define FLAME_PIN 33
#define WATER_LEVEL_PIN 32

Servo feederServo;
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
unsigned long lastUploadMillis = 0;
const long uploadInterval = 3000; 

// ✅ NEW: DHT timing control
unsigned long lastDHTRead = 0;
const long dhtInterval = 2000;
float temp = 0.0;
float hum = 0.0;

bool feedingON = false;
int ldrThreshold = 1500; 

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);

  digitalWrite(LASER_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  feederServo.attach(SERVO_PIN);
  feederServo.write(0);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WIFI CONNECTED");
}

void loop() {
  unsigned long currentMillis = millis();

  // -------- DHT SAFE READ --------
  if (currentMillis - lastDHTRead >= dhtInterval) {
    lastDHTRead = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum = h;
    } else {
      Serial.println("⚠️ DHT Read Failed");
    }
  }

  // -------- SENSOR LOGIC --------
  int ldrValue = analogRead(LDR_PIN);
  int gasValue = analogRead(MQ135_PIN);
  int waterRaw = analogRead(WATER_LEVEL_PIN);
  int flameStatus = digitalRead(FLAME_PIN);

  // ✅ VALID RANGE FIX (avoid 793, 973 issue)
  if (temp < 0 || temp > 60) temp = 0;
  if (hum < 0 || hum > 100) hum = 0;

  int wastePercent = map(waterRaw, 0, 4095, 0, 100);
  wastePercent = constrain(wastePercent, 0, 100);

  // -------- INTRUDER DETECTION FIX --------
  bool intruderDetected = (ldrValue > ldrThreshold);

  if (intruderDetected) {
    digitalWrite(BUZZER_PIN, HIGH );   // ON
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // OFF
  }

  // -------- ACTUATOR LOGIC --------
  if (!feedingON && currentMillis - previousMillis >= 10000) {
    feedingON = true; previousMillis = currentMillis;
    feederServo.write(90); digitalWrite(RELAY_PIN, LOW);
  }
  if (feedingON && currentMillis - previousMillis >= 3000) {
    feedingON = false; previousMillis = currentMillis;
    feederServo.write(0); digitalWrite(RELAY_PIN, HIGH);
  }

  // -------- CLOUD UPLOAD --------
  if (currentMillis - lastUploadMillis >= uploadInterval) {
    lastUploadMillis = currentMillis;
    
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      NetworkClientSecure client;
      client.setInsecure();
      
      http.begin(client, serverUrl);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("ngrok-skip-browser-warning", "true");

      String payload = "{";
      payload += "\"Email\":\"cattlefarmer@example.com\","; 
      payload += "\"Temp (°C)\":" + String(temp) + ",";
      payload += "\"Humidity (%)\":" + String(hum) + ",";
      payload += "\"Gas_Level (ppm)\":" + String(gasValue) + ",";
      payload += "\"Dung_Level (%)\":" + String(wastePercent) + ",";
      payload += "\"Fire_Sensor (0/1)\":" + String(flameStatus == LOW ? 1 : 0) + ",";
      payload += "\"Threat_Detection\":" + String(intruderDetected ? 1 : 0) + ",";
      payload += "\"Feeder_Status\":\"" + String(feedingON ? "OPEN" : "CLOSED") + "\",";
      payload += "\"Pump_Status\":\"" + String(feedingON ? "ON" : "OFF") + "\"";
      payload += "}";

      int httpResponseCode = http.POST(payload);
      Serial.print("📡 Sync Code: "); Serial.println(httpResponseCode);
      http.end();
    }
  }
}