#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOADER_LINE_MAX 512
#define LOADER_MAX_STOPS 128

static char *loaderTrim(char *text) {
    char *end;

    if (text == NULL) {
        return NULL;
    }

    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        text++;
    }

    if (*text == '\0') {
        return text;
    }

    end = text + strlen(text) - 1;
    while (end > text &&
           (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }

    return text;
}

static int loaderAddStopNames(Graph *graph, const char *stopsPath) {
    FILE *file;
    char line[LOADER_LINE_MAX];
    char *name;

    file = fopen(stopsPath, "r");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        name = loaderTrim(line);
        if (name[0] == '\0' || name[0] == '#') {
            continue;
        }
        if (graphAddVertex(graph, name) < 0) {
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}

int loaderBuildGraph(const char *dataDir, Graph **graphOut) {
    char stopsPath[LOADER_LINE_MAX];
    char edgesPath[LOADER_LINE_MAX];
    FILE *file;
    char line[LOADER_LINE_MAX];
    Graph *graph;
    char *savePtr;
    char *src;
    char *dest;
    char *weightText;
    int srcId;
    int destId;
    int weight;

    if (dataDir == NULL || graphOut == NULL) {
        return 0;
    }

    snprintf(stopsPath, sizeof(stopsPath), "%s/stops.txt", dataDir);
    snprintf(edgesPath, sizeof(edgesPath), "%s/edges.txt", dataDir);

    graph = graphCreate(LOADER_MAX_STOPS);
    if (graph == NULL) {
        return 0;
    }

    if (!loaderAddStopNames(graph, stopsPath)) {
        graphFree(graph);
        return 0;
    }

    file = fopen(edgesPath, "r");
    if (file == NULL) {
        graphFree(graph);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        src = strtok_r(line, "|", &savePtr);
        dest = strtok_r(NULL, "|", &savePtr);
        weightText = strtok_r(NULL, "|\r\n", &savePtr);

        if (src == NULL || dest == NULL || weightText == NULL) {
            continue;
        }

        src = loaderTrim(src);
        dest = loaderTrim(dest);
        weightText = loaderTrim(weightText);
        weight = atoi(weightText);

        srcId = graphFindVertex(graph, src);
        destId = graphFindVertex(graph, dest);

        if (srcId < 0 || destId < 0 || weight <= 0) {
            fclose(file);
            graphFree(graph);
            return 0;
        }

        graphAddEdge(graph, srcId, destId, weight);
    }

    fclose(file);
    *graphOut = graph;
    return 1;
}

int loaderBuildTrie(const char *dataDir, Trie **trieOut) {
    char stopsPath[LOADER_LINE_MAX];
    FILE *file;
    char line[LOADER_LINE_MAX];
    Trie *trie;
    char *name;

    if (dataDir == NULL || trieOut == NULL) {
        return 0;
    }

    snprintf(stopsPath, sizeof(stopsPath), "%s/stops.txt", dataDir);
    trie = trieCreate();
    if (trie == NULL) {
        return 0;
    }

    file = fopen(stopsPath, "r");
    if (file == NULL) {
        trieFree(trie);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        name = loaderTrim(line);
        if (name[0] == '\0' || name[0] == '#') {
            continue;
        }
        if (!trieInsert(trie, name)) {
            fclose(file);
            trieFree(trie);
            return 0;
        }
    }

    fclose(file);
    *trieOut = trie;
    return 1;
}

int loaderLoadBuses(const char *dataDir, BusList *busList) {
    char busesPath[LOADER_LINE_MAX];
    FILE *file;
    char line[LOADER_LINE_MAX];
    char *savePtr;
    char *token;
    const char *stops[BUS_MAX_STOPS];
    int busId;
    int seats;
    char parkingZone[BUS_NAME_LEN];
    int stopCount;

    if (dataDir == NULL || busList == NULL) {
        return 0;
    }

    snprintf(busesPath, sizeof(busesPath), "%s/buses.txt", dataDir);
    file = fopen(busesPath, "r");
    if (file == NULL) {
        return 0;
    }

    busListInit(busList);

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        token = strtok_r(line, "|\r\n", &savePtr);
        if (token == NULL) {
            continue;
        }
        busId = atoi(token);

        token = strtok_r(NULL, "|", &savePtr);
        if (token == NULL) {
            continue;
        }
        seats = atoi(token);

        token = strtok_r(NULL, "|", &savePtr);
        if (token == NULL) {
            continue;
        }
        strncpy(parkingZone, loaderTrim(token), BUS_NAME_LEN - 1);
        parkingZone[BUS_NAME_LEN - 1] = '\0';

        stopCount = 0;
        while ((token = strtok_r(NULL, "|", &savePtr)) != NULL &&
               stopCount < BUS_MAX_STOPS) {
            stops[stopCount++] = loaderTrim(token);
        }

        if (stopCount == 0) {
            continue;
        }

        if (!busListAdd(busList, busId, seats, parkingZone, stops, stopCount)) {
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}
