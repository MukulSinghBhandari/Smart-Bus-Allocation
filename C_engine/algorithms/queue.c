#include "queue.h"
#include <stdlib.h>

struct RequestQueue {
    AllocationRequest *items;
    int capacity;
    int front;
    int rear;
    int size;
};

RequestQueue *queue_create(int capacity)
{
    RequestQueue *queue;

    if (capacity <= 0) return NULL;

    queue = (RequestQueue *)malloc(sizeof(RequestQueue));
    if (queue == NULL) return NULL;

    queue->items = (AllocationRequest *)malloc(
        (size_t)capacity * sizeof(AllocationRequest)
    );

    if (queue->items == NULL) {
        free(queue);
        return NULL;
    }

    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;

    return queue;
}

void queue_destroy(RequestQueue *queue)
{
    if (queue == NULL) return;
    free(queue->items);
    free(queue);
}

int queue_is_empty(const RequestQueue *queue)
{
    return (queue == NULL || queue->size == 0);
}

int queue_is_full(const RequestQueue *queue)
{
    return (queue != NULL && queue->size == queue->capacity);
}

int queue_size(const RequestQueue *queue)
{
    return (queue == NULL) ? 0 : queue->size;
}

int queue_enqueue(RequestQueue *queue, AllocationRequest request)
{
    if (queue == NULL || queue_is_full(queue)) return 0;

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->items[queue->rear] = request;
    queue->size++;

    return 1;
}

int queue_dequeue(RequestQueue *queue, AllocationRequest *request)
{
    if (queue == NULL || request == NULL || queue_is_empty(queue))
        return 0;

    *request = queue->items[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    return 1;
}
