#include "minheap.h"

#include <stdlib.h>

static void heapSwap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

static void heapifyUp(MinHeap *heap, int index) {
    int parent;

    while (index > 0) {
        parent = (index - 1) / 2;
        if (heap->data[parent].key <= heap->data[index].key) {
            break;
        }
        heapSwap(&heap->data[parent], &heap->data[index]);
        index = parent;
    }
}

static void heapifyDown(MinHeap *heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size &&
        heap->data[left].key < heap->data[smallest].key) {
        smallest = left;
    }

    if (right < heap->size &&
        heap->data[right].key < heap->data[smallest].key) {
        smallest = right;
    }

    if (smallest != index) {
        heapSwap(&heap->data[index], &heap->data[smallest]);
        heapifyDown(heap, smallest);
    }
}

MinHeap *heapCreate(int capacity) {
    MinHeap *heap;

    if (capacity <= 0) {
        return NULL;
    }

    heap = (MinHeap *)malloc(sizeof(MinHeap));
    if (heap == NULL) {
        return NULL;
    }

    heap->data = (HeapNode *)malloc((size_t)capacity * sizeof(HeapNode));
    if (heap->data == NULL) {
        free(heap);
        return NULL;
    }

    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void heapFree(MinHeap *heap) {
    if (heap == NULL) {
        return;
    }
    free(heap->data);
    free(heap);
}

static int heapGrow(MinHeap *heap) {
    HeapNode *newData;
    int newCapacity;

    if (heap->capacity > 1000000000 / 2) {
        return 0;
    }

    newCapacity = heap->capacity * 2;
    newData = (HeapNode *)realloc(heap->data,
                                  (size_t)newCapacity * sizeof(HeapNode));
    if (newData == NULL) {
        return 0;
    }

    heap->data = newData;
    heap->capacity = newCapacity;
    return 1;
}

int heapInsert(MinHeap *heap, int key, int value) {
    int index;

    if (heap == NULL) {
        return 0;
    }

    if (heap->size >= heap->capacity) {
        if (!heapGrow(heap)) {
            return 0;
        }
    }

    index = heap->size;
    heap->data[index].key = key;
    heap->data[index].value = value;
    heap->size++;
    heapifyUp(heap, index);
    return 1;
}

int heapExtractMin(MinHeap *heap, int *key, int *value) {
    if (heap == NULL || heap->size == 0 || key == NULL || value == NULL) {
        return 0;
    }

    *key = heap->data[0].key;
    *value = heap->data[0].value;

    heap->size--;
    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        heapifyDown(heap, 0);
    }

    return 1;
}

int heapIsEmpty(const MinHeap *heap) {
    if (heap == NULL) {
        return 1;
    }
    return heap->size == 0;
}
