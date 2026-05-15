/*
------------------------------------------------
ECOGROW Smart Farm Monitoring System
ESP32 + IoT + AI Irrigation Automation

Features:
- Soil Moisture Monitoring
- Smart Irrigation
- Rain Detection
- Intruder Detection
- Tank Monitoring
- LDR Smart Lighting
- Cloud Analytics
------------------------------------------------
*/

#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// ------------------- PINS -------------------
#define SOIL1_PIN 34
#define SOIL2_PIN 35
#define SOIL3_PIN 32

#define RELAY_PIN 26
#define RAIN_PIN 15
#define TRIG_PIN 18
#define ECHO_PIN 5

#define IR_PIN 13
#define BUZZER_PIN 25

#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

#define LDR_PIN 36       // LDR analog input
#define LED_PIN 2        // PWM LED output

// ------------------- CALIBRATION -------------------
int dryValue = 3800;
int wetValue = 1800;

int LDR_DARK_THRESHOLD = 2000; // adjust based on ambient sunlight

// ------------------- WATER -------------------
bool pumpState = false;
String pumpReason = "";
float waterUsed = 0;
float FLOW_RATE = 0.05; // liters per min
float DAILY_LIMIT = 100.0;
unsigned long pumpStart = 0;

// ------------------- TANK -------------------
#define TANK_EMPTY_DISTANCE 12
#define TANK_FULL_DISTANCE 2
#define TANK_LOW_PERCENT 20

int tankPercent = 0;
long lastValidDistance = 12;

// ------------------- WIFI -------------------
const char* ssid = "USERNAM";
const char* password = "PASS";
const char* serverName = "YOUR_SERVER_URL"; //ngrok based

// ------------------- TIMING -------------------
unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  pinMode(RAIN_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  // Setup PWM for LED on ESP32
  ledcSetup(0, 5000, 8); // Channel 0, 5kHz, 8-bit resolution
  ledcAttachPin(LED_PIN, 0);

  dht.begin();
  connectWiFi();
}

// ------------------- LOOP -------------------
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < interval) return;
  previousMillis = currentMillis;

  // WiFi reconnect if disconnected
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  // --- SENSORS ---
  int raw1 = analogRead(SOIL1_PIN);
  int raw2 = analogRead(SOIL2_PIN);
  int raw3 = analogRead(SOIL3_PIN);

  int m1 = moisturePercent(raw1);
  int m2 = moisturePercent(raw2);
  int m3 = moisturePercent(raw3);

  float temp = NAN, hum = NAN;
  for (int i = 0; i < 3; i++) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    if (!isnan(temp) && !isnan(hum)) break;
    delay(200);
  }
  if (isnan(temp)) temp = -1;
  if (isnan(hum)) hum = -1;

  long d = readHCSR04();
  if (d > 0) lastValidDistance = d;
  tankPercent = tankPercentage(lastValidDistance);

  bool isRaining = rain();
  int irValue = digitalRead(IR_PIN);
  bool intruder = (irValue == LOW);

  // --- BUZZER ---
  if (intruder) {
    tone(BUZZER_PIN, 2000); delay(200);
    tone(BUZZER_PIN, 1500); delay(200);
    tone(BUZZER_PIN, 1000); delay(200);
  } else noTone(BUZZER_PIN);

  // --- PUMP LOGIC ---
  int avgMoisture = (m1 + m2 + m3) / 3;
  int driestZone = min(m1, min(m2, m3));

  if (tankPercent <= TANK_LOW_PERCENT || isRaining) {
    stopPump(); pumpReason = "Safety Stop";
  } else if (driestZone < 40) {
    startPump(); pumpReason = "Critical Dry Zone";
  } else if (avgMoisture < 55) {
    startPump(); pumpReason = "Smart Irrigation";
  } else {
    stopPump(); pumpReason = "Moisture OK";
  }

  if (pumpState) {
    waterUsed += FLOW_RATE * interval / 60000.0;
    if (waterUsed > DAILY_LIMIT) waterUsed = DAILY_LIMIT;
  }

  int health = computeHealth(temp, hum, m1, m2, m3);

  // --- LDR & LED LOGIC ---
  int ldrValue = analogRead(LDR_PIN);
  int ledBrightness = 0;

  if (ldrValue > LDR_DARK_THRESHOLD) {
    // Low sunlight → LED ON with higher brightness
    ledBrightness = map(ldrValue, LDR_DARK_THRESHOLD, 4095, 255, 50);
  } else {
    // Sufficient sunlight → LED dim/off
    ledBrightness = 0;
  }

  ledcWrite(0, constrain(ledBrightness, 0, 255));

  // --- SEND DATA ---
  sendData(m1, m2, m3, temp, hum, health, intruder, isRaining, raw1, raw2, raw3, d, ldrValue, ledBrightness);
}

