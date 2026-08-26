// common.js
// Shared UI helpers used by conductor.js (dashboard.html) and route.js
// (route.html). Depends on nothing but the DOM.

window.ConductorCommon = {
  seatStatus: function (available, total) {
    var ratio = total > 0 ? available / total : 0;
    if (ratio > 0.4) return { label: "Plenty", badgeClass: "bg-emerald-50 text-emerald-700" };
    if (ratio > 0.15) return { label: "Filling Up", badgeClass: "bg-amber-50 text-amber-700" };
    return { label: "Nearly Full", badgeClass: "bg-rose-50 text-rose-700" };
  },

  tripStatus: function (bus) {
    if (!bus.parkingZone) {
      return { label: "Not Started", dotClass: "bg-slate-300", pillClass: "bg-slate-100 text-slate-600" };
    }
    var lastStop = bus.route[bus.route.length - 1];
    if (lastStop && bus.currentStopId === lastStop.stopId) {
      return { label: "Completed", dotClass: "bg-emerald-500", pillClass: "bg-emerald-50 text-emerald-700" };
    }
    return { label: "In Progress", dotClass: "bg-indigo-500", pillClass: "bg-indigo-50 text-indigo-700" };
  },

  // Renders the ordered stop list into an <ol>. Marks stops before the
  // current one as done, the current one as active, the rest as upcoming.
  renderTimeline: function (listEl, bus) {
    listEl.innerHTML = "";
    var currentIdx = bus.route.findIndex(function (s) { return s.stopId === bus.currentStopId; });

    bus.route.forEach(function (stop, i) {
      var isDone = bus.parkingZone ? i < currentIdx : false;
      var isCurrent = bus.parkingZone ? i === currentIdx : false;

      var li = document.createElement("li");
      li.className = "flex items-center gap-3 py-2";

      var dot = document.createElement("span");
      dot.className = "h-3 w-3 flex-shrink-0 rounded-full " +
        (isCurrent ? "bg-indigo-600 ring-4 ring-indigo-100" : isDone ? "bg-emerald-500" : "bg-slate-200");

      var text = document.createElement("span");
      text.className = "text-sm " +
        (isCurrent ? "font-bold text-indigo-700" : isDone ? "text-slate-400 line-through" : "text-slate-700");
      text.textContent = stop.stopName;

      li.appendChild(dot);
      li.appendChild(text);
      listEl.appendChild(li);
    });
  },
};
