import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const padData = JSON.parse(fs.readFileSync(`${base}/outputs/pcb_placed_pads.json`, "utf8"));
const P = (ref, num) => {
  const p = padData.byRef[ref]?.pads[String(num)];
  if (!p) throw new Error(`Missing pad ${ref}.${num}`);
  return p;
};

const TOP = 1;
const BOT = 2;
const tracks = []; // [net, layer, x1, y1, x2, y2, width]
const vias = [];   // [net, x, y, holeMil, diaMil]

const addPath = (net, layer, width, pts) => {
  for (let i = 0; i < pts.length - 1; i++) {
    const [x1, y1] = pts[i];
    const [x2, y2] = pts[i + 1];
    if (Math.abs(x1 - x2) < 0.01 && Math.abs(y1 - y2) < 0.01) continue;
    tracks.push([net, layer, x1, y1, x2, y2, width]);
  }
};
const addVia = (net, x, y, hole = 12, dia = 22) => {
  vias.push([net, x, y, hole, dia]);
};

// =========================================================================
// 1. USB-C CRITICAL NETS (Zero-crossing, >= 10 mil clearances everywhere!)
// =========================================================================
// VBUS: A4B9 -> F1.1 (Top 14mil); B4A9 -> via(1295,2325) well clear of both B1A12 and USB_DN(1325) -> Bot -> via(1290,2730) -> F1.1
{
  const a4 = P("J_USB", "A4B9");
  const b4 = P("J_USB", "B4A9");
  const f1 = P("F1", "1");
  addPath("VBUS", TOP, 14, [[a4.x, a4.y], [1310, a4.y], [1310, f1.y], [f1.x, f1.y]]);
  addPath("VBUS", TOP, 12, [[b4.x, b4.y], [1295, b4.y], [1295, 2325]]);
  addVia("VBUS", 1295, 2325);
  addVia("VBUS", 1290, f1.y);
  addPath("VBUS", BOT, 18, [[1295, 2325], [1290, 2340], [1290, f1.y]]);
  addPath("VBUS", TOP, 16, [[1290, f1.y], [1310, f1.y]]);
}

// USB_CC1 (J_USB.A5 -> R1.1), USB_CC2 (J_USB.B5 -> R2.1) — routed clean outside DP/DN fanout
{
  const a5 = P("J_USB", "A5"), r1 = P("R1", "1");
  addPath("USB_CC1", TOP, 7, [[a5.x, a5.y], [1295, a5.y], [1325, r1.y], [r1.x, r1.y]]);
  const b5 = P("J_USB", "B5"), r2 = P("R2", "1");
  addPath("USB_CC2", TOP, 7, [[b5.x, b5.y], [1385, b5.y], [1400, r2.y], [r2.x, r2.y]]);
}

