import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const placed = JSON.parse(fs.readFileSync(`${base}/outputs/sch_placed_pins.json`, "utf8"));
const byRef = Object.fromEntries(placed.map((c) => [c.ref, c]));

const pinByNum = (ref, num) => {
  const c = byRef[ref];
  if (!c) throw new Error(`Missing component ${ref}`);
  const p = c.pins.find((x) => x.number === String(num));
  if (!p) throw new Error(`Missing pin ${num} on ${ref}`);
  return p;
};
const pinByName = (ref, name) => {
  const c = byRef[ref];
  const p = c.pins.find((x) => x.name === name);
  if (!p) throw new Error(`Missing pin name ${name} on ${ref}`);
  return p;
};
const topPin = (ref) => {
  const ps = byRef[ref].pins;
  return ps[0].y >= ps[1].y ? ps[0] : ps[1];
};
const botPin = (ref) => {
  const ps = byRef[ref].pins;
  return ps[0].y < ps[1].y ? ps[0] : ps[1];
};
const leftPin = (ref) => {
  const ps = byRef[ref].pins;
  return ps[0].x <= ps[1].x ? ps[0] : ps[1];
};
const rightPin = (ref) => {
  const ps = byRef[ref].pins;
  return ps[0].x > ps[1].x ? ps[0] : ps[1];
};

// Sanity-assert polarity orientations before wiring so zero short-circuits can happen
for (const ref of ["LED_PWR", "LED_L", "LED_TX", "LED_RX"]) {
  const a = pinByName(ref, "A");
  const k = pinByName(ref, "K");
  if (!(a.y > k.y)) {
    throw new Error(`Polarity assertion failed on ${ref}: A(${a.x},${a.y}) must be above K(${k.x},${k.y})`);
  }
}
{
  const a = pinByName("D1", "A");
  const k = pinByName("D1", "K");
  if (!(k.y > a.y)) {
    throw new Error(`Polarity assertion failed on D1: K(${k.x},${k.y}) must be above A(${a.x},${a.y})`);
  }
}

// Declarative wire + net-flag + heading plan
// Rule to guarantee ZERO ONENETNAME warnings:
//  - Any connected wire group anchored to a Power/Ground net flag leaves `net` undefined.
//  - Any multi-segment signal bus assigns `net` on ONE segment only; connected segments leave `net` undefined.
const wires = []; // { line: [[x1,y1,...]], net?: string }
const flags = []; // { type: "Power"|"Ground", net: string, x, y, rot }
const texts = []; // { x, y, content, size }

const addStub = (pt, dx, dy, net) => {
  wires.push({ line: [[pt.x, pt.y, pt.x + dx, pt.y + dy]], net });
};
const addPowerFlag = (pt, dx, dy, net) => {
  const fx = pt.x + dx;
  const fy = pt.y + dy;
  if (dx !== 0 || dy !== 0) {
    wires.push({ line: [[pt.x, pt.y, fx, fy]] }); // net inherited cleanly from Power flag
  }
  flags.push({ type: "Power", net, x: fx, y: fy, rot: 0 });
};
const addGndFlag = (pt, dx, dy) => {
  const fx = pt.x + dx;
  const fy = pt.y + dy;
  if (dx !== 0 || dy !== 0) {
    wires.push({ line: [[pt.x, pt.y, fx, fy]] }); // net inherited cleanly from Ground flag
  }
  flags.push({ type: "Ground", net: "GND", x: fx, y: fy, rot: 0 });
};

// --- Block section headings ---
texts.push({ x: 60,  y: 775, content: "1. USB-C & 5V POWER (USB-C ONLY)", size: 12 });
texts.push({ x: 60,  y: 505, content: "2. 3.3V LDO REGULATOR", size: 12 });
texts.push({ x: 60,  y: 375, content: "3. CH340C USB-UART BRIDGE", size: 12 });
texts.push({ x: 430, y: 775, content: "4. ATMEGA328P-AU MAIN MCU (16MHz / 5V)", size: 12 });
texts.push({ x: 420, y: 375, content: "5. 16MHz CRYSTAL & BYPASS", size: 12 });
texts.push({ x: 625, y: 375, content: "6. RESET CIRCUIT", size: 12 });
texts.push({ x: 870, y: 775, content: "7. UNO R3 COMPATIBLE HEADERS & ICSP", size: 12 });
texts.push({ x: 805, y: 365, content: "8. STATUS LEDS (PWR / L / TX / RX)", size: 12 });

