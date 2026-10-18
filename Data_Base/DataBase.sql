-- Table 1: All Unique Bus Stops in the City
CREATE TABLE Stops (
    stop_id INTEGER PRIMARY KEY AUTOINCREMENT,
    stop_name TEXT NOT NULL UNIQUE
);

-- Table 2: Fleet Details (Updated to match the new shift structure)
CREATE TABLE Buses (
    bus_id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_number TEXT NOT NULL,
    registration_number TEXT,
    morning_shift_one TEXT,
    morning_shift_two TEXT,
    evening_shift TEXT,
    conductor_name TEXT,
    conductor_phone TEXT,
    total_capacity INTEGER DEFAULT 50
);

-- Table 3: The Route Checklist (Maps the ordered sequence of stops for each bus route)
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
    status_id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id INTEGER,
    current_parking_zone TEXT,
    current_stop_id INTEGER,
    available_seats INTEGER,
    FOREIGN KEY(bus_id) REFERENCES Buses(bus_id),
    FOREIGN KEY(current_stop_id) REFERENCES Stops(stop_id)
);

-- AUTOMATIC TRIGGER: Initializes Live_Status row safely without forcing stop_id = 1
CREATE TRIGGER trg_init_live_status
AFTER INSERT ON Buses
BEGIN
    INSERT INTO Live_Status (bus_id, current_parking_zone, current_stop_id, available_seats)
    VALUES (NEW.bus_id, 'Gehu Gate 2', NULL, NEW.total_capacity);
END;