#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    Finds the next stop in a route using graph traversal.

    route[]       = ordered stop IDs
    route_count   = number of stops
    current_stop  = current stop ID

    Returns:
        next stop ID
        -1 if there is no next stop
*/
int graph_get_next_stop(
    const int route[],
    int route_count,
    int current_stop
);

#ifdef __cplusplus
}
#endif

#endif