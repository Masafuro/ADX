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

try { await eda.pcb_Document.zoomToBoardOutline(); } catch {}
const saved = await eda.pcb_Document.save();
await sleep(3000);

const out = { saved };

try {
  const gerber = await eda.pcb_ManufactureData.getGerberFile("unomodoki_gerber");
  out.gerber = gerber ? { name: gerber.name, size: gerber.size, b64: await toB64(gerber) } : null;
} catch (e) { out.gerber = { error: String(e?.message || e) }; }

try {
  const cpl = await eda.pcb_ManufactureData.getPickAndPlaceFile("unomodoki_cpl", "xlsx", "mm");
  out.cpl = cpl ? { name: cpl.name, size: cpl.size, b64: await toB64(cpl) } : null;
} catch (e) { out.cpl = { error: String(e?.message || e) }; }

try {
  const bom = await eda.pcb_ManufactureData.getBomFile("unomodoki_bom_pcb", "csv");
  out.bom = bom ? { name: bom.name, size: bom.size, b64: await toB64(bom) } : null;
} catch (e) { out.bom = { error: String(e?.message || e) }; }

try {
  const img = await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
  out.png = img ? { size: img.size, b64: await toB64(img) } : null;
} catch (e) { out.png = { error: String(e?.message || e) }; }

return out;
`;

const tmp = `${base}/outputs/.tmp_export_mfg.js`;
fs.writeFileSync(tmp, js);
const t0 = Date.now();
const raw = JSON.parse(execFileSync("node", [helper, "invoke", "--session", sid, "--file", tmp, "--timeout", "120000"], { encoding: "utf8" }));
fs.unlinkSync(tmp);
const d = raw.data?.result ?? raw.data ?? raw;

console.log(`U5 Mfg Export done in ${((Date.now() - t0) / 1000).toFixed(1)}s (saved=${d.saved})`);
const manifest = { saved: d.saved, files: {} };

if (d.gerber?.b64) {
  const p = `${base}/outputs/unomodoki_gerber.zip`;
  fs.writeFileSync(p, Buffer.from(d.gerber.b64, "base64"));
  console.log(`Gerber: ${p} (${d.gerber.size} bytes, orig=${d.gerber.name})`);
  manifest.files.gerber = { path: "outputs/unomodoki_gerber.zip", origName: d.gerber.name, size: d.gerber.size };
} else {
  console.error("Gerber export failed:", d.gerber);
  manifest.files.gerber = d.gerber;
}

if (d.cpl?.b64) {
  const buf = Buffer.from(d.cpl.b64, "base64");
  const isZipXlsx = buf[0] === 0x50 && buf[1] === 0x4b;
  const p = `${base}/outputs/unomodoki_cpl${isZipXlsx ? ".xlsx" : ".csv"}`;
  fs.writeFileSync(p, buf);
  console.log(`CPL:    ${p} (${buf.length} bytes, orig=${d.cpl.name}, xlsxMagic=${isZipXlsx})`);
  manifest.files.cpl = { path: isZipXlsx ? "outputs/unomodoki_cpl.xlsx" : "outputs/unomodoki_cpl.csv", origName: d.cpl.name, size: buf.length };
} else {
  console.error("CPL export failed:", d.cpl);
  manifest.files.cpl = d.cpl;
}

if (d.bom?.b64) {
  const buf = Buffer.from(d.bom.b64, "base64");
  const isUtf16 = buf.length >= 2 && buf[0] === 0xff && buf[1] === 0xfe;
  const text = isUtf16 ? buf.slice(2).toString("utf16le") : buf.toString("utf8");
  const p = `${base}/outputs/unomodoki_bom_pcb.csv`;
  fs.writeFileSync(p, text, "utf8");
  console.log(`BOM:    ${p} (${Buffer.byteLength(text)} bytes UTF-8, decodedUtf16=${isUtf16})`);
  manifest.files.bomPcb = { path: "outputs/unomodoki_bom_pcb.csv", origName: d.bom.name, size: Buffer.byteLength(text), decodedUtf16: isUtf16 };
} else {
  console.log("PCB BOM note:", d.bom);
}

if (d.png?.b64) {
  const p = `${base}/outputs/unomodoki_pcb_final_clean.png`;
  fs.writeFileSync(p, Buffer.from(d.png.b64, "base64"));
  console.log(`PNG:    ${p} (${d.png.size} bytes)`);
  manifest.files.pngClean = { path: "outputs/unomodoki_pcb_final_clean.png", size: d.png.size };
}

fs.writeFileSync(`${base}/outputs/step5_mfg_manifest.json`, JSON.stringify(manifest, null, 2));
