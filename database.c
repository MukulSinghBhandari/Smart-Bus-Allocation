#include "database.h"

#include "sqlite3.h"

#include <stdio.h>
#include <string.h>

static void databaseClose(sqlite3 *db, sqlite3_stmt *stmt) {
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
    if (db != NULL) {
        sqlite3_close(db);
    }
}

int databaseWriteMiniGraph(const char *dbPath) {
    sqlite3 *db = NULL;
    char *err = NULL;
    int rc;
    const char *sql =
        "CREATE TABLE Stops ("
        "  stop_id INTEGER PRIMARY KEY,"
        "  stop_name TEXT NOT NULL"
        ");"
        "CREATE TABLE Edges ("
        "  edge_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  from_stop_id INTEGER NOT NULL,"
        "  to_stop_id INTEGER NOT NULL,"
        "  distance INTEGER NOT NULL"
        ");"
        "INSERT INTO Stops(stop_id, stop_name) VALUES"
        "  (1, 'A'), (2, 'B'), (3, 'C'), (4, 'D');"
        "INSERT INTO Edges(from_stop_id, to_stop_id, distance) VALUES"
        "  (1, 2, 5), (1, 3, 2), (2, 4, 3), (3, 4, 4);"
        "CREATE TABLE Buses ("
        "  bus_id INTEGER PRIMARY KEY,"
        "  bus_number TEXT NOT NULL"
        ");"
        "CREATE TABLE Route_Checklist ("
        "  checklist_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  bus_id INTEGER,"
        "  stop_id INTEGER,"
        "  stop_order INTEGER"
        ");"
        "CREATE TABLE Live_Status ("
        "  status_id INTEGER PRIMARY KEY,"
        "  bus_id INTEGER,"
        "  current_parking_zone TEXT,"
        "  current_stop_id INTEGER,"
        "  available_seats INTEGER"
        ");"
        "INSERT INTO Buses(bus_id, bus_number) VALUES (17, 'BUS_17'), (22, 'BUS_22');"
        "INSERT INTO Route_Checklist(bus_id, stop_id, stop_order) VALUES"
        "  (17, 1, 1), (17, 3, 2), (17, 4, 3),"
        "  (22, 1, 1), (22, 3, 2), (22, 4, 3);"
        "INSERT INTO Live_Status"
        "  (status_id, bus_id, current_parking_zone, current_stop_id, available_seats)"
        "  VALUES (1, 17, 'ZoneA', 1, 12), (2, 22, 'ZoneB', 1, 0);";

    if (dbPath == NULL) {
        return 0;
    }

    remove(dbPath);

    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return 0;
    }

    rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    sqlite3_free(err);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return 0;
    }

    databaseClose(db, NULL);
    return 1;
}

int databaseWriteEmptyGraph(const char *dbPath) {
    sqlite3 *db = NULL;
    char *err = NULL;
    int rc;
    const char *sql =
        "CREATE TABLE Stops ("
        "  stop_id INTEGER PRIMARY KEY,"
        "  stop_name TEXT NOT NULL"
        ");"
        "CREATE TABLE Edges ("
        "  edge_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  from_stop_id INTEGER NOT NULL,"
        "  to_stop_id INTEGER NOT NULL,"
        "  distance INTEGER NOT NULL"
        ");";

    if (dbPath == NULL) {
        return 0;
    }

    remove(dbPath);
    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return 0;
    }

    rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    sqlite3_free(err);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return 0;
    }

    databaseClose(db, NULL);
    return 1;
}

