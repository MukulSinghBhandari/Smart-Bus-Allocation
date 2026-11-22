
#include "priority_queue.h" 
#include <stdlib.h>
struct BusPriorityQueue {
    BusCandidate *heap;  
    int size; 
    int capacity;
};

// Compares two buses and returns 1 if bus 'a' has higher priority than bus 'b'
static int higher_priority(BusCandidate a, BusCandidate b){
    // First priority: bus with more available seats comes first
    if (a.available_seats != b.available_seats)
        return a.available_seats > b.available_seats;
    // Second priority: bus with smaller pickup order comes first
    if (a.pickup_order != b.pickup_order)
        return a.pickup_order < b.pickup_order;
    // Third priority: bus with smaller ID comes first
    return a.bus_id < b.bus_id;
}

// Swaps the details of two bus candidates
static void swap_candidates(BusCandidate *a, BusCandidate *b){
    BusCandidate temp = *a;
    *a = *b;
    *b = temp; 
}

// Moves a newly inserted element upward to maintain Max Heap property
static void sift_up(BusPriorityQueue *pq, int index){
    while (index > 0) {
        // Finds the parent index in an array-based binary heap
        int parent = (index - 1) / 2;
        // Stop if the current element does not have higher priority
        if (!higher_priority(pq->heap[index], pq->heap[parent]))
            break;
        // Swap child with parent if child has higher priority
        swap_candidates(&pq->heap[index], &pq->heap[parent]);
        // Continue checking from the parent's position
        index = parent;
    }
}

// Moves an element downward to maintain Max Heap property
static void sift_down(BusPriorityQueue *pq, int index){
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        // Initially assumes the current element has highest priority
        int best = index;
        // Checks whether the left child has higher priority
        if (left < pq->size && higher_priority(pq->heap[left], pq->heap[best]))
            best = left;
        // Checks whether the right child has higher priority
        if (right < pq->size && higher_priority(pq->heap[right], pq->heap[best]))
            best = right;
        // Stop if the current element is already in the correct position
        if (best == index) break;
        // Swaps the current element with its highest-priority child
        swap_candidates(&pq->heap[index], &pq->heap[best]);
        // Continue checking from the child's position
        index = best;
    }
}

// Creates a new priority queue with the given capacity
BusPriorityQueue *pq_create(int capacity){
    BusPriorityQueue *pq;
    // Capacity must be positive
    if (capacity <= 0) return NULL;
    // Dynamically allocates memory for the priority queue structure
    pq = (BusPriorityQueue *)malloc(sizeof(BusPriorityQueue));
    // Checks whether memory allocation was successful
    if (pq == NULL) return NULL;
    // Dynamically allocates an array to store bus candidates
    pq->heap = (BusCandidate *)malloc((size_t)capacity * sizeof(BusCandidate));
    // If heap allocation fails, release the previously allocated memory
    if (pq->heap == NULL) {
        free(pq);
        return NULL;
    }
    // Initializes the queue
    pq->size = 0;
    pq->capacity = capacity;
    return pq;  // Returns the address of the created priority queue
}

// Destroys the priority queue and releases allocated memory
void pq_destroy(BusPriorityQueue *pq){
    if (pq == NULL) return;  // Avoids accessing an invalid pointer
    free(pq->heap);  // Releases the dynamic array memory
    free(pq);        // Releases the priority queue structure memory
}

// Checks whether the priority queue is empty
int pq_is_empty(const BusPriorityQueue *pq){
    return (pq == NULL || pq->size == 0);
}

// Returns the number of elements currently in the queue
int pq_size(const BusPriorityQueue *pq){
    return (pq == NULL) ? 0 : pq->size;
}

// Inserts a new bus candidate into the priority queue
int pq_insert(BusPriorityQueue *pq, BusCandidate candidate){
    // Rejects insertion if queue is invalid or already full
    if (pq == NULL || pq->size == pq->capacity) return 0;
    // Places the new candidate at the next available array position
    pq->heap[pq->size] = candidate;
    // Moves the candidate upward if it has higher priority
    sift_up(pq, pq->size);
    // Increases the number of elements after insertion
    pq->size++;
    return 1;  // Indicates successful insertion
}

// Removes and returns the highest-priority bus
int pq_extract_max(BusPriorityQueue *pq, BusCandidate *candidate){
    // Checks for invalid queue, invalid output pointer or empty queue
    if (pq == NULL || candidate == NULL || pq->size == 0)
        return 0;
    // Root element (index 0) contains the highest-priority bus
    *candidate = pq->heap[0];
    // Decreases the heap size because the root is being removed
    pq->size--;
    if (pq->size > 0) {
        // Moves the last element to the root position
        pq->heap[0] = pq->heap[pq->size];
        // Restores Max Heap property by moving the root downward
        sift_down(pq, 0);
    }
    return 1;  // Indicates successful extraction
}
// Returns the highest-priority bus without removing it
int pq_peek_max(const BusPriorityQueue *pq, BusCandidate *candidate){
    // Checks for invalid queue, invalid output pointer or empty queue
    if (pq == NULL || candidate == NULL || pq->size == 0)
        return 0;
    // Copies the root element into the provided output variable
    *candidate = pq->heap[0];
    return 1;  // Indicates successful retrieval
}
