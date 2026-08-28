#include "bus.h"

#include <stdio.h>
#include <string.h>

void busListInit(BusList *list) {
    if (list == NULL) {
        return;
    }
    list->busCount = 0;
}

int busListAdd(BusList *list, int busId, int availableSeats,
               const char *parkingZone, const char *stops[], int stopCount) {
    Bus *bus;
    int i;

    if (list == NULL || parkingZone == NULL || stops == NULL ||
        stopCount <= 0 || stopCount > BUS_MAX_STOPS ||
        list->busCount >= BUS_MAX_BUSES) {
        return 0;
    }

    bus = &list->buses[list->busCount];
    bus->busId = busId;
    bus->availableSeats = availableSeats;
    bus->stopCount = stopCount;
    strncpy(bus->parkingZone, parkingZone, BUS_NAME_LEN - 1);
    bus->parkingZone[BUS_NAME_LEN - 1] = '\0';

    for (i = 0; i < stopCount; i++) {
        strncpy(bus->stops[i], stops[i], BUS_NAME_LEN - 1);
        bus->stops[i][BUS_NAME_LEN - 1] = '\0';
    }

    list->busCount++;
    return 1;
}

static int busStopIndex(const Bus *bus, const char *stopName) {
    int i;

    for (i = 0; i < bus->stopCount; i++) {
        if (strcmp(bus->stops[i], stopName) == 0) {
            return i;
        }
    }
    return -1;
}

int busCoversRoute(const Bus *bus, const char *start, const char *dest) {
    int startIndex;
    int destIndex;

    if (bus == NULL || start == NULL || dest == NULL) {
        return 0;
    }

    startIndex = busStopIndex(bus, start);
    destIndex = busStopIndex(bus, dest);

    if (startIndex == -1 || destIndex == -1) {
        return 0;
    }

    return startIndex < destIndex;
}

BusMatch busFindBestAvailable(const BusList *list, const char *start,
                              const char *dest) {
    BusMatch match;
    int i;
    int bestSeats = -1;

    match.found = 0;
    match.busId = -1;
    match.availableSeats = 0;
    match.parkingZone[0] = '\0';

    if (list == NULL || start == NULL || dest == NULL) {
        return match;
    }

    for (i = 0; i < list->busCount; i++) {
        const Bus *bus = &list->buses[i];

        if (bus->availableSeats <= 0) {
            continue;
        }

        if (!busCoversRoute(bus, start, dest)) {
            continue;
        }

        if (bus->availableSeats > bestSeats) {
            bestSeats = bus->availableSeats;
            match.found = 1;
            match.busId = bus->busId;
            match.availableSeats = bus->availableSeats;
            strncpy(match.parkingZone, bus->parkingZone, BUS_NAME_LEN - 1);
            match.parkingZone[BUS_NAME_LEN - 1] = '\0';
        }
    }

    return match;
}
