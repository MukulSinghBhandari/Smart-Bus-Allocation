#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

// Helper for case-insensitive string comparison
int my_strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        int c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

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

    // --- COMMAND: GET ALL BUSES ---
    if (strcmp(command, "get_all_buses") == 0) {
        const char *sql = "SELECT bus_id, bus_number, registration_number, morning_shift_one, morning_shift_two, evening_shift, conductor_name, conductor_phone FROM Buses;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
            printf("[]\n");
            sqlite3_close(db);
            return 1;
        }

        printf("[");
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *bNo = sqlite3_column_text(stmt, 1);
            const unsigned char *regNo = sqlite3_column_text(stmt, 2);
            const unsigned char *m1 = sqlite3_column_text(stmt, 3);
            const unsigned char *m2 = sqlite3_column_text(stmt, 4);
            const unsigned char *ev = sqlite3_column_text(stmt, 5);
            const unsigned char *conductor = sqlite3_column_text(stmt, 6);
            const unsigned char *phone = sqlite3_column_text(stmt, 7);

            // Fetch live status for seats and zone
            char zone[64] = "Gehu Gate 2";
            int seats = 50;
            sqlite3_stmt *stmt_live;
            const char *sql_live = "SELECT current_parking_zone, available_seats FROM Live_Status WHERE bus_id = ?;";
            if (sqlite3_prepare_v2(db, sql_live, -1, &stmt_live, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmt_live, 1, id);
                if (sqlite3_step(stmt_live) == SQLITE_ROW) {
                    const unsigned char *z = sqlite3_column_text(stmt_live, 0);
                    if (z) strcpy(zone, (const char*)z);
                    int s = sqlite3_column_int(stmt_live, 1);
                    if (s > 0) seats = s;
                }
                sqlite3_finalize(stmt_live);
            }

            if (!first) printf(",");
            printf("{\"busId\": %d, \"busNo\": \"%s\", \"registrationNo\": \"%s\", \"morningShiftOne\": \"%s\", \"morningShiftTwo\": \"%s\", \"eveningShift\": \"%s\", \"conductorName\": \"%s\", \"conductorPhone\": \"%s\", \"parkingZone\": \"%s\", \"availableSeats\": %d, \"totalSeats\": 50, \"route\": [",
                    id,
                    bNo ? (const char*)bNo : "",
                    regNo ? (const char*)regNo : "",
                    m1 ? (const char*)m1 : "",
                    m2 ? (const char*)m2 : "",
                    ev ? (const char*)ev : "",
                    conductor ? (const char*)conductor : "Not Assigned",
                    phone ? (const char*)phone : "",
                    zone,
                    seats);

            // Query Route Stops for this bus
            const char *sql_route = "SELECT Stops.stop_name FROM Route_Checklist "
                                    "JOIN Stops ON Route_Checklist.stop_id = Stops.stop_id "
                                    "WHERE Route_Checklist.bus_id = ? "
                                    "ORDER BY Route_Checklist.stop_order ASC;";
            sqlite3_stmt *stmt_route;
            if (sqlite3_prepare_v2(db, sql_route, -1, &stmt_route, 0) == SQLITE_OK) {
                sqlite3_bind_int(stmt_route, 1, id);
                int first_stop = 1;
                while (sqlite3_step(stmt_route) == SQLITE_ROW) {
                    const unsigned char *stop_name = sqlite3_column_text(stmt_route, 0);
                    if (!first_stop) printf(",");
                    printf("\"%s\"", stop_name ? (const char*)stop_name : "");
                    first_stop = 0;
                }
                sqlite3_finalize(stmt_route);
            }
            printf("]}");
            first = 0;
        }
        printf("]\n");
        sqlite3_finalize(stmt);
    }
    // --- COMMAND: GET BUS INFO ---
    else if (strcmp(command, "get_bus") == 0) {
        if (argc < 3) {
            printf("{\"error\": \"Missing bus number\"}\n");
            sqlite3_close(db);
            return 1;
        }
        char *bus_no = argv[2];

        // Select everything via JOIN ensuring broad match on bus_id or bus_number
        const char *sql_bus = "SELECT Buses.*, Live_Status.* FROM Buses "
                            "LEFT JOIN Live_Status ON Buses.bus_id = Live_Status.bus_id "
                            "WHERE CAST(Buses.bus_id AS TEXT) = ? OR CAST(Buses.bus_number AS TEXT) = ?;";

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql_bus, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int bus_id = 0;
            const char *bNo = "";
            const char *regNo = "";
            const char *m1 = "";
            const char *m2 = "";
            const char *eShift = "";
            const char *conductor = "Not Assigned";
            const char *phone = "";
            const char *zone = "Gehu Gate 2";
            int seats = 50;
            int current_stop_id = 0;

            int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const char *col_name = sqlite3_column_name(stmt, i);
                const unsigned char *val = sqlite3_column_text(stmt, i);
                if (!col_name) continue;

                if (my_strcasecmp(col_name, "bus_id") == 0 && val) {
                    if (bus_id == 0) bus_id = atoi((const char*)val);
                } else if ((my_strcasecmp(col_name, "bus_number") == 0 || my_strcasecmp(col_name, "bus_no") == 0) && val) {
                    bNo = (const char*)val;
                } else if ((my_strcasecmp(col_name, "registration_number") == 0 || my_strcasecmp(col_name, "reg_no") == 0) && val) {
                    regNo = (const char*)val;
                } else if ((my_strcasecmp(col_name, "morning_shift_one") == 0 || my_strcasecmp(col_name, "morning_shift") == 0) && val) {
                    m1 = (const char*)val;
                } else if ((my_strcasecmp(col_name, "morning_shift_two") == 0 || my_strcasecmp(col_name, "afternoon_shift") == 0) && val) {
                    m2 = (const char*)val;
                } else if (my_strcasecmp(col_name, "evening_shift") == 0 && val) {
                    eShift = (const char*)val;
                } else if ((my_strcasecmp(col_name, "conductor_name") == 0 || my_strcasecmp(col_name, "conductor") == 0) && val) {
                    conductor = (const char*)val;
                } else if ((my_strcasecmp(col_name, "conductor_phone") == 0 || my_strcasecmp(col_name, "phone") == 0) && val) {
                    phone = (const char*)val;
                } else if (my_strcasecmp(col_name, "current_parking_zone") == 0 && val) {
                    zone = (const char*)val;
                } else if (my_strcasecmp(col_name, "available_seats") == 0 && val) {
                    int s = atoi((const char*)val);
                    if (s >= 0) seats = s;
                } else if (my_strcasecmp(col_name, "current_stop_id") == 0 && val) {
                    current_stop_id = atoi((const char*)val);
                }
            }

            sqlite3_finalize(stmt);

            // Query Route Checklist
            const char *sql_route = "SELECT Stops.stop_id, Stops.stop_name FROM Route_Checklist "
                                    "JOIN Stops ON Route_Checklist.stop_id = Stops.stop_id "
                                    "WHERE Route_Checklist.bus_id = ? "
                                    "ORDER BY Route_Checklist.stop_order ASC;";
            sqlite3_stmt *stmt_route;
            sqlite3_prepare_v2(db, sql_route, -1, &stmt_route, 0);
            sqlite3_bind_int(stmt_route, 1, bus_id);

            printf("{\"ok\": true, \"busId\": %d, \"busNo\": \"%s\", \"registrationNo\": \"%s\", "
                   "\"morningShiftOne\": \"%s\", \"morningShiftTwo\": \"%s\", \"eveningShift\": \"%s\", "
                   "\"conductorName\": \"%s\", \"conductorPhone\": \"%s\", "
                   "\"parkingZone\": \"%s\", \"availableSeats\": %d, \"totalSeats\": 50, "
                   "\"currentStopId\": %d, \"route\": [",
                   bus_id, bNo, regNo, m1, m2, eShift, conductor, phone, zone, seats, current_stop_id);

            int first = 1;
            while (sqlite3_step(stmt_route) == SQLITE_ROW) {
                int stop_id = sqlite3_column_int(stmt_route, 0);
                const unsigned char *stop_name = sqlite3_column_text(stmt_route, 1);
                if (!first) printf(",");
                printf("{\"stopId\": %d, \"stopName\": \"%s\"}", stop_id, stop_name ? (const char*)stop_name : "");
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

        const char *sql = "UPDATE Live_Status SET current_parking_zone = ? WHERE bus_id = ? OR bus_id IN (SELECT bus_id FROM Buses WHERE CAST(bus_id AS TEXT) = ? OR bus_number = ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, zone, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, bus_no, -1, SQLITE_STATIC);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        printf("{\"ok\": true, \"message\": \"Parking zone updated\"}\n");
    }
    // --- COMMAND: GET BUSES BY STOP NAME ---
    else if (strcmp(command, "get_buses_by_stop") == 0) {
        if (argc < 3) {
            printf("{\"error\": \"Missing station name\"}\n");
            sqlite3_close(db);
            return 1;
        }
        char *stop_query = argv[2];

        const char *sql = "SELECT DISTINCT Buses.bus_id, Buses.bus_number, Buses.registration_number, "
                          "Buses.morning_shift_one, Buses.morning_shift_two, Buses.evening_shift, "
                          "Buses.conductor_name, Buses.conductor_phone "
                          "FROM Buses "
                          "JOIN Route_Checklist ON Buses.bus_id = Route_Checklist.bus_id "
                          "JOIN Stops ON Route_Checklist.stop_id = Stops.stop_id "
                          "WHERE Stops.stop_name LIKE ?;";

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        
        char wildcard_query[256];
        snprintf(wildcard_query, sizeof(wildcard_query), "%%%s%%", stop_query);
        sqlite3_bind_text(stmt, 1, wildcard_query, -1, SQLITE_STATIC);

        printf("{\"ok\": true, \"station\": \"%s\", \"buses\": [", stop_query);
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int bus_id = sqlite3_column_int(stmt, 0);
            const unsigned char *bNo = sqlite3_column_text(stmt, 1);
            const unsigned char *regNo = sqlite3_column_text(stmt, 2);
            const unsigned char *m1 = sqlite3_column_text(stmt, 3);
            const unsigned char *m2 = sqlite3_column_text(stmt, 4);
            const unsigned char *ev = sqlite3_column_text(stmt, 5);
            const unsigned char *conductor = sqlite3_column_text(stmt, 6);
            const unsigned char *phone = sqlite3_column_text(stmt, 7);

            if (!first) printf(",");
            printf("{\"busId\": %d, \"busNo\": \"%s\", \"registrationNo\": \"%s\", \"morningShiftOne\": \"%s\", \"morningShiftTwo\": \"%s\", \"eveningShift\": \"%s\", \"conductorName\": \"%s\", \"conductorPhone\": \"%s\"}",
                   bus_id,
                   bNo ? (const char*)bNo : "",
                   regNo ? (const char*)regNo : "",
                   m1 ? (const char*)m1 : "",
                   m2 ? (const char*)m2 : "",
                   ev ? (const char*)ev : "",
                   conductor ? (const char*)conductor : "Not Assigned",
                   phone ? (const char*)phone : "");
            first = 0;
        }
        printf("]}\n");
        sqlite3_finalize(stmt);
    }
    // --- COMMAND: UPDATE SEATS ---
    else if (strcmp(command, "update_seats") == 0) {
        if (argc < 4) {
            printf("{\"error\": \"Missing parameters\"}\n");
            sqlite3_close(db);
            return 1;
        }
        char *bus_no = argv[2];
        int seats = atoi(argv[3]);

        const char *sql = "UPDATE Live_Status SET available_seats = ? WHERE bus_id = ? OR bus_id IN (SELECT bus_id FROM Buses WHERE CAST(bus_id AS TEXT) = ? OR bus_number = ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_int(stmt, 1, seats);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, bus_no, -1, SQLITE_STATIC);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        printf("{\"ok\": true, \"availableSeats\": %d}\n", seats);
    }
    else {
        printf("{\"error\": \"Unknown command\"}\n");
    }

    sqlite3_close(db);
    return 0;
}