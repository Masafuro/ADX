import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

// Delete only the clashing/obsolete primitives added in step4c
const delVias = [
  "59eec4883f880948", // (2510, 2285) clashed with D1_TX
  "8830260f5079194d", // (2925, 2012.8) clashed with D2
  "f97d04ea22a958e7"  // (3190, 1900) inside isolated L2 box; not needed once Y1 connects on L1 to C4.2
];
const delLines = [
  "1f2d9499b7bca9d2", // C5.2 -> (2510, 2285)
  "cfb5cb5cbea8c6dd", // U1.3 -> (2925, 2012.8)
  "bb75863b1c0f4ed5", // J_USB B1A12 -> slot
  "cbaad45247565af0", // J_USB B1A12 -> slot
  "7f4b6107f972cffb", // J_USB A1B12 -> slot
  "05339921c0359ade"  // J_USB A1B12 -> slot
];

// Mathematically clearance-verified replacements (via BEFORE tracks!)
const addVias = [
  [2460, 2239] // C5.2 stitching via (12.2 mil clearance to D1_TX & D13)
];
const addLines = [
  // 1) C5.2 -> stitching via (2460, 2239)
  [1, 2510, 2237.6, 2460, 2239, 10],
  // 2) U1.3 (2992, 2012.8) -> U1.5 (2992, 2075.7) directly inside U1 courtyard on L1 (zero vias!)
  [1, 2992, 2012.8, 2930, 2012.8, 10],
  [1, 2930, 2012.8, 2930, 2075.7, 10],
  [1, 2930, 2075.7, 2992, 2075.7, 10],
  // 3) Y1.2 + Y1.4 (joined at (3190, 1900)) -> C4.2 (3100, 2177.6) on L1 (11.2+ mil clearance!)
  [1, 3190, 1900, 3190, 2177.6, 10],
  [1, 3190, 2177.6, 3100, 2177.6, 10],
  // 4) J_USB A1B12 (1257.4, 2626) -> right of slot -> Top GND pour at (1255, 2715)
  [1, 1257.4, 2626, 1280, 2626, 8],
  [1, 1280, 2626, 1280, 2685, 8],
  [1, 1280, 2685, 1255, 2715, 8],
  // 5) J_USB B1A12 (1257.4, 2374) -> right of slot -> Top GND pour at (1240, 2260)
  [1, 1257.4, 2374, 1274, 2374, 6],
  [1, 1274, 2374, 1274, 2290, 6],
  [1, 1274, 2290, 1240, 2260, 6]
];

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

const delVias = ${JSON.stringify(delVias)};
const delLines = ${JSON.stringify(delLines)};
const addVias = ${JSON.stringify(addVias)};
const addLines = ${JSON.stringify(addLines)};

const dV = await eda.pcb_PrimitiveVia.delete(delVias);
const dL = await eda.pcb_PrimitiveLine.delete(delLines);

// Add Via BEFORE Lines
for (const [x, y] of addVias) {
  await eda.pcb_PrimitiveVia.create("GND", x, y, 12, 22, 0, null, false);
}
for (const [lyr, x1, y1, x2, y2, w] of addLines) {
  await eda.pcb_PrimitiveLine.create("GND", lyr, x1, y1, x2, y2, w, false);
}

// Rebuild Top & Bottom copper pours
await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
await sleep(3000);
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
const saved = await eda.pcb_Document.save();
await sleep(4000);

// Capture CLEAN PNG BEFORE running DRC!
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

// Verify ratlines === 0 & full PCB DRC
const rl = await eda.pcb_Document.getRatlines();
const drcBool = await eda.pcb_Drc.check(true, false, false);
const drcVerbose = await eda.pcb_Drc.check(true, false, true);

const finalLines = (await eda.pcb_PrimitiveLine.getAll()).length;
const finalVias = (await eda.pcb_PrimitiveVia.getAll()).length;

return { dV, dL, saved, finalLines, finalVias, ratlinesTotal: rl?.total ?? null, ratlinesItems: rl?.items || [], drcBool, drcVerbose, png };
`;

const tmp = `${base}/outputs/.tmp_polish_4d.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`Step 4d Polish done in ${((Date.now() - t0) / 1000).toFixed(1)}s (saved=${d.saved}, lines=${d.finalLines}, vias=${d.finalVias})`);
console.log(`Ratlines remaining: ${d.ratlinesTotal}`);
console.log(`DRC strict pass: ${d.drcBool}, categories: ${(d.drcVerbose || []).length}`);
for (const cat of (d.drcVerbose || [])) {
  for (const sub of (cat.list || [])) {
    console.log(`  DRC: ${cat.name} / ${sub.name} (${(sub.list || []).length})`);
    for (const it of (sub.list || []).slice(0, 8)) {
      console.log("   ", it.layer, (it.obj1?.typeName || "") + (it.obj1?.suffix || ""), it.obj2 ? "<-> " + (it.obj2.typeName || "") + (it.obj2.suffix || "") : "", it.explanation?.param?.minDistance || it.explanation?.str || "");
    }
  }
}
if (d.png?.b64) {
  fs.writeFileSync(`${base}/outputs/unomodoki_pcb_final.png`, Buffer.from(d.png.b64, "base64"));
  console.log("Updated outputs/unomodoki_pcb_final.png:", d.png.size, "bytes");
}
fs.writeFileSync(`${base}/outputs/step4_final_drc.json`, JSON.stringify({
  saved: d.saved,
  finalLines: d.finalLines,
  finalVias: d.finalVias,
  ratlinesTotal: d.ratlinesTotal,
  ratlinesItems: d.ratlinesItems,
  drcBool: d.drcBool,
  drcVerbose: d.drcVerbose
}, null, 2));
