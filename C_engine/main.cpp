//The C++ code in this file is part of the Campus Transit System project. 
// It serves as the main entry point for the application, handling various commands related to bus management and live status updates.
// The code interacts with an SQLite database to retrieve and update information about buses, their routes, and their current status.
#include <iostream>
#include <string>
#include <vector>

#include "DataBaseHandler.hpp"
#include "BusModel.hpp"

/*
    C Algorithm Layer
    -----------------
    graph.c is compiled as C.

    extern "C" prevents C++ name mangling
    so that C++ can call the C function.
*/
extern "C"
{
#include "algorithms/graph.h"
#include "algorithms/queue.h"
#include "algorithms/priority_queue.h"
}

using namespace std;


// =========================================================
// Convert SQLite text safely to std::string
// =========================================================

string getText(sqlite3_stmt* stmt, int column)
{
    const unsigned char* text =
        sqlite3_column_text(stmt, column);

    if (text == nullptr)
    {
        return "";
    }

    return reinterpret_cast<const char*>(text);
}


// =========================================================
// Escape string for JSON
// IT IS IMPORTANT TO ESCAPE STRINGS FOR JSON TO AVOID SYNTAX ERRORS AND SECURITY ISSUES.
// =========================================================

string jsonEscape(const string& input)
{
    string output;

    for (char c : input)
    {
        switch (c)
        {
            case '"':
                output += "\\\"";
                break;

            case '\\':
                output += "\\\\";
                break;

            case '\n':
                output += "\\n";
                break;

            case '\r':
                output += "\\r";
                break;

            case '\t':
                output += "\\t";
                break;

            default:
                output += c;
        }
    }

    return output;
}


// =========================================================
// Find bus_id using BUS NUMBER ONLY 
// =========================================================

int findBusId(sqlite3* db, const string& busNo)
{
    const char* sql =
        "SELECT bus_id "
        "FROM Buses "
        "WHERE bus_number = ? "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        return -1;
    }

    sqlite3_bind_text(
        stmt,
        1,
        busNo.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    int busId = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        busId =
            sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return busId;
}


// =========================================================
// GET ALL BUSES
// =========================================================

bool getBusRoute(sqlite3* db, int busId);

bool getAllBuses(sqlite3* db)
{
    const char* sql =
        "SELECT "
        "B.bus_id, "
        "B.bus_number, "
        "B.registration_number, "
        "B.morning_shift_one, "
        "B.morning_shift_two, "
        "B.evening_shift, "
        "B.conductor_name, "
        "B.conductor_phone, "
        "B.total_capacity, "
        "L.current_parking_zone, "
        "L.available_seats, "
        "L.current_stop_id "
        "FROM Buses B "
        "LEFT JOIN Live_Status L "
        "ON B.bus_id = L.bus_id "
        "ORDER BY B.bus_id;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "{\"ok\":false,"
             << "\"error\":\"Failed to prepare get_all_buses query\"}\n";
        return false;
    }

    cout << "[";

    bool first = true;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int busId = sqlite3_column_int(stmt, 0);
        string busNo = getText(stmt, 1);
        string registrationNo = getText(stmt, 2);
        string morningShiftOne = getText(stmt, 3);
        string morningShiftTwo = getText(stmt, 4);
        string eveningShift = getText(stmt, 5);
        string conductorName = getText(stmt, 6);
        string conductorPhone = getText(stmt, 7);
        int totalCapacity = sqlite3_column_int(stmt, 8);
        string parkingZone = getText(stmt, 9);

        int availableSeats = 0;
        if (sqlite3_column_type(stmt, 10) != SQLITE_NULL)
        {
            availableSeats = sqlite3_column_int(stmt, 10);
        }

        int currentStopId = -1;
        if (sqlite3_column_type(stmt, 11) != SQLITE_NULL)
        {
            currentStopId = sqlite3_column_int(stmt, 11);
        }

        if (!first)
        {
            cout << ",";
        }

        first = false;

        cout << "{"
             << "\"busId\":" << busId << ","
             << "\"busNo\":\"" << jsonEscape(busNo) << "\","
             << "\"registrationNo\":\"" << jsonEscape(registrationNo) << "\","
             << "\"morningShiftOne\":\"" << jsonEscape(morningShiftOne) << "\","
             << "\"morningShiftTwo\":\"" << jsonEscape(morningShiftTwo) << "\","
             << "\"eveningShift\":\"" << jsonEscape(eveningShift) << "\","
             << "\"conductorName\":\""
             << jsonEscape(conductorName.empty() ? "Not Assigned" : conductorName)
             << "\","
             << "\"conductorPhone\":\"" << jsonEscape(conductorPhone) << "\","
             << "\"parkingZone\":\"" << jsonEscape(parkingZone) << "\","
             << "\"availableSeats\":" << availableSeats << ","
             << "\"totalSeats\":" << totalCapacity << ","
             << "\"currentStopId\":" << currentStopId << ",";

        if (!getBusRoute(db, busId))
        {
            cout << "\"route\":[]";
        }

        cout << "}";
    }

    cout << "]\n";

    sqlite3_finalize(stmt);
    return true;
}

