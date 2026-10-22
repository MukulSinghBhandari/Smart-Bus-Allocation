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
    const char* sql =
        "SELECT DISTINCT "
        "Buses.bus_id, "
        "Buses.bus_number "
        "FROM Buses "
        "JOIN Route_Checklist "
        "ON Buses.bus_id = Route_Checklist.bus_id "
        "JOIN Stops "
        "ON Route_Checklist.stop_id = Stops.stop_id "
        "WHERE Stops.stop_name LIKE ? "
        "ORDER BY Buses.bus_id;";

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
        int busId =
            sqlite3_column_int(stmt, 0);

        string busNo =
            getText(stmt, 1);

        if (!first)
        {
            cout << ",";
        }

        first = false;

        cout
            << "{"
            << "\"busId\":"
            << busId
            << ","
            << "\"busNo\":\""
            << jsonEscape(busNo)
            << "\""
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