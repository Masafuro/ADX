import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

// Final 2 GND islands -> 0 ratlines, 0 clearance errors:
// 1) Replace vertical L1 segment (3190,1900)->(3190,2177.6) with via (3190,2030) FIRST + split segments
//    -> connects Y1.2 + Y1.4 + C4.2 directly to MAIN L2 GND plane (12.5 mil clearance!)
// 2) Connect U1.5/U1.3 via (3020,2075.7)[dia=20] -> L2 (3015,1910) -> via (3015,1910)[dia=22] -> L1 (3190,1910)
//    -> joins U1.3 + U1.5 into the exact same MAIN GND spine (10.5..35 mil clearance!)
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

// Find existing vertical line (3190, 1900) -> (3190, 2177.6) so we recreate it AFTER placing the via
const allLines = await eda.pcb_PrimitiveLine.getAll();
const toDel = [];
for (const l of allLines) {
  if (l.getState_Net() === "GND" && l.getState_Layer() === 1) {
    const sx = Math.round(l.getState_StartX()), sy = Math.round(l.getState_StartY());
    const ex = Math.round(l.getState_EndX()), ey = Math.round(l.getState_EndY());
    if ((sx === 3190 && ex === 3190 && ((sy === 1900 && ey === 2178) || (sy === 2178 && ey === 1900)))) {
      toDel.push(l.getState_PrimitiveId());
    }
  }
}
if (toDel.length) await eda.pcb_PrimitiveLine.delete(toDel);

// Create all 3 GND vias BEFORE tracks (EasyEDA Pro binding rule!)
await eda.pcb_PrimitiveVia.create("GND", 3190, 2030,   12, 22, 0, null, false);
await eda.pcb_PrimitiveVia.create("GND", 3020, 2075.7, 10, 20, 0, null, false);
await eda.pcb_PrimitiveVia.create("GND", 3015, 1910,   12, 22, 0, null, false);

// Create connecting tracks
const tracks = [
  // Split Y1 -> C4.2 vertical spine cleanly through via (3190, 2030)
  [1, 3190, 1900,   3190, 2030,   10],
  [1, 3190, 2030,   3190, 2177.6, 10],
  // U1.5 (2992, 2075.7) -> via (3020, 2075.7) -> L2 -> via (3015, 1910) -> L1 -> (3190, 1910)
  [1, 2992, 2075.7, 3020, 2075.7, 8],
  [2, 3020, 2075.7, 3015, 2070,   8],
  [2, 3015, 2070,   3015, 1910,   8],
  [1, 3015, 1910,   3190, 1910,   10]
];
for (const [lyr, x1, y1, x2, y2, w] of tracks) {
  await eda.pcb_PrimitiveLine.create("GND", lyr, x1, y1, x2, y2, w, false);
}

// Rebuild Top & Bottom copper pours
await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
await sleep(3000);
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
const saved = await eda.pcb_Document.save();
await sleep(4000);

// Capture CLEAN PNG BEFORE running DRC (no yellow X markers!)
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

// Verify ratlines === 0 and PCB DRC
const rl = await eda.pcb_Document.getRatlines();
const drcBool = await eda.pcb_Drc.check(true, false, false);
const drcVerbose = await eda.pcb_Drc.check(true, false, true);
const finalLines = (await eda.pcb_PrimitiveLine.getAll()).length;
const finalVias = (await eda.pcb_PrimitiveVia.getAll()).length;

return { deletedVert: toDel.length, saved, finalLines, finalVias, ratlinesTotal: rl?.total ?? null, ratlinesItems: rl?.items || [], drcBool, drcVerbose, png };
`;

const tmp = `${base}/outputs/.tmp_polish_4e.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`Step 4e Zero-Ratline Finish done in ${((Date.now() - t0) / 1000).toFixed(1)}s (saved=${d.saved}, lines=${d.finalLines}, vias=${d.finalVias})`);
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
