from flask import Flask, render_template, request, redirect, session, jsonify
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from datetime import datetime
from twilio.rest import Client
import numpy as np
from sklearn.linear_model import LinearRegression
import csv
from flask import Response
from io import StringIO
from flask_mail import Mail, Message
import os
from dotenv import load_dotenv

# -------------------- LOAD ENV VARIABLES --------------------
load_dotenv()

app = Flask(__name__)
app.secret_key = "supersecretkey"

# -------------------- TWILIO CONFIGURATION (SECURE) --------------------
TWILIO_SID = os.getenv("TWILIO_SID")
TWILIO_TOKEN = os.getenv("TWILIO_TOKEN")
TWILIO_PHONE = os.getenv("TWILIO_PHONE")

# -------------------- GOOGLE SHEETS SETUP --------------------
SCOPE = ["https://spreadsheets.google.com/feeds", "https://www.googleapis.com/auth/drive"]
CREDS = ServiceAccountCredentials.from_json_keyfile_name("credentials.json", SCOPE)
CLIENT = gspread.authorize(CREDS)

SPREADSHEET_NAME = "Smart_Agriculture_Central_System"

SHEET_USERS = CLIENT.open(SPREADSHEET_NAME).worksheet("user_roles")
SHEET_FARM = CLIENT.open(SPREADSHEET_NAME).worksheet("farm_land_data")
SHEET_AQUA = CLIENT.open(SPREADSHEET_NAME).worksheet("Aquaculture_Data")
SHEET_CATTLE = CLIENT.open(SPREADSHEET_NAME).worksheet("Cattle_Data")
SHEET_POULTRY = CLIENT.open(SPREADSHEET_NAME).worksheet("Poultry_Data")
SHEET_RESOURCES = CLIENT.open(SPREADSHEET_NAME).worksheet("resource_sharing")

# -------------------- EMAIL CONFIGURATION (SECURE) --------------------
app.config['MAIL_SERVER'] = 'smtp.gmail.com'
app.config['MAIL_PORT'] = 587
app.config['MAIL_USE_TLS'] = True
app.config['MAIL_USERNAME'] = os.getenv("MAIL_USER")
app.config['MAIL_PASSWORD'] = os.getenv("MAIL_PASS")
app.config['MAIL_DEFAULT_SENDER'] = os.getenv("MAIL_USER")

mail = Mail(app)

# -------------------- EMAIL HELPER FUNCTION --------------------
def send_alert_email(farmer_email, subject, body):
    try:
        test_recipient = "testingfromkiran@gmail.com"

        msg = Message(subject, recipients=[test_recipient])

        msg.body = f"EcoGrow AI System Notification\n" \
                   f"Target Account: {farmer_email}\n" \
                   f"------------------------------\n\n{body}"

        mail.send(msg)
        print(f"📧 Alert sent to {test_recipient}")
    except Exception as e:
        print(f"❌ Email Error: {e}")

# -------------------- SMS HELPER FUNCTION --------------------
def send_alert_sms(farmer_email, message):
    try:
        users = SHEET_USERS.get_all_records()
        user = next((u for u in users if str(u["Email"]).strip().lower() == farmer_email.lower()), None)

        if user and user.get("Phone"):
            client = Client(TWILIO_SID, TWILIO_TOKEN)

            target_phone = str(user["Phone"])
            if not target_phone.startswith('+'):
                target_phone = "+91" + target_phone

            client.messages.create(
                body=f"⚠️ ECOGROW ALERT for {user['Farmer_Name']}: {message}",
                from_=TWILIO_PHONE,
                to=target_phone
            )

            print(f"✅ SMS Sent to {user['Farmer_Name']}")

    except Exception as e:
        print(f"❌ Twilio SMS Error: {e}")

