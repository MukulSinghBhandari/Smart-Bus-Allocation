#ifndef MINHEAP_H
#define MINHEAP_H

typedef struct {
    int key;    /* priority (distance in Dijkstra) */
    int value;  /* vertex id */
} HeapNode;

typedef struct {
    HeapNode *data;
    int size;
    int capacity;
} MinHeap;

MinHeap *heapCreate(int capacity);
void heapFree(MinHeap *heap);
int heapInsert(MinHeap *heap, int key, int value);
int heapExtractMin(MinHeap *heap, int *key, int *value);
int heapIsEmpty(const MinHeap *heap);

#endif
