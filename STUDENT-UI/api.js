/* =========================================================
   api.js — STUDENT PORTAL API BRIDGE
   ========================================================= */

(function () {
  const API_BASE_URL = "http://localhost:5000";

  // ---------------------------------------------------------
  // Convert time such as "7:00 AM" into minutes
  // ---------------------------------------------------------
  function timeToMinutes(timing) {
    const m = /^(\d{1,2}):(\d{2})\s*(AM|PM)$/i.exec(
      String(timing || "").trim()
    );

    if (!m) return 0;

    let h = parseInt(m[1], 10) % 12;

    if (m[3].toUpperCase() === "PM") {
      h += 12;
    }

    return h * 60 + parseInt(m[2], 10);
  }

  // ---------------------------------------------------------
  // Normalize route returned by C++
  // ---------------------------------------------------------
  function normalizeRoute(route) {
    if (!Array.isArray(route)) {
      return [];
    }

    return route
      .map((s) => {
        if (typeof s === "string") {
          return s;
        }

        if (s && typeof s === "object") {
          return s.stopName || s.name || "";
        }

        return "";
      })
      .filter(Boolean);
  }

  // ---------------------------------------------------------
  // Calculate route progress
  //
  // currentStopId comes from C++ / SQLite.
  //
  // Example:
  // currentStopId = 14
  //
  // Route:
  // 80 -> 81 -> 10 -> 14
  //
  // Result:
  // currentStop    = "Gehu Gate 2"
  // completedStops = all 4 stops
  // upcomingStops   = []
  // ---------------------------------------------------------
  function calculateRouteStatus(routeData, currentStopId) {
    if (!Array.isArray(routeData) || routeData.length === 0) {
      return {
        currentStop: null,
        completedStops: [],
        upcomingStops: []
      };
    }

    // Convert ID safely to number.
    // -1 means the bus has not reached any stop yet.
    const currentId =
      currentStopId !== undefined &&
      currentStopId !== null &&
      currentStopId !== ""
        ? Number(currentStopId)
        : -1;

    // Find current stop by stopId.
    const currentIndex = routeData.findIndex((stop) => {
      if (!stop || typeof stop !== "object") {
        return false;
      }

      return Number(stop.stopId) === currentId;
    });

    // No valid current stop yet.
    if (currentIndex === -1) {
      return {
        currentStop: null,
        completedStops: [],
        upcomingStops: routeData.map((stop) => {
          return stop.stopName || stop.name || "";
        }).filter(Boolean)
      };
    }

    // Current stop has been reached,
    // therefore it is included in completed stops.
    const completedStops = routeData
      .slice(0, currentIndex + 1)
      .map((stop) => {
        return stop.stopName || stop.name || "";
      })
      .filter(Boolean);

    // Stops after current stop are upcoming.
    const upcomingStops = routeData
      .slice(currentIndex + 1)
      .map((stop) => {
        return stop.stopName || stop.name || "";
      })
      .filter(Boolean);

    const currentStop =
      routeData[currentIndex].stopName ||
      routeData[currentIndex].name ||
      null;

    return {
      currentStop,
      completedStops,
      upcomingStops
    };
  }

  // ---------------------------------------------------------
  // Normalize bus object
  // ---------------------------------------------------------
  function normalizeBus(b) {
    if (!b) {
      return null;
    }

    // -------------------------------------------------------
    // Keep the original route objects temporarily.
    // We need stopId to calculate current/completed/upcoming.
    // -------------------------------------------------------
    const rawRoute =
      Array.isArray(b.route)
        ? b.route
        : [];

    // -------------------------------------------------------
    // Calculate route status from currentStopId
    // -------------------------------------------------------
    const routeStatus =
      calculateRouteStatus(
        rawRoute,
        b.currentStopId
      );

    return {
      busId:
        b.busId ??
        b.id ??
        null,

      busNo: String(
        b.busNo ??
        b.bus_number ??
        b.busId ??
        b.id ??
        ""
      ),

      registrationNo:
        b.registrationNo ??
        b.registration_number ??
        "",

      timing:
        b.timing ??
        b.morningShiftOne ??
        b.morningShiftTwo ??
        b.eveningShift ??
        "7:00 AM",

      morningShiftOne:
        b.morningShiftOne ?? "",

      morningShiftTwo:
        b.morningShiftTwo ?? "",

      eveningShift:
        b.eveningShift ?? "",

      parkingZone:
        b.parkingZone ??
        b.parking_zone ??
        "Main Parking",

      conductor:
        b.conductorName ??
        b.conductor ??
        "Not Assigned",

      phone:
        b.conductorPhone ??
        b.phone ??
        "",

      // -----------------------------------------------------
      // LIVE SEAT DATA
      // -----------------------------------------------------
      availableSeats:
        b.availableSeats !== undefined &&
        b.availableSeats !== null
          ? Number(b.availableSeats)
          : 0,

      totalSeats:
        b.totalSeats !== undefined &&
        b.totalSeats !== null
          ? Number(b.totalSeats)
          : 50,

      // -----------------------------------------------------
      // Keep route as simple stop-name array.
      // This preserves your existing Student UI.
      // -----------------------------------------------------
      route:
        normalizeRoute(rawRoute),

      // -----------------------------------------------------
      // CURRENT STOP ID
      // -----------------------------------------------------
      currentStopId:
        b.currentStopId !== undefined &&
        b.currentStopId !== null
          ? Number(b.currentStopId)
          : null,

      // -----------------------------------------------------
      // CURRENT STOP NAME
      // -----------------------------------------------------
      currentStop:
        routeStatus.currentStop,

      // -----------------------------------------------------
      // COMPLETED STOPS
      // -----------------------------------------------------
      completedStops:
        routeStatus.completedStops,

      // -----------------------------------------------------
      // UPCOMING STOPS
      // -----------------------------------------------------
      upcomingStops:
        routeStatus.upcomingStops
    };
  }

  // ---------------------------------------------------------
  // API OBJECT
  // ---------------------------------------------------------
  const API = {

    API_BASE_URL,

    timeToMinutes,

    // -------------------------------------------------------
    // Get ALL buses
    // -------------------------------------------------------
    async getBuses() {
      try {
        const res = await fetch(
          `${API_BASE_URL}/api/buses`
        );

        if (!res.ok) {
          return [];
        }

        const data = await res.json();

        if (!Array.isArray(data)) {
          return [];
        }

        return data
          .map(normalizeBus)
          .filter(Boolean);

      } catch (err) {
        console.error(
          "Failed to get buses:",
          err
        );

        return [];
      }
    },

    // -------------------------------------------------------
    // Get ONE bus
    // -------------------------------------------------------
    async getBus(identifier) {
      try {
        const res = await fetch(
          `${API_BASE_URL}/api/bus/${encodeURIComponent(identifier)}`
        );

        if (!res.ok) {
          return null;
        }

        const data = await res.json();

        if (!data.ok) {
          return null;
        }

        return normalizeBus(data);

      } catch (err) {
        console.error(
          "Failed to get bus:",
          err
        );

        return null;
      }
    },

    // -------------------------------------------------------
    // Search buses by destination / stop
    //
    // /api/station returns only busId + busNo.
    // Therefore we fetch complete bus information for
    // every matching bus.
    // -------------------------------------------------------
    async searchBuses(destination) {

      const q =
        String(destination || "").trim();

      if (!q) {
        return await API.getBuses();
      }

      try {

        const res = await fetch(
          `${API_BASE_URL}/api/station/${encodeURIComponent(q)}`
        );

        if (!res.ok) {
          return [];
        }

        const data = await res.json();

        const buses =
          data.buses ||
          (Array.isArray(data) ? data : []);

        // ---------------------------------------------------
        // Get complete information for each matching bus
        // ---------------------------------------------------
        const completeBuses =
          await Promise.all(
            buses.map(async (bus) => {

              const identifier =
                bus.busNo ??
                bus.bus_number ??
                bus.busId ??
                bus.id;

              if (
                identifier === undefined ||
                identifier === null
              ) {
                return null;
              }

              return await API.getBus(
                identifier
              );
            })
          );

        return completeBuses
          .filter(Boolean)
          .sort(
            (a, b) =>
              timeToMinutes(a.timing) -
              timeToMinutes(b.timing)
          );

      } catch (err) {

        console.error(
          "Failed to search buses:",
          err
        );

        return [];
      }
    },

    // -------------------------------------------------------
    // Get all unique stops
    // -------------------------------------------------------
    async getStops() {

      const buses =
        await API.getBuses();

      const set =
        new Set();

      buses.forEach((bus) => {

        bus.route.forEach((stop) => {
          set.add(stop);
        });

      });

      return Array.from(set).sort();
    },

    // -------------------------------------------------------
    // Get last available bus
    // -------------------------------------------------------
    async getLastAvailableBus(destination) {

      const buses =
        await API.searchBuses(destination);

      const withSeats =
        buses.filter(
          (bus) =>
            Number(bus.availableSeats) > 0
        );

      if (!withSeats.length) {
        return null;
      }

      return withSeats.reduce(
        (last, bus) =>
          timeToMinutes(bus.timing) >
          timeToMinutes(last.timing)
            ? bus
            : last
      );
    }
  };

  // ---------------------------------------------------------
  // Make API available to app.js
  // ---------------------------------------------------------
  window.API = API;

})();