// ============================================================
// BLOCK 1: USB-C & 5V POWER
// ============================================================
const usbRightNets = {
  A1B12: "GND",
  B1A12: "GND",
  A4B9:  "VBUS",
  B4A9:  "VBUS",
  A5:    "USB_CC1",
  B5:    "USB_CC2",
  A6:    "USB_DP",
  B6:    "USB_DP",
  A7:    "USB_DN",
  B7:    "USB_DN"
};
for (const [pnum, net] of Object.entries(usbRightNets)) {
  addStub(pinByNum("J_USB", pnum), 35, 0, net);
}
// Tie 4 shell pins (1,2,3,4) cleanly onto a vertical GND bus anchored to 1 GND flag
const ehPins = ["1", "2", "3", "4"].map((n) => pinByNum("J_USB", n)).sort((a, b) => b.y - a.y);
const ehBusX = ehPins[0].x - 20;
for (let i = 0; i < ehPins.length; i++) {
  wires.push({ line: [[ehPins[i].x, ehPins[i].y, ehBusX, ehPins[i].y]] });
  if (i < ehPins.length - 1) {
    wires.push({ line: [[ehBusX, ehPins[i].y, ehBusX, ehPins[i + 1].y]] });
  }
}
const ehBot = ehPins[ehPins.length - 1];
wires.push({ line: [[ehBusX, ehBot.y, ehBusX, ehBot.y - 15]] });
flags.push({ type: "Ground", net: "GND", x: ehBusX, y: ehBot.y - 15, rot: 0 });

// R1 (CC1 5.1k), R2 (CC2 5.1k)
addStub(topPin("R1"), 0, 20, "USB_CC1");
addGndFlag(botPin("R1"), 0, -15);
addStub(topPin("R2"), 0, 20, "USB_CC2");
addGndFlag(botPin("R2"), 0, -15);

// F1 (500mA PTC): left=VBUS, right=+5V, C7 (10uF) on +5V rail anchored to 1 Power flag
addStub(leftPin("F1"), -25, 0, "VBUS");
const f1R = rightPin("F1");
const c7T = topPin("C7");
wires.push({ line: [[f1R.x, f1R.y, c7T.x, f1R.y]] });
wires.push({ line: [[c7T.x, f1R.y, c7T.x, c7T.y]] });
wires.push({ line: [[c7T.x, f1R.y, c7T.x, f1R.y + 15]] });
flags.push({ type: "Power", net: "+5V", x: c7T.x, y: f1R.y + 15, rot: 0 });
addGndFlag(botPin("C7"), 0, -15);

// ============================================================
// BLOCK 2: 3.3V LDO (AMS1117-3.3)
// ============================================================
addStub(pinByNum("U3", "1"), -25, 0, "GND");
addStub(pinByNum("U3", "2"), -25, 0, "+3V3");
const u3Vin = pinByNum("U3", "3");
const c8T = topPin("C8");
wires.push({ line: [[u3Vin.x, u3Vin.y, c8T.x, u3Vin.y]] });
wires.push({ line: [[c8T.x, u3Vin.y, c8T.x, c8T.y]] });
wires.push({ line: [[c8T.x, c8T.y, c8T.x, c8T.y + 20]] });
flags.push({ type: "Power", net: "+5V", x: c8T.x, y: c8T.y + 20, rot: 0 });
addGndFlag(botPin("C8"), 0, -15);

