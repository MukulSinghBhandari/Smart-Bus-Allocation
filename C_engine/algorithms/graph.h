// Header file for the graph module used in bus route management.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/*
    Function: graph_get_next_stop()
    Purpose:
        Finds the next stop in a bus route based on the current stop.
    Parameters:
        route[]       - Array containing stop IDs in route order.
        route_count   - Total number of stops in the route.
        current_stop  - ID of the bus's current stop.
    Returns:
        Next stop ID if a next stop exists.
        -1 if there is no next stop or the stop is invalid.
*/
int graph_get_next_stop(
    const int route[],  
    int route_count,    
    int current_stop    
);
#ifdef __cplusplus
}
#endif