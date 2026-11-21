// This file defines the DataBaseHandler class.
// It manages the connection between our C++ engine and SQLite database.
// It provides functions to open, close and execute SQL commands.
// It also handles database errors and prevents copying or moving the object.
#pragma once  
#include <string>     
#include <stdexcept>  
// Including SQLite C library inside C linkage
// extern "C" prevents C++ name mangling
extern "C" {
    #include "sqlite3.h"  
}

using namespace std; 
// -------------------- DATABASE HANDLER CLASS --------------------
class DataBaseHandler {
private:
    sqlite3* db;  // Pointer to the SQLite database connection
public:
// explicit prevents unwanted automatic conversion into DataBaseHandler objects
    explicit DataBaseHandler(const string& dbName): db(nullptr) {
        // dbName.c_str() converts the string into a C-style string
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc != SQLITE_OK) {
            string error = "Failed to open SQLite database";
            // Checks whether the database pointer contains a valid handle
            if (db != nullptr) {
                // Adds a separator to the error message
                error += ": ";
                // Gets the actual error message from SQLite
                error += sqlite3_errmsg(db);
                // Closes the database connection
                sqlite3_close(db);
                // Resets the database pointer to nullptr
                db = nullptr;
            }
            // Throws an exception if database opening fails
            throw runtime_error(error);
        }
    }
    
    ~DataBaseHandler() {
        // Checks whether the database connection exists
        if (db != nullptr) {
            // Closes the SQLite database connection
            sqlite3_close(db);
            // Resets the pointer to nullptr
            db = nullptr;
        }
    }
    // -------------------- GET DATABASE HANDLE --------------------
    // Returns the SQLite database connection pointer
    sqlite3* getHandle() const {
        return db;
    }
    // -------------------- EXECUTE SQL COMMAND --------------------
    // Executes an SQL command that does not need to return rows
    bool execute(const string& sql) {
        char* errorMessage = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage);
        // Checks whether SQL command execution failed
        if (rc != SQLITE_OK) {
            if (errorMessage != nullptr) {
                // Frees the memory allocated for the error message
                sqlite3_free(errorMessage);
            }
            // Returns false if SQL execution fails
            return false;
        }
        return true;
    }
    // -------------------- GET LAST ERROR --------------------
    string getLastError() const {
        if (db == nullptr) {
            return "Database connection is null";
        }
        // Returns the last SQLite error message
        return sqlite3_errmsg(db);
    }
    // -------------------- PREVENT COPYING --------------------
    // Copy constructor is deleted
    // Prevents creating another DataBaseHandler object by copying an existing one
    DataBaseHandler(const DataBaseHandler&) = delete;

    // Copy assignment operator is deleted
    // Prevents assigning one database handler object to another
    DataBaseHandler& operator=(const DataBaseHandler&) = delete;
    // -------------------- PREVENT MOVING --------------------
    // Move constructor is deleted
    // Prevents transferring the database handler object using move construction
    DataBaseHandler(DataBaseHandler&&) = delete;

    // Move assignment operator is deleted
    // Prevents transferring the database handler using move assignment
    DataBaseHandler& operator=(DataBaseHandler&&) = delete;
};