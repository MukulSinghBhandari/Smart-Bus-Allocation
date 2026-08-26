// api.js
// Talks to the Flask bridge (server.py) and normalizes its JSON into the
// camelCase shape the rest of the frontend uses.
//
// ASSUMED response contract (built from DataBase.sql's column names, since
// the actual server.py / api.c response format wasn't available):
//
//   GET /api/bus/<bus_id>  ->
//     {
//       "bus_id": 1,
//       "bus_number": "...",
//       "registration_number": "...",
//       "shift_timing": "...",
//       "conductor_name": "...",
//       "conductor_phone": "...",
//       "total_capacity": 50,
//       "available_seats": 32,
//       "current_parking_zone": "Bus Stand" | null,
//       "current_stop_id": 3,
//       "route": [ { "stop_id": 1, "stop_name": "...", "stop_order": 1 }, ... ]
//     }
//
//   POST /api/bus/<bus_id>/parking-zone  body: { "zone": "Bus Stand" }
//
// If your real server responds with different keys or shape, send me one
// real response body and I'll update normalizeBus() below — it's the only
// place field names are translated, so nothing else needs to change.

(function () {
  var BASE_URL = "http://localhost:5000";
  var DEMO_BUS_ID = 1; // no conductor login yet — matches index.html's "No sign-in required"

  function normalizeBus(raw) {
    return {
      busId: raw.bus_id,
      busNo: raw.bus_number,
      registrationNo: raw.registration_number,
      timing: raw.shift_timing,
      conductorName: raw.conductor_name,
      conductorPhone: raw.conductor_phone,
      totalSeats: raw.total_capacity,
      availableSeats: raw.available_seats,
      parkingZone: raw.current_parking_zone || null,
      currentStopId: raw.current_stop_id,
      route: (raw.route || []).map(function (s) {
        return { stopId: s.stop_id, stopName: s.stop_name, stopOrder: s.stop_order };
      }),
    };
  }

  async function getBus(busId) {
    try {
      var response = await fetch(BASE_URL + "/api/bus/" + busId);
      if (!response.ok) {
        return { ok: false, error: "Server responded with " + response.status };
      }
      var raw = await response.json();
      return { ok: true, data: normalizeBus(raw) };
    } catch (err) {
      return { ok: false, error: (err && err.message) || "Could not reach the server" };
    }
  }

  async function getAssignedBus() {
    return getBus(DEMO_BUS_ID);
  }

  async function confirmParkingZone(busId, zone) {
    try {
      var response = await fetch(BASE_URL + "/api/bus/" + busId + "/parking-zone", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ zone: zone }),
      });
      if (!response.ok) {
        return { ok: false, error: "Server responded with " + response.status };
      }
      var raw = await response.json();
      return { ok: true, data: raw };
    } catch (err) {
      return { ok: false, error: (err && err.message) || "Could not reach the server" };
    }
  }

  // --- No documented endpoint exists yet for these three actions, so they
  // update local state only (same "mock-data-driven" pattern the UI already
  // used). Swap each for a real fetch() the moment server.py exposes it. ---

  function adjustSeats(bus, delta) {
    var next = Math.max(0, Math.min(bus.totalSeats, bus.availableSeats + delta));
    return { ok: true, data: Object.assign({}, bus, { availableSeats: next }) };
  }

  function advanceStop(bus) {
    var idx = bus.route.findIndex(function (s) { return s.stopId === bus.currentStopId; });
    var next = bus.route[idx + 1];
    if (!next) return { ok: true, data: bus, finished: true };
    return { ok: true, data: Object.assign({}, bus, { currentStopId: next.stopId }) };
  }

  function endTrip(bus) {
    return { ok: true, data: bus };
  }

  window.ConductorAPI = {
    getAssignedBus: getAssignedBus,
    getBus: getBus,
    confirmParkingZone: confirmParkingZone,
    adjustSeats: adjustSeats,
    advanceStop: advanceStop,
    endTrip: endTrip,
  };
})();
