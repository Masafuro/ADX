import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const geom = JSON.parse(fs.readFileSync(`${base}/spec_geometry.json`, "utf8"));
const probe = JSON.parse(fs.readFileSync(`${base}/outputs/pcb_initial_probe.json`, "utf8"));
const probeByRef = Object.fromEntries(probe.map((c) => [c.ref, c]));

const OX = geom.coordinateSystem.easyedaPcbOffsetMil.x; // 1000
const OY = geom.coordinateSystem.easyedaPcbOffsetMil.y; // 1000

// Helper: compute exact component center so Pin 1 lands on target (gx, gy) for rot=0 headers
function headerCenterFromPin1(ref, targetLocalX, targetLocalY) {
  const c = probeByRef[ref];
  const p1 = c.pins.find((p) => p.num === "1");
  return {
    x: Math.round((OX + targetLocalX - p1.dx) * 100) / 100,
    y: Math.round((OY + targetLocalY - p1.dy) * 100) / 100
  };
}

// Compute exact initial centers for the 4 linear UNO R3 shield headers (rot=0)
const jDigHiPos = headerCenterFromPin1("J_DIG_HI", 740, 2000);
const jDigLoPos = headerCenterFromPin1("J_DIG_LO", 1800, 2000);
const jPowerPos = headerCenterFromPin1("J_POWER",  1100, 100);
const jAnalogPos = headerCenterFromPin1("J_ANALOG", 2000, 100);

// Full 37-component PCB placement plan (all on Top layer = 1; absolute PCB mil coordinates = OX+localX, OY+localY)
const L = (lx, ly, rot = 0) => ({ x: OX + lx, y: OY + ly, rot });

const targets = {
  // Exact mechanical UNO R3 headers (0.00 mil error verified via closed-loop pin assertion)
  J_DIG_HI: { ...jDigHiPos, rot: 0 },
  J_DIG_LO: { ...jDigLoPos, rot: 0 },
  J_POWER:  { ...jPowerPos, rot: 0 },
  J_ANALOG: { ...jAnalogPos, rot: 0 },
  J_ICSP:   L(2555, 1100, 270), // auto-verified & fine-tuned by closed-loop check below

  // USB-C left-edge connector (centered on Y=1500 mil = original USB-B center, Board Outline clearance >= 27 mil > 11.8 mil DRC rule)
  J_USB:    L(160, 1500, 270),  // auto-verified so signal pads face +X (inward) and mouth faces -X (left edge)

  // Reset tactile button (top-left corner, clear of MH4 at 600,2000)
  SW1:      L(260, 1880, 0),

  // USB-C CC resistors & 500mA PTC + 5V bulk cap
  R1:       L(440, 1580, 0),
  R2:       L(440, 1480, 0),
  F1:       L(470, 1730, 0),
  C7:       L(650, 1730, 90),

  // CH340C USB-UART & close passives (right of USB-C for short D+/D-)
  U2:       L(720, 1240, 0),
  C1:       L(640, 1000, 90),   // V3 100nF
  C2:       L(560, 1460, 0),    // CH340C VCC 100nF (clear above Pin 16!)
  C3:       L(840, 1460, 0),    // DTR -> RESET 100nF
  R3:       L(980, 1210, 0),    // 1k TX->RX
  R4:       L(980, 1110, 0),    // 1k RX->TX

  // 3.3V LDO & input/output bulk caps (generous spacing clear of pads & logo)
  U3:       L(420, 560, 0),
  C8:       L(700, 680, 90),    // +5V LDO in
  C9:       L(700, 440, 90),    // +3V3 LDO out

  // Main MCU ATmega328P-AU (rotated so D12..D5 face Top digital header cleanly!)
  U1:       L(1820, 1060, 90),  // auto-verified so D12..D5 sit on +Y side, D12 left of D5

  // 16MHz Crystal + load caps right next to U1 XTAL1/XTAL2
  Y1:       L(2190, 900, 0),
  C10:      L(2340, 960, 0),
  C11:      L(2340, 840, 0),

  // MCU bypass & AREF caps close to U1 power pins
  C4:       L(2140, 1150, 90),
  C5:       L(1510, 1210, 90),
  C6:       L(1510, 1100, 90),

  // Reset pull-up & clamp diode
  R9:       L(1450, 820, 0),
  D1:       L(1450, 720, 0),

  // 4 Status LEDs + series resistors lined up neatly in top-center visible area
  R5:       L(1060, 1690, 0),
  LED_PWR:  L(1200, 1690, 0),
  R8:       L(1060, 1590, 0),
  LED_L:    L(1200, 1590, 0),
  R6:       L(1060, 1490, 0),
  LED_TX:   L(1200, 1490, 0),
  R7:       L(1060, 1390, 0),
  LED_RX:   L(1200, 1390, 0)
};

