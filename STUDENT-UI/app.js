/* =========================================================
   app.js — all UI logic (vanilla JS, no framework)
   ========================================================= */

(function () {
  const form = document.getElementById("search-form");
  const input = document.getElementById("search-input");
  const stopList = document.getElementById("stop-list");
  const results = document.getElementById("results");
  const resultsTitle = document.getElementById("results-title");
  const lastBusBox = document.getElementById("last-bus");
  const scheduleBody = document.getElementById("schedule-body");
  const details = document.getElementById("details");
  const detailsTitle = document.getElementById("details-title");
  const detailsBody = document.getElementById("details-body");

  function esc(v) {
    return String(v).replace(/[&<>"]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c]));
  }

  function seatColor(bus) {
    if (bus.availableSeats === 0) return "bg-red-100 text-red-700";
    if (bus.availableSeats <= 10) return "bg-amber-100 text-amber-700";
    return "bg-green-100 text-green-700";
  }

  /* ---------- BUS CARD ---------- */
  function busCard(bus) {
    return `
      <article class="bus-card rounded-xl bg-white p-4 shadow">
        <div class="flex items-start justify-between">
          <div>
            <p class="text-lg font-bold text-indigo-600">Bus ${esc(bus.busNo)}</p>
            <p class="text-sm text-slate-500">${esc(bus.timing)}</p>
          </div>
          <span class="rounded-lg px-2 py-1 text-xs font-semibold ${seatColor(bus)}">
            ${bus.availableSeats} / ${bus.totalSeats} seats
          </span>
        </div>
        <dl class="mt-3 space-y-1 text-sm text-slate-600">
          <div><span class="text-slate-400">Parking zone:</span> ${esc(bus.parkingZone)}</div>
          <div><span class="text-slate-400">Conductor:</span> ${esc(bus.conductor)}</div>
          <div><span class="text-slate-400">Destination:</span> ${esc(bus.route[bus.route.length - 1])}</div>
        </dl>
        <button data-bus="${esc(bus.busNo)}"
          class="view-btn mt-4 w-full rounded-lg bg-indigo-600 py-2 text-sm font-semibold text-white hover:bg-indigo-700">
          View Details
        </button>
      </article>`;
  }

  /* ---------- LAST AVAILABLE BUS ---------- */
  function lastBusCard(bus) {
    if (!bus) return "";
    return `
      <div class="rounded-xl bg-indigo-600 p-4 text-white shadow">
        <p class="text-xs font-semibold uppercase tracking-wide text-indigo-200">Last Available Bus</p>
        <div class="mt-2 flex flex-wrap items-center justify-between gap-3">
          <div>
            <p class="text-xl font-bold">Bus ${esc(bus.busNo)} · ${esc(bus.timing)}</p>
            <p class="text-sm text-indigo-100">${esc(bus.availableSeats)} seats left · ${esc(bus.parkingZone)} · ${esc(bus.conductor)}</p>
          </div>
          <button data-bus="${esc(bus.busNo)}" class="view-btn rounded-lg bg-white px-4 py-2 text-sm font-semibold text-indigo-700">View Details</button>
        </div>
      </div>`;
  }

  /* ---------- ROUTE PROGRESS ---------- */
  function routeProgress(bus) {
    return bus.route
      .map((stop, i) => {
        const done = bus.completedStops.includes(i);
        const current = i === bus.currentStop;
        const mark = done ? "✓" : current ? "●" : "○";
        const cls = done ? "stop-done" : current ? "stop-current" : "stop-upcoming";
        const label = done ? "Completed" : current ? "Current stop" : "Upcoming";
        return `<li class="flex items-center gap-2 py-1.5">
            <span class="stop-mark ${cls}">${mark}</span>
            <span class="text-sm ${current ? "font-bold text-indigo-700" : done ? "text-slate-500 line-through" : "text-slate-700"}">${esc(stop)}</span>
            <span class="ml-auto text-xs text-slate-400">${label}</span>
          </li>`;
      })
      .join("");
  }

  /* ---------- DETAILS ---------- */
  async function openDetails(busNo) {
    const bus = await API.getBus(busNo);
    if (!bus) return;
    const upcoming = bus.route.filter((_, i) => i > bus.currentStop);
    detailsTitle.textContent = `Bus ${bus.busNo} · ${bus.timing}`;
    detailsBody.innerHTML = `
      <div class="grid grid-cols-2 gap-3 text-sm">
        <div class="rounded-lg bg-slate-50 p-3"><p class="text-xs text-slate-400">Available seats</p><p class="font-bold">${bus.availableSeats} / ${bus.totalSeats}</p></div>
        <div class="rounded-lg bg-slate-50 p-3"><p class="text-xs text-slate-400">Parking zone</p><p class="font-bold">${esc(bus.parkingZone)}</p></div>
        <div class="rounded-lg bg-slate-50 p-3"><p class="text-xs text-slate-400">Conductor</p><p class="font-bold">${esc(bus.conductor)}</p></div>
        <div class="rounded-lg bg-slate-50 p-3"><p class="text-xs text-slate-400">Current stop</p><p class="font-bold">${esc(bus.route[bus.currentStop])}</p></div>
      </div>

      <h3 class="mt-5 text-sm font-bold">Full Route</h3>
      <ul class="mt-2 divide-y divide-slate-100">${routeProgress(bus)}</ul>

      <div class="mt-4 grid gap-2 text-sm sm:grid-cols-2">
        <div><span class="text-slate-400">Completed:</span> ${bus.completedStops.length} stops</div>
        <div><span class="text-slate-400">Upcoming:</span> ${upcoming.length} stops</div>
      </div>

      <a href="tel:${esc(bus.phone)}" class="mt-5 block rounded-lg bg-indigo-600 py-2.5 text-center text-sm font-semibold text-white hover:bg-indigo-700">Call Conductor</a>
    `;
    details.classList.remove("hidden");
    details.classList.add("flex");
  }

  function closeDetails() {
    details.classList.add("hidden");
    details.classList.remove("flex");
  }

  /* ---------- RENDER ---------- */
  async function render(query) {
    const buses = await API.searchBuses(query);
    resultsTitle.textContent = query ? `Buses to "${query}" (${buses.length})` : `All Buses (${buses.length})`;
    results.innerHTML = buses.length
      ? buses.map(busCard).join("")
      : `<p class="rounded-xl bg-white p-6 text-center text-sm text-slate-500 sm:col-span-2">No buses found for this stop.</p>`;
    lastBusBox.innerHTML = lastBusCard(await API.getLastAvailableBus(query));
  }

  async function renderSchedule() {
    const buses = (await API.getBuses()).sort(
      (a, b) => API.timeToMinutes(a.timing) - API.timeToMinutes(b.timing)
    );
    scheduleBody.innerHTML = buses
      .map(
        (b) => `<tr class="border-t border-slate-100">
          <td class="px-4 py-3 font-semibold">${esc(b.timing)}</td>
          <td class="px-4 py-3">Bus ${esc(b.busNo)}</td>
          <td class="px-4 py-3">${esc(b.route[b.route.length - 1])}</td>
          <td class="px-4 py-3">${b.availableSeats === 0 ? '<span class="text-red-600 font-semibold">Full</span>' : b.availableSeats}</td>
        </tr>`
      )
      .join("");
  }

  /* ---------- EVENTS ---------- */
  form.addEventListener("submit", (e) => {
    e.preventDefault();
    render(input.value.trim());
  });

  document.addEventListener("click", (e) => {
    const btn = e.target.closest(".view-btn");
    if (btn) openDetails(btn.dataset.bus);
  });

  document.getElementById("details-close").addEventListener("click", closeDetails);
  details.addEventListener("click", (e) => {
    if (e.target === details) closeDetails();
  });

  /* ---------- INIT ---------- */
  (async function init() {
    const stops = await API.getStops();
    stopList.innerHTML = stops.map((s) => `<option value="${esc(s)}"></option>`).join("");
    await render("");
    await renderSchedule();
  })();
})();
