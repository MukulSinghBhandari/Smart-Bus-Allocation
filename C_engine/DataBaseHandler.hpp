// C_engine/DataBaseHandler.hpp
//DataBaseHandler.hpp defines the DataBaseHandler class, which encapsulates the management of an SQLite database connection. 
//It provides methods for opening and closing the database, executing SQL commands, and retrieving error messages. 
//The class ensures proper resource management and prevents copying or moving to maintain a single database connection instance.
#pragma once

#include <string>
#include <stdexcept>

extern "C" {
    #include "sqlite3.h"
}

class DataBaseHandler {
private:
    sqlite3* db;

public:
    // Constructor
    explicit DataBaseHandler(const std::string& dbName)
        : db(nullptr) {

        int rc = sqlite3_open(dbName.c_str(), &db);

        if (rc != SQLITE_OK) {
            std::string error = "Failed to open SQLite database";

            if (db != nullptr) {
                error += ": ";
                error += sqlite3_errmsg(db);
                sqlite3_close(db);
                db = nullptr;
            }

            throw std::runtime_error(error);
        }
    }

    // Destructor
    ~DataBaseHandler() {
        if (db != nullptr) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    // Get SQLite database handle
    sqlite3* getHandle() const {
        return db;
    }

    // Execute SQL which does not require returning rows
    bool execute(const std::string& sql) {

        char* errorMessage = nullptr;

        int rc = sqlite3_exec(
            db,
            sql.c_str(),
            nullptr,
            nullptr,
            &errorMessage
        );

        if (rc != SQLITE_OK) {

            if (errorMessage != nullptr) {
                sqlite3_free(errorMessage);
            }

            return false;
        }

        return true;
    }

    // Get last SQLite error
    std::string getLastError() const {

        if (db == nullptr) {
            return "Database connection is null";
        }

        return sqlite3_errmsg(db);
    }

    // Prevent copying
    DataBaseHandler(const DataBaseHandler&) = delete;
    DataBaseHandler& operator=(const DataBaseHandler&) = delete;

    // Prevent accidental moving
    DataBaseHandler(DataBaseHandler&&) = delete;
    DataBaseHandler& operator=(DataBaseHandler&&) = delete;
};