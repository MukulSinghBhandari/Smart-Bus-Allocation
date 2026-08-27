import sqlite3
import csv

# 1. Connect to the database
conn = sqlite3.connect('campus_transit.db')
cursor = conn.cursor()

# 2. Open the CSV file
with open('buses.csv', 'r', encoding='utf-8') as file:
    # Read the CSV as a dictionary
    reader = csv.DictReader(file)
    
    for row in reader:
        bus_no = row['Bus No'].strip()
        reg_no = row['Reg No'].strip()
        timing = row['Timing'].strip()
        conductor = row['Conductor'].strip()
        phone = row['Phone'].strip()
        route_string = row['Route'].strip()

        # 3. Insert the Bus details
        cursor.execute('''
            INSERT INTO Buses (bus_number, registration_number, shift_timing, conductor_name, conductor_phone) 
            VALUES (?, ?, ?, ?, ?)
        ''', (bus_no, reg_no, timing, conductor, phone))
        
        bus_id = cursor.lastrowid # Get the ID of the bus we just inserted

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
            
            # 6. Map the Bus to the Stop with its Order (1st, 2nd, 3rd, etc.)
            cursor.execute('''
                INSERT INTO Route_Checklist (bus_id, stop_id, stop_order) 
                VALUES (?, ?, ?)
            ''', (bus_id, stop_id, order))

# Save all the changes and close the connection
conn.commit()
conn.close()

print("Successfully imported all buses and generated the routes!")