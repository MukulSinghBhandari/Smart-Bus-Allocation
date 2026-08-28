# c_engine Integration Contract

Member 1 owns this executable. Member 3 (Flask) calls it. Member 2 owns the `.db` file and writes. The conductor portal must **not** call this engine to update seats; it should `UPDATE Live_Status` (via Flask / Member 2). This engine only **reads**.

Call with a **list of arguments**, not a shell string.

```python
subprocess.run(
    ["c_engine.exe", "--db", db_path, "--from", start, "--to", dest, "--format", "machine"],
    capture_output=True,
    text=True,
    encoding="utf-8",
)
```

Parse **stdout** only. Exit code `0` = success, non-zero = error.

---

## Production route (SQLite)

```text
c_engine.exe --db <campus_transit.db> --from "<start_stop>" --to "<dest_stop>" --format machine
```

Stop names must match `Stops.stop_name` exactly.

### Success

```text
STATUS=OK
DISTANCE=6
STOP_COUNT=3
ROUTE=A|C|D
BUS_ID=17
SEATS=12
PARKING_ZONE=ZoneA
```

| Field | Meaning |
|--------|---------|
| `DISTANCE` | Integer path weight (use metres in `Edges.distance`) |
| `ROUTE` | Stop names, start to dest, separated by `\|` |
| `BUS_ID` | Chosen bus, or `NONE` if no bus has seats and covers start→dest |
| `SEATS` | `Live_Status.available_seats` (`0` means full / unused) |
| `PARKING_ZONE` | From `Live_Status`, or `NONE` |

A full bus (`available_seats <= 0`) is never chosen. Dijkstra still uses the road graph (`Stops` + `Edges`). Bus choice uses `Buses`, `Route_Checklist`, and `Live_Status`.

### Error

```text
STATUS=ERROR
CODE=INVALID_STOP
MESSAGE=Unknown stop name
```

| CODE | When |
|------|------|
| `INVALID_ARGS` | Missing `--from` or `--to` |
| `DB_NOT_FOUND` | `.db` path missing or cannot open |
| `EMPTY_GRAPH` | No rows in `Stops` |
| `LOAD_ERROR` | Schema/query problem |
| `INVALID_STOP` | Start or dest name not in `Stops` |
| `NO_ROUTE` | Stops exist but no path |
| `ROUTE_ERROR` | Internal routing failure |

---

## Dev / tests (text files)

```text
c_engine.exe --data test_data --from "<start>" --to "<dest>" --format machine
c_engine.exe --data test_data --prefix "<text>" --format machine
c_engine.exe --test
```

`--data` is for Member 1 console tests. Flask should use `--db` in production.

### Autocomplete (still `--data` for now)

```text
STATUS=OK
MATCH_COUNT=2
MATCHES=Library|Library Gate
```

---

## Tables this engine reads

| Table | Use |
|--------|-----|
| `Stops` | Vertices |
| `Edges` | Weighted roads (`from_stop_id`, `to_stop_id`, `distance`) |
| `Buses` | Bus ids |
| `Route_Checklist` | Stops on a bus, in order |
| `Live_Status` | `available_seats`, `current_parking_zone` |

Member 2 must keep `Edges` populated. Conductor updates only `Live_Status` (and related live fields), not the C binary.
