// C_engine/BusModel.hpp
//BusModel.hpp defines the Bus class, which inherits from the Vehicle class. 
//It encapsulates bus-specific attributes and behaviors, such as bus number, shifts, parking zone, and available seats. 
//The class provides getter methods for these attributes and implements a displayInfo method to output bus details.
#pragma once
#include <string>
#include <iostream>

class Vehicle {
protected:
    int vehicleId;
    std::string registrationNo;
    int totalSeats;
public:
    Vehicle(int id, std::string reg, int seats) 
        : vehicleId(id), registrationNo(reg), totalSeats(seats) {}
    virtual ~Vehicle() = default;
    
    // Pure virtual function for polymorphism
    virtual void displayInfo() const = 0; 
};

class Bus : public Vehicle {
private:
    std::string busNumber;
    std::string morningShift;
    std::string eveningShift;
    std::string parkingZone;
    int availableSeats;

public:
    // Constructor
    Bus(int id, std::string bNo, std::string reg, std::string mShift, std::string eShift, std::string zone, int seats, int availSeats)
        : Vehicle(id, reg, seats), busNumber(bNo), morningShift(mShift), eveningShift(eShift), parkingZone(zone), availableSeats(availSeats) {}

    // Encapsulated getter methods
    std::string getBusNumber() const { return busNumber; }
    std::string getParkingZone() const { return parkingZone; }
    int getAvailableSeats() const { return availableSeats; }

    // Implementing the inherited virtual method (Polymorphism)
    void displayInfo() const override {
        std::cout << "Bus No: " << busNumber << " | Zone: " << parkingZone << " | Seats Left: " << availableSeats << "/" << totalSeats << std::endl;
    }
};