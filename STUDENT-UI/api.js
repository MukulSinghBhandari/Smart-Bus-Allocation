/* =========================================================
   api.js — THE ONLY FILE YOU EDIT TO CONNECT YOUR C/C++ BACKEND
   Every screen talks to the app through window.API.
   Set API_BASE_URL and uncomment the fetch() lines.
   ========================================================= */

(function () {
  const API_BASE_URL = null; // e.g. "http://localhost:8080/api"

  function data() {
    return JSON.parse(JSON.stringify(window.BUS_DATA || []));
  }

  // "4:00 PM" -> 960 (minutes since midnight), used for sorting
  function timeToMinutes(timing) {
    const m = /^(\d{1,2}):(\d{2})\s*(AM|PM)$/i.exec(String(timing).trim());
    if (!m) return 0;
    let h = parseInt(m[1], 10) % 12;
    if (m[3].toUpperCase() === "PM") h += 12;
    return h * 60 + parseInt(m[2], 10);
  }

  const API = {
    API_BASE_URL,
    timeToMinutes,

    /* GET /buses */
    async getBuses() {
      // return (await fetch(`${API_BASE_URL}/buses`)).json();
      return data();
    },

    /* GET /buses/:busNo */
    async getBus(busNo) {
      // return (await fetch(`${API_BASE_URL}/buses/${busNo}`)).json();
      return data().find((b) => String(b.busNo) === String(busNo)) || null;
    },

    /* GET /buses?destination=... */
    async searchBuses(destination) {
      // return (await fetch(`${API_BASE_URL}/buses?destination=${encodeURIComponent(destination)}`)).json();
      const q = String(destination || "").trim().toLowerCase();
      if (!q) return data();
      return data()
        .filter((b) => b.route.some((stop) => stop.toLowerCase().includes(q)))
        .sort((a, b) => timeToMinutes(a.timing) - timeToMinutes(b.timing));
    },

    /* GET /stops */
    async getStops() {
      // return (await fetch(`${API_BASE_URL}/stops`)).json();
      const set = new Set();
      data().forEach((b) => b.route.forEach((s) => set.add(s)));
      return Array.from(set).sort();
    },

    /* GET /buses/last-available?destination=... — latest bus that still has seats */
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
