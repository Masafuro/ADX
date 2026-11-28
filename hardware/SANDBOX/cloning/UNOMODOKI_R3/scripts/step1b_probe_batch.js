// Generic small-batch pinout prober: expects args = { items: [...] }
const obj = typeof args === "string" ? JSON.parse(args) : (args || {});
const list = obj.items || [];
const out = [];
for (const item of list) {
  try {
    const dev = await eda.lib_Device.getByLcscIds(item.lcsc);
    if (!dev) {
      out.push({ ...item, found: false });
      continue;
    }
    const op = dev.otherProperty || {};
    const refObj = { libraryType: "3", libraryUuid: dev.libraryUuid, uuid: dev.uuid };
    const comp = await eda.sch_PrimitiveComponent.create(refObj, 500, 450, undefined, 0, false, true, true);
    const pid = comp.getState_PrimitiveId();
    const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(pid);
    const pinList = (pins || []).map((p) => ({
      number: String(p.pinNumber ?? p.getState_PinNumber?.() ?? ""),
      name: String(p.pinName ?? p.getState_PinName?.() ?? ""),
      dx: Math.round(((p.x ?? p.getState_X?.() ?? 0) - 500) * 100) / 100,
      dy: Math.round(((p.y ?? p.getState_Y?.() ?? 0) - 450) * 100) / 100
    }));
    await eda.sch_PrimitiveComponent.delete(pid);
    out.push({
      ...item,
      found: true,
      name: dev.name,
      uuid: dev.uuid,
      libraryUuid: dev.libraryUuid,
      jlcpcbClass: op["JLCPCB Part Class"] || null,
      manufacturer: op["Manufacturer"] || null,
      mpn: op["Manufacturer Part"] || null,
      package: op["Supplier Footprint"] || op["Package"] || null,
      has3dModel: !!(dev.model3dUuid || op["3D Model"] || op["3D Model Title"]),
      pinCount: pinList.length,
      pins: pinList
    });
  } catch (e) {
    out.push({ ...item, found: false, error: String(e?.message || e) });
  }
}
await eda.sch_Document.save();
return out;
