#ifndef ALLOCATION_QUEUE_H
#define ALLOCATION_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AllocationRequest {
    int request_id;
    int pickup_stop_id;
    int destination_stop_id;
    int seats_requested;
} AllocationRequest;

typedef struct RequestQueue RequestQueue;

RequestQueue *queue_create(int capacity);
void queue_destroy(RequestQueue *queue);
int queue_is_empty(const RequestQueue *queue);
int queue_is_full(const RequestQueue *queue);
int queue_size(const RequestQueue *queue);
int queue_enqueue(RequestQueue *queue, AllocationRequest request);
int queue_dequeue(RequestQueue *queue, AllocationRequest *request);

#ifdef __cplusplus
}
#endif

#endif
