#pragma once
#ifdef __cplusplus
extern "C" { 
    #endif
    // Stores the information of a passenger's allocation request
    typedef struct AllocationRequest {
        int request_id;   
        int pickup_stop_id; 
        int destination_stop_id;
        int seats_requested; 
    } AllocationRequest;
    typedef struct RequestQueue RequestQueue;                           // Opaque structure: hides the internal implementation of the queue
    RequestQueue *queue_create(int capacity);                           // Creates a new queue with the specified maximum capacity
    void queue_destroy(RequestQueue *queue);                            // Destroys the queue and releases its dynamically allocated memory
    int queue_is_empty(const RequestQueue *queue);                      // Checks whether the queue contains no requests
    int queue_is_full(const RequestQueue *queue);                       // Checks whether the queue has reached its maximum capacity
    int queue_size(const RequestQueue *queue);                          // Returns the current number of requests stored in the queue
    int queue_enqueue(RequestQueue *queue, AllocationRequest request);  // Inserts a new allocation request at the rear of the queue
    int queue_dequeue(RequestQueue *queue, AllocationRequest *request); // Removes the oldest request from the front of the queue
    #ifdef __cplusplus
}  // Ends the C linkage block
#endif
