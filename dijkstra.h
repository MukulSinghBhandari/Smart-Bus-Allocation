#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

#define DIJKSTRA_INF 1000000000

typedef struct {
    int *path;      /* vertex indices from start to dest */
    int pathLen;
    int distance;
    int found;      /* 1 if path exists */
} PathResult;

PathResult *dijkstraShortestPath(const Graph *graph, int start, int dest);
void pathResultFree(PathResult *result);
void pathResultPrint(const Graph *graph, const PathResult *result);

#endif
