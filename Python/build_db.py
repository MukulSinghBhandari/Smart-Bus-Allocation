import sqlite3
import os
# 1. This creates a new database file called campus_transit.db
conn = sqlite3.connect('campus_transit.db')
cursor = conn.cursor()

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SQL_FILE = os.path.join(BASE_DIR, '../Data_Base/DataBase.sql')

# 3. This executes the SQL commands to build the tables and insert the GEU data
with open(SQL_FILE, 'r') as file:
    sql_script = file.read()
cursor.executescript(sql_script)

# 4. Save the changes and close the connection
conn.commit()
conn.close()

print("Database built successfully!")