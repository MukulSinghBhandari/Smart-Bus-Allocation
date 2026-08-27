#ifndef DATA_H
#define DATA_H

#include "api.h"

#define MAX_PARKING_ZONES 5

extern const char PARKING_ZONES[MAX_PARKING_ZONES][MAX_STRING_LEN];
extern const int PARKING_ZONES_COUNT;
extern Bus conductorBus;

#endif
