import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const bom = JSON.parse(fs.readFileSync(`${base}/bom_verified.json`, "utf8"));
const byKey = Object.fromEntries(bom.parts.map((p) => [p.key, p]));

// Full 37-component clean functional-block placement on A4 sheet (1170 x 825, unit=10mil)
// Note: In EasyEDA Pro SCH create(), rot=90 is CW (turns +X to -Y) and rot=270 is CCW (+X to +Y).
// Verified orientations:
//   - D1 (A:+20, K:-20): rot=90 -> K on top (+Y), A on bottom (-Y)
//   - LED_L (A:-20, K:+20): rot=90 -> A on top (+Y), K on bottom (-Y)
//   - LED_PWR, LED_TX, LED_RX (A:+20, K:-20): rot=270 -> A on top (+Y), K on bottom (-Y)
const placements = [
  // Block 1 (USB-C & 5V) + Block 2 (3.3V LDO) + Block 3 (CH340C)
  { ref: "J_USB", key: "USB_C",     x: 140, y: 665, rot: 180 },
  { ref: "R1",    key: "R_5K1",     x: 245, y: 615, rot: 90  },
  { ref: "R2",    key: "R_5K1",     x: 295, y: 615, rot: 90  },
  { ref: "F1",    key: "PTC_500M",  x: 265, y: 730, rot: 0   },
  { ref: "C7",    key: "C_10U",     x: 345, y: 685, rot: 90  },

  { ref: "U3",    key: "LDO_3V3",   x: 210, y: 455, rot: 0   },
  { ref: "C8",    key: "C_10U",     x: 115, y: 440, rot: 90  },
  { ref: "C9",    key: "C_10U",     x: 315, y: 440, rot: 90  },

  { ref: "U2",    key: "USB_UART",  x: 205, y: 215, rot: 0   },
  { ref: "C1",    key: "C_100N",    x: 95,  y: 165, rot: 90  },
  { ref: "C2",    key: "C_100N",    x: 335, y: 290, rot: 90  },
  { ref: "C3",    key: "C_100N",    x: 320, y: 220, rot: 0   },

  // UART series resistors + Block 4 (MCU + Crystal + Bypass + Reset)
  { ref: "R3",    key: "R_1K",      x: 225, y: 345, rot: 0   },
  { ref: "R4",    key: "R_1K",      x: 225, y: 310, rot: 0   },

  { ref: "U1",    key: "MCU",       x: 620, y: 595, rot: 0   },
  { ref: "Y1",    key: "XTAL_16M",  x: 495, y: 325, rot: 0   },
  { ref: "C10",   key: "C_15P",     x: 445, y: 250, rot: 90  },
  { ref: "C11",   key: "C_15P",     x: 565, y: 250, rot: 90  },

  { ref: "C4",    key: "C_100N",    x: 445, y: 135, rot: 90  },
  { ref: "C5",    key: "C_100N",    x: 500, y: 135, rot: 90  },
  { ref: "C6",    key: "C_100N",    x: 555, y: 135, rot: 90  },

  { ref: "R9",    key: "R_10K",     x: 650, y: 325, rot: 90  },
  { ref: "D1",    key: "DIODE_RST", x: 710, y: 325, rot: 90  },
  { ref: "SW1",   key: "SW_RST",    x: 680, y: 225, rot: 0   },

  // Block 5 (Headers + ICSP + Status LEDs)
  { ref: "J_DIG_HI", key: "HDR_1X10F", x: 960,  y: 700, rot: 0 },
  { ref: "J_DIG_LO", key: "HDR_1X8F",  x: 1095, y: 700, rot: 0 },
  { ref: "J_POWER",  key: "HDR_1X8F",  x: 960,  y: 530, rot: 0 },
  { ref: "J_ANALOG", key: "HDR_1X6F",  x: 1095, y: 530, rot: 0 },
  { ref: "J_ICSP",   key: "HDR_2X3M",  x: 1015, y: 405, rot: 0 },

  { ref: "R5",      key: "R_1K",    x: 830,  y: 315, rot: 90  },
  { ref: "LED_PWR", key: "LED_PWR", x: 830,  y: 245, rot: 270 },
  { ref: "R8",      key: "R_2K2",   x: 915,  y: 315, rot: 90  },
  { ref: "LED_L",   key: "LED_L",   x: 915,  y: 245, rot: 90  },
  { ref: "R6",      key: "R_1K",    x: 1000, y: 315, rot: 90  },
  { ref: "LED_TX",  key: "LED_TX",  x: 1000, y: 245, rot: 270 },
  { ref: "R7",      key: "R_1K",    x: 1085, y: 315, rot: 90  },
  { ref: "LED_RX",  key: "LED_RX",  x: 1085, y: 245, rot: 270 }
];

