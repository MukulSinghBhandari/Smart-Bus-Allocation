// Handle incoming bridge commands from Python: update a bus parking zone or return bus info as JSON.
#include "api.h"    
#include "data.h"
#include <string.h>
#include <stdio.h>

static Bus snapshot(const Bus* src) {
    Bus dest;
    memcpy(&dest, src, sizeof(Bus));
    return dest;
}

static Response ok(Bus currentBus) {
    Response r;
    memset(&r, 0, sizeof(Response));
    r.ok = true;
    r.data = currentBus;
    return r;
}

static Response fail(const char* errorMessage) {
    Response r;
    memset(&r, 0, sizeof(Response));
    r.ok = false;
    strncpy(r.error, errorMessage, sizeof(r.error) - 1);
    return r;
}

Response getAssignedBus(void) {
    return ok(snapshot(&conductorBus));
}

Response getBusStatus(const char* busId) {
    if (busId != NULL && strcmp(busId, conductorBus.busNo) != 0) {
        char err[100];
        snprintf(err, sizeof(err), "Bus not found: %s", busId);
        return fail(err);
    }
    return ok(snapshot(&conductorBus));
}

Response updateSeats(const char* busId, int change) {
    int next = conductorBus.availableSeats + change;
    if (next < 0) {
        return fail("Available seats cannot go below 0.");
    }
    if (next > conductorBus.totalSeats) {
        char err[100];
        snprintf(err, sizeof(err), "Available seats cannot exceed capacity (%d).", conductorBus.totalSeats);
        return fail(err);
    }
    conductorBus.availableSeats = next;
    return ok(snapshot(&conductorBus));
}

Response setParkingZone(const char* busId, const char* zone) {
    if (zone == NULL || strlen(zone) == 0) {
        return fail("Please select a parking zone first.");
    }
    bool found = false;
    for (int i = 0; i < PARKING_ZONES_COUNT; i++) {
        if (strcmp(PARKING_ZONES[i], zone) == 0) {
            found = true;
            break;
        }
    }
    if (!found) {
        char err[100];
        snprintf(err, sizeof(err), "Unknown parking zone: %s", zone);
        return fail(err);
    }
    strncpy(conductorBus.parkingZone, zone, MAX_STRING_LEN - 1);
    return ok(snapshot(&conductorBus));
}

Response markNextStop(const char* busId) {
    if (conductorBus.tripStatus == TRIP_COMPLETED) {
        return fail("Trip has already ended.");
    }
    if (conductorBus.currentStop >= conductorBus.routeLength) {
        return fail("No next stop — the route is already complete.");
    }
    const char* arrivedName = conductorBus.route[conductorBus.currentStop];
    for (int i = 0; i < conductorBus.completedStopsCount; i++) {
        if (strcmp(conductorBus.completedStops[i], arrivedName) == 0) {
            char err[100];
            snprintf(err, sizeof(err), "%s is already marked as completed.", arrivedName);
            return fail(err);
        }
    }
    strncpy(conductorBus.completedStops[conductorBus.completedStopsCount], arrivedName, MAX_STRING_LEN - 1);
    conductorBus.completedStopsCount++;
    conductorBus.currentStop++;
    bool finished = (conductorBus.currentStop >= conductorBus.routeLength);
    Response r = ok(snapshot(&conductorBus));
    strncpy(r.arrivedName, arrivedName, MAX_STRING_LEN - 1);
    r.finished = finished;
    return r;
}

Response endTrip(const char* busId) {
    if (conductorBus.tripStatus == TRIP_COMPLETED) {
        return fail("Trip has already ended.");
    }
    conductorBus.tripStatus = TRIP_COMPLETED;
    return ok(snapshot(&conductorBus));
}