const u3Vo4 = pinByNum("U3", "4");
const c9T = topPin("C9");
wires.push({ line: [[u3Vo4.x, u3Vo4.y, c9T.x, u3Vo4.y]] });
wires.push({ line: [[c9T.x, u3Vo4.y, c9T.x, c9T.y]] });
wires.push({ line: [[c9T.x, u3Vo4.y, c9T.x, u3Vo4.y + 20]] });
flags.push({ type: "Power", net: "+3V3", x: c9T.x, y: u3Vo4.y + 20, rot: 0 });
addGndFlag(botPin("C9"), 0, -15);

// ============================================================
// BLOCK 3: CH340C USB-UART BRIDGE
// ============================================================
const ch340Left = {
  "1": "GND",
  "2": "CH340_TX",
  "3": "CH340_RX",
  "4": "CH340_V3",
  "5": "USB_DP",
  "6": "USB_DN"
};
for (const [pnum, net] of Object.entries(ch340Left)) {
  addStub(pinByNum("U2", pnum), -35, 0, net);
}
addStub(pinByNum("U2", "16"), 35, 0, "+5V");
const dtrP = pinByNum("U2", "13");
const c3L = leftPin("C3");
const c3R = rightPin("C3");
wires.push({ line: [[dtrP.x, dtrP.y, c3L.x, c3L.y]], net: "CH340_DTR" });
addStub(c3R, 30, 0, "RESET");

addStub(topPin("C1"), 0, 20, "CH340_V3");
addGndFlag(botPin("C1"), 0, -15);
addPowerFlag(topPin("C2"), 0, 15, "+5V");
addGndFlag(botPin("C2"), 0, -15);

addStub(leftPin("R3"), -30, 0, "CH340_TX");
addStub(rightPin("R3"), 30, 0, "D0_RX");
addStub(leftPin("R4"), -30, 0, "CH340_RX");
addStub(rightPin("R4"), 30, 0, "D1_TX");

// ============================================================
// BLOCK 4: ATMEGA328P-AU MAIN MCU
// ============================================================
const mcuLeft = {
  "1": "D3",   "2": "D4",   "3": "GND",   "4": "+5V",
  "5": "GND",  "6": "+5V",  "7": "XTAL1", "8": "XTAL2",
  "9": "D5",   "10": "D6",  "11": "D7",   "12": "D8",
  "13": "D9",  "14": "D10", "15": "D11",  "16": "D12"
};
const mcuRight = {
  "17": "D13",    "18": "+5V",
  "20": "AREF",   "21": "GND",
  "23": "A0",     "24": "A1",     "25": "A2",     "26": "A3",
  "27": "A4_SDA", "28": "A5_SCL", "29": "RESET",  "30": "D0_RX",
  "31": "D1_TX",  "32": "D2"
};
for (const [pnum, net] of Object.entries(mcuLeft)) {
  addStub(pinByNum("U1", pnum), -35, 0, net);
}
for (const [pnum, net] of Object.entries(mcuRight)) {
  addStub(pinByNum("U1", pnum), 35, 0, net);
}

// ============================================================
// BLOCK 5: 16MHz CRYSTAL & MCU BYPASS CAPS
// ============================================================
const y1Osc1 = pinByNum("Y1", "1");
const y1Gnd2 = pinByNum("Y1", "2");
const y1Osc2 = pinByNum("Y1", "3");
const y1Gnd4 = pinByNum("Y1", "4");
const c10T = topPin("C10");
const c11T = topPin("C11"); // now at X=565, well clear of y1Gnd2 stub (525..540)

wires.push({ line: [[y1Osc1.x, y1Osc1.y, c10T.x, y1Osc1.y, c10T.x, c10T.y]], net: "XTAL1" });
wires.push({ line: [[y1Osc2.x, y1Osc2.y, c11T.x, y1Osc2.y, c11T.x, c11T.y]], net: "XTAL2" });
addStub(y1Gnd4, -15, 0, "GND");
addStub(y1Gnd2, 15, 0, "GND");
addGndFlag(botPin("C10"), 0, -15);
addGndFlag(botPin("C11"), 0, -15);

