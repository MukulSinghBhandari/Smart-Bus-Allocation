import sqlite3
import csv
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CSV_FILE = os.path.join(BASE_DIR, '../Data_Base/buses.csv')
DB_FILE = os.path.join(BASE_DIR, '../campus_transit.db')

# 1. Connect to the database
conn = sqlite3.connect(DB_FILE)
cursor = conn.cursor()

# 2. Open the CSV file
with open(CSV_FILE, 'r', encoding='utf-8') as file:
    reader = csv.DictReader(file)
    
    for row in reader:
        bus_no = row['Bus No'].strip()
        reg_no = row['Reg No'].strip()
        morning_one = row['Morning Shift One'].strip()
        morning_two = row['Morning Shift Two'].strip()
        evening = row['Evening Shift'].strip()
        conductor = row['Conductor'].strip()
        phone = row['Phone'].strip()
        route_string = row['Route'].strip()

        # 3. Insert the Bus details with the new shift columns
        cursor.execute('''
            INSERT INTO Buses (
                bus_number, 
                registration_number, 
                morning_shift_one, 
                morning_shift_two, 
                evening_shift, 
                conductor_name, 
                conductor_phone
            ) 
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (bus_no, reg_no, morning_one, morning_two, evening, conductor, phone))
        
        bus_id = cursor.lastrowid # Get the ID of the bus just inserted

        # 4. Split the route by the hyphen '-' and remove extra spaces
        stops = [stop.strip() for stop in route_string.split('-')]
        
        # 5. Loop through each stop in the route
        for order, stop_name in enumerate(stops, start=1):
            
            # Check if this stop already exists in our database
            cursor.execute('SELECT stop_id FROM Stops WHERE stop_name = ?', (stop_name,))
            result = cursor.fetchone()
            
            if result:
                stop_id = result[0] # Stop exists, get its ID
            else:
                # Stop doesn't exist, create it!
                cursor.execute('INSERT INTO Stops (stop_name) VALUES (?)', (stop_name,))
                stop_id = cursor.lastrowid
            
            # 6. Map the Bus to the Stop with its Order
            cursor.execute('''
                INSERT INTO Route_Checklist (bus_id, stop_id, stop_order) 
                VALUES (?, ?, ?)
            ''', (bus_id, stop_id, order))

# Save all the changes and close the connection
conn.commit()
conn.close()

print("Successfully imported all buses, shifts, and generated the routes!")