// USB_DP (A6=2509.8, B6=2470.5 -> U2.5@1745) and USB_DN (B7=2529.5, A7=2490.2 -> U2.6@1795)
// Topology proof for ZERO crossings on Bottom:
//   - Outer lane: USB_DN drops at X=1325 -> Bot Y=2000 -> right to X=1795 (U2.6)
//   - Inner lane: USB_DP drops at X=1365 -> Bot Y=2030 -> right to X=1745 (U2.5)
{
  const a6 = P("J_USB", "A6"), b6 = P("J_USB", "B6"), u2dp = P("U2", "5");
  const a7 = P("J_USB", "A7"), b7 = P("J_USB", "B7"), u2dn = P("U2", "6");

  // USB_DN: B7 & A7 fan out to staggered vias on X=1325 (Y=2535 and Y=2475 -> 60mil vertical gap!)
  addPath("USB_DN", TOP, 6, [[b7.x, b7.y], [1300, b7.y], [1310, 2538], [1325, 2538]]);
  addVia("USB_DN", 1325, 2538);
  addPath("USB_DN", TOP, 6, [[a7.x, a7.y], [1305, a7.y], [1315, 2480], [1325, 2480]]);
  addVia("USB_DN", 1325, 2480);
  // Bot DN runs down X=1325 (outer lane) -> Y=1995 -> X=1795 -> via(1795,2060) -> U2.6
  addPath("USB_DN", BOT, 9, [[1325, 2538], [1325, 2480], [1325, 1995], [1795, 1995], [1795, 2060]]);
  addVia("USB_DN", 1795, 2060);
  addPath("USB_DN", TOP, 9, [[1795, 2060], [u2dn.x, u2dn.y]]);

  // USB_DP: A6 (2509.8) threads cleanly between DN vias (2538 & 2480 -> center=2509! 29mil gap each side!)
  // to X=1365 on Top; B6 (2470.5) goes under DN via(2480) at Y=2452 to X=1365 -> joins A6 on Top!
  addPath("USB_DP", TOP, 6, [[a6.x, a6.y], [1365, a6.y]]);
  addPath("USB_DP", TOP, 6, [[b6.x, b6.y], [1300, b6.y], [1318, 2452], [1365, 2452], [1365, a6.y]]);
  // Single via for DP at (1365, 2452) -> Bot inner lane X=1365 -> Y=2030 -> X=1745 -> via(1745,2060) -> U2.5
  addVia("USB_DP", 1365, 2452);
  addPath("USB_DP", BOT, 9, [[1365, 2452], [1365, 2030], [1745, 2030], [1745, 2060]]);
  addVia("USB_DP", 1745, 2060);
  addPath("USB_DP", TOP, 9, [[1745, 2060], [u2dp.x, u2dp.y]]);
}

// =========================================================================
// 2. 16MHz CRYSTAL (XTAL1, XTAL2) + U1 LOCAL +5V (Zero crossing with C4!)
// =========================================================================
{
  const u1x1 = P("U1", "7"), y1x1 = P("Y1", "1"), c10 = P("C10", "1");
  const u1x2 = P("U1", "8"), y1x2 = P("Y1", "3"), c11 = P("C11", "1");
  const u1v6 = P("U1", "6"), u1v4 = P("U1", "4"), c4 = P("C4", "1");

  // U1.6 & U1.4 (+5V) -> C4.1 straight on Top (clear of U1.5 GND at X=2992, Y=2075.7)
  addPath("+5V", TOP, 10, [[u1v6.x, u1v6.y], [3085, u1v6.y], [3100, c4.y], [c4.x, c4.y]]);
  addPath("+5V", TOP, 10, [[u1v4.x, u1v4.y], [3085, u1v4.y], [3085, u1v6.y]]);

  // XTAL2 (Top only, 0 vias!): U1.8(2992,2170.2) -> over C4 at Y=2235 -> down X=3233.3 -> Y1.3 -> C11.1
  addPath("XTAL2", TOP, 9, [[u1x2.x, u1x2.y], [3065, u1x2.y], [3065, 2235], [y1x2.x, 2235], [y1x2.x, y1x2.y]]);
  addPath("XTAL2", TOP, 9, [[y1x2.x, y1x2.y], [3275, y1x2.y], [3275, c11.y], [c11.x, c11.y]]);

  // XTAL1: U1.7(2992,2138.7) -> via(3045,2138.7) -> Bot cleanly under +5V & Y1 -> Y1.1 & C10.1
  addPath("XTAL1", TOP, 9, [[u1x1.x, u1x1.y], [3045, u1x1.y]]);
  addVia("XTAL1", 3045, u1x1.y);
  addVia("XTAL1", y1x1.x, 1805);
  addVia("XTAL1", c10.x, 2015);
  addPath("XTAL1", BOT, 9, [[3045, u1x1.y], [3045, 1805], [y1x1.x, 1805], [3255, 1805], [3255, 2015], [c10.x, 2015]]);
  addPath("XTAL1", TOP, 9, [[y1x1.x, 1805], [y1x1.x, y1x1.y]]);
  addPath("XTAL1", TOP, 9, [[c10.x, 2015], [c10.x, c10.y]]);
}

