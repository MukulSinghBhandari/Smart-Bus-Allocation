#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Graph *graphCreate(int initialCapacity) {
    int i;
    Graph *graph;

    if (initialCapacity <= 0) {
        return NULL;
    }

    graph = (Graph *)malloc(sizeof(Graph));
    if (graph == NULL) {
        return NULL;
    }

    graph->numVertices = 0;
    graph->capacity = initialCapacity;
    graph->names = (char **)calloc((size_t)initialCapacity, sizeof(char *));
    graph->adj = (EdgeNode **)calloc((size_t)initialCapacity, sizeof(EdgeNode *));

    if (graph->names == NULL || graph->adj == NULL) {
        free(graph->names);
        free(graph->adj);
        free(graph);
        return NULL;
    }

    for (i = 0; i < initialCapacity; i++) {
        graph->names[i] = NULL;
        graph->adj[i] = NULL;
    }

    return graph;
}

static int graphGrow(Graph *graph) {
    int i;
    int newCapacity;
    char **newNames;
    EdgeNode **newAdj;

    if (graph->capacity > 1000000000 / 2) {
        return 0;
    }

    newCapacity = graph->capacity * 2;
    newNames = (char **)calloc((size_t)newCapacity, sizeof(char *));
    newAdj = (EdgeNode **)calloc((size_t)newCapacity, sizeof(EdgeNode *));
    if (newNames == NULL || newAdj == NULL) {
        free(newNames);
        free(newAdj);
        return 0;
    }

    for (i = 0; i < graph->numVertices; i++) {
        newNames[i] = graph->names[i];
        newAdj[i] = graph->adj[i];
    }

    free(graph->names);
    free(graph->adj);
    graph->names = newNames;
    graph->adj = newAdj;
    graph->capacity = newCapacity;
    return 1;
}

int graphAddVertex(Graph *graph, const char *name) {
    char *copy;
    int id;

    if (graph == NULL || name == NULL || name[0] == '\0') {
        return -1;
    }

    if (graphFindVertex(graph, name) != -1) {
        return graphFindVertex(graph, name);
    }

    if (graph->numVertices >= graph->capacity) {
        if (!graphGrow(graph)) {
            return -1;
        }
    }

    copy = (char *)malloc(strlen(name) + 1);
    if (copy == NULL) {
        return -1;
    }
    strcpy(copy, name);

    id = graph->numVertices;
    graph->names[id] = copy;
    graph->adj[id] = NULL;
    graph->numVertices++;
    return id;
}

int graphFindVertex(const Graph *graph, const char *name) {
    int i;

    if (graph == NULL || name == NULL) {
        return -1;
    }

    for (i = 0; i < graph->numVertices; i++) {
        if (strcmp(graph->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static void graphAddDirectedEdge(Graph *graph, int src, int dest, int weight) {
    EdgeNode *edge;
    EdgeNode *existing;

    if (graph == NULL || src < 0 || dest < 0 || src >= graph->numVertices ||
        dest >= graph->numVertices || weight <= 0 || src == dest) {
        return;
    }

    existing = graph->adj[src];
    while (existing != NULL) {
        if (existing->dest == dest) {
            return;
        }
        existing = existing->next;
    }

    edge = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (edge == NULL) {
        return;
    }

    edge->dest = dest;
    edge->weight = weight;
    edge->next = graph->adj[src];
    graph->adj[src] = edge;
}

void graphAddEdge(Graph *graph, int src, int dest, int weight) {
    /* Campus roads work both ways */
    graphAddDirectedEdge(graph, src, dest, weight);
    graphAddDirectedEdge(graph, dest, src, weight);
}

void graphPrint(const Graph *graph) {
    int i;
    const EdgeNode *edge;

    if (graph == NULL) {
        return;
    }

    printf("Graph (%d stops):\n", graph->numVertices);
    for (i = 0; i < graph->numVertices; i++) {
        printf("  %s ->", graph->names[i]);
        edge = graph->adj[i];
        while (edge != NULL) {
            printf(" %s(%d)", graph->names[edge->dest], edge->weight);
            edge = edge->next;
        }
        printf("\n");
    }
}

void graphFree(Graph *graph) {
    int i;
    EdgeNode *edge;
    EdgeNode *next;

    if (graph == NULL) {
        return;
    }

    for (i = 0; i < graph->numVertices; i++) {
        edge = graph->adj[i];
        while (edge != NULL) {
            next = edge->next;
            free(edge);
            edge = next;
        }
        free(graph->names[i]);
    }

    free(graph->names);
    free(graph->adj);
    free(graph);
}
