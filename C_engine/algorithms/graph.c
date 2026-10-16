// Graph implementation for finding the next stop in a route.
#include "graph.h"
#include <stdlib.h>

/*
    Internal graph edge.
*/
typedef struct Edge
{
    int destination;
    struct Edge *next;

} Edge;


/*
    Internal graph.
*/
typedef struct Graph
{
    int vertices;
    Edge **adjacency;

} Graph;


/*
    Create graph.
*/
static Graph *create_graph(int vertices)
{
    Graph *graph;

    graph = (Graph *)malloc(sizeof(Graph));

    if (graph == NULL)
    {
        return NULL;
    }

    graph->vertices = vertices;

    graph->adjacency =
        (Edge **)calloc(vertices, sizeof(Edge *));

    if (graph->adjacency == NULL)
    {
        free(graph);
        return NULL;
    }

    return graph;
}


/*
    Add directed edge.
*/
static void add_edge(
    Graph *graph,
    int source,
    int destination
)
{
    Edge *newEdge;

    if (graph == NULL)
    {
        return;
    }

    if (source < 0 || source >= graph->vertices)
    {
        return;
    }

    newEdge =
        (Edge *)malloc(sizeof(Edge));

    if (newEdge == NULL)
    {
        return;
    }

    newEdge->destination = destination;

    newEdge->next =
        graph->adjacency[source];

    graph->adjacency[source] =
        newEdge;
}


/*
    Free graph.
*/
static void free_graph(Graph *graph)
{
    int i;
    Edge *current;
    Edge *next;

    if (graph == NULL)
    {
        return;
    }

    for (i = 0; i < graph->vertices; i++)
    {
        current =
            graph->adjacency[i];

        while (current != NULL)
        {
            next = current->next;

            free(current);

            current = next;
        }
    }

    free(graph->adjacency);
    free(graph);
}


/*
    Find position of a stop in the route.
*/
static int find_position(
    const int route[],
    int route_count,
    int stop_id
)
{
    int i;

    for (i = 0; i < route_count; i++)
    {
        if (route[i] == stop_id)
        {
            return i;
        }
    }

    return -1;
}


/*
    Main graph algorithm.

    The route is converted into a directed graph:

        route[0] -> route[1]
        route[1] -> route[2]
        route[2] -> route[3]
        ...

    Then the graph is used to determine
    the next stop.
*/
int graph_get_next_stop(
    const int route[],
    int route_count,
    int current_stop
)
{
    Graph *graph;
    int current_position;
    int i;
    int next_stop;

    if (route == NULL)
    {
        return -1;
    }

    if (route_count <= 0)
    {
        return -1;
    }

    /*
        If current_stop is -1,
        the trip has not started.

        Therefore return first stop.
    */
    if (current_stop == -1)
    {
        return route[0];
    }

    current_position =
        find_position(
            route,
            route_count,
            current_stop
        );

    if (current_position == -1)
    {
        return -1;
    }

    /*
        Last stop reached.
    */
    if (current_position >= route_count - 1)
    {
        return -1;
    }

    /*
        Create graph.
    */
    graph =
        create_graph(route_count);

    if (graph == NULL)
    {
        return -1;
    }

    /*
        Build directed route graph.
    */
    for (i = 0; i < route_count - 1; i++)
    {
        add_edge(
            graph,
            i,
            i + 1
        );
    }

    /*
        The current vertex points
        to the next vertex.
    */
    next_stop =
        route[current_position + 1];

    free_graph(graph);

    return next_stop;
}