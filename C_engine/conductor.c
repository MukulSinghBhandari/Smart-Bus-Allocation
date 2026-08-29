//conductor.c
#include "conductor.h"
#include "common.h"
#include "data.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void renderBusInfo(Bus bus) {
    printf("========================================================================\n");
    printf(" \033[1;36mTOPBAR:\033[0m BUS %s   |   Reg: %s   |   Schedule: %s\n", bus.busNo, bus.registrationNo, bus.timing);
    printf(" Staff: %s (%s)\n", bus.conductor, bus.phone);
    printf("========================================================================\n");
}

static void renderTripStatus(Bus bus) {
    printf(" STATUS PILL: %s | Live Location: \033[1;34m%s\033[0m\n", 
           tripStatusLabel(bus.tripStatus), currentStopName(bus) != NULL ? currentStopName(bus) : "Completed");
    printf("------------------------------------------------------------------------\n");
}

static void renderNextStop(Bus bus) {
    const char* name = currentStopName(bus);
    printf("\n--- NEXT STOP CARD ---\n");
    if (bus.tripStatus == TRIP_COMPLETED) {
        printf("[TRIP ENDED] This trip has been completed.\n");
        return;
    }
    if (name == NULL) {
        printf("[✓ ROUTE COMPLETED] Final Stop Reached.\n");
        return;
    }
    printf(" Target Stop: \033[1;33m%s\033[0m\n", name);
}

static void renderSeats(Bus bus, bool pop) {
    printf("\n--- CAPACITY TRACKING ---\n");
    SeatStatusResult status = seatStatus(bus);
    printf("Badge: %s\n", status.label);
    printf("Seats Available: %d / Total Capacity: %d\n", bus.availableSeats, bus.totalSeats);
}

static void renderZone(Bus bus) {
    printf("\n--- DEPLOYMENT ZONE ---\n");
    printf("Confirmed Zone Status: %s\n", (strlen(bus.parkingZone) > 0) ? bus.parkingZone : "Not set");
}

static void renderRoute(Bus bus) {
    printf("\n--- ROUTE TIMELINE ---\n");
    for (int i = 0; i < bus.routeLength; i++) {
        bool isDone = i < bus.currentStop;
        bool isCurrent = (i == bus.currentStop && bus.tripStatus != TRIP_COMPLETED);
        if (isDone) printf("  \033[1;32m✓\033[0m %-15s [Completed]\n", bus.route[i]);
        else if (isCurrent) printf("  \033[1;34m» %-15s [Next Stop]\033[0m\n", bus.route[i]);
        else printf("    %-15s [Upcoming]\n", bus.route[i]);
    }
    printf("========================================================================\n");
}

void renderAll(bool popAnimationTrigger) {
    Response res = getBusStatus(NULL);
    if (!res.ok) { showToast(res.error, TOAST_ERROR); return; }
    Bus bus = res.data;
    renderBusInfo(bus);
    renderTripStatus(bus);
    renderNextStop(bus);
    renderSeats(bus, popAnimationTrigger);
    renderZone(bus);
    renderRoute(bus);
}

void runDashboardLoop(void) {
    int choice;
    renderAll(false);
    while (true) {
        printf("\n\033[1;35mOPERATIONAL CONTROLS:\033[0m\n");
        printf("1. Seat [-1]  |  2. Seat [+1]  |  3. Assign Parking Zone  |  4. Arrived at Stop  |  5. End Trip  |  6. Exit\n");
        printf("Enter Command Option: ");
        if (scanf("%d", &choice) != 1) { int c; while ((c = getchar()) != '\n' && c != EOF); continue; }
        switch (choice) {
            case 1: updateSeats(NULL, -1); renderAll(true); break;
            case 2: updateSeats(NULL, 1); renderAll(true); break;
            case 3: {
                int zChoice;
                printf("Select Zone (1: Zone A, 2: Zone B, 3: Zone C): ");
                if (scanf("%d", &zChoice) == 1 && zChoice >= 1 && zChoice <= 3) {
                    setParkingZone(NULL, PARKING_ZONES[zChoice - 1]);
                }
                renderAll(false); break;
            }
            case 4: markNextStop(NULL); renderAll(false); break;
            case 5: {
                ConfirmOptions p = {.title="End Trip?", .message="Confirm ending trip?", .danger=true};
                if (showConfirm(p)) endTrip(NULL);
                renderAll(false); break;
            }
            case 6: return;
            default: break;
        }
    }
}