// =========================================================
// GET ROUTE FOR A BUS
// =========================================================

bool getBusRoute(sqlite3* db, int busId)
{
    const char* sql =
        "SELECT "
        "R.stop_id, "
        "S.stop_name, "
        "R.stop_order "
        "FROM Route_Checklist R "
        "JOIN Stops S "
        "ON R.stop_id = S.stop_id "
        "WHERE R.bus_id = ? "
        "ORDER BY R.stop_order;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(
        stmt,
        1,
        busId
    );

    cout << "\"route\":[";

    bool first = true;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int stopId =
            sqlite3_column_int(stmt, 0);

        string stopName =
            getText(stmt, 1);

        int stopOrder =
            sqlite3_column_int(stmt, 2);

        if (!first)
        {
            cout << ",";
        }

        first = false;

        cout
            << "{"
            << "\"stopId\":"
            << stopId
            << ","

            << "\"stopName\":\""
            << jsonEscape(stopName)
            << "\","

            << "\"stopOrder\":"
            << stopOrder

            << "}";
    }

    cout << "]";

    sqlite3_finalize(stmt);

    return true;
}


// =========================================================
// GET SINGLE BUS
// =========================================================

bool getBus(
    sqlite3* db,
    const string& busNo
)
{
    const char* sql =
        "SELECT "
        "B.bus_id, "
        "B.bus_number, "
        "B.registration_number, "
        "B.morning_shift_one, "
        "B.morning_shift_two, "
        "B.evening_shift, "
        "B.conductor_name, "
        "B.conductor_phone, "
        "B.total_capacity, "
        "L.current_parking_zone, "
        "L.available_seats, "
        "L.current_stop_id "
        "FROM Buses B "
        "LEFT JOIN Live_Status L "
        "ON B.bus_id = L.bus_id "
        "WHERE B.bus_number = ? "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to prepare get_bus query\"}\n";

        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        busNo.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);

        cout
            << "{\"ok\":false,"
            << "\"error\":\"Bus not found in database\"}\n";

        return false;
    }

    int busId =
        sqlite3_column_int(stmt, 0);

    string busNumber =
        getText(stmt, 1);

    string registrationNo =
        getText(stmt, 2);

    string morningShiftOne =
        getText(stmt, 3);

    string morningShiftTwo =
        getText(stmt, 4);

    string eveningShift =
        getText(stmt, 5);

    string conductorName =
        getText(stmt, 6);

    string conductorPhone =
        getText(stmt, 7);

    int totalCapacity =
        sqlite3_column_int(stmt, 8);

    string parkingZone =
        getText(stmt, 9);

    int availableSeats = 0;

    if (sqlite3_column_type(stmt, 10)
        != SQLITE_NULL)
    {
        availableSeats =
            sqlite3_column_int(stmt, 10);
    }

    int currentStopId = -1;

    if (sqlite3_column_type(stmt, 11)
        != SQLITE_NULL)
    {
        currentStopId =
            sqlite3_column_int(stmt, 11);
    }

    sqlite3_finalize(stmt);

    cout
        << "{"
        << "\"ok\":true,"

        << "\"busId\":"
        << busId
        << ","

        << "\"busNo\":\""
        << jsonEscape(busNumber)
        << "\","

        << "\"registrationNo\":\""
        << jsonEscape(registrationNo)
        << "\","

        << "\"morningShiftOne\":\""
        << jsonEscape(morningShiftOne)
        << "\","

        << "\"morningShiftTwo\":\""
        << jsonEscape(morningShiftTwo)
        << "\","

        << "\"eveningShift\":\""
        << jsonEscape(eveningShift)
        << "\","

        << "\"conductorName\":\""
        << jsonEscape(
            conductorName.empty()
                ? "Not Assigned"
                : conductorName
        )
        << "\","

        << "\"conductorPhone\":\""
        << jsonEscape(conductorPhone)
        << "\","

        << "\"parkingZone\":\""
        << jsonEscape(parkingZone)
        << "\","

        << "\"availableSeats\":"
        << availableSeats
        << ","

        << "\"totalSeats\":"
        << totalCapacity
        << ","

        << "\"currentStopId\":"
        << currentStopId
        << ",";

    if (!getBusRoute(db, busId))
    {
        cout
            << "\"route\":[]";
    }

    cout << "}\n";

    return true;
}


