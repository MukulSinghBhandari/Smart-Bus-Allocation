// api_engine.js
// Talks to the Flask bridge (server.py) and normalizes JSON data
// to match your updated SQLite database columns.

(function () {
  var BASE_URL = "http://localhost:5000";

  function normalizeBus(raw) {
    return {
      busId: raw.bus_id || raw.busId,
      busNo: raw.bus_number || raw.busNo,
      registrationNo: raw.registration_number || raw.registrationNo,
      morningShiftOne: raw.morning_shift_one || raw.morningShiftOne,
      morningShiftTwo: raw.morning_shift_two || raw.morningShiftTwo,
      eveningShift: raw.evening_shift || raw.eveningShift,
      conductorName: raw.conductor_name || raw.conductorName,
      conductorPhone: raw.conductor_phone || raw.conductorPhone,
      totalSeats: raw.total_capacity || raw.totalSeats || 50,
      availableSeats: raw.available_seats !== undefined ? raw.available_seats : raw.availableSeats,
      parkingZone: raw.current_parking_zone || raw.parkingZone || null,
      currentStopId: raw.current_stop_id || raw.currentStopId,
      route: (raw.route || []).map(function (s, index) {
        return { 
          stopId: s.stop_id || s.stopId, 
          stopName: s.stop_name || s.stopName, 
          stopOrder: s.stop_order || (index + 1) 
        };
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
      if (!raw.ok && raw.error) {
        return { ok: false, error: raw.error };
      }
      return { ok: true, data: normalizeBus(raw) };
    } catch (err) {
      return { ok: false, error: (err && err.message) || "Could not reach the server" };
    }
  }

  async function getAssignedBus() {
    const urlParams = new URLSearchParams(window.location.search);
    const busId = urlParams.get('bus_id') || "1";
    
    try {
      var response = await fetch(BASE_URL + "/api/bus/" + busId);
      if (!response.ok) return { ok: false, error: "Bus not found in database" };
      var raw = await response.json();
      if (!raw.ok && raw.error) {
        return { ok: false, error: raw.error };
      }
      return { ok: true, data: normalizeBus(raw) };
    } catch (err) {
      return { ok: false, error: err.message };
    }
  }

  async function getAllBuses() {
    try {
      var response = await fetch(BASE_URL + "/api/buses");
      if (!response.ok) return { ok: false, error: "Failed to fetch buses" };
      var data = await response.json();
      return { ok: true, data: data };
    } catch (err) {
      return { ok: false, error: err.message };
    }
  }

  async function confirmParkingZone(busId, zone) {
    try {
      var response = await fetch(BASE_URL + "/api/bus/" + busId + "/parking-zone", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ zone: zone })
      });
      if (!response.ok) return { ok: false, error: "Failed to update zone" };
      var data = await response.json();
      return { ok: true, data: data };
    } catch (err) {
      return { ok: false, error: err.message };
    }
  }

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
    getAllBuses: getAllBuses,
    confirmParkingZone: confirmParkingZone,
    adjustSeats: adjustSeats,
    advanceStop: advanceStop,
    endTrip: endTrip,
  };
})();