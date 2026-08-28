#ifndef DATABASE_H
#define DATABASE_H

#include "bus.h"
#include "graph.h"

typedef enum {
    DATABASE_OK = 0,
    DATABASE_BAD_ARGS,
    DATABASE_OPEN_FAIL,
    DATABASE_EMPTY,
    DATABASE_QUERY_FAIL
} DatabaseStatus;

DatabaseStatus databaseLoadGraph(const char *dbPath, Graph **graphOut);
DatabaseStatus databaseLoadBuses(const char *dbPath, BusList *busList);
int databaseWriteMiniGraph(const char *dbPath);
int databaseWriteEmptyGraph(const char *dbPath);

#endif
