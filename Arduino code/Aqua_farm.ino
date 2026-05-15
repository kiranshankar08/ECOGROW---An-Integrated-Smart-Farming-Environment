#include <WiFi.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// ================= WIFI SETTINGS =================
const char* ssid     = "USERNAME";         
const char* password = "PASSWORD"; 

// ================= FLASK SERVER =================
const char* serverName = "YOUR_SERVER_URL";

// ================= USER IDENTIFICATION =================
const char* EMAIL = "aquafarmer@example.com";
const char* DEVICE_ID = "AQUA_ESP_01"; //optional

// ================= PIN DEFINITIONS =================
#define ONE_WIRE_BUS    4
#define PH_PIN          35
#define TURBIDITY_PIN   32
#define WATER_LEVEL_PIN 34
#define SERVO_PIN       19
#define RELAY_PIN       2

// ================= SENSOR SETUP =================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// ================= SERVO & RELAY =================
Servo feedServo;
bool servoAt180 = false;

// ================= TIMER =================
unsigned long lastFeedTime = 0;
unsigned long lastDataSend = 0;
const unsigned long feedInterval = 60000;  // 1 minute
const unsigned long dataInterval = 4000;   // 4 seconds

// ================= CALIBRATION =================
float getPH(int rawValue) {
  float voltage = rawValue * (3.3 / 4095.0);
  float ph = 3.0 * voltage + 3.0; // Linear approx 0-3.3V -> 3-10 pH
  return constrain(ph, 1, 10);
}

float getTurbidity(int rawValue) {
  float turb = (rawValue / 4095.0) * 10.0; // 0-4095 -> 0-10
  return constrain(turb, 1, 10);
}

float getWaterLevel(int rawValue) {
  float level = (rawValue / 4095.0) * 10.0;
  return constrain(level, 1, 10);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  // Sensors
  tempSensor.begin();
  pinMode(PH_PIN, INPUT);
  pinMode(TURBIDITY_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT);

  // Servo
  feedServo.attach(SERVO_PIN);
  feedServo.write(0);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF

  Serial.println("✅ All systems ready");
}

// ================= LOOP =================
void loop() {
  unsigned long currentTime = millis();

  // ---------- SENSOR READINGS ----------
  tempSensor.requestTemperatures();
  float temperature = tempSensor.getTempCByIndex(0);
  float phValue = getPH(analogRead(PH_PIN));
  float turbidity = getTurbidity(analogRead(TURBIDITY_PIN));
  float waterLevel = getWaterLevel(analogRead(WATER_LEVEL_PIN));

  // ---------- FEED CONTROL ----------
  String feedStatus = "IDLE";
  if (currentTime - lastFeedTime >= feedInterval) {
    feedServo.write(180);
    delay(700);
    feedServo.write(0);
    lastFeedTime = currentTime;
    feedStatus = "DISPENSED";
  }

  // ---------- RELAY/PUMP CONTROL ----------
  bool pumpNeeded = (waterLevel < 3.0);
  digitalWrite(RELAY_PIN, pumpNeeded ? LOW : HIGH);  // LOW = ON

  // ---------- SEND TO CLOUD ----------
  if (currentTime - lastDataSend >= dataInterval && WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Accept self-signed/Ngrok SSL

    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    // ================= JSON PAYLOAD =================
    StaticJsonDocument<512> doc;
    doc["Email"] = EMAIL;
    doc["Device_ID"] = String(DEVICE_ID);
    doc["Temp"] = round(temperature * 10) / 10.0;  // match Flask key names
    doc["pH"] = round(phValue * 10) / 10.0;
    doc["Turbidity"] = round(turbidity * 10) / 10.0;
    doc["Water_Level"] = round(waterLevel * 10) / 10.0;
    doc["Feeder"] = feedStatus;

    String json;
    serializeJson(doc, json);

    // Debug
    Serial.println("Sending JSON:");
    Serial.println(json);

    int httpCode = http.POST(json);
    if (httpCode == 200) {
      Serial.println("✅ Data sent successfully");
    } else {
      Serial.printf("❌ HTTP error: %d\n", httpCode);
    }

    http.end();
    lastDataSend = currentTime;
  }

  delay(100);
}