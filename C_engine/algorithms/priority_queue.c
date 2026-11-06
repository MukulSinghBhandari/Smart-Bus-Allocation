#include "priority_queue.h"
#include <stdlib.h>

struct BusPriorityQueue {
    BusCandidate *heap;
    int size;
    int capacity;
};

static int higher_priority(BusCandidate a, BusCandidate b)
{
    if (a.available_seats != b.available_seats)
        return a.available_seats > b.available_seats;

    if (a.pickup_order != b.pickup_order)
        return a.pickup_order < b.pickup_order;

    return a.bus_id < b.bus_id;
}

static void swap_candidates(BusCandidate *a, BusCandidate *b)
{
    BusCandidate temp = *a;
    *a = *b;
    *b = temp;
}

static void sift_up(BusPriorityQueue *pq, int index)
{
    while (index > 0) {
        int parent = (index - 1) / 2;

        if (!higher_priority(pq->heap[index], pq->heap[parent]))
            break;

        swap_candidates(&pq->heap[index], &pq->heap[parent]);
        index = parent;
    }
}

static void sift_down(BusPriorityQueue *pq, int index)
{
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int best = index;

        if (left < pq->size &&
            higher_priority(pq->heap[left], pq->heap[best]))
            best = left;

        if (right < pq->size &&
            higher_priority(pq->heap[right], pq->heap[best]))
            best = right;

        if (best == index) break;

        swap_candidates(&pq->heap[index], &pq->heap[best]);
        index = best;
    }
}

BusPriorityQueue *pq_create(int capacity)
{
    BusPriorityQueue *pq;

    if (capacity <= 0) return NULL;

    pq = (BusPriorityQueue *)malloc(sizeof(BusPriorityQueue));
    if (pq == NULL) return NULL;

    pq->heap = (BusCandidate *)malloc(
        (size_t)capacity * sizeof(BusCandidate)
    );

    if (pq->heap == NULL) {
        free(pq);
        return NULL;
    }

    pq->size = 0;
    pq->capacity = capacity;

    return pq;
}

void pq_destroy(BusPriorityQueue *pq)
{
    if (pq == NULL) return;
    free(pq->heap);
    free(pq);
}

int pq_is_empty(const BusPriorityQueue *pq)
{
    return (pq == NULL || pq->size == 0);
}

int pq_size(const BusPriorityQueue *pq)
{
    return (pq == NULL) ? 0 : pq->size;
}

int pq_insert(BusPriorityQueue *pq, BusCandidate candidate)
{
    if (pq == NULL || pq->size == pq->capacity) return 0;

    pq->heap[pq->size] = candidate;
    sift_up(pq, pq->size);
    pq->size++;

    return 1;
}

int pq_extract_max(BusPriorityQueue *pq, BusCandidate *candidate)
{
    if (pq == NULL || candidate == NULL || pq->size == 0)
        return 0;

    *candidate = pq->heap[0];
    pq->size--;

    if (pq->size > 0) {
        pq->heap[0] = pq->heap[pq->size];
        sift_down(pq, 0);
    }

    return 1;
}

int pq_peek_max(const BusPriorityQueue *pq, BusCandidate *candidate)
{
    if (pq == NULL || candidate == NULL || pq->size == 0)
        return 0;

    *candidate = pq->heap[0];
    return 1;
}
