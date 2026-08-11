import sqlite3

# 1. This creates a new database file called campus_transit.db
conn = sqlite3.connect('campus_transit.db')
cursor = conn.cursor()

# 2. This opens your SQL text file and reads all the blueprints
with open('DataBase_setup.sql', 'r') as file:
    sql_script = file.read()

# 3. This executes the SQL commands to build the tables and insert the GEU data
cursor.executescript(sql_script)

# 4. Save the changes and close the connection
conn.commit()
conn.close()

print("Database built successfully!")