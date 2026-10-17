# Smart Commuter Bus Allocation and Time-Slotted Transit Scheduler

A campus bus information system for **Graphic Era (GEU / GEHU)** that replaces paper timetables and word-of-mouth with one live source of truth. Students can see where a bus is, where it is parked, and whether seats are free. Conductors update that information from a simple portal.

Repository: <https://github.com/MukulSinghBhandari/Smart-Bus-Allocation>

---

## 1. The problem

Today, a student who commutes by campus bus has to:

- **Guess where the bus is.** There is no shared view of which stop a bus has reached, so students wait at stops without knowing if the bus has already passed.
- **Hunt for the parking location.** After arriving on campus, buses park at different zones (Gate 1, Gate 2, Convention Center, ...). Students do not know where to board for the return trip.
- **Not know about seats.** A student can wait for a bus that is already full.
- **Rely on static timetables.** Shift timings (morning one, morning two, evening) and conductor contacts live in scattered lists.

Conductors have the opposite problem. There is no tool to record where a bus is, how many seats are left, or which zone it is parked in. Every query from a student becomes a phone call.

## 2. The solution

Two portals share one engine and one database.

| Portal | Users | What it does |
|---|---|---|
| **Student Portal** (`STUDENT-UI/`) | Students | Search by bus number or stop name. See live seats, current stop, parking zone, shift timings, full route and the last bus of the day. |
| **Conductor Portal** (`HTML/` + `JavaScript/`) | Conductors | Log in with a bus number. Update parking zone and available seats, advance the bus to the next stop, and end the trip. |

### Key features

- **Seat availability** with colour-coded status (full / filling up / available).
- **Parking location** per bus, set by the conductor.
- **Stop-by-stop progress**: the conductor taps "advance stop" and the next stop is computed from the ordered route.
- **Search by station** to list every bus that serves a stop, with its timings.
- **Shift-aware timetable** (morning shift one, morning shift two, evening).
- **Conductor contact details** attached to each bus.
- **Bulk data import** from `buses.csv`, so the fleet data can be maintained in a spreadsheet.

### How tracking works (and what it is not)

"Tracking" here is **conductor-reported**, not GPS-based. The bus position is the last stop the conductor marked. This keeps the system free of hardware and cheap to deploy, but accuracy depends on conductors updating it. GPS or IoT tracking is a possible future extension, and we would only add it if the measured accuracy of manual updates turns out to be inadequate.

## 3. Architecture

```
 ┌──────────────┐   ┌──────────────────┐
 │ Student UI   │   │ Conductor Portal │      Browser (HTML / CSS / vanilla JS)
 └──────┬───────┘   └────────┬─────────┘
        │   HTTP / JSON      │
        ▼                    ▼
 ┌────────────────────────────────────┐
 │ Flask server  (Python/server.py)   │      REST API on :5000, CORS enabled
 └──────────────────┬─────────────────┘
                    │ subprocess call: transit_engine.exe <command> <args>
                    ▼
 ┌────────────────────────────────────┐
 │ C++ engine (C_engine/main.cpp)     │      Business logic, JSON output
 │  └─ C graph module (graph.c)       │      Next-stop lookup on a route
 └──────────────────┬─────────────────┘
                    ▼
 ┌────────────────────────────────────┐
 │ SQLite  (campus_transit.db)        │      Stops, Buses, Route_Checklist, Live_Status
 └────────────────────────────────────┘
```

**Design choice:** the Flask server holds no logic. It forwards each request to the compiled C++ engine as a command-line call and returns the engine's JSON. This keeps the core logic in one place. The trade-off is that every request spawns a process, which is fine for a campus-scale load but would not scale to thousands of concurrent users.

### Database schema

Defined in `Data_Base/DataBase.sql`:

| Table | Purpose |
|---|---|
| `Stops` | Unique stop names |
| `Buses` | Bus number, registration, three shift timings, conductor name and phone, capacity (default 50) |
| `Route_Checklist` | Ordered stops per bus (`bus_id`, `stop_id`, `stop_order`) |
| `Live_Status` | Current parking zone, current stop, available seats |

A trigger (`trg_init_live_status`) automatically creates a `Live_Status` row for every new bus, with the default zone `Gehu Gate 2` and all seats free.

### REST API

| Method | Endpoint | Body | Description |
|---|---|---|---|
| GET | `/api/buses` | | All buses |
| GET | `/api/bus/<bus_no>` | | One bus with route and live status |
| GET | `/api/station/<station_name>` | | Buses serving a stop |
| POST | `/api/bus/<bus_no>/parking-zone` | `{"zone": "..."}` | Update parking zone |
| POST | `/api/bus/<bus_no>/seats` | `{"available_seats": n}` | Update available seats |
| POST | `/api/bus/<bus_no>/advance-stop` | | Move the bus to its next stop |
| POST | `/api/bus/<bus_no>/end-trip` | | End the current trip |