addPowerFlag(topPin("C4"), 0, 15, "+5V");
addGndFlag(botPin("C4"), 0, -15);
addPowerFlag(topPin("C5"), 0, 15, "+5V");
addGndFlag(botPin("C5"), 0, -15);
addStub(topPin("C6"), 0, 20, "AREF");
addGndFlag(botPin("C6"), 0, -15);

// ============================================================
// BLOCK 6: RESET CIRCUIT (R9 10k, D1 1N4148WS, SW1 Tactile)
// ============================================================
const r9T = topPin("R9");
const r9B = botPin("R9");
const d1K = pinByName("D1", "K");
const d1A = pinByName("D1", "A");
const sw1 = pinByNum("SW1", "1");
const sw2 = pinByNum("SW1", "2");
const sw3 = pinByNum("SW1", "3");
const sw4 = pinByNum("SW1", "4");

// Top +5V rail (no duplicate wire net labels — inherited from 1 Power flag)
const rstTopY = Math.max(r9T.y, d1K.y) + 15;
const rstMidX = Math.round((r9T.x + d1K.x) / 2);
wires.push({ line: [[r9T.x, r9T.y, r9T.x, rstTopY, rstMidX, rstTopY]] });
wires.push({ line: [[d1K.x, d1K.y, d1K.x, rstTopY, rstMidX, rstTopY]] });
wires.push({ line: [[rstMidX, rstTopY, rstMidX, rstTopY + 15]] });
flags.push({ type: "Power", net: "+5V", x: rstMidX, y: rstTopY + 15, rot: 0 });

// Mid RESET rail (assign "RESET" on ONE segment only to prevent ONENETNAME warning)
const rstBusY = Math.min(r9B.y, d1A.y) - 15;
wires.push({ line: [[r9B.x, r9B.y, r9B.x, rstBusY]], net: "RESET" });
wires.push({ line: [[d1A.x, d1A.y, d1A.x, rstBusY]] });
wires.push({ line: [[r9B.x, rstBusY, sw1.x, rstBusY]] });
wires.push({ line: [[sw1.x, rstBusY, sw2.x, rstBusY]] });
wires.push({ line: [[sw2.x, rstBusY, d1A.x, rstBusY]] });
wires.push({ line: [[sw1.x, rstBusY, sw1.x, sw1.y]] });
wires.push({ line: [[sw2.x, rstBusY, sw2.x, sw2.y]] });

// SW1.3 & SW1.4 -> GND (inherited from 1 Ground flag)
const swGndY = Math.min(sw3.y, sw4.y) - 15;
const swMidX = Math.round((sw3.x + sw4.x) / 2);
wires.push({ line: [[sw3.x, sw3.y, sw3.x, swGndY, swMidX, swGndY]] });
wires.push({ line: [[sw4.x, sw4.y, sw4.x, swGndY, swMidX, swGndY]] });
wires.push({ line: [[swMidX, swGndY, swMidX, swGndY - 15]] });
flags.push({ type: "Ground", net: "GND", x: swMidX, y: swGndY - 15, rot: 0 });

// ============================================================
// BLOCK 7: UNO R3 HEADERS & ICSP
// ============================================================
const digHiNets = ["A5_SCL","A4_SDA","AREF","GND","D13","D12","D11","D10","D9","D8"];
digHiNets.forEach((net, idx) => addStub(pinByNum("J_DIG_HI", idx + 1), -35, 0, net));

const digLoNets = ["D7","D6","D5","D4","D3","D2","D1_TX","D0_RX"];
digLoNets.forEach((net, idx) => addStub(pinByNum("J_DIG_LO", idx + 1), -35, 0, net));

const pwrMap = { "2": "+5V", "3": "RESET", "4": "+3V3", "5": "+5V", "6": "GND", "7": "GND" };
for (const [pnum, net] of Object.entries(pwrMap)) {
  addStub(pinByNum("J_POWER", pnum), -35, 0, net);
}

const analogNets = ["A0","A1","A2","A3","A4_SDA","A5_SCL"];
analogNets.forEach((net, idx) => addStub(pinByNum("J_ANALOG", idx + 1), -35, 0, net));