// =========================================================================
// 3. 3.3V LDO POWER RAIL (+3V3, 20mil) + LOCAL +5V / AREF / RESET PAIRS
// =========================================================================
{
  const u3o4 = P("U3", "4"), u3o2 = P("U3", "2"), c9 = P("C9", "1"), jp33 = P("J_POWER", "4");
  addPath("+3V3", TOP, 20, [[u3o4.x, u3o4.y], [u3o2.x, u3o2.y], [1630, u3o2.y], [1630, c9.y], [c9.x, c9.y]]);
  addPath("+3V3", TOP, 20, [[c9.x, c9.y], [c9.x, 1340], [jp33.x, 1340], [jp33.x, jp33.y]]);

  const u3in = P("U3", "3"), c8 = P("C8", "1");
  addPath("+5V", TOP, 20, [[u3in.x, u3in.y], [c8.x, u3in.y], [c8.x, c8.y]]);

  // F1.2 -> C7.1 entering cleanly from below C7.2(GND at 2757.6)!
  const f1o = P("F1", "2"), c7 = P("C7", "1");
  addPath("+5V", TOP, 16, [[f1o.x, f1o.y], [1565, c7.y], [c7.x, c7.y]]);

  const u2v = P("U2", "16"), c2 = P("C2", "1");
  addPath("+5V", TOP, 12, [[u2v.x, u2v.y], [u2v.x, 2410], [c2.x, 2410], [c2.x, c2.y]]);

  const u1av = P("U1", "18"), c5 = P("C5", "1");
  addPath("+5V", TOP, 12, [[u1av.x, u1av.y], [2575, u1av.y], [2575, c5.y], [c5.x, c5.y]]);

  const r9p = P("R9", "1"), d1k = P("D1", "1");
  addPath("+5V", TOP, 12, [[r9p.x, r9p.y], [d1k.x, r9p.y], [d1k.x, d1k.y]]);

  const u1aref = P("U1", "20"), c6 = P("C6", "1");
  addPath("AREF", TOP, 10, [[u1aref.x, u1aref.y], [c6.x, u1aref.y], [c6.x, c6.y]]);

  const r9r = P("R9", "2"), d1a = P("D1", "2");
  addPath("RESET", TOP, 10, [[r9r.x, r9r.y], [d1a.x, r9r.y], [d1a.x, d1a.y]]);
}

// =========================================================================
// 4. CH340C LOCAL CRITICAL NETS (V3, DTR) + LED SERIES RESISTORS (4x)
// =========================================================================
{
  // CH340_V3: U2.4(1695,2126.9) -> straight down X=1695 (55mil clear right of C1.2 GND!) -> left into C1.1(1640,1972.4)
  const u2v3 = P("U2", "4"), c1 = P("C1", "1");
  addPath("CH340_V3", TOP, 10, [[u2v3.x, u2v3.y], [u2v3.x, c1.y], [c1.x, c1.y]]);

  const u2dtr = P("U2", "13"), c3 = P("C3", "1");
  addPath("CH340_DTR", TOP, 10, [[u2dtr.x, u2dtr.y], [u2dtr.x, c3.y], [c3.x, c3.y]]);

  const ledPairs = [
    ["LED_PWR_A", "R5", "2", "LED_PWR", "2", +38],
    ["LED_L_A",   "R8", "2", "LED_L",   "1", +38],
    ["LED_TX_A",  "R6", "2", "LED_TX",  "1", +38],
    ["LED_RX_A",  "R7", "2", "LED_RX",  "2", +38]
  ];
  for (const [net, rRef, rPin, lRef, lPin, dy] of ledPairs) {
    const a = P(rRef, rPin), b = P(lRef, lPin);
    addPath(net, TOP, 10, [[a.x, a.y], [a.x, a.y + dy], [b.x, a.y + dy], [b.x, b.y]]);
  }
}