## 4. Project structure

```
Smart-Bus-Allocation/
├── C_engine/            C++ engine, C graph module, bundled SQLite amalgamation
│   ├── main.cpp         Command dispatcher (get_all_buses, get_bus, update_zone, ...)
│   ├── BusModel.hpp
│   ├── DataBaseHandler.hpp
│   └── algorithms/      graph.c / graph.h (next-stop logic)
├── Data_Base/
│   ├── DataBase.sql     Schema and trigger
│   └── buses.csv        Fleet, shifts, conductors and routes
├── Python/
│   ├── server.py        Flask API bridge
│   ├── build_db.py      Creates campus_transit.db from the schema
│   └── auto_import.py   Loads buses.csv into the database
├── HTML/ + JavaScript/  Conductor portal
├── STUDENT-UI/          Student portal
└── README.md
```

## 5. Getting started

### Prerequisites

- Python 3.9+ with `flask` and `flask-cors`
- A C and C++ compiler (MinGW-w64 `gcc`/`g++` on Windows)

### Setup

```bash
# 1. Python dependencies
pip install flask flask-cors

# 2. Build the database and import the fleet data
python Python/build_db.py
python Python/auto_import.py

# 3. Compile the engine (from the repository root)
gcc -c C_engine/sqlite3.c -o C_engine/sqlite3.o
gcc -c C_engine/algorithms/graph.c -o C_engine/algorithms/graph.o
g++ C_engine/main.cpp C_engine/sqlite3.o C_engine/algorithms/graph.o -o C_engine/transit_engine.exe

# 4. Start the API server (run from the repository root)
python Python/server.py
```

The API is now live at `http://localhost:5000`.

> **Notes**
> - `server.py` expects the executable at `C_engine/transit_engine.exe`, so run it from the repository root.
> - `build_db.py` writes `campus_transit.db` into the **current directory**, while `auto_import.py` reads `../campus_transit.db` relative to `Python/`. Run both from the repository root so they use the same file, and re-check the path if the import reports missing tables.
> - `*.db` and `*.exe` are git-ignored, so each developer builds them locally.
> - Re-running `auto_import.py` on a populated database may insert duplicate buses. Rebuild the database first if you want a clean state.

### Running the portals

Open the HTML files in a browser, or serve them locally:

```bash
python -m http.server 8000
```

- Student portal: `http://localhost:8000/STUDENT-UI/index.html`
- Conductor portal: `http://localhost:8000/HTML/index.html` (enter a bus number to open the dashboard)

Both portals call the API at `http://localhost:5000`. To change it, edit `API_BASE_URL` in `STUDENT-UI/api.js` and `BASE_URL` in `JavaScript/api_engine.js`.

## 6. Usage walkthrough

**Student:** open the student portal, then search for your stop (for example "ISBT") or a bus number. You see each matching bus with its seats, current stop, parking zone and timings.

**Conductor:** open the conductor portal and enter your bus number. On the dashboard you can:

1. Set the **parking zone** after arriving on campus.
2. Update **available seats** as students board.
3. **Advance stop** at each stop along the route.
4. **End trip** when the run is finished.

## 7. Known limitations

- Position data is only as fresh as the conductor's last update.
- The conductor portal has **no authentication**. Anyone who knows a bus number can modify its status. This is the most important thing to fix before real deployment.
- The engine is invoked once per request, which limits concurrency.
- No automated tests yet.
- Seat counts are entered manually. They are not reconciled with any ticketing or boarding data.

## 8. Roadmap and evaluation ideas

Suggested next steps, ordered by value:

1. **Conductor authentication** (per-bus PIN or login) and input validation (seat count between 0 and capacity).
2. **Baseline and success metrics.** Compare against the current process: time a student spends finding a bus, share of buses with an up-to-date status, and number of "where is the bus?" phone calls to conductors. Collect these with a small pilot on a few routes.
3. **Automated tests** for the engine commands and API endpoints (including edge cases: last stop, unknown bus, negative seats).
4. **Staleness indicator** ("updated 12 min ago") so students can judge how much to trust the data.
5. **Optional GPS or IoT tracking**, only if the pilot shows manual updates are not accurate enough.
6. **Replace the subprocess bridge** with a persistent service or direct bindings if load grows.

## 9. Contributors

Project contriutors are :- 
Mukul Singh Bhandari - Backend Development
Devanshu Negi - Database 
Aditya - Api integration
Sarthak Panwar - Frontend Development 

