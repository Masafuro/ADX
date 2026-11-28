import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

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

// 1. Delete the 10/20 via (fa12bb681d91b488) and the 2 short tracks touching (3020, 2075.7)
const linesBefore = await eda.pcb_PrimitiveLine.getAll();
const delLines = [];
for (const l of linesBefore) {
  if (l.getState_Net() !== "GND") continue;
  const sx = Math.round(l.getState_StartX()), sy = Math.round(l.getState_StartY());
  const ex = Math.round(l.getState_EndX()), ey = Math.round(l.getState_EndY());
  if ((sx === 3020 && sy === 2076) || (ex === 3020 && ey === 2076)) {
    delLines.push(l.getState_PrimitiveId());
  }
}
if (delLines.length) await eda.pcb_PrimitiveLine.delete(delLines);
await eda.pcb_PrimitiveVia.delete(["fa12bb681d91b488"]);

// 2. Create BOTH 12/22 GND vias BEFORE tracks:
//    - (3018, 2075.7) [12/22]: replaces 10/20 via (11.5+ mil clearance to XTAL1 & +5V!)
//    - (3320, 2450)   [12/22]: stitches the wide open L2 x=3200..3400 GND strip directly to the open Top GND pour (~80 mil clearance!)
//    - (3320, 2220)   [12/22]: second redundant stitch via in the same open corridor for low-impedance crystal return!
await eda.pcb_PrimitiveVia.create("GND", 3018, 2075.7, 12, 22, 0, null, false);
await eda.pcb_PrimitiveVia.create("GND", 3320, 2450,   12, 22, 0, null, false);
await eda.pcb_PrimitiveVia.create("GND", 3320, 2220,   12, 22, 0, null, false);

// 3. Re-create the 2 tracks touching (3018, 2075.7) + explicit L2 bridge (3190,2030)->(3320,2160)->(3320,2220)
await eda.pcb_PrimitiveLine.create("GND", 1, 2992, 2075.7, 3018, 2075.7, 8, false);
await eda.pcb_PrimitiveLine.create("GND", 2, 3018, 2075.7, 3015, 2070,   8, false);
await eda.pcb_PrimitiveLine.create("GND", 2, 3190, 2030,   3320, 2160,   12, false);
await eda.pcb_PrimitiveLine.create("GND", 2, 3320, 2160,   3320, 2220,   12, false);

// 4. Rebuild copper pours & save FIRST (safe checkpoint!)
await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
await sleep(3000);
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
const saved1 = await eda.pcb_Document.save();
await sleep(3000);

// 5. Check ratlines & DRC before importChanges
const rl1 = await eda.pcb_Document.getRatlines();
const drc1Bool = await eda.pcb_Drc.check(true, false, false);
const drc1Verbose = await eda.pcb_Drc.check(true, false, true);

// 6. If ONLY Netlist Error remains (0 ratlines, 0 clearance/physical/connection errors), sync designRule via importChanges(undefined, true)
const cats1 = (drc1Verbose || []).map(c => c.name);
let didImport = false, importOk = null, saved2 = null, rl2 = rl1, drc2Bool = drc1Bool, drc2Verbose = drc1Verbose;
if ((rl1?.total ?? -1) === 0 && cats1.length === 1 && cats1[0] === "Netlist Error") {
  didImport = true;
  importOk = await eda.pcb_Document.importChanges(undefined, true);
  await sleep(3000);
  await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
  await sleep(3000);
  try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
  saved2 = await eda.pcb_Document.save();
  await sleep(4000);
  rl2 = await eda.pcb_Document.getRatlines();
  drc2Bool = await eda.pcb_Drc.check(true, false, false);
  drc2Verbose = await eda.pcb_Drc.check(true, false, true);
}

// 7. Capture CLEAN PNG (note: if drc2Bool === true, zero yellow X markers exist anyway!)
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
await sleep(1000);
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

const finalLines = (await eda.pcb_PrimitiveLine.getAll()).length;
const finalVias = (await eda.pcb_PrimitiveVia.getAll()).length;
const finalComps = (await eda.pcb_PrimitiveComponent.getAll()).length;
const finalPads = (await eda.pcb_PrimitivePad.getAll()).length;

return {
  delLinesCount: delLines.length,
  saved1, rl1Total: rl1?.total ?? null, drc1Bool, cats1,
  didImport, importOk, saved2,
  finalLines, finalVias, finalComps, finalPads,
  ratlinesTotal: rl2?.total ?? null,
  ratlinesItems: rl2?.items || [],
  drcBool: drc2Bool,
  drcVerbose: drc2Verbose,
  png
};
`;

const tmp = `${base}/outputs/.tmp_polish_4f.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`Step 4f Final Zero done in ${((Date.now() - t0) / 1000).toFixed(1)}s`);
console.log(`Phase1: saved=${d.saved1}, rl=${d.rl1Total}, drcBool=${d.drc1Bool}, cats=${JSON.stringify(d.cats1)}`);
console.log(`ImportChanges: did=${d.didImport}, ok=${d.importOk}, saved2=${d.saved2}`);
console.log(`FINAL: comps=${d.finalComps}, pads=${d.finalPads}, lines=${d.finalLines}, vias=${d.finalVias}`);
console.log(`FINAL Ratlines: ${d.ratlinesTotal}, DRC strict PASS: ${d.drcBool}, categories: ${(d.drcVerbose || []).length}`);
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
  saved1: d.saved1,
  didImport: d.didImport,
  importOk: d.importOk,
  saved2: d.saved2,
  finalComps: d.finalComps,
  finalPads: d.finalPads,
  finalLines: d.finalLines,
  finalVias: d.finalVias,
  ratlinesTotal: d.ratlinesTotal,
  ratlinesItems: d.ratlinesItems,
  drcBool: d.drcBool,
  drcVerbose: d.drcVerbose
}, null, 2));
