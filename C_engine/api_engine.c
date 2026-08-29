//api_engine.c
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
        const char *sql = "SELECT * FROM Buses;";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
            printf("[]\n");
            sqlite3_close(db);
            return 1;
        }

        printf("[");
        int first = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = 0;
            const char *bNo = "";
            const char *regNo = "";
            const char *m1 = "";
            const char *m2 = "";
            const char *ev = "";

            int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const char *col_name = sqlite3_column_name(stmt, i);
                const unsigned char *val = sqlite3_column_text(stmt, i);
                if (!col_name || !val) continue;

                if (my_strcasecmp(col_name, "bus_id") == 0) id = atoi((const char*)val);
                else if (my_strcasecmp(col_name, "bus_number") == 0 || my_strcasecmp(col_name, "bus_no") == 0) bNo = (const char*)val;
                else if (my_strcasecmp(col_name, "registration_number") == 0 || my_strcasecmp(col_name, "reg_no") == 0 || my_strcasecmp(col_name, "regno") == 0) regNo = (const char*)val;
                else if (my_strcasecmp(col_name, "morning_shift_one") == 0 || my_strcasecmp(col_name, "morning_shift") == 0) m1 = (const char*)val;
                else if (my_strcasecmp(col_name, "morning_shift_two") == 0 || my_strcasecmp(col_name, "afternoon_shift") == 0) m2 = (const char*)val;
                else if (my_strcasecmp(col_name, "evening_shift") == 0) ev = (const char*)val;
            }

            if (!first) printf(",");
            printf("{\"id\": %d, \"busNo\": \"%s\", \"registrationNo\": \"%s\", \"morningShiftOne\": \"%s\", \"morningShiftTwo\": \"%s\", \"eveningShift\": \"%s\"}", 
                   id, bNo, regNo, m1, m2, ev);
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

        // Select everything using JOIN so dynamic mapping can catch all columns regardless of order
        const char *sql_bus = "SELECT * FROM Buses "
                            "LEFT JOIN Live_Status ON Buses.bus_id = Live_Status.bus_id "
                            "WHERE CAST(Buses.bus_number AS TEXT) = ? OR CAST(Buses.bus_id AS TEXT) = ?;";

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
            const char *zone = "";
            int seats = 50;

            int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const char *col_name = sqlite3_column_name(stmt, i);
                const unsigned char *val = sqlite3_column_text(stmt, i);
                if (!col_name) continue;

                if (my_strcasecmp(col_name, "bus_id") == 0 && val) {
                    bus_id = atoi((const char*)val);
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
                    seats = atoi((const char*)val);
                }
            }
            if (seats <= 0) seats = 50;

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
                   "\"parkingZone\": \"%s\", \"availableSeats\": %d, \"totalSeats\": 50, \"route\": [",
                   bus_id, bNo, regNo, m1, m2, eShift, conductor, phone, zone, seats);

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

        const char *sql = "UPDATE Live_Status SET current_parking_zone = ? WHERE bus_id = ? OR bus_id = (SELECT bus_id FROM Buses WHERE bus_number = ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, zone, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bus_no, -1, SQLITE_STATIC);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        printf("{\"ok\": true, \"message\": \"Parking zone updated\"}\n");
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

        const char *sql = "UPDATE Live_Status SET available_seats = ? WHERE bus_id = ? OR bus_id = (SELECT bus_id FROM Buses WHERE bus_number = ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_int(stmt, 1, seats);
        sqlite3_bind_text(stmt, 2, bus_no, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bus_no, -1, SQLITE_STATIC);

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