// =========================================================================
// 5. TOP DIGITAL PARALLEL BUS (D12..D5: 8 non-crossing Top-layer tracks)
// =========================================================================
{
  const topBus = [
    ["D12", "U1", "16", "J_DIG_HI", "6",  2360],
    ["D11", "U1", "15", "J_DIG_HI", "7",  2395],
    ["D10", "U1", "14", "J_DIG_HI", "8",  2430],
    ["D9",  "U1", "13", "J_DIG_HI", "9",  2465],
    ["D8",  "U1", "12", "J_DIG_HI", "10", 2500],
    ["D7",  "U1", "11", "J_DIG_LO", "1",  2535],
    ["D6",  "U1", "10", "J_DIG_LO", "2",  2500],
    ["D5",  "U1", "9",  "J_DIG_LO", "3",  2465]
  ];
  for (const [net, uRef, uPin, hRef, hPin, kneeY] of topBus) {
    const a = P(uRef, uPin), b = P(hRef, hPin);
    const dx = b.x - a.x;
    const diag = Math.min(Math.abs(dx), 2900 - kneeY);
    const sx = dx >= 0 ? a.x + diag : a.x - diag;
    addPath(net, TOP, 10, [
      [a.x, a.y], [a.x, kneeY], [sx, kneeY + diag], [b.x, kneeY + diag], [b.x, b.y]
    ]);
  }
}

// =========================================================================
// 6. BOTTOM ANALOG PARALLEL BUS (A0..A5: 6 non-crossing Top-layer tracks)
// =========================================================================
{
  const a0u = P("U1", "23"), a0h = P("J_ANALOG", "1");
  const a1u = P("U1", "24"), a1h = P("J_ANALOG", "2");
  addPath("A0", TOP, 10, [[a0u.x, a0u.y], [2585, a0u.y], [2585, 1620], [a0h.x, 1205], [a0h.x, a0h.y]]);
  addPath("A1", TOP, 10, [[a1u.x, a1u.y], [2615, a1u.y], [2615, 1640], [a1h.x, 1155], [a1h.x, a1h.y]]);

  const botBus = [
    ["A2", "U1", "25", "J_ANALOG", "3", 1740],
    ["A3", "U1", "26", "J_ANALOG", "4", 1775],
    ["A4_SDA", "U1", "27", "J_ANALOG", "5", 1810],
    ["A5_SCL", "U1", "28", "J_ANALOG", "6", 1845]
  ];
  for (const [net, uRef, uPin, hRef, hPin, kneeY] of botBus) {
    const a = P(uRef, uPin), b = P(hRef, hPin);
    const dx = b.x - a.x;
    const diag = Math.min(Math.abs(dx), kneeY - 1220);
    const sx = a.x + diag;
    addPath(net, TOP, 10, [
      [a.x, a.y], [a.x, kneeY], [sx, kneeY - diag], [b.x, kneeY - diag], [b.x, b.y]
    ]);
  }
}

console.log(`Prepared clean U4-a routing: ${tracks.length} track segments, ${vias.length} vias.`);

