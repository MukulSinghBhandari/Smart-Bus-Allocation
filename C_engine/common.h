#ifndef COMMON_H
#define COMMON_H

#include "api.h"

typedef enum { TOAST_SUCCESS, TOAST_ERROR, TOAST_INFO } ToastType;
typedef enum { TONE_OK, TONE_LOW, TONE_FULL } ToneType;

typedef struct {
    char label[MAX_STRING_LEN * 2];
    ToneType tone;
} SeatStatusResult;

typedef struct {
    char title[MAX_STRING_LEN];
    char message[MAX_STRING_LEN * 3];
    char confirmText[MAX_STRING_LEN];
    char cancelText[MAX_STRING_LEN];
    bool danger;
} ConfirmOptions;

void showToast(const char* message, ToastType type);
bool showConfirm(ConfirmOptions options);
void escapeHtml(const char* input, char* output, int outputBufferMax);
SeatStatusResult seatStatus(Bus bus);
const char* tripStatusLabel(TripStatus status);
const char* currentStopName(Bus bus);

#endif
