#include "common.h"
#include <stdio.h>
#include <string.h>

void showToast(const char* message, ToastType type) {
    char cleanMessage[MAX_STRING_LEN * 4];
    escapeHtml(message, cleanMessage, sizeof(cleanMessage));
    switch (type) {
        case TOAST_ERROR: printf("\n\033[1;37;41m [!] %s \033[0m\n\n", cleanMessage); break;
        case TOAST_INFO:  printf("\n\033[1;37;100m [i] %s \033[0m\n\n", cleanMessage); break;
        case TOAST_SUCCESS:
        default:          printf("\n\033[1;37;42m [✓] %s \033[0m\n\n", cleanMessage); break;
    }
}

bool showConfirm(ConfirmOptions options) {
    const char* title = (strlen(options.title) > 0) ? options.title : "Are you sure?";
    const char* confirm = (strlen(options.confirmText) > 0) ? options.confirmText : "Confirm";
    const char* cancel = (strlen(options.cancelText) > 0) ? options.cancelText : "Cancel";
    printf("\n============================================\n");
    if (options.danger) printf("\033[1;31m%s\033[0m\n", title);
    else printf("\033[1;34m%s\033[0m\n", title);
    printf("%s\n--------------------------------------------\n", options.message);
    printf(" 1. %s\n 2. %s\n============================================\n", confirm, cancel);
    printf("Select option (1 or 2): ");
    int choice = 0;
    while (true) {
        if (scanf("%d", &choice) == 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            if (choice == 1) return true;
            if (choice == 2) return false;
        } else {
            int c; while ((c = getchar()) != '\n' && c != EOF);
        }
        printf("Invalid choice. Enter 1 or 2: ");
    }
}

void escapeHtml(const char* input, char* output, int outputBufferMax) {
    if (input == NULL || output == NULL || outputBufferMax <= 0) return;
    int outIdx = 0, limit = outputBufferMax - 1;
    for (int i = 0; input[i] != '\0' && outIdx < limit; i++) {
        switch (input[i]) {
            case '&':  if (outIdx + 5 <= limit) { strcpy(&output[outIdx], "&amp;"); outIdx += 5; } break;
            case '<':  if (outIdx + 4 <= limit) { strcpy(&output[outIdx], "&lt;"); outIdx += 4; } break;
            case '>':  if (outIdx + 4 <= limit) { strcpy(&output[outIdx], "&gt;"); outIdx += 4; } break;
            case '"':  if (outIdx + 6 <= limit) { strcpy(&output[outIdx], "&quot;"); outIdx += 6; } break;
            case '\'': if (outIdx + 5 <= limit) { strcpy(&output[outIdx], "&#39;"); outIdx += 5; } break;
            default:   output[outIdx++] = input[i]; break;
        }
    }
    output[outIdx] = '\0';
}

SeatStatusResult seatStatus(Bus bus) {
    SeatStatusResult result; memset(&result, 0, sizeof(result));
    if (bus.availableSeats <= 0) { strcpy(result.label, "BUS FULL"); result.tone = TONE_FULL; }
    else if (bus.availableSeats <= 5) { strcpy(result.label, "FEW SEATS LEFT"); result.tone = TONE_LOW; }
    else { snprintf(result.label, sizeof(result.label), "%d seats available", bus.availableSeats); result.tone = TONE_OK; }
    return result;
}

const char* tripStatusLabel(TripStatus status) {
    return (status == TRIP_ACTIVE) ? "ACTIVE" : "COMPLETED";
}

const char* currentStopName(Bus bus) {
    return (bus.currentStop >= bus.routeLength) ? NULL : bus.route[bus.currentStop];
}
