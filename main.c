#include <stdio.h>
#include <string.h>

#include "bus.h"
#include "database.h"
#include "dijkstra.h"
#include "graph.h"
#include "loader.h"
#include "minheap.h"
#include "output.h"
#include "trie.h"

typedef struct {
    const char *dataDir;
    const char *dbPath;
    const char *fromStop;
    const char *toStop;
    const char *prefix;
    OutputFormat format;
    int runTests;
    int verbose;
} AppConfig;

static void printUsage(void) {
    printf("Usage:\n");
    printf("  c_engine --data <dir> --from \"<start>\" --to \"<dest>\" [--format human|machine]\n");
    printf("  c_engine --db <file.db> --from \"<start>\" --to \"<dest>\" [--format human|machine]\n");
    printf("  c_engine --data <dir> --prefix \"<text>\" [--format human|machine]\n");
    printf("  c_engine --test\n");
}

static int parseArgs(int argc, char *argv[], AppConfig *config) {
    int i;

    config->dataDir = "test_data";
    config->dbPath = NULL;
    config->fromStop = NULL;
    config->toStop = NULL;
    config->prefix = NULL;
    config->format = OUTPUT_HUMAN;
    config->runTests = 0;
    config->verbose = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) {
            config->runTests = 1;
        } else if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            config->dataDir = argv[++i];
        } else if (strcmp(argv[i], "--db") == 0 && i + 1 < argc) {
            config->dbPath = argv[++i];
        } else if (strcmp(argv[i], "--from") == 0 && i + 1 < argc) {
            config->fromStop = argv[++i];
        } else if (strcmp(argv[i], "--to") == 0 && i + 1 < argc) {
            config->toStop = argv[++i];
        } else if (strcmp(argv[i], "--prefix") == 0 && i + 1 < argc) {
            config->prefix = argv[++i];
        } else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            if (strcmp(argv[++i], "machine") == 0) {
                config->format = OUTPUT_MACHINE;
            } else {
                config->format = OUTPUT_HUMAN;
            }
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config->verbose = 1;
        } else {
            return 0;
        }
    }

    return 1;
}

static int runRouteMode(const AppConfig *config) {
    Graph *graph;
    BusList buses;
    PathResult *path;
    BusMatch bus;
    DatabaseStatus status;
    int startId;
    int destId;

    if (config->fromStop == NULL || config->toStop == NULL) {
        outputPrintError(config->format, "INVALID_ARGS",
                         "Both --from and --to are required");
        return 1;
    }

    if (config->dbPath != NULL) {
        status = databaseLoadGraph(config->dbPath, &graph);
        if (status == DATABASE_OPEN_FAIL) {
            outputPrintError(config->format, "DB_NOT_FOUND",
                             "Database file not found or cannot be opened");
            return 1;
        }
        if (status == DATABASE_EMPTY) {
            outputPrintError(config->format, "EMPTY_GRAPH",
                             "Database has no stops");
            return 1;
        }
        if (status != DATABASE_OK) {
            outputPrintError(config->format, "LOAD_ERROR",
                             "Failed to load graph from database");
            return 1;
        }
    } else if (!loaderBuildGraph(config->dataDir, &graph)) {
        outputPrintError(config->format, "LOAD_ERROR",
                         "Failed to load graph data");
        return 1;
    }

    busListInit(&buses);
    if (config->dbPath != NULL) {
        status = databaseLoadBuses(config->dbPath, &buses);
        if (status != DATABASE_OK) {
            graphFree(graph);
            outputPrintError(config->format, "LOAD_ERROR",
                             "Failed to load buses from database");
            return 1;
        }
    } else if (!loaderLoadBuses(config->dataDir, &buses)) {
        graphFree(graph);
        outputPrintError(config->format, "LOAD_ERROR",
                         "Failed to load bus data");
        return 1;
    }

    startId = graphFindVertex(graph, config->fromStop);
    destId = graphFindVertex(graph, config->toStop);

    if (startId < 0 || destId < 0) {
        graphFree(graph);
        outputPrintError(config->format, "INVALID_STOP", "Unknown stop name");
        return 1;
    }

    path = dijkstraShortestPath(graph, startId, destId);
    if (path == NULL) {
        graphFree(graph);
        outputPrintError(config->format, "ROUTE_ERROR", "Routing failed");
        return 1;
    }

    bus = busFindBestAvailable(&buses, config->fromStop, config->toStop);
    outputPrintRoute(config->format, graph, path, &bus);

    pathResultFree(path);
    graphFree(graph);
    return 0;
}

