# 🌱 ECOGROW — An Integrated Smart Farming Environment

## 📌 Overview

ECOGROW is a complete IoT-enabled smart agriculture platform designed to improve modern farming through automation, environmental monitoring, AI-assisted decision-making, and centralized farm management.

The system integrates:

* 🌾 Smart Crop Farming
* 🐟 Aquaculture Monitoring
* 🐄 Cattle Farm Management
* 🐓 Poultry Farm Automation
* ☁️ Cloud-Based Monitoring Dashboard
* 📊 Real-Time Data Logging
* 📧 SMS & Email Alerts
* 🤖 AI-Based Irrigation Prediction

The project combines:

* ESP32-based hardware modules
* Flask web application backend
* Google Sheets cloud logging
* Twilio alert system
* Smart automation logic
* Real-time monitoring dashboards

---

# 🚀 Features

## 🌾 Smart Farming Module

* Soil moisture monitoring (3 zones)
* Smart irrigation automation
* Rain detection system
* Water tank level monitoring
* Crop health estimation
* Intruder detection & buzzer alert
* Smart LED lighting using LDR
* AI moisture prediction using Linear Regression

## 🐟 Aquaculture Module

* Water temperature monitoring
* pH monitoring
* Turbidity monitoring
* Water level monitoring
* Feeding system control
* Cloud logging support

## 🐄 Cattle Monitoring Module

* Temperature & humidity monitoring
* Gas level monitoring
* Dung level monitoring
* Fire detection
* Threat/intruder detection
* Automatic feeder monitoring
* Pump control system

## 🐓 Poultry Monitoring Module

* Temperature monitoring
* Humidity monitoring
* Gas detection
* Fire detection
* Smart lighting system
* Automatic door status monitoring

---

# 🧠 AI Features

ECOGROW includes AI-assisted irrigation prediction using:

* NumPy
* Scikit-Learn
* Linear Regression

The system analyzes previous soil moisture readings and predicts future moisture levels to optimize irrigation and water usage.

---

# ☁️ Cloud & Backend Features

## Flask Backend

* Multi-user role-based dashboards
* Real-time monitoring
* Data ingestion APIs
* CSV report generation
* User authentication
* Session management

## Google Sheets Integration

Sensor data is automatically stored in Google Sheets for:

* cloud logging
* analytics
* reports
* remote monitoring

## Twilio Integration

* SMS alerts for emergencies
* Intruder alerts
* Water tank alerts
* Fire alerts

## Email Alert System

Automated email notifications for:

* rainfall detection
* water shortage
* security alerts
* environmental warnings

---

# 🛠️ Technologies Used

## Software

* Python
* Flask
* HTML
* CSS
* JavaScript
* Arduino C++
* Google Sheets API
* Twilio API
* Scikit-Learn
* NumPy

## Hardware

* ESP32
* Soil Moisture Sensors
* DHT11 Sensor
* Ultrasonic Sensor
* Rain Sensor
* LDR Sensor
* IR Sensor
* Relay Module
* Water Pump
* Buzzers
* LEDs
* Gas Sensors

---

# 📂 Project Structure

```bash
ECOGROW/
│
├── app.py
├── requirements.txt
├── README.md
├── LICENSE
├── .gitignore
│
├── Arduino code/
│   ├── field_farm.ino
│   ├── Aqua_farm.ino
│   ├── Cattle_farm.ino
│   └── poultry_farmer.ino
│
├── templates/
├── static/
└── credentials.json (ignored)
```

---

# 🔌 Hardware Architecture

## 🌾 Smart Farming Flow

```text
Sensors → ESP32 → Flask Server → Google Sheets → Dashboard
                                ↓
                         SMS / Email Alerts
```

---

# 📊 Dashboard Features

* Real-time monitoring
* AI moisture prediction
* Resource sharing system
* Downloadable CSV reports
* Multi-role dashboard support
* User profile management
* Community resource sharing

---

# 📸 Screenshots

> Add screenshots of your:

* Login page
* Farm dashboard
* Reports page
* Hardware setup
* Circuit connections
* Sensor outputs

---

# ⚙️ Installation & Setup

## 1️⃣ Clone Repository

```bash
git clone https://github.com/kiranshankar08/ECOGROW---An-Integrated-Smart-Farming-Environment.git
```

## 2️⃣ Navigate to Project Folder

```bash
cd ECOGROW---An-Integrated-Smart-Farming-Environment
```

## 3️⃣ Install Dependencies

```bash
pip install -r requirements.txt
```

---

# 🔐 Environment Variables

Create a `.env` file:

```env
TWILIO_SID=your_twilio_sid
TWILIO_TOKEN=your_twilio_token
TWILIO_PHONE=your_twilio_phone

MAIL_USER=your_email@gmail.com
MAIL_PASS=your_gmail_app_password

SECRET_KEY=your_secret_key
```

---

# 📄 Google Sheets Setup

1. Create a Google Cloud Project
2. Enable Google Sheets API
3. Create Service Account Credentials
4. Download `credentials.json`
5. Place it in the project root directory

⚠️ Never upload `.env` or `credentials.json` to GitHub.

---

# ▶️ Run Flask Server

```bash
python app.py
```

Server will start at:

```text
http://127.0.0.1:5000
```

---

# 📡 ESP32 Setup

Open Arduino IDE and install:

* ESP32 Board Package
* ArduinoJson
* WiFi Library
* HTTPClient
* DHT Sensor Library

Update these placeholders before uploading:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverName = "YOUR_SERVER_URL";
```

Then upload the corresponding `.ino` file to the ESP32.

---

# 🔒 Security Notes

Sensitive files are excluded using `.gitignore`:

```gitignore
.env
credentials.json
*.pyc
__pycache__/
```

---

# 🌍 Future Improvements

* Mobile application integration
* AI disease detection
* Live camera monitoring
* MQTT integration
* Cloud deployment
* GPS-based farm tracking
* Voice assistant integration
* Firebase integration
* Solar-powered automation
* Real-time analytics dashboard

---

# 👨‍💻 Author

## Kiran Shankar

IoT | Embedded Systems | Smart Agriculture | AI Automation

GitHub:
[https://github.com/kiranshankar08](https://github.com/kiranshankar08)

---

# 📜 License

This project is licensed under the MIT License.

---

# ⭐ Support

If you found this project useful:

* ⭐ Star the repository
* 🍴 Fork the project
* 🛠️ Contribute improvements

---

# 🌱 ECOGROW Vision

ECOGROW aims to bridge traditional agriculture with modern IoT and AI technologies to create a smarter, safer, and more sustainable farming ecosystem.
