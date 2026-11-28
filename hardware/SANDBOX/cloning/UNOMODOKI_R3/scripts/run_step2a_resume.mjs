import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const bom = JSON.parse(fs.readFileSync(`${base}/bom_verified.json`, "utf8"));
const byKey = Object.fromEntries(bom.parts.map((p) => [p.key, p]));

const placements = [
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

const targetRefs = new Set(placements.map((p) => p.ref));
const enriched = placements.map((p) => ({
  ...p,
  lcsc: byKey[p.key].lcsc,
  uuid: byKey[p.key].uuid,
  libraryUuid: byKey[p.key].libraryUuid
}));

function run(name, js, timeout = 60000) {
  const f = `${base}/outputs/.tmp_${name}.js`;
  fs.writeFileSync(f, js);
  const t0 = Date.now();
  const out = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", f, "--timeout", String(timeout)], { encoding: "utf8" }));
  fs.unlinkSync(f);
  console.log(`[OK] ${name} (${((Date.now() - t0) / 1000).toFixed(1)}s)`);
  return out.data?.result ?? out.data ?? out;
}

// Inspect existing components (ignore Drawing-Symbol title block; remove any stray unnamed part)
const existing = run("inspect", `
const comps = await eda.sch_PrimitiveComponent.getAll() || [];
const out = [];
for (const c of comps) {
  const id = c.getState_PrimitiveId();
  const ref = c.getState_Designator();
  const name = c.getState_Name?.() || "";
  const typ = c.getState_ComponentType?.() || "";
  out.push({ id, ref, name, typ });
}
return out;
`);
const alreadyPlaced = new Set(existing.map((e) => e.ref).filter((r) => targetRefs.has(r)));
console.log("Already placed:", alreadyPlaced.size, "stray items:", existing.filter((e) => !targetRefs.has(e.ref)));

const remaining = enriched.filter((p) => !alreadyPlaced.has(p.ref));
console.log("Remaining to place:", remaining.length);

const BATCH = 5;
for (let i = 0; i < remaining.length; i += BATCH) {
  const chunk = remaining.slice(i, i + BATCH);
  run(`place_${i/BATCH+1}`, `
const items = ${JSON.stringify(chunk)};
const placed = [];
for (const it of items) {
  const refObj = { libraryType: "3", libraryUuid: it.libraryUuid, uuid: it.uuid };
  const c = await eda.sch_PrimitiveComponent.create(refObj, it.x, it.y, undefined, it.rot, false, true, true);
  const id = c.getState_PrimitiveId();
  const cur = await eda.sch_PrimitiveComponent.get(id);
  const op = cur.getState_OtherProperty() || {};
  await eda.sch_PrimitiveComponent.modify(id, { designator: it.ref, otherProperty: op });
  placed.push(it.ref);
}
await eda.sch_Document.save();
return placed;
`, 60000);
}

// Refresh all 37 placed components' exact measured pin coordinates
const refreshed = run("refresh_all", `
const snap = (v) => Math.round(v * 100) / 100;
const want = new Set(${JSON.stringify([...targetRefs])});
const comps = await eda.sch_PrimitiveComponent.getAll() || [];
const out = [];
for (const c of comps) {
  const ref = c.getState_Designator();
  if (!want.has(ref)) continue;
  const id = c.getState_PrimitiveId();
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  out.push({
    ref, id,
    x: snap(c.getState_X?.() ?? 0),
    y: snap(c.getState_Y?.() ?? 0),
    rot: c.getState_Rotation?.() ?? 0,
    pins: (pins || []).map((p) => ({
      number: String(p.pinNumber ?? p.getState_PinNumber?.() ?? ""),
      name: String(p.pinName ?? p.getState_PinName?.() ?? ""),
      x: snap(p.x ?? p.getState_X?.() ?? 0),
      y: snap(p.y ?? p.getState_Y?.() ?? 0)
    }))
  });
}
return out;
`, 60000);

fs.writeFileSync(`${base}/outputs/sch_placed_pins.json`, JSON.stringify(refreshed, null, 2));
console.log("Refreshed total components:", refreshed.length);
for (const p of refreshed.filter((x) => ["D1","LED_PWR","LED_L","LED_TX","LED_RX","C11"].includes(x.ref))) {
  console.log(" Verify:", p.ref.padEnd(8), "rot="+p.rot, p.pins.map(k => `${k.name}@(${k.x},${k.y})`).join("  "));
}
