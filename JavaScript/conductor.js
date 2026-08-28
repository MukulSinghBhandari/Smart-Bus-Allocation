// conductor.js
// Page logic for dashboard.html. 

(function () {
  var bus = null;

  function $(id) { return document.getElementById(id); }

  function render() {
    if (!bus) return;

    $("topbar-busno").textContent = "BUS " + bus.busNo;
    $("bus-title").textContent = "BUS " + bus.busNo;
    $("bus-reg").textContent = bus.registrationNo;
    
    // Updated to display your three shift columns nicely
    var shiftsText = [bus.morningShiftOne, bus.morningShiftTwo, bus.eveningShift].filter(Boolean).join(" | ");
    $("bus-timing-badge").textContent = shiftsText || "Regular Shift";
    
    $("bus-conductor").textContent = bus.conductorName;
    $("bus-phone").textContent = bus.conductorPhone;

    var trip = window.ConductorCommon.tripStatus(bus);
    $("trip-status-text").textContent = trip.label;
    $("trip-status-pill").className = "inline-flex items-center gap-2 rounded-full px-3 py-1 text-sm font-bold " + trip.pillClass;
    $("trip-status-dot").className = "h-2 w-2 rounded-full " + trip.dotClass;

    $("stat-bus").textContent = bus.busNo;
    $("stat-timing").textContent = shiftsText || "Regular";
    $("stat-zone").textContent = bus.parkingZone || "\u2014";
    var currentStop = bus.route.find(function (s) { return s.stopId === bus.currentStopId; });
    $("stat-stop").textContent = currentStop ? currentStop.stopName : "\u2014";

    var currentIdx = bus.route.findIndex(function (s) { return s.stopId === bus.currentStopId; });
    var next = bus.route[currentIdx + 1];
    $("next-stop-name").textContent = next ? next.stopName : "Trip Complete";
    $("next-stop-sub").textContent = next ? "Tap the button when you arrive." : "All stops covered.";
    $("next-progress").textContent = bus.route.length ? (Math.max(currentIdx, 0) + 1) + " / " + bus.route.length : "\u2014";
    $("btn-arrived").disabled = !bus.parkingZone || !next;

    var seatInfo = window.ConductorCommon.seatStatus(bus.availableSeats, bus.totalSeats);
    $("seat-status-badge").textContent = seatInfo.label;
    $("seat-status-badge").className = "rounded-full px-3 py-1 text-xs font-bold " + seatInfo.badgeClass;
    $("seat-count").textContent = bus.availableSeats;
    $("seat-total").textContent = bus.totalSeats;
    $("seat-bar").style.width = (bus.totalSeats ? (1 - bus.availableSeats / bus.totalSeats) * 100 : 0) + "%";
    $("btn-seat-minus").disabled = bus.availableSeats <= 0;
    $("btn-seat-plus").disabled = bus.availableSeats >= bus.totalSeats;

    if (bus.parkingZone) {
      $("zone-confirmed").classList.remove("hidden");
      $("zone-confirmed").classList.add("flex");
      $("zone-confirmed-name").textContent = bus.parkingZone;
      $("zone-select").value = bus.parkingZone;
    }
    $("btn-end-trip").disabled = !next;

    window.ConductorCommon.renderTimeline($("route-timeline"), bus);
  }

  function populateZoneSelect() {
    var select = $("zone-select");
    // Fallback array if ConductorData is removed
    var zones = (window.ConductorData && window.ConductorData.PARKING_ZONES) || ["Bus Stand", "Main Gate", "Hostel Quad", "Engineering Block"];
    zones.forEach(function (zone) {
      var opt = document.createElement("option");
      opt.value = zone;
      opt.textContent = zone;
      select.appendChild(opt);
    });
  }

  async function init() {
    populateZoneSelect();

    var res = await window.ConductorAPI.getAssignedBus();
    if (!res.ok) {
      console.error("Failed to load assigned bus:", res.error);
      return;
    }
    bus = res.data;
    render();

    $("btn-seat-minus").addEventListener("click", function () {
      bus = window.ConductorAPI.adjustSeats(bus, -1).data;
      render();
    });

    $("btn-seat-plus").addEventListener("click", function () {
      bus = window.ConductorAPI.adjustSeats(bus, 1).data;
      render();
    });

    $("btn-confirm-zone").addEventListener("click", async function () {
      var zone = $("zone-select").value;
      if (!zone) return;
      var res = await window.ConductorAPI.confirmParkingZone(bus.busId, zone);
      if (!res.ok) {
        console.error("Failed to confirm parking zone:", res.error);
        return;
      }
      bus.parkingZone = zone;
      render();
    });

    $("btn-arrived").addEventListener("click", function () {
      bus = window.ConductorAPI.advanceStop(bus).data;
      render();
    });

    $("btn-end-trip").addEventListener("click", function () {
      if (!window.confirm("End this trip?")) return;
      bus = window.ConductorAPI.endTrip(bus).data;
      render();
    });
  }

  init();
})();