// Build outline flat polygon array [x0, y0, "L", x1, y1, ..., x0, y0]
const outlinePts = geom.outlinePolygonCCWMil;
const outlineArr = [OX + outlinePts[0].x, OY + outlinePts[0].y, "L"];
for (let i = 1; i < outlinePts.length; i++) {
  outlineArr.push(OX + outlinePts[i].x, OY + outlinePts[i].y);
}
outlineArr.push(OX + outlinePts[0].x, OY + outlinePts[0].y);

// Mounting holes (NPTH Ø125 mil = 3.175 mm)
const holes = geom.mountingHolesMil.map((h) => ({
  id: h.id,
  x: OX + h.x,
  y: OY + h.y,
  dia: h.diameterMil
}));

// Build Top Silkscreen labels (every shield pin + ICSP + LEDs + RST + UNOMODOKI R3 logo!)
const silks = [];
// Top digital headers (vertical 90-deg text directly below Y=2000 row)
for (const hdrName of ["J_DIG_HI", "J_DIG_LO"]) {
  for (const p of geom.headersMil[hdrName].pins) {
    silks.push({ x: OX + p.x, y: OY + 1855, text: p.label, size: 36, lw: 6, rot: 90, align: 5 });
  }
}
// Bottom power & analog headers (vertical 90-deg text cleanly above Y=100 row & header designators)
for (const hdrName of ["J_POWER", "J_ANALOG"]) {
  for (const p of geom.headersMil[hdrName].pins) {
    silks.push({ x: OX + p.x, y: OY + 275, text: p.label, size: 36, lw: 6, rot: 90, align: 5 });
  }
}
// ICSP + LEDs + SW1 + Board Title
silks.push({ x: OX + 2390, y: OY + 1100, text: "ICSP", size: 42, lw: 6, rot: 90, align: 5 });
silks.push({ x: OX + 2445, y: OY + 1200, text: "1",    size: 38, lw: 6, rot: 0,  align: 5 });
silks.push({ x: OX + 1315, y: OY + 1690, text: "PWR",  size: 34, lw: 6, rot: 0,  align: 2 });
silks.push({ x: OX + 1315, y: OY + 1590, text: "L",    size: 34, lw: 6, rot: 0,  align: 2 });
silks.push({ x: OX + 1315, y: OY + 1490, text: "TX",   size: 34, lw: 6, rot: 0,  align: 2 });
silks.push({ x: OX + 1315, y: OY + 1390, text: "RX",   size: 34, lw: 6, rot: 0,  align: 2 });
silks.push({ x: OX + 260,  y: OY + 1735, text: "RST",  size: 36, lw: 6, rot: 0,  align: 5 });
silks.push({ x: OX + 1320, y: OY + 560,  text: "UNOMODOKI R3", size: 88, lw: 12, rot: 0, align: 5 });
silks.push({ x: OX + 1320, y: OY + 455,  text: "ADX SANDBOX / USB-C", size: 40, lw: 6, rot: 0, align: 5 });

// Expected pad coordinates for automated zero-error assertion
const expectedPins = [];
for (const [hdrName, hdr] of Object.entries(geom.headersMil)) {
  for (const p of hdr.pins) {
    expectedPins.push({ ref: hdrName, pin: String(p.pin), label: p.label, ex: OX + p.x, ey: OY + p.y });
  }
}