static int runPrefixMode(const AppConfig *config) {
    Trie *trie;
    char results[32][64];
    int count;

    if (config->prefix == NULL) {
        outputPrintError(config->format, "INVALID_ARGS",
                         "--prefix is required");
        return 1;
    }

    if (!loaderBuildTrie(config->dataDir, &trie)) {
        outputPrintError(config->format, "LOAD_ERROR",
                         "Failed to load stop names");
        return 1;
    }

    triePrefixSearch(trie, config->prefix, results, 32, &count);
    outputPrintPrefix(config->format, results, count);
    trieFree(trie);
    return 0;
}

static int expectTrue(int ok, const char *name) {
    if (ok) {
        printf("  PASS %s\n", name);
        return 0;
    }
    printf("  FAIL %s\n", name);
    return 1;
}

static int testGraph(void) {
    Graph *graph;
    int a, b, c;
    int failed = 0;

    printf("=== Graph ===\n");
    graph = graphCreate(10);
    a = graphAddVertex(graph, "A");
    b = graphAddVertex(graph, "B");
    c = graphAddVertex(graph, "C");
    graphAddEdge(graph, a, b, 5);

    failed |= expectTrue(graph->numVertices == 3, "three vertices");
    failed |= expectTrue(graphFindVertex(graph, "B") == b, "find stop B");
    failed |= expectTrue(graphFindVertex(graph, "Z") == -1, "unknown stop");
    failed |= expectTrue(graph->adj[a] != NULL && graph->adj[a]->dest == b &&
                             graph->adj[a]->weight == 5,
                         "edge A->B weight 5");
    failed |= expectTrue(c >= 0, "add stop C");
    graphFree(graph);

    graph = graphCreate(2);
    failed |= expectTrue(graphAddVertex(graph, "S1") >= 0, "first stop");
    failed |= expectTrue(graphAddVertex(graph, "S2") >= 0, "second stop");
    failed |= expectTrue(graphAddVertex(graph, "S3") >= 0,
                         "grows past initial capacity");
    failed |= expectTrue(graph->numVertices == 3, "three stops after grow");
    graphFree(graph);

    graph = graphCreate(4);
    a = graphAddVertex(graph, "A");
    b = graphAddVertex(graph, "B");
    graphAddEdge(graph, a, b, 5);
    graphAddEdge(graph, a, b, 9);
    failed |= expectTrue(graph->adj[a] != NULL && graph->adj[a]->next == NULL &&
                             graph->adj[a]->weight == 5,
                         "duplicate edge kept once with first weight");

    graphFree(graph);
    return failed;
}

static int testHeap(void) {
    MinHeap *heap;
    int keys[4];
    int values[4];
    int n = 0;
    int key;
    int value;
    int failed = 0;

    printf("=== Min-heap ===\n");
    heap = heapCreate(10);
    heapInsert(heap, 7, 1);
    heapInsert(heap, 3, 2);
    heapInsert(heap, 9, 3);
    heapInsert(heap, 1, 4);
    while (n < 4 && heapExtractMin(heap, &key, &value)) {
        keys[n] = key;
        values[n] = value;
        n++;
    }
    heapFree(heap);

    failed |= expectTrue(n == 4, "extract four nodes");
    failed |= expectTrue(keys[0] == 1 && keys[1] == 3 && keys[2] == 7 &&
                             keys[3] == 9,
                         "keys in increasing order");
    failed |= expectTrue(values[0] == 4, "smallest key is value 4");

    heap = heapCreate(2);
    failed |= expectTrue(heapInsert(heap, 5, 1) && heapInsert(heap, 2, 2) &&
                             heapInsert(heap, 4, 3),
                         "grows past initial capacity");
    failed |= expectTrue(heapExtractMin(heap, &key, &value) && key == 2,
                         "min after grow is 2");
    heapFree(heap);
    return failed;
}

static int testDijkstra(void) {
    Graph *graph;
    PathResult *result;
    PathResult *same;
    PathResult *none;
    int a, b, c, d, isolated;
    int failed = 0;

    printf("=== Dijkstra ===\n");
    graph = graphCreate(10);
    a = graphAddVertex(graph, "A");
    b = graphAddVertex(graph, "B");
    c = graphAddVertex(graph, "C");
    d = graphAddVertex(graph, "D");
    isolated = graphAddVertex(graph, "X");
    graphAddEdge(graph, a, b, 5);
    graphAddEdge(graph, a, c, 2);
    graphAddEdge(graph, b, d, 3);
    graphAddEdge(graph, c, d, 4);

    result = dijkstraShortestPath(graph, a, d);
    failed |= expectTrue(result != NULL && result->found, "A to D exists");
    failed |= expectTrue(result != NULL && result->distance == 6,
                         "A to D distance 6");
    failed |= expectTrue(result != NULL && result->pathLen == 3 &&
                             result->path[0] == a && result->path[1] == c &&
                             result->path[2] == d,
                         "A to D path is A-C-D");
    pathResultFree(result);

    same = dijkstraShortestPath(graph, a, a);
    failed |= expectTrue(same != NULL && same->found && same->distance == 0 &&
                             same->pathLen == 1,
                         "start equals destination");
    pathResultFree(same);

    none = dijkstraShortestPath(graph, a, isolated);
    failed |= expectTrue(none != NULL && !none->found, "no route to X");
    pathResultFree(none);

    graphFree(graph);
    return failed;
}

