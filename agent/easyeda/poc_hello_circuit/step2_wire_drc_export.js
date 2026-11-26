// M4 PoC Step 2: Place VCC/GND net flags, draw orthogonal wires meeting cleanly at
// exact measured pin coordinates & T-junction vertices, save, wait 8s, run SCH DRC,
// and export schematic PNG, BOM (CSV), and Netlist back as Base64.

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

// Clean up unused Board2 if created alongside default Board1
let boardCleanup = "skipped";
try {
  const boards = await eda.dmt_Board.getAllBoardsInfo();
  if (boards?.some((b) => b.name === "Board2") && eda.dmt_Board.deleteBoard) {
    await eda.dmt_Board.deleteBoard("Board2");
    boardCleanup = "deleted Board2";
  }
} catch (e) {
  boardCleanup = "warn: " + String(e?.message || e);
}

// Re-read all placed components & exact pin coordinates from active sheet
const allComps = await eda.sch_PrimitiveComponent.getAll();
const byRef = {};
for (const c of allComps) {
  const ref = c.getState_Designator();
  const id = c.getState_PrimitiveId();
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  const pinMap = {};
  for (const p of pins) {
    const num = String(p.pinNumber ?? p.getState_PinNumber?.());
    const name = String(p.pinName ?? p.getState_PinName?.() ?? num);
    const x = snap(p.x ?? p.getState_X?.());
    const y = snap(p.y ?? p.getState_Y?.());
    pinMap[num] = { number: num, name, x, y };
  }
  byRef[ref] = { id, pins: pinMap };
}

// Place VCC & GND net flags
const vccFlag = await eda.sch_PrimitiveComponent.createNetFlag("Power", "VCC", 570, 660, 0, false);
const gndFlag = await eda.sch_PrimitiveComponent.createNetFlag("Ground", "GND", 560, 300, 0, false);

const vccPins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(vccFlag.getState_PrimitiveId());
const gndPins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(gndFlag.getState_PrimitiveId());
const vccPt = vccPins?.[0]
  ? { x: snap(vccPins[0].x ?? vccPins[0].getState_X?.()), y: snap(vccPins[0].y ?? vccPins[0].getState_Y?.()) }
  : { x: 570, y: 660 };
const gndPt = gndPins?.[0]
  ? { x: snap(gndPins[0].x ?? gndPins[0].getState_X?.()), y: snap(gndPins[0].y ?? gndPins[0].getState_Y?.()) }
  : { x: 560, y: 300 };

const R1 = byRef.R1.pins;
const R2 = byRef.R2.pins;
const C1 = byRef.C1.pins;
const R3 = byRef.R3.pins;
const LED1 = byRef.LED1.pins;

// Identify top/bottom pins explicitly from measured coordinates
const topPin = (p) => (p["1"].y >= p["2"].y ? p["1"] : p["2"]);
const botPin = (p) => (p["1"].y <  p["2"].y ? p["1"] : p["2"]);

const r1Top = topPin(R1), r1Bot = botPin(R1);
const r2Top = topPin(R2), r2Bot = botPin(R2);
const c1Top = topPin(C1), c1Bot = botPin(C1);
const r3Top = topPin(R3), r3Bot = botPin(R3);
// LED1: verify Anode is top and Cathode is bottom
const ledTop = topPin(LED1), ledBot = botPin(LED1);

const topRailY = 630;
const midRailY = 480;
const botRailY = 330;

// All T-junctions are split into endpoint-to-endpoint orthogonal segments
const wirePlan = [
  // VCC trunk & branches
  ["VCC->TopRail",      [[vccPt.x, vccPt.y, vccPt.x, topRailY]]],
  ["TopRail->R1.top",   [[vccPt.x, topRailY, r1Top.x, topRailY, r1Top.x, r1Top.y]]],
  ["TopRail->R3.top",   [[vccPt.x, topRailY, r3Top.x, topRailY, r3Top.x, r3Top.y]]],

  // Divider + RC filter node (MID)
  ["R1.bot->MidNode",   [[r1Bot.x, r1Bot.y, r1Bot.x, midRailY]]],
  ["MidNode->R2.top",   [[r1Bot.x, midRailY, r2Top.x, r2Top.y]]],
  ["MidNode->C1.top",   [[r1Bot.x, midRailY, c1Top.x, midRailY, c1Top.x, c1Top.y]]],

  // LED branch (R3 -> LED1 Anode)
  ["R3.bot->LED1.A",    [[r3Bot.x, r3Bot.y, ledTop.x, ledTop.y]]],

  // GND rail & branches
  ["R2.bot->GndNode",   [[r2Bot.x, r2Bot.y, r2Bot.x, botRailY, gndPt.x, botRailY]]],
  ["C1.bot->GndNode",   [[c1Bot.x, c1Bot.y, c1Bot.x, botRailY]]],
  ["LED1.K->GndNode",   [[ledBot.x, ledBot.y, ledBot.x, botRailY, gndPt.x, botRailY]]],
  ["GndNode->GND",      [[gndPt.x, botRailY, gndPt.x, gndPt.y]]],
];

const wires = [];
for (const [tag, line] of wirePlan) {
  const w = await eda.sch_PrimitiveWire.create(line);
  wires.push({
    tag,
    id: w?.getState_PrimitiveId?.() ?? null,
    net: w?.getState_Net?.() ?? null,
  });
}

// Frame circuit nicely on Windows GUI and save
try {
  await eda.dmt_EditorControl.zoomToAllPrimitives();
} catch {}
await eda.sch_Document.save();

// Mandatory settle wait before DRC & manufacture exports (per agent_rules.md)
await sleep(8000);

const drcStrictPass = await eda.sch_Drc.check(true, false, false);
const drcVerbose = await eda.sch_Drc.check(true, false, true);

// Export PNG, BOM (CSV), and Netlist
const exportsOut = {};
try {
  const png = await eda.sch_ManufactureData.getPngFile("hello_circuit_sch", { width: 1600 });
  exportsOut.png = png ? { name: png.name, size: png.size, b64: await toB64(png) } : null;
} catch (e) {
  exportsOut.png = { error: String(e?.message || e) };
}

try {
  const bom = await eda.sch_ManufactureData.getBomFile("hello_circuit_bom", "csv");
  exportsOut.bom = bom ? { name: bom.name, size: bom.size, b64: await toB64(bom) } : null;
} catch (e) {
  exportsOut.bom = { error: String(e?.message || e) };
}

try {
  const net = await eda.sch_ManufactureData.getNetlistFile("hello_circuit_netlist");
  exportsOut.netlist = net ? { name: net.name, size: net.size, b64: await toB64(net) } : null;
} catch (e) {
  exportsOut.netlist = { error: String(e?.message || e) };
}

return {
  boardCleanup,
  vccPt,
  gndPt,
  ledOrientation: { top: ledTop, bottom: ledBot },
  wires,
  drcStrictPass,
  drcVerbose,
  exports: exportsOut,
};