const js = `
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function toB64(blob) {
  if (!blob) return null;
  return new Promise((res, rej) => {
    const fr = new FileReader();
    fr.onload = () => res(String(fr.result).split(",")[1] || "");
    fr.onerror = (e) => rej(e);
    fr.readAsDataURL(blob);
  });
}

// Clean old copper tracks & vias (leave BoardOutline on layer 11 untouched)
for (const lyr of [1, 2]) {
  try {
    const oldLines = await eda.pcb_PrimitiveLine.getAll(undefined, lyr) || [];
    if (oldLines.length) await eda.pcb_PrimitiveLine.delete(oldLines.map(x => x.getState_PrimitiveId()));
  } catch {}
}
try {
  const oldVias = await eda.pcb_PrimitiveVia.getAll() || [];
  if (oldVias.length) await eda.pcb_PrimitiveVia.delete(oldVias.map(x => x.getState_PrimitiveId()));
} catch {}

const tracks = ${JSON.stringify(tracks)};
const vias = ${JSON.stringify(vias)};
let tOk = 0, vOk = 0;
const errs = [];

// IMPORTANT: Create Vias BEFORE Tracks so EasyEDA Pro net connectivity graph binds Top<->Bottom cleanly!
for (const [net, x, y, hole, dia] of vias) {
  try {
    await eda.pcb_PrimitiveVia.create(net, x, y, hole, dia, 0, null, false);
    vOk++;
  } catch (e) { errs.push("via " + net + ": " + (e?.message || e)); }
}
for (const [net, lyr, x1, y1, x2, y2, w] of tracks) {
  try {
    await eda.pcb_PrimitiveLine.create(net, lyr, x1, y1, x2, y2, w, false);
    tOk++;
  } catch (e) { errs.push("track " + net + ": " + (e?.message || e)); }
}

try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
await eda.pcb_Document.save();
await sleep(3000);

// Capture CLEAN PNG before calling full DRC!
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

const ratlines = await eda.pcb_Document.getRatlines();
const drc = await eda.pcb_Drc.check(true, false, true);

const nonConn = [];
let remainingNets = [];
for (const cat of (drc || [])) {
  if (cat.name === "Connection Error") {
    for (const sub of (cat.list || [])) {
      remainingNets.push({ net: sub.name, unroutedItems: (sub.list || []).length });
    }
  } else {
    nonConn.push(cat);
  }
}

return {
  tOk, vOk, errCount: errs.length, errs: errs.slice(0, 10),
  ratlinesTotal: ratlines?.total ?? null,
  nonConnectionDrc: nonConn,
  remainingNets,
  png
};
`;

const tmpFile = `${base}/outputs/.tmp_step4a_v2.js`;
fs.writeFileSync(tmpFile, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmpFile, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmpFile);
const d = raw.data?.result ?? raw.data ?? raw;
console.log(`Done in ${((Date.now()-t0)/1000).toFixed(1)}s: tracks=${d.tOk}/${tracks.length}, vias=${d.vOk}/${vias.length}, errs=${d.errCount}`);
console.log(`Ratlines remaining: ${d.ratlinesTotal} (down from 112!)`);
console.log(`Non-Connection DRC categories: ${(d.nonConnectionDrc || []).length}`);
for (const cat of (d.nonConnectionDrc || [])) {
  for (const sub of (cat.list || [])) {
    console.log("  DRC:", cat.name, "/", sub.name, "count:", sub.list.length);
    for (const it of sub.list) {
      console.log("   ", it.layer, it.obj1?.typeName + it.obj1?.suffix, "<->", it.obj2?.typeName + it.obj2?.suffix, it.explanation?.param?.minDistance, "vs", it.explanation?.param?.shouldBe);
    }
  }
}
console.log("Remaining unrouted nets:", (d.remainingNets || []).map(x => `${x.net}(${x.unroutedItems})`).join(", "));
if (d.png?.b64) {
  fs.writeFileSync(`${base}/outputs/unomodoki_pcb_u4a.png`, Buffer.from(d.png.b64, "base64"));
  console.log("Wrote outputs/unomodoki_pcb_u4a.png:", d.png.size, "bytes");
}
fs.writeFileSync(`${base}/outputs/step4a_routing_report.json`, JSON.stringify({
  tracksCreated: d.tOk,
  viasCreated: d.vOk,
  ratlinesBefore: 112,
  ratlinesAfter: d.ratlinesTotal,
  nonConnectionDrc: d.nonConnectionDrc,
  remainingNets: d.remainingNets
}, null, 2));
