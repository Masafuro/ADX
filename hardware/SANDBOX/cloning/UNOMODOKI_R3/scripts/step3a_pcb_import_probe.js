const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const snap = (v) => Math.round(v * 100) / 100;

const pcbs = await eda.dmt_Pcb.getAllPcbsInfo();
if (!pcbs || pcbs.length === 0) throw new Error("No PCB found");
const pcb = pcbs[0];
await eda.dmt_EditorControl.openDocument(pcb.uuid);
await sleep(3000);

const importOk = await eda.pcb_Document.importChanges(undefined, true);
await sleep(4000);

const comps = await eda.pcb_PrimitiveComponent.getAll() || [];
const probe = [];
for (const c of comps) {
  const id = c.getState_PrimitiveId();
  const ref = c.getState_Designator();
  const cx = snap(c.getState_X?.() ?? 0);
  const cy = snap(c.getState_Y?.() ?? 0);
  const rot = c.getState_Rotation?.() ?? 0;
  const pins = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  const pList = (pins || []).map((p) => ({
    num: String(p.getState_PadNumber?.() ?? p.padNumber ?? p.pinNumber ?? ""),
    net: String(p.getState_Net?.() ?? p.net ?? ""),
    x: snap(p.getState_X?.() ?? p.x ?? 0),
    y: snap(p.getState_Y?.() ?? p.y ?? 0),
    dx: snap((p.getState_X?.() ?? p.x ?? 0) - cx),
    dy: snap((p.getState_Y?.() ?? p.y ?? 0) - cy)
  }));
  probe.push({ ref, id, x: cx, y: cy, rot, pinCount: pList.length, pins: pList });
}
await eda.pcb_Document.save();
return { pcbName: pcb.name, importOk, compCount: probe.length, probe };
