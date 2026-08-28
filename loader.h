#ifndef LOADER_H
#define LOADER_H

#include "bus.h"
#include "graph.h"
#include "trie.h"

int loaderBuildGraph(const char *dataDir, Graph **graphOut);
int loaderBuildTrie(const char *dataDir, Trie **trieOut);
int loaderLoadBuses(const char *dataDir, BusList *busList);

#endif
