
// BusModel.hpp
// This file defines the Bus class, which inherits from the Vehicle class.
// It stores bus details such as bus number, shifts, parking zone and available seats.
// It also provides getter functions and a function to display bus information.
#pragma once  // Prevents this header file from being included multiple times
#include <string>   
#include <iostream> 
using namespace std; 
// -------------------- VEHICLE CLASS --------------------
// Vehicle is the parent (base) class.
// It contains common properties of a vehicle.
class Vehicle {
protected:
    int vehicleId;            
    string registrationNo;    
    int totalSeats;            

public: 
    Vehicle(int id, string reg, int seats) : vehicleId(id),registrationNo(reg),totalSeats(seats) {}   
    virtual ~Vehicle() = default;
    virtual void displayInfo() const = 0;
};
// -------------------- BUS CLASS --------------------
// Bus is the child (derived) class of Vehicle.
// Public inheritance means Bus is also a type of Vehicle.
class Bus : public Vehicle {

private: 
    string busNumber;       
    string morningShift;    
    string eveningShift;    
    string parkingZone;     
    int availableSeats;     

public:
    Bus(int id, string bNo, string reg,string mShift, string eShift,string zone, int seats, int availSeats): 
        Vehicle(id, reg, seats),
        busNumber(bNo),             
        morningShift(mShift),       
        eveningShift(eShift),       
        parkingZone(zone),          
        availableSeats(availSeats)  
    {}


    // -------------------- GETTER FUNCTIONS --------------------
    string getBusNumber() const {
        return busNumber;
    }
    string getParkingZone() const {
        return parkingZone;
    }
    int getAvailableSeats() const {
        return availableSeats;
    }
    // -------------------- DISPLAY FUNCTION --------------------
    // override confirms that we are overriding the parent class function
    void displayInfo() const override {
        cout << "Bus No: " << busNumber
             << " | Zone: " << parkingZone
             << " | Seats Left: " << availableSeats
             << "/" << totalSeats
             << endl;
    }
};
