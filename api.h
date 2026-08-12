#ifndef API_H
#define API_H

#include <stdbool.h>

#define MAX_STRING_LEN 50
#define MAX_STOPS 20

typedef enum {
    TRIP_ACTIVE,
    TRIP_COMPLETED
} TripStatus;

typedef struct {
    char busNo[MAX_STRING_LEN];
    char registrationNo[MAX_STRING_LEN];
    char timing[MAX_STRING_LEN];
    char conductor[MAX_STRING_LEN];
    char phone[MAX_STRING_LEN];
    int totalSeats;
    int availableSeats;
    char parkingZone[MAX_STRING_LEN];
    char route[MAX_STOPS][MAX_STRING_LEN];
    int routeLength;
    char completedStops[MAX_STOPS][MAX_STRING_LEN];
    int completedStopsCount;
    int currentStop;
    TripStatus tripStatus;
} Bus;

typedef struct {
    bool ok;
    char error[100];
    Bus data;
    char arrivedName[MAX_STRING_LEN];
    bool finished;
} Response;

Response getAssignedBus(void);
Response getBusStatus(const char* busId);
Response updateSeats(const char* busId, int change);
Response setParkingZone(const char* busId, const char* zone);
Response markNextStop(const char* busId);
Response endTrip(const char* busId);

#endif
