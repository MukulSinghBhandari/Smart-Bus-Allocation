// api_engine.js
// Conductor Portal API Bridge
// Communicates with Flask -> C++ -> SQLite

(function () {
  var BASE_URL = "http://localhost:5000";

  function normalizeBus(raw) {
    return {
      busId: raw.bus_id !== undefined ? raw.bus_id : raw.busId,
      busNo: raw.bus_number !== undefined ? raw.bus_number : raw.busNo,
      registrationNo:
        raw.registration_number !== undefined
          ? raw.registration_number
          : raw.registrationNo,

      morningShiftOne:
        raw.morning_shift_one !== undefined
          ? raw.morning_shift_one
          : raw.morningShiftOne,

      morningShiftTwo:
        raw.morning_shift_two !== undefined
          ? raw.morning_shift_two
          : raw.morningShiftTwo,

      eveningShift:
        raw.evening_shift !== undefined
          ? raw.evening_shift
          : raw.eveningShift,

      conductorName:
        raw.conductor_name !== undefined
          ? raw.conductor_name
          : raw.conductorName,

      conductorPhone:
        raw.conductor_phone !== undefined
          ? raw.conductor_phone
          : raw.conductorPhone,

      totalSeats:
        raw.total_capacity !== undefined
          ? raw.total_capacity
          : (raw.totalSeats || 50),

      availableSeats:
        raw.available_seats !== undefined
          ? raw.available_seats
          : raw.availableSeats,

      parkingZone:
        raw.current_parking_zone !== undefined
          ? raw.current_parking_zone
          : (raw.parkingZone || null),

      currentStopId:
        raw.current_stop_id !== undefined
          ? raw.current_stop_id
          : raw.currentStopId,

      route: Array.isArray(raw.route)
        ? raw.route.map(function (s, index) {
            return {
              stopId:
                s.stop_id !== undefined
                  ? s.stop_id
                  : s.stopId,

              stopName:
                s.stop_name !== undefined
                  ? s.stop_name
                  : s.stopName,

              stopOrder:
                s.stop_order !== undefined
                  ? s.stop_order
                  : (s.stopOrder !== undefined
                      ? s.stopOrder
                      : index + 1)
            };
          })
        : []
    };
  }

  async function getBus(busId) {
    try {
      var response = await fetch(
        BASE_URL + "/api/bus/" + encodeURIComponent(busId)
      );

      if (!response.ok) {
        return {
          ok: false,
          error: "Server responded with " + response.status
        };
      }

      var raw = await response.json();

      if (!raw.ok) {
        return {
          ok: false,
          error: raw.error || "Failed to get bus"
        };
      }

      return {
        ok: true,
        data: normalizeBus(raw)
      };

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  async function getAssignedBus() {
    var urlParams = new URLSearchParams(window.location.search);

    /*
      IMPORTANT:
      The URL may contain either bus_id or bus_no.

      Example:
      dashboard.html?bus_id=134
    */

    var busId = urlParams.get("bus_id");

    if (!busId) {
      busId = "1";
    }

    return await getBus(busId);
  }

  async function getAllBuses() {
    try {
      var response = await fetch(BASE_URL + "/api/buses");

      if (!response.ok) {
        return {
          ok: false,
          error: "Failed to fetch buses"
        };
      }

      var result = await response.json();

      if (!result.ok) {
        return {
          ok: false,
          error: result.error || "Failed to fetch buses"
        };
      }

      var buses = Array.isArray(result.buses)
        ? result.buses
        : (Array.isArray(result) ? result : []);

      return {
        ok: true,
        data: buses.map(normalizeBus)
      };

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  async function confirmParkingZone(busId, zone) {
    try {
      var response = await fetch(
        BASE_URL + "/api/bus/" +
        encodeURIComponent(busId) +
        "/parking-zone",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify({
            zone: zone
          })
        }
      );

      var data = await response.json();

      if (!response.ok || data.ok === false) {
        return {
          ok: false,
          error: data.error || "Failed to update parking zone"
        };
      }

      return {
        ok: true,
        data: data
      };

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  async function adjustSeats(bus, delta) {
    var nextSeats =
      Number(bus.availableSeats) + Number(delta);

    /*
      Client-side validation.
      Backend will also validate later.
    */

    if (nextSeats < 0) {
      return {
        ok: false,
        error: "Available seats cannot be negative"
      };
    }

    if (nextSeats > Number(bus.totalSeats)) {
      return {
        ok: false,
        error: "Available seats cannot exceed total capacity"
      };
    }

    try {
      var response = await fetch(
        BASE_URL + "/api/bus/" +
        encodeURIComponent(bus.busNo) +
        "/seats",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify({
            available_seats: nextSeats
          })
        }
      );

      var data = await response.json();

      if (!response.ok || data.ok === false) {
        return {
          ok: false,
          error: data.error || "Failed to update seats"
        };
      }

      /*
        Do not trust only the local value.
        Fetch the bus again from the database.
      */

      return await getBus(bus.busNo);

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  async function advanceStop(bus) {
    try {
      var response = await fetch(
        BASE_URL + "/api/bus/" +
        encodeURIComponent(bus.busNo) +
        "/advance-stop",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          }
        }
      );

      var data = await response.json();

      if (!response.ok || data.ok === false) {
        return {
          ok: false,
          error: data.error || "Failed to advance stop"
        };
      }

      /*
        Fetch the updated bus from SQLite.
      */

      return await getBus(bus.busNo);

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  async function endTrip(bus) {
    try {
      var response = await fetch(
        BASE_URL + "/api/bus/" +
        encodeURIComponent(bus.busNo) +
        "/end-trip",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          }
        }
      );

      var data = await response.json();

      if (!response.ok || data.ok === false) {
        return {
          ok: false,
          error: data.error || "Failed to end trip"
        };
      }

      /*
        Fetch final state from SQLite.
      */

      return await getBus(bus.busNo);

    } catch (err) {
      return {
        ok: false,
        error: err.message || "Could not reach the server"
      };
    }
  }

  window.ConductorAPI = {
    getAssignedBus: getAssignedBus,
    getBus: getBus,
    getAllBuses: getAllBuses,
    confirmParkingZone: confirmParkingZone,
    adjustSeats: adjustSeats,
    advanceStop: advanceStop,
    endTrip: endTrip
  };

})();