// =========================================================
// UPDATE PARKING ZONE
// =========================================================

bool updateZone(
    sqlite3* db,
    const string& busNo,
    const string& zone
)
{
    int busId =
        findBusId(db, busNo);

    if (busId == -1)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Bus not found\"}\n";

        return false;
    }

    const char* sql =
        "UPDATE Live_Status "
        "SET current_parking_zone = ? "
        "WHERE bus_id = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to prepare zone update\"}\n";

        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        zone.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_int(
        stmt,
        2,
        busId
    );

    int rc =
        sqlite3_step(stmt);

    int changes =
        sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE || changes == 0)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Parking zone update failed\"}\n";

        return false;
    }

    cout
        << "{\"ok\":true,"
        << "\"message\":\"Parking zone updated\","
        << "\"parkingZone\":\""
        << jsonEscape(zone)
        << "\"}\n";

    return true;
}


// =========================================================
// UPDATE SEATS
// =========================================================

bool updateSeats(
    sqlite3* db,
    const string& busNo,
    int seats
)
{
    int busId =
        findBusId(db, busNo);

    if (busId == -1)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Bus not found\"}\n";

        return false;
    }

    const char* capacitySql =
        "SELECT total_capacity "
        "FROM Buses "
        "WHERE bus_id = ?;";

    sqlite3_stmt* capacityStmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            capacitySql,
            -1,
            &capacityStmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to read bus capacity\"}\n";

        return false;
    }

    sqlite3_bind_int(
        capacityStmt,
        1,
        busId
    );

    int capacity = 50;

    if (sqlite3_step(capacityStmt)
        == SQLITE_ROW)
    {
        capacity =
            sqlite3_column_int(
                capacityStmt,
                0
            );
    }

    sqlite3_finalize(capacityStmt);

    if (seats < 0 || seats > capacity)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Invalid seat count\","
            << "\"totalSeats\":"
            << capacity
            << "}\n";

        return false;
    }

    const char* sql =
        "UPDATE Live_Status "
        "SET available_seats = ? "
        "WHERE bus_id = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to prepare seat update\"}\n";

        return false;
    }

    sqlite3_bind_int(
        stmt,
        1,
        seats
    );

    sqlite3_bind_int(
        stmt,
        2,
        busId
    );

    int rc =
        sqlite3_step(stmt);

    int changes =
        sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE || changes == 0)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Seat update failed\"}\n";

        return false;
    }

    cout
        << "{\"ok\":true,"
        << "\"availableSeats\":"
        << seats
        << ","
        << "\"totalSeats\":"
        << capacity
        << "}\n";

    return true;
}


// =========================================================
// GET BUSES BY STOP
// =========================================================

bool getBusesByStop(
    sqlite3* db,
    const string& stopQuery
)
{
    // Return a bus only if the searched stop is still ahead of it
    // or is its current stop. A bus that has already passed the
    // searched stop must not appear in the student search results.
    const char* sql =
        "SELECT DISTINCT "
        "B.bus_id, "
        "B.bus_number "
        "FROM Buses B "
        "JOIN Route_Checklist R "
        "ON B.bus_id = R.bus_id "
        "JOIN Stops S "
        "ON R.stop_id = S.stop_id "
        "LEFT JOIN Live_Status L "
        "ON B.bus_id = L.bus_id "
        "WHERE S.stop_name LIKE ? "
        "AND ( "
        "    L.current_stop_id IS NULL "
        "    OR EXISTS ( "
        "        SELECT 1 "
        "        FROM Route_Checklist C "
        "        WHERE C.bus_id = B.bus_id "
        "          AND C.stop_id = L.current_stop_id "
        "          AND C.stop_order <= R.stop_order "
        "    ) "
        ") "
        "ORDER BY B.bus_id;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to prepare stop search\"}\n";

        return false;
    }

    string wildcard =
        "%" + stopQuery + "%";

    sqlite3_bind_text(
        stmt,
        1,
        wildcard.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    cout
        << "{\"ok\":true,"
        << "\"station\":\""
        << jsonEscape(stopQuery)
        << "\","
        << "\"buses\":[";

    bool first = true;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int busId = sqlite3_column_int(stmt, 0);
        string busNo = getText(stmt, 1);

        if (!first)
        {
            cout << ",";
        }

        first = false;

        cout
            << "{"
            << "\"busId\":" << busId << ","
            << "\"busNo\":\"" << jsonEscape(busNo) << "\""
            << "}";
    }

    cout << "]}\n";
    sqlite3_finalize(stmt);
    return true;
}

