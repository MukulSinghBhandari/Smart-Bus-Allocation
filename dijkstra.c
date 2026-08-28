#include "dijkstra.h"

#include "minheap.h"

#include <stdio.h>
#include <stdlib.h>

static PathResult *pathResultCreate(int capacity) {
    PathResult *result;

    result = (PathResult *)malloc(sizeof(PathResult));
    if (result == NULL) {
        return NULL;
    }

    result->path = (int *)malloc((size_t)capacity * sizeof(int));
    if (result->path == NULL) {
        free(result);
        return NULL;
    }

    result->pathLen = 0;
    result->distance = DIJKSTRA_INF;
    result->found = 0;
    return result;
}

static int reconstructPath(const Graph *graph, int start, int dest,
                           const int *parent, PathResult *result) {
    int current;
    int count = 0;
    int i;
    int *reverse;

    if (parent[dest] == -1 && start != dest) {
        return 0;
    }

    reverse = (int *)malloc((size_t)graph->numVertices * sizeof(int));
    if (reverse == NULL) {
        return 0;
    }

    current = dest;
    while (current != -1) {
        reverse[count++] = current;
        if (current == start) {
            break;
        }
        current = parent[current];
    }

    if (count == 0 || reverse[count - 1] != start) {
        free(reverse);
        return 0;
    }

    result->pathLen = count;
    for (i = 0; i < count; i++) {
        result->path[i] = reverse[count - 1 - i];
    }

    free(reverse);
    return 1;
}

PathResult *dijkstraShortestPath(const Graph *graph, int start, int dest) {
    int n;
    int i;
    int *dist;
    int *parent;
    int *processed;
    MinHeap *heap;
    PathResult *result;
    int key;
    int u;

    if (graph == NULL || start < 0 || dest < 0 || start >= graph->numVertices ||
        dest >= graph->numVertices) {
        return NULL;
    }

    n = graph->numVertices;
    result = pathResultCreate(n);
    if (result == NULL) {
        return NULL;
    }

    if (start == dest) {
        result->found = 1;
        result->distance = 0;
        result->pathLen = 1;
        result->path[0] = start;
        return result;
    }

    dist = (int *)malloc((size_t)n * sizeof(int));
    parent = (int *)malloc((size_t)n * sizeof(int));
    processed = (int *)calloc((size_t)n, sizeof(int));
    if (dist == NULL || parent == NULL || processed == NULL) {
        free(dist);
        free(parent);
        free(processed);
        pathResultFree(result);
        return NULL;
    }

    for (i = 0; i < n; i++) {
        dist[i] = DIJKSTRA_INF;
        parent[i] = -1;
    }

    dist[start] = 0;
    heap = heapCreate(n * 2);
    if (heap == NULL) {
        free(dist);
        free(parent);
        free(processed);
        pathResultFree(result);
        return NULL;
    }

    heapInsert(heap, 0, start);

    while (!heapIsEmpty(heap)) {
        const EdgeNode *edge;
        int newDist;

        if (!heapExtractMin(heap, &key, &u)) {
            break;
        }

        if (processed[u] || key > dist[u]) {
            continue;
        }

        processed[u] = 1;

        if (u == dest) {
            break;
        }

        edge = graph->adj[u];
        while (edge != NULL) {
            int v = edge->dest;
            newDist = dist[u] + edge->weight;

            if (!processed[v] && newDist < dist[v]) {
                dist[v] = newDist;
                parent[v] = u;
                heapInsert(heap, newDist, v);
            }
            edge = edge->next;
        }
    }

    if (dist[dest] < DIJKSTRA_INF &&
        reconstructPath(graph, start, dest, parent, result)) {
        result->found = 1;
        result->distance = dist[dest];
    }

    heapFree(heap);
    free(dist);
    free(parent);
    free(processed);
    return result;
}

void pathResultFree(PathResult *result) {
    if (result == NULL) {
        return;
    }
    free(result->path);
    free(result);
}

void pathResultPrint(const Graph *graph, const PathResult *result) {
    int i;

    if (graph == NULL || result == NULL) {
        return;
    }

    if (!result->found) {
        printf("No route found\n");
        return;
    }

    printf("Distance: %d\n", result->distance);
    printf("Stop count: %d\n", result->pathLen);
    printf("Route: ");
    for (i = 0; i < result->pathLen; i++) {
        printf("%s", graph->names[result->path[i]]);
        if (i < result->pathLen - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}
