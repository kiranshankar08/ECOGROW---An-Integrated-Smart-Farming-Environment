import sqlite3
from werkzeug.security import generate_password_hash

DB_FILE = "smart_farm.db"

farmers = [
    ("land_farmer", "land123", "farm_land"),
    ("poultry_farmer", "poultry123", "poultry"),
    ("cattle_farmer", "cattle123", "cattle"),
    ("aqua_farmer", "aqua123", "aquaculture")
]

conn = sqlite3.connect(DB_FILE)
c = conn.cursor()

for username, password, module in farmers:
    c.execute(
        "INSERT OR IGNORE INTO farmers (username, password, module) VALUES (?, ?, ?)",
        (username, generate_password_hash(password), module)
    )

conn.commit()
conn.close()

print("✅ Farmers created successfully")