// =========================================================
// GET ROUTE STOP IDS
//
// This function gets only the stop IDs.
// The actual NEXT STOP calculation will be
// performed by the C Graph algorithm.
// =========================================================

bool getRouteStopIds(
    sqlite3* db,
    int busId,
    vector<int>& route
)
{
    route.clear();

    const char* sql =
        "SELECT stop_id "
        "FROM Route_Checklist "
        "WHERE bus_id = ? "
        "ORDER BY stop_order;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        ) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(
        stmt,
        1,
        busId
    );

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int stopId =
            sqlite3_column_int(stmt, 0);

        route.push_back(stopId);
    }

    sqlite3_finalize(stmt);

    return !route.empty();
}


// =========================================================
// ADVANCE BUS TO NEXT STOP
//
// IMPORTANT:
// The next-stop decision is now performed by
// the C Graph Algorithm:
//
//     graph_get_next_stop()
//
// C++ handles:
//     - SQLite
//     - database update
//     - JSON
//
// C handles:
//     - route graph
//     - next-stop calculation
// =========================================================

bool advanceStop(
    sqlite3* db,
    const string& busNo
)
{
    // -----------------------------------------------------
    // Find bus ID
    // -----------------------------------------------------

    int busId =
        findBusId(db, busNo);

    if (busId == -1)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Bus not found\"}\n";

        return false;
    }


    // -----------------------------------------------------
    // Get current stop
    // -----------------------------------------------------

    const char* currentSql =
        "SELECT current_stop_id "
        "FROM Live_Status "
        "WHERE bus_id = ?;";

    sqlite3_stmt* currentStmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            currentSql,
            -1,
            &currentStmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to read current stop\"}\n";

        return false;
    }

    sqlite3_bind_int(
        currentStmt,
        1,
        busId
    );

    int currentStopId = -1;

    if (sqlite3_step(currentStmt)
        == SQLITE_ROW)
    {
        if (
            sqlite3_column_type(
                currentStmt,
                0
            ) != SQLITE_NULL
        )
        {
            currentStopId =
                sqlite3_column_int(
                    currentStmt,
                    0
                );
        }
    }

    sqlite3_finalize(currentStmt);


    // -----------------------------------------------------
    // Get complete route
    // -----------------------------------------------------

    vector<int> route;

    if (!getRouteStopIds(
            db,
            busId,
            route
        ))
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"No route found for bus\"}\n";

        return false;
    }


    // -----------------------------------------------------
    // CALL C GRAPH ALGORITHM
    // -----------------------------------------------------

    int nextStopId =
        graph_get_next_stop(
            route.data(),
            static_cast<int>(route.size()),
            currentStopId
        );


    // -----------------------------------------------------
    // No next stop
    // -----------------------------------------------------

    if (nextStopId == -1)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Already at final stop\"}\n";

        return false;
    }


    // -----------------------------------------------------
    // Update current_stop_id
    // -----------------------------------------------------

    const char* updateSql =
        "UPDATE Live_Status "
        "SET current_stop_id = ? "
        "WHERE bus_id = ?;";

    sqlite3_stmt* updateStmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            updateSql,
            -1,
            &updateStmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to update current stop\"}\n";

        return false;
    }

    sqlite3_bind_int(
        updateStmt,
        1,
        nextStopId
    );

    sqlite3_bind_int(
        updateStmt,
        2,
        busId
    );

    int rc =
        sqlite3_step(updateStmt);

    int changes =
        sqlite3_changes(db);

    sqlite3_finalize(updateStmt);

    if (rc != SQLITE_DONE || changes == 0)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Current stop update failed\"}\n";

        return false;
    }


    // -----------------------------------------------------
    // Return complete updated bus
    // -----------------------------------------------------

    return getBus(
        db,
        busNo
    );
}


// =========================================================
// END TRIP
//
// Resets the bus for a completely new trip:
//   1. Current stop -> first stop of the route
//   2. Parking zone -> NULL
//   3. Available seats -> full capacity
//
// The bus number is used to identify the bus.
// =========================================================

