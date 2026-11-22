#include "queue.h"
#include <stdlib.h>
// Internal structure of the Queue
struct RequestQueue {
    AllocationRequest *items;
    int capacity; 
    int front;
    int rear;  
    int size;  
};
// Creates a queue of the given capacity
RequestQueue *queue_create(int capacity){
    RequestQueue *queue;
    // Capacity must be positive
    if (capacity <= 0) return NULL;
    // Allocate memory for the queue structure
    queue = (RequestQueue *)malloc(sizeof(RequestQueue));
    if (queue == NULL) return NULL;
    // Allocate memory for storing allocation requests
    queue->items = (AllocationRequest *)malloc((size_t)capacity * sizeof(AllocationRequest));
    // If array allocation fails, release queue memory
    if (queue->items == NULL) {
        free(queue);
        return NULL;
    }
    // Initialize queue variables
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    // Return the created queue
    return queue;
}

// Releases all dynamically allocated memory
void queue_destroy(RequestQueue *queue){
    if (queue == NULL) return;
    free(queue->items);  // Free request array
    free(queue);         // Free queue structure
}

// Checks whether queue is empty
int queue_is_empty(const RequestQueue *queue){
    return (queue == NULL || queue->size == 0);
}

// Checks whether queue is full
int queue_is_full(const RequestQueue *queue){
    return (queue != NULL && queue->size == queue->capacity);
}

// Returns current number of requests
int queue_size(const RequestQueue *queue){
    return (queue == NULL) ? 0 : queue->size;
}

// Inserts a request at the rear
int queue_enqueue(RequestQueue *queue, AllocationRequest request){
    // Reject invalid queue or full queue
    if (queue == NULL || queue_is_full(queue)) return 0;
    // Move rear to the next circular position
    queue->rear = (queue->rear + 1) % queue->capacity;
    // Insert request at rear
    queue->items[queue->rear] = request;
    // Increase queue size
    queue->size++;
    return 1;  // Successful insertion
}

// Removes the oldest request from the front
int queue_dequeue(RequestQueue *queue, AllocationRequest *request){
    // Reject invalid queue, output pointer or empty queue
    if (queue == NULL || request == NULL || queue_is_empty(queue))
        return 0;
    // Copy front request to the caller's variable
    *request = queue->items[queue->front];
    // Move front to the next circular position
    queue->front = (queue->front + 1) % queue->capacity;
    // Decrease queue size
    queue->size--;
    return 1;  // Successful removal
}