const byId = ["C54973826", "C5116479", "C390688", "C965804", "C12624", "C2287", "C2286"];
const idResults = [];
for (const id of byId) {
  try {
    const d = await eda.lib_Device.getByLcscIds(id);
    const op = d?.otherProperty || {};
    idResults.push({
      lcsc: id,
      found: !!d,
      name: d?.name,
      cls: op["JLCPCB Part Class"],
      pkg: op["Supplier Footprint"] || op["Package"] || d?.footprintName,
      mfr: op["Manufacturer"]
    });
  } catch (e) {
    idResults.push({ lcsc: id, found: false, error: String(e?.message || e) });
  }
}

const searches = [
  { key: "F10", q: "PM2.54-1*10" },
  { key: "F10b", q: "DS1023-01-1*10" },
  { key: "F10c", q: "H8.5 1x10P 2.54" },
  { key: "F8",  q: "PM2.54-1*8" },
  { key: "F8b", q: "H8.5 1x8P 2.54" },
  { key: "F6",  q: "PM2.54-1*6" },
  { key: "F6b", q: "H8.5 1x6P 2.54" },
  { key: "M2x3", q: "2.54-2*3" }
];
const searchResults = {};
for (const s of searches) {
  try {
    const arr = await eda.lib_Device.search(s.q, undefined, undefined, undefined, 6, 1);
    searchResults[s.key] = (arr || []).slice(0, 6).map((d) => {
      const op = d.otherProperty || {};
      return {
        lcsc: op["Supplier Part"] || null,
        name: d.name,
        cls: op["JLCPCB Part Class"] || null,
        pkg: op["Supplier Footprint"] || op["Package"] || d.footprintName || null,
        has3d: !!d.model3dUuid
      };
    });
  } catch (e) {
    searchResults[s.key] = { error: String(e?.message || e) };
  }
}
return { idResults, searchResults };