function runStep(name, js, timeout = 90000) {
  const f = `${base}/outputs/.tmp_${name}.js`;
  fs.writeFileSync(f, js);
  const t0 = Date.now();
  const out = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", f, "--timeout", String(timeout)], { encoding: "utf8" }));
  fs.unlinkSync(f);
  console.log(`[OK] ${name} (${((Date.now() - t0) / 1000).toFixed(1)}s)`);
  return out.data?.result ?? out.data ?? out;
}

// Step 1: Draw 9-vertex UNO R3 BoardOutline + 4 NPTH M3 Mounting Holes + Place all 37 components
const res1 = runStep("pcb_mech_place", `
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const snap = (v) => Math.round(v * 100) / 100;
const BOARD_OUTLINE = 11;
const MULTI = 12;

// Remove existing board outline primitives
try {
  const polys = await eda.pcb_PrimitivePolyline.getAll(undefined, BOARD_OUTLINE) || [];
  if (polys.length) await eda.pcb_PrimitivePolyline.delete(polys.map(p => p.getState_PrimitiveId()));
} catch {}
try {
  const lines = await eda.pcb_PrimitiveLine.getAll(undefined, BOARD_OUTLINE) || [];
  if (lines.length) await eda.pcb_PrimitiveLine.delete(lines.map(l => l.getState_PrimitiveId()));
} catch {}
// Remove existing standalone mounting hole pads (MH1..MH4)
try {
  const pads = await eda.pcb_PrimitivePad.getAll() || [];
  const mhIds = pads.filter(p => /^MH[1-4]$/.test(String(p.getState_PadNumber?.() || ""))).map(p => p.getState_PrimitiveId());
  if (mhIds.length) await eda.pcb_PrimitivePad.delete(mhIds);
} catch {}

// Create 9-vertex UNO R3 closed polygon BoardOutline
const outlinePoly = eda.pcb_MathPolygon.createPolygon(${JSON.stringify(outlineArr)});
const outlinePrim = await eda.pcb_PrimitivePolyline.create("", BOARD_OUTLINE, outlinePoly, 10, false);

// Create 4 NPTH M3 mounting holes (Ø125 mil = 3.175mm, metallization=false)
const holes = ${JSON.stringify(holes)};
const createdHoles = [];
for (const h of holes) {
  const pad = await eda.pcb_PrimitivePad.create(
    MULTI, h.id, h.x, h.y, 0,
    ["ELLIPSE", h.dia, h.dia], "", ["ROUND", h.dia],
    0, 0, 0, false, 0
  );
  createdHoles.push({ id: h.id, pid: pad?.getState_PrimitiveId?.() ?? null, x: h.x, y: h.y });
}

// Place all 37 components on Top layer (1)
const targets = ${JSON.stringify(targets)};
const comps = await eda.pcb_PrimitiveComponent.getAll() || [];
const byRef = {};
for (const c of comps) {
  const ref = c.getState_Designator();
  const id = c.getState_PrimitiveId();
  byRef[ref] = id;
  const t = targets[ref];
  if (t) {
    await eda.pcb_PrimitiveComponent.modify(id, { layer: 1, x: t.x, y: t.y, rotation: t.rot });
  }
}
await sleep(1500);

// Closed-loop orientation & fine-position auto-calibration for J_USB, J_ICSP, U1, and all 4 shield headers!
const calLog = [];

async function getPins(id) {
  const arr = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  return (arr || []).map(p => ({
    num: String(p.getState_PadNumber?.() ?? p.padNumber ?? p.pinNumber ?? ""),
    net: String(p.getState_Net?.() ?? p.net ?? ""),
    x: snap(p.getState_X?.() ?? p.x ?? 0),
    y: snap(p.getState_Y?.() ?? p.y ?? 0)
  }));
}

// 1) J_USB: ensure signal pad A6 sits to the RIGHT (+X, inward) of front shell pad 3
if (byRef.J_USB) {
  let pins = await getPins(byRef.J_USB);
  const pA6 = pins.find(p => p.num === "A6");
  const pS3 = pins.find(p => p.num === "3");
  if (pA6 && pS3 && pA6.x < pS3.x) {
    await eda.pcb_PrimitiveComponent.modify(byRef.J_USB, { rotation: 90 });
    await sleep(400);
    pins = await getPins(byRef.J_USB);
    calLog.push("J_USB flipped to rot=90 so mouth faces left edge (-X)");
  }
  const pA6f = pins.find(p => p.num === "A6");
  const pS3f = pins.find(p => p.num === "3");
  calLog.push("J_USB verified: A6@(" + pA6f?.x + "," + pA6f?.y + ") frontShell3@(" + pS3f?.x + "," + pS3f?.y + ")");
}

// 2) J_ICSP: ensure Pin 1 (MISO) is top-left (x < Pin2.x and y > Pin5.y) AND exact (3505, 2200)
if (byRef.J_ICSP) {
  for (const tryRot of [270, 90, 0, 180]) {
    let pins = await getPins(byRef.J_ICSP);
    const p1 = pins.find(p => p.num === "1");
    const p2 = pins.find(p => p.num === "2");
    const p5 = pins.find(p => p.num === "5");
    if (p1 && p2 && p5 && p1.x < p2.x && p1.y > p5.y) {
      const comp = await eda.pcb_PrimitiveComponent.get(byRef.J_ICSP);
      const cx = comp.getState_X(), cy = comp.getState_Y();
      const dx = snap(3505 - p1.x), dy = snap(2200 - p1.y);
      if (Math.abs(dx) > 0.05 || Math.abs(dy) > 0.05) {
        await eda.pcb_PrimitiveComponent.modify(byRef.J_ICSP, { x: snap(cx + dx), y: snap(cy + dy) });
        await sleep(300);
      }
      calLog.push("J_ICSP locked: rot=" + comp.getState_Rotation() + " shift=(" + dx + "," + dy + ")");
      break;
    }
    await eda.pcb_PrimitiveComponent.modify(byRef.J_ICSP, { rotation: tryRot });
    await sleep(300);
  }
}

// 3) U1: ensure D12 (pin 16) and D5 (pin 9) sit on TOP (+Y) side facing top digital header
if (byRef.U1) {
  let pins = await getPins(byRef.U1);
  const p16 = pins.find(p => p.num === "16");
  const p9  = pins.find(p => p.num === "9");
  const u1c = await eda.pcb_PrimitiveComponent.get(byRef.U1);
  if (p16 && p9 && p16.y < u1c.getState_Y()) {
    await eda.pcb_PrimitiveComponent.modify(byRef.U1, { rotation: 270 });
    await sleep(400);
    pins = await getPins(byRef.U1);
    calLog.push("U1 flipped to rot=270 so D12..D5 face top digital header");
  }
  const p16f = pins.find(p => p.num === "16");
  const p9f  = pins.find(p => p.num === "9");
  calLog.push("U1 verified: D12@(" + p16f?.x + "," + p16f?.y + ") D5@(" + p9f?.x + "," + p9f?.y + ")");
}

// 4) Fine-tune the 4 linear shield headers so Pin 1 hits exact target to 0.01 mil
const headerPin1Targets = {
  J_DIG_HI: { x: 1740, y: 3000 },
  J_DIG_LO: { x: 2800, y: 3000 },
  J_POWER:  { x: 2100, y: 1100 },
  J_ANALOG: { x: 3000, y: 1100 }
};
for (const [ref, tgt] of Object.entries(headerPin1Targets)) {
  if (!byRef[ref]) continue;
  const pins = await getPins(byRef[ref]);
  const p1 = pins.find(p => p.num === "1");
  if (!p1) continue;
  const dx = snap(tgt.x - p1.x), dy = snap(tgt.y - p1.y);
  if (Math.abs(dx) > 0.05 || Math.abs(dy) > 0.05) {
    const comp = await eda.pcb_PrimitiveComponent.get(byRef[ref]);
    await eda.pcb_PrimitiveComponent.modify(byRef[ref], { x: snap(comp.getState_X() + dx), y: snap(comp.getState_Y() + dy) });
    calLog.push(ref + " fine-tuned by (" + dx + "," + dy + ")");
  }
}

await eda.pcb_Document.save();
return { outlineId: outlinePrim?.getState_PrimitiveId?.() ?? null, createdHoles, placedCount: Object.keys(byRef).length, calLog };
`, 90000);
console.log("Step 1 (mech & place):", JSON.stringify(res1, null, 2));

