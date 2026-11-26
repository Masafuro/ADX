// M4 PCB PoC Step 1: Open PCB1 of Agent_Hello_Circuit, import schematic changes,
// create a clean 1000x800 mil (25.4x20.32 mm) BoardOutline, place all 5 0603 parts
// mirroring the schematic layout, and return exact pad coordinates & nets.

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const snap = (v) => Math.round(v * 100) / 100;

const pcbs = await eda.dmt_Pcb.getAllPcbsInfo();
if (!pcbs || pcbs.length === 0) {
  throw new Error("No PCB document found in current project");
}
const pcb = pcbs[0];
const tabId = await eda.dmt_EditorControl.openDocument(pcb.uuid);
await sleep(2500);

// Import schematic changes automatically without interactive modal (force = true)
const importOk = await eda.pcb_Document.importChanges(undefined, true);
await sleep(3000);

// Remove any auto-generated board outline polylines/lines on layer 11 so we have one clean outline
const BOARD_OUTLINE = 11;
let removedOutlines = 0;
try {
  const existingPolys = await eda.pcb_PrimitivePolyline.getAll(undefined, BOARD_OUTLINE);
  if (existingPolys?.length) {
    for (const p of existingPolys) {
      await eda.pcb_PrimitivePolyline.delete(p.getState_PrimitiveId());
      removedOutlines++;
    }
  }
} catch {}
try {
  const existingLines = await eda.pcb_PrimitiveLine.getAll(undefined, BOARD_OUTLINE);
  if (existingLines?.length) {
    for (const l of existingLines) {
      await eda.pcb_PrimitiveLine.delete(l.getState_PrimitiveId());
      removedOutlines++;
    }
  }
} catch {}

// Create 1000 x 800 mil (25.4 x 20.32 mm) closed rectangular BoardOutline at (1000,1000)-(2000,1800)
const X0 = 1000, Y0 = 1000, W = 1000, H = 800;
const X1 = X0 + W, Y1 = Y0 + H;
const outlinePoly = eda.pcb_MathPolygon.createPolygon([
  X0, Y0, "L", X1, Y0, X1, Y1, X0, Y1, X0, Y0,
]);
const outlinePrim = await eda.pcb_PrimitivePolyline.create("", BOARD_OUTLINE, outlinePoly, 10, false);

// Place 5 components cleanly on Top layer (layer = 1), mirroring schematic layout
const targets = {
  R1:   { x: 1300, y: 1550, rotation: 90 },
  R2:   { x: 1300, y: 1250, rotation: 90 },
  C1:   { x: 1500, y: 1250, rotation: 90 },
  R3:   { x: 1700, y: 1550, rotation: 90 },
  LED1: { x: 1700, y: 1250, rotation: 90 },
};

const comps = await eda.pcb_PrimitiveComponent.getAll();
const placed = [];

for (const comp of comps) {
  const id = comp.getState_PrimitiveId();
  const ref = comp.getState_Designator();
  const t = targets[ref];
  if (t) {
    await eda.pcb_PrimitiveComponent.modify(id, {
      layer: 1,
      x: t.x,
      y: t.y,
      rotation: t.rotation,
    });
  }
}

await sleep(1500);

// Inspect pads and flip rotation by 180 deg if needed so desired top net sits at higher Y
const desiredTopNet = {
  R1:   "VCC",
  R2:   "$1N2",
  C1:   "$1N2",
  R3:   "VCC",
  LED1: "$1N3",
};

const compsAfter = await eda.pcb_PrimitiveComponent.getAll();
for (const comp of compsAfter) {
  const id = comp.getState_PrimitiveId();
  const ref = comp.getState_Designator();
  let pins = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(id);
  const readPins = (arr) =>
    arr.map((p) => ({
      number: String(p.getState_PadNumber?.() ?? p.padNumber ?? p.pinNumber ?? ""),
      net: String(p.getState_Net?.() ?? p.net ?? ""),
      x: snap(p.getState_X?.() ?? p.x ?? 0),
      y: snap(p.getState_Y?.() ?? p.y ?? 0),
    }));

  let pList = readPins(pins);
  const wantTop = desiredTopNet[ref];
  if (wantTop && pList.length === 2) {
    const topP = pList[0].y >= pList[1].y ? pList[0] : pList[1];
    if (topP.net !== wantTop && pList.some((p) => p.net === wantTop)) {
      const curRot = comp.getState_Rotation?.() ?? targets[ref]?.rotation ?? 90;
      const newRot = (curRot + 180) % 360;
      await eda.pcb_PrimitiveComponent.modify(id, { rotation: newRot });
      await sleep(300);
      pins = await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(id);
      pList = readPins(pins);
    }
  }

  const finalComp = await eda.pcb_PrimitiveComponent.get(id);
  placed.push({
    ref,
    id,
    x: snap(finalComp.getState_X?.() ?? 0),
    y: snap(finalComp.getState_Y?.() ?? 0),
    rotation: finalComp.getState_Rotation?.() ?? 0,
    layer: finalComp.getState_Layer?.() ?? 1,
    pads: pList,
  });
}

try {
  await eda.pcb_Document.zoomToBoardOutline();
} catch {}
await eda.pcb_Document.save();

return {
  pcb: { name: pcb.name, uuid: pcb.uuid },
  tabId,
  importOk,
  removedOutlines,
  outlineId: outlinePrim?.getState_PrimitiveId?.() ?? null,
  boardRectMil: { X0, Y0, X1, Y1, W, H },
  placed,
};