addStub(pinByNum("J_ICSP", "1"), -35, 0, "D12");
addStub(pinByNum("J_ICSP", "3"), -35, 0, "D13");
addStub(pinByNum("J_ICSP", "5"), -35, 0, "RESET");
addStub(pinByNum("J_ICSP", "2"),  35, 0, "+5V");
addStub(pinByNum("J_ICSP", "4"),  35, 0, "D11");
addStub(pinByNum("J_ICSP", "6"),  35, 0, "GND");

// ============================================================
// BLOCK 8: STATUS LEDS (PWR, L, TX, RX) - all verified A_TOP!
// ============================================================
const ledCols = [
  { rRef: "R5", ledRef: "LED_PWR", topNet: "+5V", topType: "Power",  midNet: "LED_PWR_A", botNet: "GND",      botType: "Ground" },
  { rRef: "R8", ledRef: "LED_L",   topNet: "D13", topType: "Signal", midNet: "LED_L_A",   botNet: "GND",      botType: "Ground" },
  { rRef: "R6", ledRef: "LED_TX",  topNet: "+5V", topType: "Power",  midNet: "LED_TX_A",  botNet: "CH340_TX", botType: "Signal" },
  { rRef: "R7", ledRef: "LED_RX",  topNet: "+5V", topType: "Power",  midNet: "LED_RX_A",  botNet: "CH340_RX", botType: "Signal" }
];
for (const col of ledCols) {
  const rT = topPin(col.rRef);
  const rB = botPin(col.rRef);
  const lA = pinByName(col.ledRef, "A");
  const lK = pinByName(col.ledRef, "K");
  if (col.topType === "Power") addPowerFlag(rT, 0, 15, col.topNet);
  else addStub(rT, 0, 20, col.topNet);

  if (rB.x === lA.x) {
    wires.push({ line: [[rB.x, rB.y, lA.x, lA.y]], net: col.midNet });
  } else {
    const my = Math.round((rB.y + lA.y) / 2);
    wires.push({ line: [[rB.x, rB.y, rB.x, my, lA.x, my, lA.x, lA.y]], net: col.midNet });
  }

  if (col.botType === "Ground") addGndFlag(lK, 0, -15);
  else addStub(lK, 0, -20, col.botNet);
}

console.log(`Prepared clean plan: ${wires.length} wires, ${flags.length} flags, ${texts.length} headings.`);

function execBatch(name, js, timeout = 120000) {
  const f = `${base}/outputs/.tmp_${name}.js`;
  fs.writeFileSync(f, js);
  const t0 = Date.now();
  const out = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", f, "--timeout", String(timeout)], { encoding: "utf8" }));
  fs.unlinkSync(f);
  console.log(`[OK] ${name} in ${((Date.now() - t0) / 1000).toFixed(1)}s`);
  return out.data?.result ?? out.data ?? out;
}

const mid = Math.ceil(wires.length / 2);
const wireChunk1 = wires.slice(0, mid);
const wireChunk2 = wires.slice(mid);

const resA = execBatch("wire1", `
const flags = ${JSON.stringify(flags)};
const texts = ${JSON.stringify(texts)};
const wires = ${JSON.stringify(wireChunk1)};
let fOk = 0, tOk = 0, wOk = 0;
const errs = [];
for (const f of flags) {
  try { await eda.sch_PrimitiveComponent.createNetFlag(f.type, f.net, f.x, f.y, f.rot, false); fOk++; }
  catch (e) { errs.push("flag " + f.net + ": " + (e?.message || e)); }
}
for (const t of texts) {
  try { await eda.sch_PrimitiveText.create(t.x, t.y, t.content, 0, "#2255AA", null, t.size, true, false); tOk++; }
  catch (e) { errs.push("text: " + (e?.message || e)); }
}
for (const w of wires) {
  try {
    if (w.net !== undefined) await eda.sch_PrimitiveWire.create(w.line, w.net);
    else await eda.sch_PrimitiveWire.create(w.line);
    wOk++;
  } catch (e) { errs.push("wire " + w.net + ": " + (e?.message || e)); }
}
return { fOk, tOk, wOk, errCount: errs.length, errs: errs.slice(0, 10) };
`, 120000);
console.log("Batch A:", JSON.stringify(resA));

