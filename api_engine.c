#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("{\"error\": \"No command provided\"}\n");
        return 1;
    }

    char *command = argv[1];
    sqlite3 *db;
    
    int rc = sqlite3_open("campus_transit.db", &db);
    if (rc != SQLITE_OK) {
        printf("{\"error\": \"Cannot open database\"}\n");
        return 1;
    }

    // --- COMMAND: GET BUS INFO ---
    if (strcmp(command, "get_bus") == 0) {
        if (argc < 3) {
            printf("{\"error\": \"Missing bus number\"}\n");
            sqlite3_close(db);
            return 1;
        }
        char *bus_no = argv[2];
        
        // 1. Query Bus Details and Live Status
        const char *sql_bus = "SELECT Buses.bus_id, Buses.bus_number, Buses.registration_number, Buses.shift_timing, Buses.conductor_name, Buses.conductor_phone, Live_Status.current_parking_zone, Live_Status.available_seats "
                              "FROM Buses "
                              "LEFT JOIN Live_Status ON Buses.bus_id = Live_Status.bus_id "
                              "WHERE Buses.bus_number = ?;";
        
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql_bus, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, bus_no, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int bus_id = sqlite3_column_int(stmt, 0);
            const unsigned char *bNo = sqlite3_column_text(stmt, 1);
            const unsigned char *regNo = sqlite3_column_text(stmt, 2);
            const unsigned char *timing = sqlite3_column_text(stmt, 3);
            const unsigned char *conductor = sqlite3_column_text(stmt, 4);
            const unsigned char *phone = sqlite3_column_text(stmt, 5);
            const unsigned char *zone = sqlite3_column_text(stmt, 6);
            int seats = sqlite3_column_int(stmt, 7);
            if(seats <= 0) seats = 50; // Default capacity fallback

            sqlite3_finalize(stmt);

            // 2. Query Route Stops for this specific bus
            const char *sql_route = "SELECT Stops.stop_id, Stops.stop_name FROM Route_Checklist "
                                    "JOIN Stops ON Route_Checklist.stop_id = Stops.stop_id "
                                    "WHERE Route_Checklist.bus_id = ? "
                                    "ORDER BY Route_Checklist.stop_order ASC;";
            sqlite3_stmt *stmt_route;
            sqlite3_prepare_v2(db, sql_route, -1, &stmt_route, 0);
            sqlite3_bind_int(stmt_route, 1, bus_id);

            // Output JSON matching conductor.js requirements exactly
            printf("{\"ok\": true, \"busNo\": \"%s\", \"registrationNo\": \"%s\", \"timing\": \"%s\", \"conductorName\": \"%s\", \"conductorPhone\": \"%s\", \"parkingZone\": \"%s\", \"availableSeats\": %d, \"totalSeats\": 50, \"currentStopId\": 1, \"tripStatus\": 0, \"route\": [",
                   bNo ? (const char*)bNo : "", 
                   regNo ? (const char*)regNo : "UK07AB1234",
                   timing ? (const char*)timing : "", 
                   conductor ? (const char*)conductor : "Not Assigned", 
                   phone ? (const char*)phone : "", 
                   zone ? (const char*)zone : "", 
                   seats);

            int first = 1;
            while (sqlite3_step(stmt_route) == SQLITE_ROW) {
                int stop_id = sqlite3_column_int(stmt_route, 0);
                const unsigned char *stop_name = sqlite3_column_text(stmt_route, 1);
                if (!first) {
                    printf(",");
                }
                printf("{\"stopId\": %d, \"stopName\": \"%s\"}", stop_id, stop_name);
                first = 0;
            }
            printf("]}\n");
            sqlite3_finalize(stmt_route);

        } else {
            printf("{\"error\": \"Bus not found in database\"}\n");
            sqlite3_finalize(stmt);
        }
    }
    // --- COMMAND: UPDATE PARKING ZONE ---
    else if (strcmp(command, "update_zone") == 0) {
        if (argc < 4) {
            printf("{\"error\": \"Missing parameters\"}\n");
            sqlite3_close(db);
            return 1;
        }
        char *bus_no = argv[2];
        char *zone = argv[3];

        const char *sql = "UPDATE Live_Status SET current_parking_zone = ? WHERE bus_id = (SELECT bus_id FROM Buses WHERE bus_number = ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, zone, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        printf("{\"ok\": true, \"message\": \"Parking zone updated\"}\n");
    }

    sqlite3_close(db);
    return 0;
}