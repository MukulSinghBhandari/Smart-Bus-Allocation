#include "output.h"

#include <stdio.h>

void outputPrintError(OutputFormat format, const char *code,
                      const char *message) {
    if (format == OUTPUT_MACHINE) {
        printf("STATUS=ERROR\n");
        printf("CODE=%s\n", code);
        printf("MESSAGE=%s\n", message);
    } else {
        printf("Error [%s]: %s\n", code, message);
    }
}

void outputPrintRoute(OutputFormat format, const Graph *graph,
                      const PathResult *path, const BusMatch *bus) {
    int i;

    if (graph == NULL || path == NULL || bus == NULL) {
        return;
    }

    if (!path->found) {
        outputPrintError(format, "NO_ROUTE", "No route found between stops");
        return;
    }

    if (format == OUTPUT_MACHINE) {
        printf("STATUS=OK\n");
        printf("DISTANCE=%d\n", path->distance);
        printf("STOP_COUNT=%d\n", path->pathLen);
        printf("ROUTE=");
        for (i = 0; i < path->pathLen; i++) {
            printf("%s", graph->names[path->path[i]]);
            if (i < path->pathLen - 1) {
                printf("|");
            }
        }
        printf("\n");

        if (bus->found) {
            printf("BUS_ID=%d\n", bus->busId);
            printf("SEATS=%d\n", bus->availableSeats);
            printf("PARKING_ZONE=%s\n", bus->parkingZone);
        } else {
            printf("BUS_ID=NONE\n");
            printf("SEATS=0\n");
            printf("PARKING_ZONE=NONE\n");
        }
        return;
    }

    printf("Distance: %d\n", path->distance);
    printf("Stop count: %d\n", path->pathLen);
    printf("Route: ");
    for (i = 0; i < path->pathLen; i++) {
        printf("%s", graph->names[path->path[i]]);
        if (i < path->pathLen - 1) {
            printf(" -> ");
        }
    }
    printf("\n");

    if (bus->found) {
        printf("Bus: BUS_%d\n", bus->busId);
        printf("Seats: %d\n", bus->availableSeats);
        printf("Parking zone: %s\n", bus->parkingZone);
    } else {
        printf("Bus: none available\n");
    }
}

void outputPrintPrefix(OutputFormat format, const char results[][64],
                       int count) {
    int i;

    if (format == OUTPUT_MACHINE) {
        printf("STATUS=OK\n");
        printf("MATCH_COUNT=%d\n", count);
        printf("MATCHES=");
        for (i = 0; i < count; i++) {
            printf("%s", results[i]);
            if (i < count - 1) {
                printf("|");
            }
        }
        printf("\n");
        return;
    }

    printf("Matches: %d\n", count);
    for (i = 0; i < count; i++) {
        printf("  %s\n", results[i]);
    }
}
