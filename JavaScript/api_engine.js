// api_engine.js
// Conductor Portal API Bridge
// Communicates with Flask -> C++ -> SQLite

(function () {
  var BASE_URL = "http://localhost:5000";

  // ---------------------------------------------------------
  // Normalize bus data returned by the backend
  // ---------------------------------------------------------
  function normalizeBus(raw) {
    return {
      busId:
        raw.bus_id !== undefined
          ? raw.bus_id
          : raw.busId,

      busNo:
        raw.bus_number !== undefined
          ? raw.bus_number
          : raw.busNo,

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

      route:
        Array.isArray(raw.route)
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
                    : (
                        s.stopOrder !== undefined
                          ? s.stopOrder
                          : index + 1
                      )
              };
            })
          : []
    };
  }

  // ---------------------------------------------------------
  // GET SINGLE BUS
  //
  // The value supplied here is the BUS NUMBER.
  // Example:
  // getBus("101")
  // ---------------------------------------------------------
  async function getBus(busNo) {
    try {
      var response = await fetch(
        BASE_URL +
        "/api/bus/" +
        encodeURIComponent(busNo)
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
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // GET ASSIGNED BUS
  //
  // IMPORTANT:
  // The URL now uses bus_no, NOT bus_id.
  //
  // Example:
  // dashboard.html?bus_no=101
  // ---------------------------------------------------------
  async function getAssignedBus() {
    var urlParams =
      new URLSearchParams(window.location.search);

    var busNo =
      urlParams.get("bus_no");

    if (!busNo) {
      return {
        ok: false,
        error: "Bus number missing from URL"
      };
    }

    return await getBus(busNo);
  }

  // ---------------------------------------------------------
  // GET ALL BUSES
  // ---------------------------------------------------------
  async function getAllBuses() {
    try {
      var response =
        await fetch(
          BASE_URL + "/api/buses"
        );

      if (!response.ok) {
        return {
          ok: false,
          error: "Failed to fetch buses"
        };
      }

      var result =
        await response.json();

      /*
        Flask currently returns the array directly.
        Example:

        [
          {...},
          {...}
        ]

        Therefore we support both:
        1. Direct array
        2. { ok:true, buses:[...] }
      */

      var buses =
        Array.isArray(result)
          ? result
          : (
              Array.isArray(result.buses)
                ? result.buses
                : []
            );

      return {
        ok: true,
        data: buses.map(normalizeBus)
      };

    } catch (err) {
      return {
        ok: false,
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // UPDATE PARKING ZONE
  // ---------------------------------------------------------
  async function confirmParkingZone(
    busNo,
    zone
  ) {
    try {
      var response =
        await fetch(
          BASE_URL +
          "/api/bus/" +
          encodeURIComponent(busNo) +
          "/parking-zone",
          {
            method: "POST",

            headers: {
              "Content-Type":
                "application/json"
            },

            body: JSON.stringify({
              zone: zone
            })
          }
        );

      var data =
        await response.json();

      if (
        !response.ok ||
        data.ok === false
      ) {
        return {
          ok: false,
          error:
            data.error ||
            "Failed to update parking zone"
        };
      }

      return {
        ok: true,
        data: data
      };

    } catch (err) {
      return {
        ok: false,
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // ADJUST SEATS
  // ---------------------------------------------------------
  async function adjustSeats(
    bus,
    delta
  ) {
    var nextSeats =
      Number(bus.availableSeats) +
      Number(delta);

    // -----------------------------------------
    // Validate minimum
    // -----------------------------------------
    if (nextSeats < 0) {
      return {
        ok: false,
        error:
          "Available seats cannot be negative"
      };
    }

    // -----------------------------------------
    // Validate maximum
    // -----------------------------------------
    if (
      nextSeats >
      Number(bus.totalSeats)
    ) {
      return {
        ok: false,
        error:
          "Available seats cannot exceed total capacity"
      };
    }

    try {

      /*
        IMPORTANT:
        Use bus.busNo here.

        This guarantees that the seat update
        is performed using the BUS NUMBER,
        not the internal database bus ID.
      */

      var response =
        await fetch(
          BASE_URL +
          "/api/bus/" +
          encodeURIComponent(bus.busNo) +
          "/seats",
          {
            method: "POST",

            headers: {
              "Content-Type":
                "application/json"
            },

            body: JSON.stringify({
              available_seats:
                nextSeats
            })
          }
        );

      var data =
        await response.json();

      if (
        !response.ok ||
        data.ok === false
      ) {
        return {
          ok: false,
          error:
            data.error ||
            "Failed to update seats"
        };
      }

      /*
        Fetch the updated bus again
        from SQLite.

        This prevents the UI from
        relying only on its local value.
      */

      return await getBus(
        bus.busNo
      );

    } catch (err) {
      return {
        ok: false,
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // ADVANCE TO NEXT STOP
  // ---------------------------------------------------------
  async function advanceStop(bus) {
    try {

      var response =
        await fetch(
          BASE_URL +
          "/api/bus/" +
          encodeURIComponent(bus.busNo) +
          "/advance-stop",
          {
            method: "POST",

            headers: {
              "Content-Type":
                "application/json"
            }
          }
        );

      var data =
        await response.json();

      if (
        !response.ok ||
        data.ok === false
      ) {
        return {
          ok: false,
          error:
            data.error ||
            "Failed to advance stop"
        };
      }

      /*
        Fetch updated bus state
        from SQLite.
      */

      return await getBus(
        bus.busNo
      );

    } catch (err) {
      return {
        ok: false,
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // END TRIP
  // ---------------------------------------------------------
  async function endTrip(bus) {
    try {

      var response =
        await fetch(
          BASE_URL +
          "/api/bus/" +
          encodeURIComponent(bus.busNo) +
          "/end-trip",
          {
            method: "POST",

            headers: {
              "Content-Type":
                "application/json"
            }
          }
        );

      var data =
        await response.json();

      if (
        !response.ok ||
        data.ok === false
      ) {
        return {
          ok: false,
          error:
            data.error ||
            "Failed to end trip"
        };
      }

      /*
        Fetch final state from SQLite.
      */

      return await getBus(
        bus.busNo
      );

    } catch (err) {
      return {
        ok: false,
        error:
          err.message ||
          "Could not reach the server"
      };
    }
  }

  // ---------------------------------------------------------
  // PUBLIC API
  // ---------------------------------------------------------
  window.ConductorAPI = {

    getAssignedBus:
      getAssignedBus,

    getBus:
      getBus,

    getAllBuses:
      getAllBuses,

    confirmParkingZone:
      confirmParkingZone,

    adjustSeats:
      adjustSeats,

    advanceStop:
      advanceStop,

    endTrip:
      endTrip
  };

})();