#include "data.h"

const char PARKING_ZONES[MAX_PARKING_ZONES][MAX_STRING_LEN] = {
    "Zone A", "Zone B", "Zone C", "Zone D", "Main Parking"
};
const int PARKING_ZONES_COUNT = MAX_PARKING_ZONES;

Bus conductorBus = {
    .busNo = "105",
    .registrationNo = "UK07AB1234",
    .timing = "4:00 PM",
    .conductor = "Rahul Kumar",
    .phone = "98765 43210",
    .totalSeats = 50,
    .availableSeats = 32,
    .parkingZone = "",
    .route = {
        "University Campus", "Doiwala", "Lachiwala", 
        "Kuanwala", "Harawala", "Miawala", "Mohkampur"
    },
    .routeLength = 7,
    .currentStop = 2, 
    .completedStops = {"University Campus", "Doiwala"},
    .completedStopsCount = 2,
    .tripStatus = TRIP_ACTIVE
};