bool endTrip(
    sqlite3* db,
    const string& busNo
)
{
    int busId = findBusId(db, busNo);

    if (busId == -1)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Bus not found\"}\n";
        return false;
    }

    vector<int> route;

    if (!getRouteStopIds(db, busId, route))
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"No route found for bus\"}\n";
        return false;
    }

    const int firstStopId = route.front();
    const int finalStopId = route.back();

    const char* capacitySql =
        "SELECT total_capacity "
        "FROM Buses "
        "WHERE bus_id = ?;";

    sqlite3_stmt* capacityStmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            capacitySql,
            -1,
            &capacityStmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to read bus capacity\"}\n";
        return false;
    }

    sqlite3_bind_int(capacityStmt, 1, busId);

    int capacity = 0;
    const int capacityResult = sqlite3_step(capacityStmt);

    if (capacityResult == SQLITE_ROW)
    {
        capacity = sqlite3_column_int(capacityStmt, 0);
    }

    sqlite3_finalize(capacityStmt);

    if (capacityResult != SQLITE_ROW || capacity <= 0)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Invalid bus capacity\"}\n";
        return false;
    }

    const char* updateSql =
        "UPDATE Live_Status "
        "SET current_stop_id = ?, "
        "current_parking_zone = NULL, "
        "available_seats = ? "
        "WHERE bus_id = ? "
        "AND current_stop_id = ? "
        "AND current_parking_zone IS NOT NULL "
        "AND current_parking_zone <> '';";

    sqlite3_stmt* updateStmt = nullptr;

    if (sqlite3_prepare_v2(
            db,
            updateSql,
            -1,
            &updateStmt,
            nullptr
        ) != SQLITE_OK)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to prepare end-trip reset\"}\n";
        return false;
    }

    sqlite3_bind_int(updateStmt, 1, firstStopId);
    sqlite3_bind_int(updateStmt, 2, capacity);
    sqlite3_bind_int(updateStmt, 3, busId);
    sqlite3_bind_int(updateStmt, 4, finalStopId);

    const int rc = sqlite3_step(updateStmt);
    const int changes = sqlite3_changes(db);

    sqlite3_finalize(updateStmt);

    if (rc != SQLITE_DONE)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Failed to reset bus for next trip\"}\n";
        return false;
    }

    if (changes == 0)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Trip can only end after reaching the final stop\"}\n";
        return false;
    }

    return getBus(db, busNo);
}


// =========================================================
// DSA MODULE: PERSISTENT STUDENT REQUEST QUEUE + BUS PRIORITY QUEUE
// =========================================================
//
// Queue:
//   Student allocation requests are processed in FIFO order.
//
// Priority Queue (Max Heap):
//   1. More available seats = higher priority.
//   2. Smaller pickup stop_order = higher priority.
//   3. Smaller bus_id = tie-breaker.
//
// Only buses that:
//   - have enough seats,
//   - contain pickup and destination in the correct order,
//   - have not already passed the pickup stop
// are inserted into the C Priority Queue.
//
// The C Queue is in-memory, so requests are first stored in SQLite.
// Each process reconstructs the FIFO queue from PENDING rows.
//
// =========================================================

bool ensureAllocationRequestTable(sqlite3* db)
{
    const char* sql =
        "CREATE TABLE IF NOT EXISTS Allocation_Requests ("
        "request_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "pickup_stop_id INTEGER NOT NULL,"
        "destination_stop_id INTEGER NOT NULL,"
        "seats_requested INTEGER NOT NULL CHECK(seats_requested > 0),"
        "status TEXT NOT NULL DEFAULT 'PENDING' "
        "CHECK(status IN ('PENDING','ALLOCATED')),"
        "assigned_bus_id INTEGER,"
        "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "processed_at TEXT,"
        "FOREIGN KEY(pickup_stop_id) REFERENCES Stops(stop_id),"
        "FOREIGN KEY(destination_stop_id) REFERENCES Stops(stop_id),"
        "FOREIGN KEY(assigned_bus_id) REFERENCES Buses(bus_id),"
        "CHECK(pickup_stop_id <> destination_stop_id)"
        ");";

    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK)
    {
        cout << "{\"ok\":false,\"error\":\""
             << jsonEscape(errorMessage ? errorMessage : sqlite3_errmsg(db))
             << "\"}\n";
        if (errorMessage) sqlite3_free(errorMessage);
        return false;
    }

    return true;
}