const enriched = placements.map((p) => {
  const b = byKey[p.key];
  if (!b || !b.uuid) throw new Error(`Missing verified BOM part for ${p.key}`);
  return { ...p, lcsc: b.lcsc, uuid: b.uuid, libraryUuid: b.libraryUuid, name: b.name };
});

// Replace schematic page with a brand-new blank page (instant, avoids slow wire-by-wire delete!)
const resetPageJs = `
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const oldPages = await eda.dmt_Schematic.getAllSchematicPagesInfo() || [];
const newPageUuid = await eda.dmt_Schematic.createSchematicPage();
await sleep(1500);
await eda.dmt_EditorControl.openDocument(newPageUuid);
await sleep(1500);
let deletedOld = 0;
for (const op of oldPages) {
  if (op.uuid !== newPageUuid) {
    try { await eda.dmt_Schematic.deleteSchematicPage(op.uuid); deletedOld++; } catch {}
  }
}
await sleep(1000);
return { newPageUuid, deletedOld, pagesNow: (await eda.dmt_Schematic.getAllSchematicPagesInfo() || []).length };
`;
fs.writeFileSync(`${base}/outputs/.tmp_reset_page.js`, resetPageJs);
console.log("Reset page:", execFileSync("node", [helper, "invoke", "--session", sid, "--file", `${base}/outputs/.tmp_reset_page.js`, "--timeout", "45000"], { encoding: "utf8" }).trim());
fs.unlinkSync(`${base}/outputs/.tmp_reset_page.js`);

const allPlaced = [];
const BATCH = 10;
for (let i = 0; i < enriched.length; i += BATCH) {
  const chunk = enriched.slice(i, i + BATCH);
  const js = `
const snap = (v) => Math.round(v * 100) / 100;
const items = ${JSON.stringify(chunk)};
const out = [];
for (const it of items) {
  const refObj = { libraryType: "3", libraryUuid: it.libraryUuid, uuid: it.uuid };
  const c = await eda.sch_PrimitiveComponent.create(refObj, it.x, it.y, undefined, it.rot, false, true, true);
  const id = c.getState_PrimitiveId();
  const cur = await eda.sch_PrimitiveComponent.get(id);
  const op = cur.getState_OtherProperty() || {};
  await eda.sch_PrimitiveComponent.modify(id, { designator: it.ref, otherProperty: op });
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  const pList = (pins || []).map((p) => ({
    number: String(p.pinNumber ?? p.getState_PinNumber?.() ?? ""),
    name: String(p.pinName ?? p.getState_PinName?.() ?? ""),
    x: snap(p.x ?? p.getState_X?.() ?? 0),
    y: snap(p.y ?? p.getState_Y?.() ?? 0)
  }));
  out.push({ ref: it.ref, key: it.key, lcsc: it.lcsc, id, x: it.x, y: it.y, rot: c.getState_Rotation?.() ?? it.rot, pins: pList });
}
await eda.sch_Document.save();
return out;
`;
  fs.writeFileSync(`${base}/outputs/.tmp_place.js`, js);
  const t0 = Date.now();
  const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", `${base}/outputs/.tmp_place.js`, "--timeout", "90000"], { encoding: "utf8" }));
  const rows = raw.data?.result ?? raw.data ?? [];
  console.log(`Batch ${Math.floor(i/BATCH)+1}: placed ${rows.length} parts in ${((Date.now()-t0)/1000).toFixed(1)}s`);
  allPlaced.push(...rows);
}

fs.unlinkSync(`${base}/outputs/.tmp_place.js`);
fs.writeFileSync(`${base}/outputs/sch_placed_pins.json`, JSON.stringify(allPlaced, null, 2));
console.log("Total placed:", allPlaced.length);
for (const p of allPlaced.filter((x) => ["D1","LED_PWR","LED_L","LED_TX","LED_RX","C11"].includes(x.ref))) {
  console.log(" Check:", p.ref.padEnd(8), "rot="+p.rot, p.pins.map(k => `${k.name}@(${k.x},${k.y})`).join("  "));
}