// Step 2: Generate all Top Silkscreen pin labels & board branding + assert ALL 38 pin coordinates + export PNG
const res2 = runStep("pcb_silk_assert_png", `
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

const TOP_SILK = 3;
// Clear any previously generated standalone silk strings on TOP_SILKSCREEN
try {
  const existingStrs = await eda.pcb_PrimitiveString.getAll(TOP_SILK) || [];
  if (existingStrs.length) await eda.pcb_PrimitiveString.delete(existingStrs.map(s => s.getState_PrimitiveId()));
} catch {}

const silks = ${JSON.stringify(silks)};
let silkOk = 0;
const silkErrs = [];
for (const s of silks) {
  try {
    await eda.pcb_PrimitiveString.create(TOP_SILK, s.x, s.y, s.text, "default", s.size, s.lw, s.align, s.rot, false, 0, false, false);
    silkOk++;
  } catch (e) {
    silkErrs.push(s.text + ": " + (e?.message || e));
  }
}

try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
await eda.pcb_Document.save();
await sleep(3000);

// Assert ALL 38 UNO R3 mechanical header & ICSP pin coordinates against spec_geometry.json!
const expected = ${JSON.stringify(expectedPins)};
const comps = await eda.pcb_PrimitiveComponent.getAll() || [];
const padLookup = {};
for (const c of comps) {
  const ref = c.getState_Designator();
  const pins = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId());
  padLookup[ref] = {};
  for (const p of (pins || [])) {
    const num = String(p.getState_PadNumber?.() ?? p.padNumber ?? p.pinNumber ?? "");
    padLookup[ref][num] = {
      x: snap(p.getState_X?.() ?? p.x ?? 0),
      y: snap(p.getState_Y?.() ?? p.y ?? 0),
      net: String(p.getState_Net?.() ?? p.net ?? "")
    };
  }
}

let maxErrMil = 0;
const mismatches = [];
for (const e of expected) {
  const actual = padLookup[e.ref]?.[e.pin];
  if (!actual) {
    mismatches.push({ ...e, error: "missing pad" });
    continue;
  }
  const err = Math.hypot(actual.x - e.ex, actual.y - e.ey);
  if (err > maxErrMil) maxErrMil = err;
  if (err > 0.5) mismatches.push({ ...e, actual, err: snap(err) });
}

// Check Arduino Gap directly from measured PCB pads
const d8 = padLookup.J_DIG_HI?.["10"];
const d7 = padLookup.J_DIG_LO?.["1"];
const gapMil = (d7 && d8) ? snap(d7.x - d8.x) : null;

// Export PCB placement preview PNG
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { name: "unomodoki_pcb_placed.png", size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

return {
  silkOk,
  silkErrCount: silkErrs.length,
  silkErrs: silkErrs.slice(0, 5),
  checkedPins: expected.length,
  maxErrMil: snap(maxErrMil),
  maxErrMm: snap(maxErrMil * 0.0254),
  mismatches,
  measuredArduinoGapMil: gapMil,
  png
};
`, 90000);

console.log("Step 2 (silk & assert):", {
  silkOk: res2.silkOk,
  silkErrCount: res2.silkErrCount,
  checkedPins: res2.checkedPins,
  maxErrMil: res2.maxErrMil,
  maxErrMm: res2.maxErrMm,
  mismatches: res2.mismatches,
  measuredArduinoGapMil: res2.measuredArduinoGapMil,
  pngSize: res2.png?.size || res2.png?.error
});
if (res2.png?.b64) {
  fs.writeFileSync(`${base}/outputs/unomodoki_pcb_placed.png`, Buffer.from(res2.png.b64, "base64"));
  console.log("Wrote outputs/unomodoki_pcb_placed.png");
}
fs.writeFileSync(`${base}/outputs/step3_geometry_assert.json`, JSON.stringify({
  checkedPins: res2.checkedPins,
  maxErrMil: res2.maxErrMil,
  maxErrMm: res2.maxErrMm,
  measuredArduinoGapMil: res2.measuredArduinoGapMil,
  mismatches: res2.mismatches,
  calLog: res1.calLog
}, null, 2));
