// data.js
// Static reference data for the conductor frontend. No dependencies.
// Loaded first: data.js -> api.js -> common.js -> page script.

window.ConductorData = {
  // Fixed set of valid starting parking zones (validated server-side too,
  // per the project spec: "validating parking zones against predefined
  // options" happens in the C engine).
  PARKING_ZONES: [
    "Geu Gate 2",
    "Gehu Gate 1",
    "Gehu Gate 2",
    "Charkhamba",
    "Bus Stand",
    "Convention Center",
  ],
};
