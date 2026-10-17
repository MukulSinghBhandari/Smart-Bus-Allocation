// conductor.js
// Page logic for conductor dashboard
// This file is loaded in the browser and has access to
// the ConductorAPI and ConductorCommon objects.

(function () {
  var bus = null;

  function $(id) {
    return document.getElementById(id);
  }

  function render() {
    if (!bus) return;

    $("topbar-busno").textContent =
      "BUS " + bus.busNo;

    $("bus-title").textContent =
      "BUS " + bus.busNo;

    $("bus-reg").textContent =
      bus.registrationNo;

    var shiftsText = [
      bus.morningShiftOne,
      bus.morningShiftTwo,
      bus.eveningShift
    ]
      .filter(Boolean)
      .join(" | ");

    $("bus-timing-badge").textContent =
      shiftsText || "Regular Shift";

    $("bus-conductor").textContent =
      bus.conductorName || "Not Assigned";

    $("bus-phone").textContent =
      bus.conductorPhone || "—";

    var trip =
      window.ConductorCommon.tripStatus(bus);

    $("trip-status-text").textContent =
      trip.label;

    $("trip-status-pill").className =
      "inline-flex items-center gap-2 rounded-full px-3 py-1 text-sm font-bold " +
      trip.pillClass;

    $("trip-status-dot").className =
      "h-2 w-2 rounded-full " +
      trip.dotClass;

    $("stat-bus").textContent =
      bus.busNo;

    $("stat-timing").textContent =
      shiftsText || "Regular";

    $("stat-zone").textContent =
      bus.parkingZone || "Not Assigned";

    // -------------------------------------------------------
    // CURRENT STOP
    // -------------------------------------------------------

    var currentStop =
      bus.route.find(function (s) {
        return s.stopId === bus.currentStopId;
      });

    $("stat-stop").textContent =
      currentStop
        ? currentStop.stopName
        : "—";

    var currentIdx =
      bus.route.findIndex(function (s) {
        return s.stopId === bus.currentStopId;
      });

    // -------------------------------------------------------
    // NEXT STOP
    // -------------------------------------------------------

    var next =
      bus.route[currentIdx + 1];

    $("next-stop-name").textContent =
      next
        ? next.stopName
        : "Trip Complete";

    $("next-stop-sub").textContent =
      next
        ? "Tap the button when you arrive."
        : "All stops covered.";

    $("next-progress").textContent =
      bus.route.length
        ? (Math.max(currentIdx, 0) + 1) +
          " / " +
          bus.route.length
        : "—";

    // -------------------------------------------------------
    // ARRIVED BUTTON
    // -------------------------------------------------------

    $("btn-arrived").disabled =
      !bus.parkingZone || !next;

    // -------------------------------------------------------
    // SEATS
    // -------------------------------------------------------

    var seatInfo =
      window.ConductorCommon.seatStatus(
        bus.availableSeats,
        bus.totalSeats
      );

    $("seat-status-badge").textContent =
      seatInfo.label;

    $("seat-status-badge").className =
      "rounded-full px-3 py-1 text-xs font-bold " +
      seatInfo.badgeClass;

    $("seat-count").textContent =
      bus.availableSeats;

    $("seat-total").textContent =
      bus.totalSeats;

    $("seat-bar").style.width =
      (
        bus.totalSeats
          ? (1 - bus.availableSeats / bus.totalSeats) * 100
          : 0
      ) + "%";

    $("btn-seat-minus").disabled =
      bus.availableSeats <= 0;

    $("btn-seat-plus").disabled =
      bus.availableSeats >= bus.totalSeats;

    // -------------------------------------------------------
    // PARKING ZONE
    // -------------------------------------------------------

    if (bus.parkingZone) {

      $("zone-confirmed")
        .classList
        .remove("hidden");

      $("zone-confirmed")
        .classList
        .add("flex");

      $("zone-confirmed-name").textContent =
        bus.parkingZone;

      $("zone-select").value =
        bus.parkingZone;

    } else {

      $("zone-confirmed")
        .classList
        .add("hidden");

      $("zone-confirmed")
        .classList
        .remove("flex");

      $("zone-confirmed-name").textContent =
        "";

      $("zone-select").value =
        "";
    }

    // -------------------------------------------------------
    // END TRIP
    //
    // Enabled ONLY when the bus reaches the
    // final stop.
    // -------------------------------------------------------

    $("btn-end-trip").disabled =
      !!next;

    // -------------------------------------------------------
    // ROUTE TIMELINE
    // -------------------------------------------------------

    window.ConductorCommon.renderTimeline(
      $("route-timeline"),
      bus
    );
  }

  function populateZoneSelect() {

    var select =
      $("zone-select");

    var zones =
      (window.ConductorData &&
       window.ConductorData.PARKING_ZONES) ||
      [
        "Geu Gate 2",
        "Gehu Gate 1",
        "Gehu Gate 2",
        "Charkhamba",
        "Convention Center"
      ];

    /*
      Prevent duplicate options if this function
      is called again.
    */

    select.innerHTML =
      '<option value="">Select location</option>';

    zones.forEach(function (zone) {

      var opt =
        document.createElement("option");

      opt.value =
        zone;

      opt.textContent =
        zone;

      select.appendChild(opt);
    });
  }

  async function init() {

    populateZoneSelect();

    var res =
      await window.ConductorAPI.getAssignedBus();

    if (!res.ok) {

      console.error(
        "Failed to load assigned bus:",
        res.error
      );

      alert(
        "Failed to load bus: " +
        res.error
      );

      return;
    }

    bus =
      res.data;

    render();

    // -------------------------------------------------------
    // SEAT DECREASE
    // -------------------------------------------------------

    $("btn-seat-minus").addEventListener(
      "click",
      async function () {

        var res =
          await window.ConductorAPI.adjustSeats(
            bus,
            -1
          );

        if (!res.ok) {

          alert(res.error);

          return;
        }

        bus =
          res.data;

        render();
      }
    );

    // -------------------------------------------------------
    // SEAT INCREASE
    // -------------------------------------------------------

    $("btn-seat-plus").addEventListener(
      "click",
      async function () {

        var res =
          await window.ConductorAPI.adjustSeats(
            bus,
            1
          );

        if (!res.ok) {

          alert(res.error);

          return;
        }

        bus =
          res.data;

        render();
      }
    );

    // -------------------------------------------------------
    // PARKING / LOCATION
    // -------------------------------------------------------

    $("btn-confirm-zone").addEventListener(
      "click",
      async function () {

        var zone =
          $("zone-select").value;

        if (!zone) {

          alert(
            "Please select a location."
          );

          return;
        }

        /*
          IMPORTANT:
          Send BUS NUMBER, not internal BUS ID.
        */

        var res =
          await window.ConductorAPI.confirmParkingZone(
            bus.busNo,
            zone
          );

        if (!res.ok) {

          alert(res.error);

          return;
        }

        /*
          Reload from database instead of
          modifying local state manually.
        */

        var fresh =
          await window.ConductorAPI.getBus(
            bus.busNo
          );

        if (!fresh.ok) {

          alert(fresh.error);

          return;
        }

        bus =
          fresh.data;

        render();
      }
    );

    // -------------------------------------------------------
    // NEXT STOP
    // -------------------------------------------------------

    $("btn-arrived").addEventListener(
      "click",
      async function () {

        var res =
          await window.ConductorAPI.advanceStop(
            bus
          );

        if (!res.ok) {

          alert(res.error);

          return;
        }

        bus =
          res.data;

        render();
      }
    );

    // -------------------------------------------------------
    // END TRIP
    // -------------------------------------------------------

    $("btn-end-trip").addEventListener(
      "click",
      async function () {

        if (!window.confirm(
          "End this trip and reset the bus?"
        )) {
          return;
        }

        var res =
          await window.ConductorAPI.endTrip(
            bus
          );

        if (!res.ok) {

          alert(res.error);

          return;
        }

        /*
          Backend returns the completely
          reset bus state.
        */

        bus =
          res.data;

        render();
      }
    );
  }

  init();

})();