bool stopExists(sqlite3* db, int stopId)
{
    const char* sql = "SELECT 1 FROM Stops WHERE stop_id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, stopId);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}


// Command:
//   transit_engine.exe submit_request <pickup_stop_id> <destination_stop_id> <seats>
bool submitAllocationRequest(
    sqlite3* db,
    int pickupStopId,
    int destinationStopId,
    int seatsRequested
)
{
    if (pickupStopId <= 0 || destinationStopId <= 0 ||
        pickupStopId == destinationStopId || seatsRequested <= 0)
    {
        cout << "{\"ok\":false,\"error\":\"Invalid pickup, destination or seat count\"}\n";
        return false;
    }

    if (!stopExists(db, pickupStopId) || !stopExists(db, destinationStopId))
    {
        cout << "{\"ok\":false,\"error\":\"Pickup or destination stop does not exist\"}\n";
        return false;
    }

    const char* sql =
        "INSERT INTO Allocation_Requests "
        "(pickup_stop_id, destination_stop_id, seats_requested, status) "
        "VALUES (?, ?, ?, 'PENDING');";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "{\"ok\":false,\"error\":\"Could not prepare allocation request\"}\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, pickupStopId);
    sqlite3_bind_int(stmt, 2, destinationStopId);
    sqlite3_bind_int(stmt, 3, seatsRequested);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        cout << "{\"ok\":false,\"error\":\"Could not save allocation request: "
             << jsonEscape(sqlite3_errmsg(db)) << "\"}\n";
        return false;
    }

    sqlite3_int64 requestId = sqlite3_last_insert_rowid(db);

    cout << "{\"ok\":true,\"message\":\"Request added to FIFO queue\","
         << "\"requestId\":" << requestId
         << ",\"status\":\"PENDING\"}\n";

    return true;
}


// Insert every currently eligible bus into the C Max Heap.
// The caller owns and must destroy the returned Priority Queue.
BusPriorityQueue* buildEligibleBusPriorityQueue(
    sqlite3* db,
    int pickupStopId,
    int destinationStopId,
    int seatsRequested
)
{
    const char* countSql = "SELECT COUNT(*) FROM Buses;";
    sqlite3_stmt* countStmt = nullptr;
    int capacity = 1;

    if (sqlite3_prepare_v2(db, countSql, -1, &countStmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(countStmt) == SQLITE_ROW)
            capacity = sqlite3_column_int(countStmt, 0);

        sqlite3_finalize(countStmt);
    }

    if (capacity < 1) capacity = 1;

    BusPriorityQueue* busQueue = pq_create(capacity);
    if (busQueue == nullptr) return nullptr;

    const char* sql =
        "SELECT B.bus_id, L.available_seats, MIN(P.stop_order) "
        "FROM Buses B "
        "JOIN Live_Status L ON L.bus_id = B.bus_id "
        "JOIN Route_Checklist P ON P.bus_id = B.bus_id "
        "JOIN Route_Checklist D ON D.bus_id = B.bus_id "
        "WHERE P.stop_id = ? "
        "AND D.stop_id = ? "
        "AND P.stop_order < D.stop_order "
        "AND L.available_seats >= ? "
        "AND ("
        "    L.current_stop_id IS NULL "
        "    OR EXISTS ("
        "        SELECT 1 FROM Route_Checklist C "
        "        WHERE C.bus_id = B.bus_id "
        "        AND C.stop_id = L.current_stop_id "
        "        AND C.stop_order <= P.stop_order"
        "    )"
        ") "
        "GROUP BY B.bus_id, L.available_seats;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        pq_destroy(busQueue);
        return nullptr;
    }

    sqlite3_bind_int(stmt, 1, pickupStopId);
    sqlite3_bind_int(stmt, 2, destinationStopId);
    sqlite3_bind_int(stmt, 3, seatsRequested);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        BusCandidate candidate;
        candidate.bus_id = sqlite3_column_int(stmt, 0);
        candidate.available_seats = sqlite3_column_int(stmt, 1);
        candidate.pickup_order = sqlite3_column_int(stmt, 2);

        if (!pq_insert(busQueue, candidate))
        {
            sqlite3_finalize(stmt);
            pq_destroy(busQueue);
            return nullptr;
        }
    }

    sqlite3_finalize(stmt);
    return busQueue;
}


