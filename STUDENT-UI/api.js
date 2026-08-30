/* =========================================================
   api.js — STUDENT PORTAL API BRIDGE
   ========================================================= */

(function () {
  const API_BASE_URL = "http://localhost:5000";

  function timeToMinutes(timing) {
    const m = /^(\d{1,2}):(\d{2})\s*(AM|PM)$/i.exec(String(timing || "").trim());
    if (!m) return 0;
    let h = parseInt(m[1], 10) % 12;
    if (m[3].toUpperCase() === "PM") h += 12;
    return h * 60 + parseInt(m[2], 10);
  }

  function normalizeRoute(route) {
    if (!Array.isArray(route)) return [];
    return route.map(s => {
      if (typeof s === 'string') return s;
      if (s && typeof s === 'object') return s.stopName || s.name || "";
      return "";
    }).filter(Boolean);
  }

  function normalizeBus(b) {
    if (!b) return null;
    return {
      busId: b.busId || b.id,
      busNo: b.busNo || b.id || "1",
      registrationNo: b.registrationNo || "",
      timing: b.timing || b.morningShiftOne || b.morningShiftTwo || b.eveningShift || "7:00 AM",
      parkingZone: b.parkingZone || "Main Parking",
      conductor: b.conductorName || b.conductor || "Not Assigned",
      phone: b.conductorPhone || b.phone || "",
      availableSeats: b.availableSeats !== undefined ? b.availableSeats : 50,
      totalSeats: b.totalSeats || 50,
      route: normalizeRoute(b.route),
      completedStops: Array.isArray(b.completedStops) ? b.completedStops : [],
      currentStop: typeof b.currentStop === 'number' ? b.currentStop : 0
    };
  }

  const API = {
    API_BASE_URL,
    timeToMinutes,

    async getBuses() {
      try {
        const res = await fetch(`${API_BASE_URL}/api/buses`);
        if (!res.ok) return [];
        const data = await res.json();
        return (Array.isArray(data) ? data : []).map(normalizeBus);
      } catch (err) {
        return [];
      }
    },

    async getBus(busNo) {
      try {
        const res = await fetch(`${API_BASE_URL}/api/bus/${encodeURIComponent(busNo)}`);
        if (!res.ok) return null;
        const data = await res.json();
        if (!data.ok) return null;
        return normalizeBus(data);
      } catch (err) {
        return null;
      }
    },

    async searchBuses(destination) {
      const q = String(destination || "").trim();
      if (!q) return await API.getBuses();
      try {
        const res = await fetch(`${API_BASE_URL}/api/station/${encodeURIComponent(q)}`);
        if (!res.ok) return [];
        const data = await res.json();
        const buses = data.buses || (Array.isArray(data) ? data : []);
        return buses.map(normalizeBus).sort((a, b) => timeToMinutes(a.timing) - timeToMinutes(b.timing));
      } catch (err) {
        return [];
      }
    },

    async getStops() {
      const buses = await API.getBuses();
      const set = new Set();
      buses.forEach((b) => {
        b.route.forEach((s) => set.add(s));
      });
      return Array.from(set).sort();
    },

    async getLastAvailableBus(destination) {
      const buses = await API.searchBuses(destination);
      const withSeats = buses.filter((b) => b.availableSeats > 0);
      if (!withSeats.length) return null;
      return withSeats.reduce((last, b) =>
        timeToMinutes(b.timing) > timeToMinutes(last.timing) ? b : last
      );
    }
  };

  window.API = API;
})();