// ------------------- FUNCTIONS -------------------
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500); Serial.print("."); retries++;
  }
  if (WiFi.status() == WL_CONNECTED) Serial.println("\n✅ WiFi Connected");
  else Serial.println("\n❌ WiFi Connection Failed");
}

int moisturePercent(int v) { return constrain(map(v, wetValue, dryValue, 100, 0), 0, 100); }

void startPump() { if (!pumpState) { pumpState = true; digitalWrite(RELAY_PIN, LOW); pumpStart = millis(); } }
void stopPump() { if (pumpState) { pumpState = false; digitalWrite(RELAY_PIN, HIGH); waterUsed += FLOW_RATE * (millis() - pumpStart) / 60000.0; if (waterUsed > DAILY_LIMIT) waterUsed = DAILY_LIMIT; } }

int computeHealth(float t, float h, int m1, int m2, int m3) {
  int score = 100;
  if (m1 < 50 || m2 < 50 || m3 < 50) score -= 15;
  if (t > 32) score -= 15;
  if (h < 40 || h > 85) score -= 10;
  return constrain(score, 0, 100);
}

long readHCSR04() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

int tankPercentage(int d) { d = constrain(d, TANK_FULL_DISTANCE, TANK_EMPTY_DISTANCE); return map(d, TANK_EMPTY_DISTANCE, TANK_FULL_DISTANCE, 0, 100); }
bool rain() { return digitalRead(RAIN_PIN) == LOW; }

void sendData(int s1, int s2, int s3, float t, float h, int health, bool intruder, bool isRaining,
              int raw1, int raw2, int raw3, long distance, int ldrValue, int ledBrightness) {
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<1024> doc;
  doc["Email"] = "farmer@example.com";
  doc["Soil_Moisture_Zone1 (%)"] = s1;
  doc["Soil_Moisture_Zone2 (%)"] = s2;
  doc["Soil_Moisture_Zone3 (%)"] = s3;
  doc["Pump_Status"] = pumpState ? "ON" : "OFF";
  doc["Pump_Reason"] = pumpReason;
  doc["Temperature (°C)"] = t;
  doc["Humidity (%)"] = h;
  doc["Rain (0/1)"] = isRaining ? 1 : 0;
  doc["Tank_Level (%)"] = tankPercent;
  doc["Tank_Low_Alert (0/1)"] = tankPercent <= 20 ? 1 : 0;
  doc["Crop_Health (%)"] = health;
  doc["Water_Used_Total"] = waterUsed;
  doc["Intruder (0/1)"] = intruder ? 1 : 0;
  doc["LDR_Value"] = ldrValue;
  doc["LED_Brightness"] = ledBrightness;

  // Optional: add raw values for debugging
  doc["Soil1_Raw"] = raw1;
  doc["Soil2_Raw"] = raw2;
  doc["Soil3_Raw"] = raw3;
  doc["Tank_Distance_cm"] = distance;

  String json; serializeJson(doc, json);
  Serial.println("Sending JSON:");
  Serial.println(json);

  int code = http.POST(json);
  if (code <= 0) {
    Serial.print("HTTP POST failed: ");
    Serial.println(http.errorToString(code));
  } else Serial.print("HTTP Code: "); Serial.println(code);

  http.end();
}