#ifndef BUS_PRIORITY_QUEUE_H
#define BUS_PRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Higher priority:
 * 1. More available seats.
 * 2. Smaller pickup stop_order (bus reaches pickup earlier in its route).
 * 3. Smaller bus_id (deterministic tie-break).
 *
 * Only ELIGIBLE buses should be inserted:
 * - enough available seats;
 * - pickup stop exists;
 * - destination occurs after pickup;
 * - bus has not already passed the pickup stop.
 */
typedef struct BusCandidate {
    int bus_id;
    int available_seats;
    int pickup_order;
} BusCandidate;

typedef struct BusPriorityQueue BusPriorityQueue;

BusPriorityQueue *pq_create(int capacity);
void pq_destroy(BusPriorityQueue *pq);
int pq_is_empty(const BusPriorityQueue *pq);
int pq_size(const BusPriorityQueue *pq);
int pq_insert(BusPriorityQueue *pq, BusCandidate candidate);
int pq_extract_max(BusPriorityQueue *pq, BusCandidate *candidate);
int pq_peek_max(const BusPriorityQueue *pq, BusCandidate *candidate);

#ifdef __cplusplus
}
#endif

#endif
