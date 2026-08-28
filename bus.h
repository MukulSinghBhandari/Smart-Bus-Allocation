#ifndef BUS_H
#define BUS_H

#define BUS_MAX_STOPS 32
#define BUS_MAX_BUSES 128
#define BUS_NAME_LEN 64

typedef struct {
    int busId;
    int availableSeats;
    char parkingZone[BUS_NAME_LEN];
    int stopCount;
    char stops[BUS_MAX_STOPS][BUS_NAME_LEN];
} Bus;

typedef struct {
    Bus buses[BUS_MAX_BUSES];
    int busCount;
} BusList;

typedef struct {
    int found;
    int busId;
    int availableSeats;
    char parkingZone[BUS_NAME_LEN];
} BusMatch;

void busListInit(BusList *list);
int busListAdd(BusList *list, int busId, int availableSeats,
               const char *parkingZone, const char *stops[], int stopCount);
int busCoversRoute(const Bus *bus, const char *start, const char *dest);
BusMatch busFindBestAvailable(const BusList *list, const char *start,
                              const char *dest);

#endif
