// route.js
// Page logic for route.html. 

(function () {
  function $(id) { return document.getElementById(id); }

  async function init() {
    var res = await window.ConductorAPI.getAssignedBus();
    if (!res.ok) {
      console.error("Failed to load assigned bus:", res.error);
      return;
    }
    var bus = res.data;

    $("r-bus-title").textContent = "BUS " + bus.busNo;
    $("r-bus-reg").textContent = bus.registrationNo;

    var trip = window.ConductorCommon.tripStatus(bus);
    $("r-trip-text").textContent = trip.label;
    $("r-trip-pill").className = "inline-flex items-center gap-2 rounded-full px-3 py-1 text-sm font-bold " + trip.pillClass;
    $("r-trip-dot").className = "h-2 w-2 rounded-full " + trip.dotClass;

    // Updated to display your three shift columns nicely
    var shiftsText = [bus.morningShiftOne, bus.morningShiftTwo, bus.eveningShift].filter(Boolean).join(" | ");
    $("r-timing").textContent = shiftsText || "Regular Shift";
    
    $("r-zone").textContent = bus.parkingZone || "\u2014";
    $("r-seats").textContent = bus.availableSeats + " / " + bus.totalSeats;

    var currentStop = bus.route.find(function (s) { return s.stopId === bus.currentStopId; });
    $("r-current").textContent = currentStop ? currentStop.stopName : "\u2014";
    $("r-conductor").textContent = bus.conductorName;
    $("r-phone").textContent = bus.conductorPhone;

    var currentIdx = bus.route.findIndex(function (s) { return s.stopId === bus.currentStopId; });
    $("r-progress").textContent = bus.route.length ? (Math.max(currentIdx, 0) + 1) + " / " + bus.route.length : "\u2014";

    window.ConductorCommon.renderTimeline($("r-timeline"), bus);
  }

  init();
})();