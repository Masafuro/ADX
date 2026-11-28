const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const snap = (v) => Math.round(v * 100) / 100;

const wantRefs = new Set([
  "J_USB","R1","R2","F1","C7","U3","C8","C9","U2","C1","C2","C3","R3","R4",
  "U1","Y1","C10","C11","C4","C5","C6","R9","D1","SW1",
  "J_DIG_HI","J_DIG_LO","J_POWER","J_ANALOG","J_ICSP",
  "R5","LED_PWR","R8","LED_L","R6","LED_TX","R7","LED_RX"
]);

// 1. Find & delete auto-renamed duplicate parts (e.g. R10, D2, SW2, R11, C12, J_DIG_HI1...)
const allComps = await eda.sch_PrimitiveComponent.getAll() || [];
const removedDups = [];
const dupIds = [];
const c456Map = {};
const oldC456FlagIds = [];
for (const c of allComps) {
  const id = c.getState_PrimitiveId();
  const ref = c.getState_Designator() || "";
  const typ = String(c.getState_ComponentType?.() || "").toLowerCase();
  const cx = snap(c.getState_X?.() ?? 0);
  const cy = snap(c.getState_Y?.() ?? 0);

  if (wantRefs.has(ref)) {
    if (["C4","C5","C6"].includes(ref)) c456Map[ref] = { id, c };
    continue;
  }
  // Keep title sheet and Power/Ground flags (except old flags around C4/C5/C6 at y < 200, x in 420..590)
  if (typ === "sheet") continue;
  if (typ.includes("netflag") || typ.includes("netport") || ref === "" || ref.startsWith("$")) {
    if (cx >= 420 && cx <= 590 && cy <= 195) {
      oldC456FlagIds.push(id);
    }
    continue;
  }
  // Any other named part not in wantRefs is an auto-renamed duplicate!
  dupIds.push(id);
  removedDups.push({ ref, typ, x: cx, y: cy });
}
if (dupIds.length) await eda.sch_PrimitiveComponent.delete(dupIds);
if (oldC456FlagIds.length) await eda.sch_PrimitiveComponent.delete(oldC456FlagIds);

// 2. Delete only the 6 tiny wires belonging to old C4/C5/C6 (in x:430..570, y:80..195)
const allWires = await eda.sch_PrimitiveWire.getAll() || [];
const oldC456WireIds = [];
for (const w of allWires) {
  const line = w.getState_Line?.();
  if (!line) continue;
  const flat = Array.isArray(line[0]) ? line.flat() : line;
  let inBox = flat.length >= 4;
  for (let i = 0; i < flat.length; i += 2) {
    const wx = flat[i], wy = flat[i + 1];
    if (!(wx >= 430 && wx <= 575 && wy >= 80 && wy <= 195)) { inBox = false; break; }
  }
  if (inBox) oldC456WireIds.push(w.getState_PrimitiveId());
}
if (oldC456WireIds.length) await eda.sch_PrimitiveWire.delete(oldC456WireIds);

// 3. Move C4, C5, C6 up to clean empty area below U1 (y=435, completely clear of title block!)
const newPos = { C4: { x: 470, y: 435 }, C5: { x: 540, y: 435 }, C6: { x: 610, y: 435 } };
for (const [ref, pos] of Object.entries(newPos)) {
  const entry = c456Map[ref];
  if (!entry) continue;
  const cur = await eda.sch_PrimitiveComponent.get(entry.id);
  const op = cur.getState_OtherProperty() || {};
  await eda.sch_PrimitiveComponent.modify(entry.id, { x: pos.x, y: pos.y, otherProperty: op });
}
await sleep(1000);

// Re-wire C4 (+5V/GND), C5 (+5V/GND), C6 (AREF/GND) at new locations
for (const [ref, topNet] of [["C4","+5V"], ["C5","+5V"], ["C6","AREF"]]) {
  const entry = c456Map[ref];
  if (!entry) continue;
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(entry.id);
  const pts = pins.map(p => ({ x: snap(p.x ?? p.getState_X?.()), y: snap(p.y ?? p.getState_Y?.()) })).sort((a,b) => b.y - a.y);
  const top = pts[0], bot = pts[1];
  if (topNet === "+5V") {
    await eda.sch_PrimitiveWire.create([[top.x, top.y, top.x, top.y + 15]]);
    await eda.sch_PrimitiveComponent.createNetFlag("Power", "+5V", top.x, top.y + 15, 0, false);
  } else {
    await eda.sch_PrimitiveWire.create([[top.x, top.y, top.x, top.y + 20]], topNet);
  }
  await eda.sch_PrimitiveWire.create([[bot.x, bot.y, bot.x, bot.y - 15]]);
  await eda.sch_PrimitiveComponent.createNetFlag("Ground", "GND", bot.x, bot.y - 15, 0, false);
}

// 4. Refresh section headings cleanly without any overlap
const oldTexts = await eda.sch_PrimitiveText.getAll() || [];
if (oldTexts.length) await eda.sch_PrimitiveText.delete(oldTexts.map(t => t.getState_PrimitiveId()));

const headings = [
  { x: 60,  y: 778, content: "1. USB-C & 5V POWER (USB-C ONLY)" },
  { x: 60,  y: 510, content: "2. 3.3V LDO REGULATOR" },
  { x: 145, y: 375, content: "3. CH340C USB-UART BRIDGE" },
  { x: 430, y: 778, content: "4. ATMEGA328P-AU MAIN MCU (16MHz / 5V)" },
  { x: 440, y: 492, content: "5. MCU BYPASS & AREF CAPS" },
  { x: 420, y: 372, content: "6. 16MHz CRYSTAL" },
  { x: 625, y: 382, content: "7. RESET CIRCUIT" },
  { x: 870, y: 778, content: "8. UNO R3 COMPATIBLE HEADERS & ICSP" },
  { x: 805, y: 376, content: "9. STATUS LEDS (PWR / L / TX / RX)" }
];
for (const h of headings) {
  await eda.sch_PrimitiveText.create(h.x, h.y, h.content, 0, "#2255AA", null, 11, true, false);
}

try { await eda.dmt_EditorControl.zoomToAllPrimitives(); } catch {}
await eda.sch_Document.save();
await sleep(5000);

const finalComps = await eda.sch_PrimitiveComponent.getAll() || [];
const partRefs = finalComps
  .filter(c => {
    const t = String(c.getState_ComponentType?.() || "").toLowerCase();
    return t === "part" || wantRefs.has(c.getState_Designator() || "");
  })
  .map(c => c.getState_Designator());

const drcVerbose = await eda.sch_Drc.check(true, false, true);
return {
  removedDups,
  removedOldFlags: oldC456FlagIds.length,
  removedOldWires: oldC456WireIds.length,
  finalPartCount: partRefs.length,
  drcVerbose
};
