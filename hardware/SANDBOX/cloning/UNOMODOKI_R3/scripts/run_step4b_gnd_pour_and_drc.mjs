import fs from "node:fs";
import { execFileSync } from "node:child_process";

const helper = process.argv[2];
const base = process.argv[3];
const sid = process.argv[4];

const OX = 1000, OY = 1000;
// 20+ mil inset polygon matching the 9-vertex UNO R3 board outline
const pourPts = [
  [20, 20], [2580, 20], [2580, 110], [2680, 210],
  [2680, 1480], [2580, 1580], [2580, 2030], [2530, 2080], [20, 2080]
];
const pourArr = [OX + pourPts[0][0], OY + pourPts[0][1], "L"];
for (let i = 1; i < pourPts.length; i++) {
  pourArr.push(OX + pourPts[i][0], OY + pourPts[i][1]);
}
pourArr.push(OX + pourPts[0][0], OY + pourPts[0][1]);

const js = `
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

// Remove any existing pours for idempotency
try {
  const oldPours = await eda.pcb_PrimitivePour.getAll() || [];
  if (oldPours.length) await eda.pcb_PrimitivePour.delete(oldPours.map(p => p.getState_PrimitiveId()));
} catch {}

const pourPoly = eda.pcb_MathPolygon.createPolygon(${JSON.stringify(pourArr)});
const topPour = await eda.pcb_PrimitivePour.create("GND", 1, pourPoly, undefined, false, "GND_TOP", 0, 10, false);
const botPour = await eda.pcb_PrimitivePour.create("GND", 2, pourPoly, undefined, false, "GND_BOT", 0, 10, false);

await eda.pcb_PrimitivePour.rebuildAllCopperRegions();
await sleep(3000);
try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
await eda.pcb_Document.save();
await sleep(4000);

// Capture CLEAN final PNG before calling full DRC
let png = null;
try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  if (img) png = { size: img.size, b64: await toB64(img) };
} catch (e) { png = { error: String(e?.message || e) }; }

const rl = await eda.pcb_Document.getRatlines();
const drcBool = await eda.pcb_Drc.check(true, false, false);
const drcVerbose = await eda.pcb_Drc.check(true, false, true);

return {
  topPourId: topPour?.getState_PrimitiveId?.() ?? null,
  botPourId: botPour?.getState_PrimitiveId?.() ?? null,
  ratlinesTotal: rl?.total ?? null,
  ratlinesItems: (rl?.items || []).slice(0, 20),
  drcBool,
  drcVerbose,
  png
};
`;

const tmp = `${base}/outputs/.tmp_gnd_pour.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`GND Pour & DRC done in ${((Date.now() - t0) / 1000).toFixed(1)}s`);
console.log(`Ratlines remaining: ${d.ratlinesTotal} (was 32 GND!)`);
console.log(`DRC strict pass: ${d.drcBool}, categories: ${(d.drcVerbose || []).length}`);
for (const cat of (d.drcVerbose || [])) {
  for (const sub of (cat.list || [])) {
    console.log(`  DRC: ${cat.name} / ${sub.name} (${(sub.list || []).length})`);
    for (const it of (sub.list || []).slice(0, 5)) {
      console.log("   ", it.layer, it.obj1?.typeName + it.obj1?.suffix, it.obj2 ? "<-> " + it.obj2.typeName + it.obj2.suffix : "", it.explanation?.param?.minDistance || it.explanation?.str || "");
    }
  }
}
if ((d.ratlinesItems || []).length > 0) {
  console.log("Remaining ratlines sample:", JSON.stringify(d.ratlinesItems.slice(0, 10)));
}
if (d.png?.b64) {
  fs.writeFileSync(`${base}/outputs/unomodoki_pcb_final.png`, Buffer.from(d.png.b64, "base64"));
  console.log("Wrote outputs/unomodoki_pcb_final.png:", d.png.size, "bytes");
}
fs.writeFileSync(`${base}/outputs/step4_final_drc.json`, JSON.stringify({
  ratlinesTotal: d.ratlinesTotal,
  ratlinesItems: d.ratlinesItems,
  drcBool: d.drcBool,
  drcVerbose: d.drcVerbose
}, null, 2));