const resB = execBatch("wire2", `
const wires = ${JSON.stringify(wireChunk2)};
let wOk = 0;
const errs = [];
for (const w of wires) {
  try {
    if (w.net !== undefined) await eda.sch_PrimitiveWire.create(w.line, w.net);
    else await eda.sch_PrimitiveWire.create(w.line);
    wOk++;
  } catch (e) { errs.push("wire " + w.net + ": " + (e?.message || e)); }
}
try { await eda.dmt_EditorControl.zoomToAllPrimitives(); } catch {}
await eda.sch_Document.save();
await new Promise(r => setTimeout(r, 6000));
return { wOk, errCount: errs.length, errs: errs.slice(0, 10) };
`, 120000);
console.log("Batch B:", JSON.stringify(resB));

// Separate fast calls for DRC, Netlist, BOM, PNG
const drc = execBatch("drc", `
const strictPass = await eda.sch_Drc.check(true, false, false);
const verbose = await eda.sch_Drc.check(true, false, true);
return { strictPass, verbose };
`, 45000);
fs.writeFileSync(`${base}/outputs/step2_drc_log.json`, JSON.stringify(drc, null, 2));
const nonFloat = (drc.verbose || []).filter((e) => e.rule !== "CHECKFLOATPIN");
const floatPins = (drc.verbose || []).find((e) => e.rule === "CHECKFLOATPIN")?.primitives || [];
console.log("DRC strictPass:", drc.strictPass, "non-float issues:", nonFloat.length, "float pins:", floatPins.length);
if (nonFloat.length > 0) console.log("Non-float DRC:", JSON.stringify(nonFloat, null, 2));

const toB64 = `
function toB64(blob) {
  if (!blob) return null;
  return new Promise((res, rej) => {
    const fr = new FileReader();
    fr.onload = () => res(String(fr.result).split(",")[1] || "");
    fr.onerror = (e) => rej(e);
    fr.readAsDataURL(blob);
  });
}
`;

try {
  const png = execBatch("png", `${toB64}
const f = await eda.sch_ManufactureData.getPngFile("unomodoki_sch", { width: 1800 });
return f ? { name: f.name, size: f.size, b64: await toB64(f) } : null;
`, 60000);
  if (png?.b64) {
    fs.writeFileSync(`${base}/outputs/unomodoki_sch.png`, Buffer.from(png.b64, "base64"));
    console.log("Wrote outputs/unomodoki_sch.png:", png.size, "bytes");
  }
} catch (e) { console.log("PNG export warn:", e.message); }

try {
  const net = execBatch("netlist", `${toB64}
const f = await eda.sch_ManufactureData.getNetlistFile("unomodoki_netlist", "Protel2");
return f ? { name: f.name, size: f.size, b64: await toB64(f) } : null;
`, 60000);
  if (net?.b64) {
    fs.writeFileSync(`${base}/outputs/unomodoki_netlist.net`, Buffer.from(net.b64, "base64"));
    console.log("Wrote outputs/unomodoki_netlist.net:", net.size, "bytes");
  }
} catch (e) { console.log("Netlist export warn:", e.message); }

try {
  const bom = execBatch("bom", `${toB64}
const f = await eda.sch_ManufactureData.getBomFile("unomodoki_bom", "csv");
return f ? { name: f.name, size: f.size, b64: await toB64(f) } : null;
`, 45000);
  if (bom?.b64) {
    fs.writeFileSync(`${base}/outputs/unomodoki_bom.csv`, Buffer.from(bom.b64, "base64"));
    console.log("Wrote outputs/unomodoki_bom.csv:", bom.size, "bytes");
  }
} catch (e) { console.log("BOM export warn:", e.message); }
