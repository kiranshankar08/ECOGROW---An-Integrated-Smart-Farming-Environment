# 🌱 ECOGROW — An Integrated Smart Farming Environment

ECOGROW is an AI-powered smart agriculture ecosystem designed to integrate multiple farming sectors into a single intelligent platform.  
The system combines IoT, AI prediction, cloud-based monitoring, and automation to help farmers manage:

- 🌾 Smart Farming
- 🐟 Aquaculture
- 🐄 Cattle Monitoring
- 🐔 Poultry Management

The platform provides real-time monitoring, predictive analytics, alert systems, and centralized dashboards for efficient farm management.

---

# 🚀 Features

## 🌾 Smart Farm Monitoring
- Real-time soil moisture monitoring
- Temperature & humidity tracking
- Rain detection system
- Automated irrigation logic
- Water tank level monitoring
- Crop health analysis
- Intruder detection alerts

## 🐄 Cattle Management
- Temperature and humidity monitoring
- Gas level monitoring
- Fire detection alerts
- Intruder detection system
- Automated feeder and water pump monitoring

## 🐟 Aquaculture System
- Water temperature monitoring
- pH level analysis
- Turbidity tracking
- Water level monitoring
- Smart feeding system

## 🐔 Poultry Monitoring
- Environmental monitoring
- Fire detection alerts
- Smart lighting control
- Automated door monitoring
- Gas leakage detection

---

# 🧠 AI & Analytics

ECOGROW uses Machine Learning for:
- Soil moisture prediction
- Trend analysis
- Smart irrigation decisions
- Resource optimization

The system uses:
- `Linear Regression`
- Real-time sensor analysis
- Predictive monitoring

---

# 📲 Alert System

The platform supports:
- 📧 Email notifications
- 📱 SMS alerts using Twilio
- Critical warning system
- Security breach notifications

---

# 🛠️ Tech Stack

## Backend
- Python
- Flask

## AI / ML
- NumPy
- Scikit-learn

## Database & Cloud
- Google Sheets API
- GSpread

## Communication
- Twilio SMS API
- Flask-Mail

## Frontend
- HTML
- CSS
- Jinja Templates

## Hardware Integration
- ESP32
- IoT Sensors
- Smart Monitoring Devices

---

# 📂 Project Structure

```bash
ECOGROW/
│
├── app.py
├── requirements.txt
├── .env.example
├── .gitignore
│
├── static/
│   ├── style.css
│   └── images/
│
├── templates/
│   ├── login.html
│   ├── farm_dashboard.html
│   ├── aqua_dashboard.html
│   ├── cattle_dashboard.html
│   ├── poultry_dashboard.html
│   ├── reports.html
│   └── profile.html
│
└── assets/
```

---

# ⚙️ Installation

## 1️⃣ Clone Repository

```bash
git clone https://github.com/kiranshankar08/ECOGROW---An-Integrated-Smart-Farming-Environment.git
cd ECOGROW---An-Integrated-Smart-Farming-Environment
```

---

## 2️⃣ Install Dependencies

```bash
pip install -r requirements.txt
```

---

## 3️⃣ Configure Environment Variables

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

## 4️⃣ Add Google Sheets Credentials

Place your Google API credentials file:

```bash
credentials.json
```

inside the root project directory.

---

## 5️⃣ Run the Application

```bash
python app.py
```

Server starts at:

```bash
http://127.0.0.1:5000
```

---

# 📊 Dashboard Modules

- 🌾 Farm Dashboard
- 🐟 Aquaculture Dashboard
- 🐄 Cattle Dashboard
- 🐔 Poultry Dashboard
- 📈 Reports & Analytics
- 👤 User Profiles
- 🌐 Community Resource Sharing

---

# 🔐 Security Features

- Environment variable protection using `.env`
- Sensitive credentials excluded using `.gitignore`
- Secure API handling
- Role-based dashboard access

---

# 📸 Screenshots

Add screenshots inside:

```bash
assets/
```

Example:

```md
## Login Page
![Login](assets/login.png)

## Farm Dashboard
![Dashboard](assets/dashboard.png)
```

---

# 🔮 Future Improvements

- Mobile Application
- AI Disease Detection
- Drone Integration
- Live Camera Monitoring
- Cloud Database Migration
- Advanced AI Prediction Models
- GPS-based Farm Mapping

---

# 👨‍💻 Author

## Kiran Shankar

AI | IoT | Smart Agriculture | Full Stack Development

GitHub:
https://github.com/kiranshankar08

---

# 📜 License

This project is licensed under the MIT License.

---

# ⭐ Support

If you found this project useful, consider giving it a ⭐ on GitHub.