// Process one request. BEGIN IMMEDIATE prevents another writer from
// reserving the same seats between candidate selection and seat update.
bool allocateOneRequest(
    sqlite3* db,
    const AllocationRequest& request,
    int& assignedBusId
)
{
    assignedBusId = -1;

    if (sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr) != SQLITE_OK)
        return false;

    BusPriorityQueue* busQueue = buildEligibleBusPriorityQueue(
        db,
        request.pickup_stop_id,
        request.destination_stop_id,
        request.seats_requested
    );

    if (busQueue == nullptr)
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    BusCandidate candidate;
    bool allocated = false;

    while (pq_extract_max(busQueue, &candidate))
    {
        const char* updateSeatsSql =
            "UPDATE Live_Status "
            "SET available_seats = available_seats - ? "
            "WHERE bus_id = ? AND available_seats >= ?;";

        sqlite3_stmt* seatsStmt = nullptr;

        if (sqlite3_prepare_v2(
                db, updateSeatsSql, -1, &seatsStmt, nullptr
            ) != SQLITE_OK)
        {
            pq_destroy(busQueue);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        sqlite3_bind_int(seatsStmt, 1, request.seats_requested);
        sqlite3_bind_int(seatsStmt, 2, candidate.bus_id);
        sqlite3_bind_int(seatsStmt, 3, request.seats_requested);

        int updateRc = sqlite3_step(seatsStmt);
        int changedRows = sqlite3_changes(db);
        sqlite3_finalize(seatsStmt);

        if (updateRc != SQLITE_DONE)
        {
            pq_destroy(busQueue);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        // A seat count may have changed since the candidate list was built.
        // If so, try the next bus in priority order.
        if (changedRows != 1)
            continue;

        const char* requestUpdateSql =
            "UPDATE Allocation_Requests "
            "SET status = 'ALLOCATED', assigned_bus_id = ?, "
            "processed_at = CURRENT_TIMESTAMP "
            "WHERE request_id = ? AND status = 'PENDING';";

        sqlite3_stmt* requestStmt = nullptr;

        if (sqlite3_prepare_v2(
                db, requestUpdateSql, -1, &requestStmt, nullptr
            ) != SQLITE_OK)
        {
            pq_destroy(busQueue);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        sqlite3_bind_int(requestStmt, 1, candidate.bus_id);
        sqlite3_bind_int(requestStmt, 2, request.request_id);

        int requestRc = sqlite3_step(requestStmt);
        int requestChanges = sqlite3_changes(db);
        sqlite3_finalize(requestStmt);

        if (requestRc != SQLITE_DONE || requestChanges != 1)
        {
            pq_destroy(busQueue);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }

        assignedBusId = candidate.bus_id;
        allocated = true;
        break;
    }

    pq_destroy(busQueue);

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // If no bus was eligible, the request remains PENDING for a later retry.
    return allocated;
}


// Rebuild the in-memory C FIFO queue from persistent PENDING requests,
// process them in request_id order, and return one JSON result per request.
bool processPendingAllocationRequests(sqlite3* db)
{
    const char* pendingSql =
        "SELECT request_id, pickup_stop_id, destination_stop_id, seats_requested "
        "FROM Allocation_Requests "
        "WHERE status = 'PENDING' "
        "ORDER BY request_id ASC;";

    sqlite3_stmt* pendingStmt = nullptr;

    if (sqlite3_prepare_v2(db, pendingSql, -1, &pendingStmt, nullptr) != SQLITE_OK)
    {
        cout << "{\"ok\":false,\"error\":\"Could not read pending requests\"}\n";
        return false;
    }

    // Take a stable snapshot of pending rows, then size the C queue exactly.
    vector<AllocationRequest> pendingRequests;

    while (sqlite3_step(pendingStmt) == SQLITE_ROW)
    {
        AllocationRequest request;
        request.request_id = sqlite3_column_int(pendingStmt, 0);
        request.pickup_stop_id = sqlite3_column_int(pendingStmt, 1);
        request.destination_stop_id = sqlite3_column_int(pendingStmt, 2);
        request.seats_requested = sqlite3_column_int(pendingStmt, 3);
        pendingRequests.push_back(request);
    }

    sqlite3_finalize(pendingStmt);

    int queueCapacity = pendingRequests.empty()
        ? 1
        : static_cast<int>(pendingRequests.size());

    RequestQueue* requestQueue = queue_create(queueCapacity);

    if (requestQueue == nullptr)
    {
        cout << "{\"ok\":false,\"error\":\"Could not create request queue\"}\n";
        return false;
    }

    for (const AllocationRequest& request : pendingRequests)
    {
        if (!queue_enqueue(requestQueue, request))
        {
            queue_destroy(requestQueue);
            cout << "{\"ok\":false,\"error\":\"Could not enqueue pending request\"}\n";
            return false;
        }
    }

    cout << "{\"ok\":true,\"processed\":[";

    bool firstResult = true;
    AllocationRequest request;

    while (queue_dequeue(requestQueue, &request))
    {
        int assignedBusId = -1;
        bool allocated = allocateOneRequest(db, request, assignedBusId);

        if (!firstResult) cout << ",";
        firstResult = false;

        cout << "{"
             << "\"requestId\":" << request.request_id << ","
             << "\"status\":\"" << (allocated ? "ALLOCATED" : "PENDING") << "\","
             << "\"assignedBusId\":";

        if (allocated)
            cout << assignedBusId;
        else
            cout << "null";

        cout << "}";

        // Strict FIFO: if the oldest request cannot be allocated now,
        // leave it pending and do not let later requests jump ahead.
        if (!allocated)
            break;
    }

    cout << "]}\n";
    queue_destroy(requestQueue);

    return true;
}


// =========================================================
// MAIN
// =========================================================

int main(
    int argc,
    char* argv[]
)
{
    if (argc < 2)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"No command provided\"}\n";

        return 1;
    }

    string command =
        argv[1];

    try
    {
        // -------------------------------------------------
        // Open database
        // -------------------------------------------------

        DataBaseHandler dbHandler(
            "campus_transit.db"
        );

        sqlite3* db =
            dbHandler.getHandle();

        // Create the allocation-request table if it does not exist.
        // Existing tables and CSV data are not modified.
        if (!ensureAllocationRequestTable(db))
            return 1;


        // -------------------------------------------------
        // GET ALL BUSES
        // -------------------------------------------------

        if (command == "get_all_buses")
        {
            return getAllBuses(db)
                ? 0
                : 1;
        }


        // -------------------------------------------------
        // GET SINGLE BUS
        // -------------------------------------------------

        else if (command == "get_bus")
        {
            if (argc < 3)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Missing bus number\"}\n";

                return 1;
            }

            return getBus(
                db,
                argv[2]
            )
                ? 0
                : 1;
        }


        // -------------------------------------------------
        // UPDATE ZONE
        // -------------------------------------------------

        else if (command == "update_zone")
        {
            if (argc < 4)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Missing parameters\"}\n";

                return 1;
            }

            return updateZone(
                db,
                argv[2],
                argv[3]
            )
                ? 0
                : 1;
        }


        // -------------------------------------------------
        // UPDATE SEATS
        // -------------------------------------------------

        else if (command == "update_seats")
        {
            if (argc < 4)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Missing parameters\"}\n";

                return 1;
            }

            try
            {
                int seats =
                    stoi(argv[3]);

                return updateSeats(
                    db,
                    argv[2],
                    seats
                )
                    ? 0
                    : 1;
            }
            catch (...)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Invalid seat value\"}\n";

                return 1;
            }
        }


        // -------------------------------------------------
        // GET BUSES BY STOP
        // -------------------------------------------------

        else if (command == "get_buses_by_stop")
        {
            if (argc < 3)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Missing station name\"}\n";

                return 1;
            }

            return getBusesByStop(
                db,
                argv[2]
            )
                ? 0
                : 1;
        }


        // -------------------------------------------------
        // ADVANCE STOP
        // -------------------------------------------------

        else if (command == "advance_stop")
        {
            if (argc < 3)
            {
                cout
                    << "{\"ok\":false,"
                    << "\"error\":\"Missing bus number\"}\n";

                return 1;
            }

            return advanceStop(
                db,
                argv[2]
            )
                ? 0
                : 1;
        }


        // ---------------------------------------------------------
