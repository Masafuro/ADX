import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

// Pinpoint fixes for the 6 isolated GND islands found after Top+Bottom copper pour:
// Remember rule: create Vias BEFORE Tracks so EasyEDA Pro binds Top<->Bottom cleanly!
const gndVias = [
  // 1) C5_2 (2510, 2237.6) & C6_2 (2510, 2127.6) -> stitching vias to Bottom GND plane
  [2510, 2285],
  [2460, 2127.6],
  // 2) R2_2 (1469.7, 2480) & C2_2 (1587.6, 2460) -> stitching vias to Bottom GND plane
  [1469.7, 2430],
  [1635, 2460],
  // 3) Y1_2 (3233.3, 1866.5) & Y1_4 (3146.7, 1933.5) -> center GND shield via right inside Y1!
  [3190, 1900],
  // 4) U1_3 (2992, 2012.8) -> stitching via just inside TQFP-32 lower-right void
  [2925, 2012.8],
  // Plus a few general board-perimeter GND stitching vias for low-impedance return plane
  [1200, 1900],
  [2000, 1500],
  [3450, 1550]
];

const gndTracks = [
  // J_USB B1A12 -> J_USB.1 (already TH grounded)
  [1, 1257.4, 2374.0, 1227.2, 2374.0, 12],
  [1, 1227.2, 2374.0, 1227.2, 2329.7, 12],
  // J_USB A1B12 -> J_USB.4 (already TH grounded)
  [1, 1257.4, 2626.0, 1227.2, 2626.0, 12],
  [1, 1227.2, 2626.0, 1227.2, 2670.3, 12],
  // Connect each isolated SMD GND pad straight to its stitching via
  [1, 2510, 2237.6, 2510, 2285, 10],
  [1, 2510, 2127.6, 2460, 2127.6, 10],
  [1, 1469.7, 2480, 1469.7, 2430, 10],
  [1, 1587.6, 2460, 1635, 2460, 10],
  [1, 3233.3, 1866.5, 3190, 1900, 10],
  [1, 3146.7, 1933.5, 3190, 1900, 10],
  [1, 2992, 2012.8, 2925, 2012.8, 10]
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

// 1. Add GND stitching vias FIRST (hole 12mil, dia 22mil, through=0, blind=null)
const gndVias = ${JSON.stringify(gndVias)};
const gndTracks = ${JSON.stringify(gndTracks)};
let vOk = 0, tOk = 0;
for (const [x, y] of gndVias) {
  await eda.pcb_PrimitiveVia.create("GND", x, y, 12, 22, 0, null, false);
  vOk++;
}
for (const [lyr, x1, y1, x2, y2, w] of gndTracks) {
  await eda.pcb_PrimitiveLine.create("GND", lyr, x1, y1, x2, y2, w, false);
  tOk++;
}

// 2. Rebuild Top & Bottom copper pours so all stitching vias merge seamlessly
await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
await sleep(3000);
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
await eda.pcb_Document.save();
await sleep(4000);

// 3. Capture CLEAN final PCB PNG BEFORE running full DRC!
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

// 4. Verify ratlines === 0 and PCB DRC!
const rl = await eda.pcb_Document.getRatlines();
const drcBool = await eda.pcb_Drc.check(true, false, false);
const drcVerbose = await eda.pcb_Drc.check(true, false, true);

return { vOk, tOk, ratlinesTotal: rl?.total ?? null, ratlinesItems: rl?.items || [], drcBool, drcVerbose, png };
`;

const tmp = `${base}/outputs/.tmp_stitch_finish.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`GND Stitch & Final DRC done in ${((Date.now() - t0) / 1000).toFixed(1)}s (vias=${d.vOk}, tracks=${d.tOk})`);
console.log(`Ratlines remaining: ${d.ratlinesTotal}`);
console.log(`DRC strict pass: ${d.drcBool}, categories: ${(d.drcVerbose || []).length}`);
for (const cat of (d.drcVerbose || [])) {
  for (const sub of (cat.list || [])) {
    console.log(`  DRC: ${cat.name} / ${sub.name} (${(sub.list || []).length})`);
    for (const it of (sub.list || []).slice(0, 6)) {
      console.log("   ", it.layer, it.obj1?.typeName + it.obj1?.suffix, it.obj2 ? "<-> " + it.obj2.typeName + it.obj2.suffix : "", it.explanation?.param?.minDistance || it.explanation?.str || "");
    }
  }
}
if (d.png?.b64) {
  fs.writeFileSync(`${base}/outputs/unomodoki_pcb_final.png`, Buffer.from(d.png.b64, "base64"));
  console.log("Updated outputs/unomodoki_pcb_final.png:", d.png.size, "bytes");
}
fs.writeFileSync(`${base}/outputs/step4_final_drc.json`, JSON.stringify({
  ratlinesTotal: d.ratlinesTotal,
  ratlinesItems: d.ratlinesItems,
  drcBool: d.drcBool,
  drcVerbose: d.drcVerbose
}, null, 2));