# -------------------- AUTHENTICATION --------------------
@app.route("/", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        username = request.form["username"].strip()
        users = SHEET_USERS.get_all_records()
        user_found = next((u for u in users if str(u["Farmer_Name"]).lower() == username.lower()), None)

        if user_found:
            session["username"] = user_found["Farmer_Name"]
            session["email"] = str(user_found["Email"]).strip().lower()
            session["role"] = user_found["Role"]

            try:
                row_index = users.index(user_found) + 2
                SHEET_USERS.update_cell(row_index, 4, "Active")
            except Exception as e:
                print(f"Error updating user status: {e}")

            return redirect("/dashboard")

        return render_template("login.html", error="Invalid credentials")

    return render_template("login.html")


@app.route("/logout")
def logout():
    if "username" in session:
        try:
            users = SHEET_USERS.get_all_records()
            user = next((u for u in users if u["Farmer_Name"] == session["username"]), None)

            if user:
                row_index = users.index(user) + 2
                SHEET_USERS.update_cell(row_index, 4, "Inactive")
        except Exception as e:
            print(f"Error updating logout status: {e}")

    session.clear()
    return redirect("/")

# -------------------- DASHBOARDS --------------------
@app.route("/dashboard")
def dashboard():
    if "username" not in session:
        return redirect("/")

    role = str(session.get("role")).strip().capitalize()
    dashboards = {
        "Farm": "/farm-dashboard",
        "Aqua": "/aqua-dashboard",
        "Cattle": "/cattle-dashboard",
        "Poultry": "/poultry-dashboard"
    }

    return redirect(dashboards.get(role, "/"))

@app.route("/farm-dashboard")
def farm_dashboard():
    if "username" not in session:
        return redirect("/")
    return render_template("farm_dashboard.html", username=session["username"])

@app.route("/aqua-dashboard")
def aqua_dashboard():
    if "username" not in session:
        return redirect("/")
    return render_template("aqua_dashboard.html", username=session["username"])

@app.route("/cattle-dashboard")
def cattle_dashboard():
    if "username" not in session:
        return redirect("/")
    return render_template("cattle_dashboard.html", username=session["username"])

@app.route("/poultry-dashboard")
def poultry_dashboard():
    if "username" not in session:
        return redirect("/")
    return render_template("poultry_dashboard.html", username=session["username"])

# -------------------- ALL OTHER ROUTES (UNCHANGED LOGIC) --------------------
# -------------------- COMMUNITY / RESOURCE / REPORTS PAGES --------------------
@app.route("/community")
def community_page():
    if "username" not in session:
        return redirect("/")
    try:
        posts = SHEET_RESOURCES.get_all_records()[::-1]  # latest first
    except Exception as e:
        print(f"Error fetching community posts: {e}")
        posts = []
    return render_template("community.html", username=session["username"], posts=posts)

@app.route("/resources")
def resources_redirect():
    return redirect("/reports")

@app.route("/reports")
def reports_page():
    if "username" not in session:
        return redirect("/")
    
    user_email = str(session.get("email", "")).strip().lower()
    role = str(session.get("role", "")).strip().capitalize()
    
    # Mapping sheet objects to roles
    mapping = {
        "Aqua": SHEET_AQUA, 
        "Cattle": SHEET_CATTLE, 
        "Poultry": SHEET_POULTRY, 
        "Farm": SHEET_FARM
    }
    target_sheet = mapping.get(role, SHEET_FARM)

    try:
        all_rows = target_sheet.get_all_records()
        # Filter rows belonging only to the current user
        user_logs = [r for r in all_rows if str(r.get("Email", "")).strip().lower() == user_email]
        
        # We pass 'role' here so the HTML knows which theme to use
        return render_template("reports.html", 
                               username=session["username"], 
                               role=role,
                               table_data=user_logs[::-1], 
                               chart_pts=user_logs[-15:])
    except Exception as e:
        print(f"Report Error: {e}")
        return render_template("reports.html", username=session["username"], role=role, table_data=[], chart_pts=[])

@app.route("/download-csv")
def download_csv():
    if "email" not in session:
        return "Unauthorized", 401
        
    user_email = session.get("email")
    role = str(session.get("role", "")).strip().capitalize()
    
    mapping = {"Aqua": SHEET_AQUA, "Cattle": SHEET_CATTLE, "Poultry": SHEET_POULTRY, "Farm": SHEET_FARM}
    target_sheet = mapping.get(role, SHEET_FARM)
    
    records = target_sheet.get_all_records()
    user_records = [r for r in records if str(r.get("Email", "")).strip().lower() == user_email]

    if not user_records:
        return "No data available", 404

    si = StringIO()
    cw = csv.DictWriter(si, fieldnames=user_records[0].keys())
    cw.writeheader()
    cw.writerows(user_records)
    
    output = si.getvalue()
    return Response(
        output,
        mimetype="text/csv",
        headers={"Content-disposition": f"attachment; filename=EcoGrow_{role}_Logs.csv"}
    )

# -------------------- HARDWARE DATA INGESTION --------------------
@app.route("/update-farm-data", methods=["POST"])
def update_farm_data():
    try:
        data = request.json or {}
        email = str(data.get("Email", "")).strip().lower()
        
        # Extract Sensor Logic
        tank_low = int(bool(data.get("Tank_Low_Alert (0/1)", 0)))
        intruder = int(bool(data.get("Intruder (0/1)", 0)))
        is_rain = int(bool(data.get("Rain (0/1)", 0))) # <--- Extract Rain status

        # 1. Tank Alert
        if tank_low:
            send_alert_sms(email, "CRITICAL: Water Tank Low!")
            send_alert_email(email, "EcoGrow: Water Tank Critical", 
                             "Your farm water tank is critically low. Please refill to ensure irrigation stability.")

        # 2. Security Alert
        if intruder:
            send_alert_sms(email, "SECURITY ALERT: Intruder detected!")
            send_alert_email(email, "EcoGrow: Security Breach", 
                             "An intruder was detected in your farm zone. Access your dashboard for live logs.")

        # 3. Rain Detection Alert (New)
        if is_rain:
            # We usually don't need an SMS for rain (to avoid spamming), 
            # but an Email is perfect for logging the optimization event.
            send_alert_email(email, "EcoGrow: Rain Detected - Water Saved", 
                             "Precipitation detected. Automated irrigation has been paused to optimize water resources.")

        # --- Your existing Google Sheets logging code ---
        row = [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            email,
            data.get("Soil_Moisture_Zone1 (%)", 0),
            data.get("Soil_Moisture_Zone2 (%)", 0),
            data.get("Soil_Moisture_Zone3 (%)", 0),
            data.get("Temperature (°C)", 0),
            data.get("Humidity (%)", 0),
            is_rain,
            data.get("Tank_Level (%)", 0),
            tank_low,
            data.get("Pump_Status", "OFF"),
            data.get("Pump_Reason", "N/A"),
            data.get("Crop_Health (%)", 0),
            data.get("Water_Used_Total", 0),
            intruder
        ]
        SHEET_FARM.append_row(row, value_input_option="USER_ENTERED")
        return {"status": "success"}, 200

    except Exception as e:
        print(f"❌ Farm Data Error: {e}")
        return {"status": "error", "message": str(e)}, 500

@app.route("/update-cattle-data", methods=["POST"])
def update_cattle_data():
    try:
        data = request.json or {}
        email = str(data.get("Email", "")).strip().lower()
        is_fire = int(bool(data.get("Fire_Sensor (0/1)", 0)))
        is_intruder = int(bool(data.get("Threat_Detection", 0)))
        if is_fire:
            send_alert_sms(email, "EMERGENCY: FIRE in Cattle Shed!")
        if is_intruder:
            send_alert_sms(email, "SECURITY ALERT: Intruder in Cattle Shed!")
        row = [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            email,
            data.get("Temp (°C)", 0),
            data.get("Humidity (%)", 0),
            data.get("Gas_Level (ppm)", 0),
            data.get("Dung_Level (%)", 0),
            is_fire,
            "SAFE",
            data.get("Feeder_Status", "CLOSED"),
            data.get("Pump_Status", "OFF"),
            is_intruder
        ]
        SHEET_CATTLE.append_row(row)
        return {"status": "success"}, 200
    except Exception as e:
        return {"status": "error", "message": str(e)}, 500

@app.route("/update-aqua-data", methods=["POST"])
def update_aqua_data():
    try:
        data = request.json or {}
        row = [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            str(data.get("Email", "")).strip().lower(),
	    str(data.get("Device_ID", "")),
            data.get("Temp", 0),
            data.get("pH", 7.0),
            data.get("Turbidity", 0),
            data.get("Water_Level", 0),
            data.get("Feeder", "OFF")
        ]
        SHEET_AQUA.append_row(row)
        return {"status": "success"}, 200
    except Exception as e:
        return {"status": "error", "message": str(e)}, 500

@app.route("/update-poultry-data", methods=["POST"])
def update_poultry_data():
    try:
        data = request.json or {}
        email = str(data.get("Email", "")).strip().lower()
        fire = int(bool(data.get("Fire_Status", 0)))
        if fire:
            send_alert_sms(email, "EMERGENCY: FIRE DETECTED in Poultry Shed!")
        row = [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            email,
            data.get("Temp (°C)", 0),
            data.get("Humidity (%)", 0),
            data.get("Gas_Level (ppm)", 0),
            data.get("Light_Level", "Dark"),
            data.get("Light_Bulb_Status", "OFF"),
            data.get("Door_Status", "CLOSED"),
            fire
        ]
        SHEET_POULTRY.append_row(row)
        return {"status": "success"}, 200
    except Exception as e:
        return {"status": "error", "message": str(e)}, 500

# -------------------- POST RESOURCE --------------------
@app.route("/post-resource", methods=["GET", "POST"])
def post_resource():
    if "username" not in session:
        return redirect("/")
    
    success = False
    if request.method == "POST":
        try:
            target_audience = request.form.get("target_audience")
            resource_name = request.form.get("resource_name")
            price = request.form.get("price") or "N/A"
            amount = request.form.get("amount")
            unit = request.form.get("unit")
            phone = request.form.get("phone")
            address = request.form.get("address")
            place = request.form.get("place")
            
            row = [
                datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                session["email"],
                session["username"],
                target_audience,
                resource_name,
                price,
                amount,
                unit,
                phone,
                address,
                place
            ]
            SHEET_RESOURCES.append_row(row, value_input_option="USER_ENTERED")
            success = True
        except Exception as e:
            print(f"Error posting resource: {e}")
    
    return render_template("post_resource.html", success=success)

@app.route("/profile")
def profile_page():
    if "username" not in session:
        return redirect("/")
    
    try:
        # Fetch user details from your SHEET_USERS
        users = SHEET_USERS.get_all_records()
        # Find the specific user data to pass to the profile.html
        user_found = next((u for u in users if u["Farmer_Name"] == session["username"]), {})
        
        # We create a clean dictionary to match your profile.html variables
        user_data = {
            "name": user_found.get("Farmer_Name", "Unknown"),
            "email": user_found.get("Email", "N/A"),
            "phone": user_found.get("Phone", "N/A")
        }
        
        return render_template("profile.html", user=user_data)
    except Exception as e:
        print(f"Profile Error: {e}")
        return redirect("/dashboard")

# -------------------- DASHBOARD DATA --------------------

@app.route("/dashboard-data")
def dashboard_data():
    if "email" not in session:
        return jsonify({"error": "unauthorized"}), 401
    
    user_email = str(session["email"]).strip().lower()
    role = str(session.get("role", "")).strip().capitalize()
    
    mapping = {
        "Aqua": SHEET_AQUA, 
        "Cattle": SHEET_CATTLE, 
        "Poultry": SHEET_POULTRY, 
        "Farm": SHEET_FARM
    }
    target_sheet = mapping.get(role, SHEET_FARM)
    
    try:
        # 1. Fetch all records
        rows = target_sheet.get_all_records()
        
        # 2. Filter for the logged-in user
        user_rows = [r for r in rows if str(r.get("Email", "")).strip().lower() == user_email]
        
        if not user_rows:
            return jsonify({"data": {}, "prediction": "N/A", "alerts": []})
        
        # 3. Get the absolute LATEST row
        latest = user_rows[-1]

        # 4. --- AI MOISTURE PREDICTION LOGIC ---
        prediction_val = "Calculating..."
        # Extract last 10 moisture readings for Zone 1
        history = [float(r.get("Soil_Moisture_Zone1 (%)", 0)) for r in user_rows[-10:]]
        
        if len(history) >= 5:
            # Prepare data for Linear Regression
            X = np.array(range(len(history))).reshape(-1, 1)
            y = np.array(history)
            
            # Fit the model
            model = LinearRegression().fit(X, y)
            
            # Predict moisture for the next interval (current length + 1)
            pred = model.predict([[len(history) + 1]])
            prediction_val = round(float(pred[0]), 2)
        # ---------------------------------------
        
        # 5. Generate Alerts
        alerts = []
        if int(latest.get("Tank_Level (%)", 100)) < 30:
            alerts.append({"type": "critical", "msg": "Water tank critically low"})
        
        if int(latest.get("Rain (0/1)", 0)) == 1:
            alerts.append({"type": "warning", "msg": "Rain detected - irrigation paused"})
            
        if int(latest.get("Intruder (0/1)", 0)) == 1:
            alerts.append({"type": "critical", "msg": "SECURITY: Intruder detected!"})

        # 6. Return data, prediction, and alerts
        return jsonify({
            "data": latest, 
            "prediction": prediction_val, 
            "alerts": alerts
        })
        
    except Exception as e:
        print(f"Dashboard Error: {e}")
        return jsonify({"error": str(e)}), 500

# -------------------- MAIN --------------------
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)