// END TRIP
// ---------------------------------------------------------

else if (command == "end_trip")
{
    if (argc < 3)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\"Missing bus number\"}\n";

        return 1;
    }

    return endTrip(
        db,
        argv[2]
    )
        ? 0
        : 1;
}


        // -------------------------------------------------
        // SUBMIT STUDENT ALLOCATION REQUEST
        // Usage: transit_engine.exe submit_request <pickup_stop_id> <destination_stop_id> <seats>
        // -------------------------------------------------

        else if (command == "submit_request")
        {
            if (argc < 5)
            {
                cout << "{\"ok\":false,\"error\":\"Usage: submit_request <pickup_stop_id> <destination_stop_id> <seats>\"}\n";
                return 1;
            }

            try
            {
                int pickupStopId = stoi(argv[2]);
                int destinationStopId = stoi(argv[3]);
                int seatsRequested = stoi(argv[4]);

                return submitAllocationRequest(
                    db, pickupStopId, destinationStopId, seatsRequested
                ) ? 0 : 1;
            }
            catch (...)
            {
                cout << "{\"ok\":false,\"error\":\"Invalid numeric request parameter\"}\n";
                return 1;
            }
        }

        // -------------------------------------------------
        // PROCESS PENDING REQUESTS USING C FIFO QUEUE + C MAX HEAP
        // Usage: transit_engine.exe process_requests
        // -------------------------------------------------

        else if (command == "process_requests")
        {
            return processPendingAllocationRequests(db) ? 0 : 1;
        }


        // -------------------------------------------------
        // UNKNOWN COMMAND
        // -------------------------------------------------

        else
        {
            cout
                << "{\"ok\":false,"
                << "\"error\":\"Unknown command\"}\n";

            return 1;
        }
    }

    catch (const exception& e)
    {
        cout
            << "{\"ok\":false,"
            << "\"error\":\""
            << jsonEscape(e.what())
            << "\"}\n";

        return 1;
    }

    return 0;
}
