const queries = [
  { key: "LED_0603_G", q: "KT-0603G" },
  { key: "LED_0603_Y", q: "KT-0603Y" },
  { key: "LED_0603_B", q: "KT-0603B" },
  { key: "C_12P_0603", q: "12pF 0603 C0G" },
  { key: "HDR_1X10F",  q: "Female Header 1x10P 2.54" },
  { key: "HDR_1X8F",   q: "Female Header 1x8P 2.54" },
  { key: "HDR_1X6F",   q: "Female Header 1x6P 2.54" },
  { key: "HDR_2X3M",   q: "Pin Header 2x3P 2.54" },
  { key: "HDR_1X10F_CN", q: "2.54mm 1*10P 排母" },
  { key: "HDR_1X8F_CN",  q: "2.54mm 1*8P 排母" },
  { key: "HDR_1X6F_CN",  q: "2.54mm 1*6P 排母" },
  { key: "HDR_2X3M_CN",  q: "2.54mm 2*3P 排针" }
];

const out = {};
for (const item of queries) {
  try {
    const res = await eda.lib_Device.search(item.q, undefined, undefined, undefined, 8, 1);
    out[item.key] = (res || []).slice(0, 8).map((d) => {
      const op = d.otherProperty || {};
      return {
        name: d.name,
        lcsc: op["Supplier Part"] || d.lcscId || null,
        cls: op["JLCPCB Part Class"] || null,
        pkg: op["Supplier Footprint"] || op["Package"] || d.footprintName || null,
        desc: (op["Description"] || d.description || "").slice(0, 60)
      };
    });
  } catch (e) {
    out[item.key] = { error: String(e?.message || e) };
  }
}
return out;
