#pragma once 
#ifdef __cplusplus
extern "C" {  
    #endif

    // Structure to store the details of a bus eligible for allocation
    typedef struct BusCandidate {
        int bus_id;
        int available_seats;
        int pickup_order;
    } BusCandidate;
    typedef struct BusPriorityQueue BusPriorityQueue;                     // Opaque structure: hides the internal implementation of the priority queue
    BusPriorityQueue *pq_create(int capacity);                            // Creates a priority queue with the specified maximum capacity
    void pq_destroy(BusPriorityQueue *pq);                                // Destroys the priority queue and releases its allocated memory 
    int pq_is_empty(const BusPriorityQueue *pq);                          // Checks whether the priority queue is empty
    int pq_size(const BusPriorityQueue *pq);                              // Returns the current number of elements in the priority queue
    int pq_insert(BusPriorityQueue *pq, BusCandidate candidate);          // Inserts a bus candidate into the priority queue according to priority
    int pq_extract_max(BusPriorityQueue *pq, BusCandidate *candidate);    // Removes and returns the highest-priority bus from the queue
    int pq_peek_max(const BusPriorityQueue *pq, BusCandidate *candidate); // Returns the highest-priority bus without removing it from the queue

    #ifdef __cplusplus  
}  // Ends the C linkage block
#endif
