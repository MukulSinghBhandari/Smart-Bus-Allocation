PRAGMA foreign_keys = ON;

-- Table 1: All unique bus stops
CREATE TABLE IF NOT EXISTS Stops (
    stop_id INTEGER PRIMARY KEY AUTOINCREMENT,
    stop_name TEXT NOT NULL UNIQUE
);

-- Table 2: Fleet details
CREATE TABLE IF NOT EXISTS Buses (
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

-- Table 3: Ordered stops for each bus route
CREATE TABLE IF NOT EXISTS Route_Checklist (
    checklist_id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id INTEGER,
    stop_id INTEGER,
    stop_order INTEGER,
    FOREIGN KEY (bus_id) REFERENCES Buses(bus_id),
    FOREIGN KEY (stop_id) REFERENCES Stops(stop_id)
);

-- Table 4: Live bus status
CREATE TABLE IF NOT EXISTS Live_Status (
    status_id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id INTEGER UNIQUE,
    current_parking_zone TEXT,
    current_stop_id INTEGER,
    available_seats INTEGER,
    FOREIGN KEY (bus_id) REFERENCES Buses(bus_id),
    FOREIGN KEY (current_stop_id) REFERENCES Stops(stop_id)
);

-- Trigger: Initialize live status for every new bus
CREATE TRIGGER IF NOT EXISTS trg_init_live_status
AFTER INSERT ON Buses
FOR EACH ROW
BEGIN
    INSERT INTO Live_Status (
        bus_id,
        current_parking_zone,
        current_stop_id,
        available_seats
    )
    VALUES (
        NEW.bus_id,
        'Gehu Gate 2',
        NULL,
        NEW.total_capacity
    );
END;