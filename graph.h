#ifndef GRAPH_H
#define GRAPH_H

/*
 * Graph = campus stop network
 * Vertex = one bus stop
 * Edge   = road between two stops (weight = distance)
 */

typedef struct EdgeNode {
    int dest;              /* index of neighbor stop */
    int weight;            /* distance (km or any unit) */
    struct EdgeNode *next; /* next edge in linked list */
} EdgeNode;

typedef struct Graph {
    int numVertices;
    int capacity;
    char **names;          /* stop name for each vertex index */
    EdgeNode **adj;        /* adjacency list: adj[i] = edges from stop i */
} Graph;

Graph *graphCreate(int initialCapacity);
int graphAddVertex(Graph *graph, const char *name);
int graphFindVertex(const Graph *graph, const char *name);
void graphAddEdge(Graph *graph, int src, int dest, int weight);
void graphPrint(const Graph *graph);
void graphFree(Graph *graph);

#endif