static int testTrie(void) {
    Trie *trie;
    char results[10][64];
    int count;
    int failed = 0;

    printf("=== Trie ===\n");
    trie = trieCreate();
    trieInsert(trie, "Library");
    trieInsert(trie, "Library Gate");
    triePrefixSearch(trie, "Lib", results, 10, &count);
    failed |= expectTrue(count == 2, "prefix Lib has 2 matches");
    failed |= expectTrue(trieSearch(trie, "Library") == 1, "exact search Library");
    trieFree(trie);
    return failed;
}

static int testBus(void) {
    BusList list;
    BusMatch match;
    BusMatch fullOnly;
    const char *route17[] = {
        "Parking Zone A", "College Exit", "Clement Town", "Rajpur Road"};
    int failed = 0;

    printf("=== Bus filter ===\n");
    busListInit(&list);
    busListAdd(&list, 17, 12, "ZoneA", route17, 4);
    busListAdd(&list, 22, 0, "ZoneB", route17, 4);

    match = busFindBestAvailable(&list, "Parking Zone A", "Rajpur Road");
    failed |= expectTrue(match.found && match.busId == 17 &&
                             match.availableSeats == 12,
                         "skips full bus, picks BUS_17");

    busListInit(&list);
    busListAdd(&list, 22, 0, "ZoneB", route17, 4);
    fullOnly = busFindBestAvailable(&list, "Parking Zone A", "Rajpur Road");
    failed |= expectTrue(!fullOnly.found, "no bus when all are full");
    return failed;
}

static int testDatabase(void) {
    Graph *graph;
    PathResult *result;
    BusList buses;
    BusMatch match;
    int startId;
    int destId;
    const char *path = "test_data/mini_graph.db";
    int failed = 0;

    printf("=== SQLite graph load ===\n");
    failed |= expectTrue(databaseWriteMiniGraph(path), "write mini database");
    failed |= expectTrue(databaseLoadGraph(path, &graph) == DATABASE_OK,
                         "load graph from db");
    if (failed) {
        remove(path);
        return failed;
    }

    startId = graphFindVertex(graph, "A");
    destId = graphFindVertex(graph, "D");
    result = dijkstraShortestPath(graph, startId, destId);
    failed |= expectTrue(result != NULL && result->found && result->distance == 6,
                         "A to D from SQLite is distance 6");
    pathResultFree(result);
    graphFree(graph);

    failed |= expectTrue(databaseLoadBuses(path, &buses) == DATABASE_OK,
                         "load buses from db");
    match = busFindBestAvailable(&buses, "A", "D");
    failed |= expectTrue(match.found && match.busId == 17 &&
                             match.availableSeats == 12,
                         "skips full BUS_22, picks BUS_17");
    remove(path);

    failed |= expectTrue(
        databaseLoadGraph("test_data/no_such_file.db", &graph) ==
            DATABASE_OPEN_FAIL,
        "missing database file");
    failed |= expectTrue(databaseWriteEmptyGraph(path), "write empty database");
    failed |= expectTrue(databaseLoadGraph(path, &graph) == DATABASE_EMPTY,
                         "empty stops table");
    remove(path);
    return failed;
}

static int runAllTests(void) {
    int failed = 0;

    failed += testGraph();
    failed += testHeap();
    failed += testDijkstra();
    failed += testTrie();
    failed += testBus();
    failed += testDatabase();

    if (failed == 0) {
        printf("All tests passed\n");
        return 0;
    }

    printf("%d test(s) failed\n", failed);
    return 1;
}

int main(int argc, char *argv[]) {
    AppConfig config;

    if (argc == 1) {
        printUsage();
        return 0;
    }

    if (!parseArgs(argc, argv, &config)) {
        printUsage();
        return 1;
    }

    if (config.runTests) {
        return runAllTests();
    }

    if (config.prefix != NULL) {
        return runPrefixMode(&config);
    }

    if (config.fromStop != NULL || config.toStop != NULL ||
        config.dbPath != NULL) {
        return runRouteMode(&config);
    }

    printUsage();
    return 1;
}