DatabaseStatus databaseLoadGraph(const char *dbPath, Graph **graphOut) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    Graph *graph;
    int rc;
    int srcId;
    int destId;
    int weight;
    const char *name;
    const char *srcName;
    const char *destName;

    if (dbPath == NULL || graphOut == NULL) {
        return DATABASE_BAD_ARGS;
    }

    rc = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return DATABASE_OPEN_FAIL;
    }

    graph = graphCreate(16);
    if (graph == NULL) {
        databaseClose(db, NULL);
        return DATABASE_QUERY_FAIL;
    }

    rc = sqlite3_prepare_v2(db, "SELECT stop_name FROM Stops ORDER BY stop_id;",
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        graphFree(graph);
        databaseClose(db, stmt);
        return DATABASE_QUERY_FAIL;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        name = (const char *)sqlite3_column_text(stmt, 0);
        if (name == NULL || name[0] == '\0' || graphAddVertex(graph, name) < 0) {
            graphFree(graph);
            databaseClose(db, stmt);
            return DATABASE_QUERY_FAIL;
        }
    }

    sqlite3_finalize(stmt);
    stmt = NULL;

    if (graph->numVertices == 0) {
        graphFree(graph);
        databaseClose(db, NULL);
        return DATABASE_EMPTY;
    }

    rc = sqlite3_prepare_v2(db,
                            "SELECT s1.stop_name, s2.stop_name, e.distance "
                            "FROM Edges e "
                            "JOIN Stops s1 ON e.from_stop_id = s1.stop_id "
                            "JOIN Stops s2 ON e.to_stop_id = s2.stop_id;",
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        graphFree(graph);
        databaseClose(db, stmt);
        return DATABASE_QUERY_FAIL;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        srcName = (const char *)sqlite3_column_text(stmt, 0);
        destName = (const char *)sqlite3_column_text(stmt, 1);
        weight = sqlite3_column_int(stmt, 2);

        srcId = graphFindVertex(graph, srcName);
        destId = graphFindVertex(graph, destName);
        if (srcId < 0 || destId < 0 || weight <= 0) {
            graphFree(graph);
            databaseClose(db, stmt);
            return DATABASE_QUERY_FAIL;
        }

        graphAddEdge(graph, srcId, destId, weight);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    *graphOut = graph;
    return DATABASE_OK;
}

static int databaseLoadBusStops(sqlite3 *db, int busId, char stopBuf[][BUS_NAME_LEN],
                                const char *stopPtrs[], int *stopCount) {
    sqlite3_stmt *stmt = NULL;
    const char *name;
    int n = 0;
    int rc;

    rc = sqlite3_prepare_v2(db,
                            "SELECT s.stop_name "
                            "FROM Route_Checklist rc "
                            "JOIN Stops s ON rc.stop_id = s.stop_id "
                            "WHERE rc.bus_id = ? "
                            "ORDER BY rc.stop_order ASC;",
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, busId);

    while (n < BUS_MAX_STOPS && sqlite3_step(stmt) == SQLITE_ROW) {
        name = (const char *)sqlite3_column_text(stmt, 0);
        if (name == NULL || name[0] == '\0') {
            sqlite3_finalize(stmt);
            return 0;
        }
        strncpy(stopBuf[n], name, BUS_NAME_LEN - 1);
        stopBuf[n][BUS_NAME_LEN - 1] = '\0';
        stopPtrs[n] = stopBuf[n];
        n++;
    }

    sqlite3_finalize(stmt);
    *stopCount = n;
    return 1;
}

DatabaseStatus databaseLoadBuses(const char *dbPath, BusList *busList) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int rc;
    int busId;
    int seats;
    const char *zone;
    char zoneBuf[BUS_NAME_LEN];
    char stopBuf[BUS_MAX_STOPS][BUS_NAME_LEN];
    const char *stopPtrs[BUS_MAX_STOPS];
    int stopCount;

    if (dbPath == NULL || busList == NULL) {
        return DATABASE_BAD_ARGS;
    }

    busListInit(busList);

    rc = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        databaseClose(db, NULL);
        return DATABASE_OPEN_FAIL;
    }

    rc = sqlite3_prepare_v2(db,
                            "SELECT b.bus_id, "
                            "COALESCE(ls.available_seats, 0), "
                            "COALESCE(ls.current_parking_zone, '') "
                            "FROM Buses b "
                            "LEFT JOIN Live_Status ls ON ls.bus_id = b.bus_id;",
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        databaseClose(db, stmt);
        return DATABASE_QUERY_FAIL;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        busId = sqlite3_column_int(stmt, 0);
        seats = sqlite3_column_int(stmt, 1);
        zone = (const char *)sqlite3_column_text(stmt, 2);
        if (zone == NULL) {
            zoneBuf[0] = '\0';
        } else {
            strncpy(zoneBuf, zone, BUS_NAME_LEN - 1);
            zoneBuf[BUS_NAME_LEN - 1] = '\0';
        }

        if (!databaseLoadBusStops(db, busId, stopBuf, stopPtrs, &stopCount)) {
            databaseClose(db, stmt);
            return DATABASE_QUERY_FAIL;
        }

        if (stopCount == 0) {
            continue;
        }

        if (!busListAdd(busList, busId, seats, zoneBuf, stopPtrs, stopCount)) {
            databaseClose(db, stmt);
            return DATABASE_QUERY_FAIL;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return DATABASE_OK;
}
