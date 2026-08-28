# c_engine — Member 1 Core C & DSA Engine

Smart Campus Bus Allocation & Transit Mapping System.

This folder is the routing engine: graph, min-heap, Dijkstra, bus seat filter, SQLite read.

## Build (Windows)

```text
cd c_engine
.\build.bat
```

First build compiles SQLite and can take a minute. Later builds are faster.

If you have `make`:

```text
make
```

Do not commit `c_engine.exe`, `*.o`, or `*.db`.

## Test

```text
c_engine.exe --test
```

## Route from text files (dev)

```text
c_engine.exe --data test_data --from "Parking Zone A" --to "Rajpur Road" --format human
```

## Route from SQLite (what Flask should call)

```text
c_engine.exe --db ..\path\to\campus_transit.db --from "Parking Zone A" --to "Rajpur Road" --format machine
```

## Autocomplete (dev files)

```text
c_engine.exe --data test_data --prefix "Raj" --format machine
```

## Modules

| File | Purpose |
|------|---------|
| graph.c | Adjacency list |
| minheap.c | Priority queue |
| dijkstra.c | Shortest path |
| trie.c | Prefix autocomplete |
| bus.c | Skip full buses; pick best available |
| loader.c | Read `test_data` text files |
| database.c | Read SQLite (`Stops`, `Edges`, buses, live seats) |
| output.c | Human vs machine stdout |
| main.c | CLI |

Flask contract: `INTEGRATION.md`

Conductor “bus is full” updates `Live_Status.available_seats` in SQLite (other members). This engine only reads that value.
