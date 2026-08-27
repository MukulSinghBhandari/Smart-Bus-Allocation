-- Table 1: All Unique Bus Stops in the City
CREATE TABLE Stops (
    stop_id INTEGER PRIMARY KEY,
    stop_name TEXT NOT NULL
);

-- Table 2: Fleet Details
CREATE TABLE Buses (
    bus_id INTEGER PRIMARY KEY,
    bus_number TEXT NOT NULL,
    registration_number TEXT,
    shift_timing TEXT,
    conductor_name TEXT,
    conductor_phone TEXT,
    total_capacity INTEGER DEFAULT 50
);

-- Table 3: The Route Checklist
CREATE TABLE Route_Checklist (
    checklist_id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id INTEGER,
    stop_id INTEGER,
    stop_order INTEGER, 
    FOREIGN KEY(bus_id) REFERENCES Buses(bus_id),
    FOREIGN KEY(stop_id) REFERENCES Stops(stop_id)
);

-- Table 4: The Live Tracker
CREATE TABLE Live_Status (
    status_id INTEGER PRIMARY KEY,
    bus_id INTEGER,
    current_parking_zone TEXT,
    current_stop_id INTEGER,
    available_seats INTEGER,
    FOREIGN KEY(bus_id) REFERENCES Buses(bus_id),
    FOREIGN KEY(current_stop_id) REFERENCES Stops(stop_id)
);