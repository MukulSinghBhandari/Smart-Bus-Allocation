#ifndef OUTPUT_H
#define OUTPUT_H

#include "bus.h"
#include "dijkstra.h"
#include "graph.h"

typedef enum {
    OUTPUT_HUMAN,
    OUTPUT_MACHINE
} OutputFormat;

void outputPrintError(OutputFormat format, const char *code, const char *message);
void outputPrintRoute(OutputFormat format, const Graph *graph,
                      const PathResult *path, const BusMatch *bus);
void outputPrintPrefix(OutputFormat format, const char results[][64], int count);

#endif
