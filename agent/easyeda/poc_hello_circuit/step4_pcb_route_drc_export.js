// M4 PCB PoC Step 2: Route all 4 nets on Top layer (15 mil) using exact measured pad coordinates,
// create Top-layer GND copper pour (inset 20 mil from BoardOutline), rebuild copper regions,
// run PCB DRC & ratline check, and export rendered PNG, Gerber ZIP, and Pick-and-Place CSV.

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const snap = (v) => Math.round(v * 100) / 100;

function toB64(blob) {
  if (!blob) return null;
  return new Promise((res, rej) => {
    const fr = new FileReader();
    fr.onload = () => res(String(fr.result).split(",")[1] || "");
    fr.onerror = (e) => rej(e);
    fr.readAsDataURL(blob);
  });
}

// Re-read exact pad coordinates from active PCB
const comps = await eda.pcb_PrimitiveComponent.getAll();
const padsByRefNet = {};
for (const c of comps) {
  const ref = c.getState_Designator();
  const pins = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId());
  padsByRefNet[ref] = {};
  for (const p of pins) {
    const net = String(p.getState_Net?.() ?? p.net ?? "");
    padsByRefNet[ref][net] = {
      x: snap(p.getState_X?.() ?? p.x ?? 0),
      y: snap(p.getState_Y?.() ?? p.y ?? 0),
    };
  }
}

const TOP = 1;
const W_TRACE = 15; // 15 mil (~0.38mm)

const R1_VCC = padsByRefNet.R1["VCC"];
const R3_VCC = padsByRefNet.R3["VCC"];
const R1_MID = padsByRefNet.R1["$1N2"];
const R2_MID = padsByRefNet.R2["$1N2"];
const C1_MID = padsByRefNet.C1["$1N2"];
const R3_LED = padsByRefNet.R3["$1N3"];
const L1_LED = padsByRefNet.LED1["$1N3"];
const R2_GND = padsByRefNet.R2["GND"];
const C1_GND = padsByRefNet.C1["GND"];
const L1_GND = padsByRefNet.LED1["GND"];

// Explicit segments (zero crossings, clean top-layer layout)
const segments = [
  // VCC bus
  ["VCC", R1_VCC.x, R1_VCC.y, R1_VCC.x, 1650],
  ["VCC", R1_VCC.x, 1650,     R3_VCC.x, 1650],
  ["VCC", R3_VCC.x, 1650,     R3_VCC.x, R3_VCC.y],

  // $1N2 divider + RC filter node
  ["$1N2", R1_MID.x, R1_MID.y, R1_MID.x, 1400],
  ["$1N2", R1_MID.x, 1400,     R2_MID.x, R2_MID.y],
  ["$1N2", R1_MID.x, 1400,     C1_MID.x, 1400],
  ["$1N2", C1_MID.x, 1400,     C1_MID.x, C1_MID.y],

  // $1N3 LED branch
  ["$1N3", R3_LED.x, R3_LED.y, L1_LED.x, L1_LED.y],

  // GND bus (also reinforced by Top GND copper pour)
  ["GND", R2_GND.x, R2_GND.y, R2_GND.x, 1150],
  ["GND", R2_GND.x, 1150,     C1_GND.x, 1150],
  ["GND", C1_GND.x, 1150,     C1_GND.x, C1_GND.y],
  ["GND", C1_GND.x, 1150,     L1_GND.x, 1150],
  ["GND", L1_GND.x, 1150,     L1_GND.x, L1_GND.y],
];

const createdLines = [];
for (const [net, x1, y1, x2, y2] of segments) {
  const ln = await eda.pcb_PrimitiveLine.create(net, TOP, x1, y1, x2, y2, W_TRACE, false);
  createdLines.push({ net, id: ln?.getState_PrimitiveId?.() ?? null });
}

// Create Top-layer GND copper pour inset 20 mil inside the 1000..2000 x 1000..1800 outline
let pourId = null;
let pourStatus = "ok";
try {
  const pourPoly = eda.pcb_MathPolygon.createPolygon([
    1020, 1020, "L", 1980, 1020, 1980, 1780, 1020, 1780, 1020, 1020,
  ]);
  const pour = await eda.pcb_PrimitivePour.create(
    "GND",
    TOP,
    pourPoly,
    undefined, // default solid fill
    false,     // do not preserve islands
    "GND_TOP",
    0,
    10,
    false
  );
  pourId = pour?.getState_PrimitiveId?.() ?? null;
  await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
} catch (e) {
  pourStatus = "warn: " + String(e?.message || e);
}

try {
  await eda.pcb_Document.zoomToBoardOutline();
} catch {}
await eda.pcb_Document.save();

// Settle wait before DRC & exports
await sleep(6000);

const drcStrictPass = await eda.pcb_Drc.check(true, false, false);
const drcVerbose = await eda.pcb_Drc.check(true, false, true);
let ratlines = null;
try {
  ratlines = await eda.pcb_Document.getRatlines();
} catch (e) {
  ratlines = { warn: String(e?.message || e) };
}

const exportsOut = {};
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  exportsOut.pcbPng = img
    ? { name: "hello_circuit_pcb.png", type: img.type, size: img.size, b64: await toB64(img) }
    : null;
} catch (e) {
  exportsOut.pcbPng = { error: String(e?.message || e) };
}

try {
  const gerber = await eda.pcb_ManufactureData.getGerberFile("hello_circuit_gerber");
  exportsOut.gerber = gerber
    ? { name: gerber.name, size: gerber.size, b64: await toB64(gerber) }
    : null;
} catch (e) {
  exportsOut.gerber = { error: String(e?.message || e) };
}

try {
  const cpl = await eda.pcb_ManufactureData.getPickAndPlaceFile("hello_circuit_cpl");
  exportsOut.cpl = cpl
    ? { name: cpl.name, size: cpl.size, b64: await toB64(cpl) }
    : null;
} catch (e) {
  exportsOut.cpl = { error: String(e?.message || e) };
}

return {
  traceCount: createdLines.length,
  pourId,
  pourStatus,
  drcStrictPass,
  drcVerbose,
  ratlines,
  exports: exportsOut,
};
