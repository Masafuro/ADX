// U1 Step 0: Look up all candidate LCSC parts via EasyEDA Pro API (LIB_Device.getByLcscIds)
// to verify active status, JLCPCB Basic/Extended class, stock/name, and footprint/symbol metadata.
// Safe read-only query script — does NOT modify any project or schematic.

const candidates = [
  // Core ICs & Connectors
  { role: "MCU",        lcsc: "C14877",   desc: "ATmega328P-AU (TQFP-32)" },
  { role: "USB_UART",   lcsc: "C84681",   desc: "CH340C (SOP-16, crystal-less)" },
  { role: "LDO_3V3",    lcsc: "C6186",    desc: "AMS1117-3.3 (SOT-223)" },
  { role: "USB_C_1",    lcsc: "C165948",  desc: "TYPE-C-31-M-12 (16P SMD)" },
  { role: "USB_C_2",    lcsc: "C2765186", desc: "Alternative USB-C 16P" },
  { role: "XTAL_16M",   lcsc: "C13738",   desc: "16MHz SMD3225-4P (CL=9pF)" },

  // Protection & UI
  { role: "PTC_500mA",  lcsc: "C70070",   desc: "500mA PTC 1206 candidate 1" },
  { role: "PTC_ALT",    lcsc: "C369159",  desc: "500mA PTC 0805/1206 candidate 2" },
  { role: "DIODE_RST",  lcsc: "C2128",    desc: "1N4148WS (SOD-323)" },
  { role: "SW_RST_1",   lcsc: "C318884",  desc: "SMD Tactile Switch (TS-1187A)" },
  { role: "SW_RST_2",   lcsc: "C127507",  desc: "SMD Tactile Switch 6x6 alt" },

  // LEDs (0603 Basic candidates)
  { role: "LED_RED",    lcsc: "C2286",    desc: "Red LED 0603" },
  { role: "LED_GREEN",  lcsc: "C72043",   desc: "Green LED 0603 (check obsolete)" },
  { role: "LED_GREEN2", lcsc: "C72041",   desc: "Blue/Green LED 0603 alt" },
  { role: "LED_GREEN3", lcsc: "C2297",    desc: "Emerald Green LED 0603" },
  { role: "LED_YELLOW", lcsc: "C72038",   desc: "Yellow LED 0603" },

  // Resistors (0603 Basic)
  { role: "R_5K1",      lcsc: "C23186",   desc: "5.1k 0603" },
  { role: "R_1K",       lcsc: "C21190",   desc: "1k 0603" },
  { role: "R_2K2",      lcsc: "C4190",    desc: "2.2k 0603" },
  { role: "R_10K",      lcsc: "C25804",   desc: "10k 0603" },

  // Capacitors (0603 Basic)
  { role: "C_100N",     lcsc: "C14663",   desc: "100nF 50V X7R 0603" },
  { role: "C_10U",      lcsc: "C19702",   desc: "10uF 0603/0805" },
  { role: "C_12P",      lcsc: "C1640",    desc: "12pF 50V C0G 0603 (for CL=9pF xtal)" },
  { role: "C_15P",      lcsc: "C1644",    desc: "15pF 50V C0G 0603 alt" },

  // THT Headers (2.54mm female / male ICSP)
  { role: "HDR_1X10F",  lcsc: "C23003",   desc: "1x10P 2.54mm Female Header" },
  { role: "HDR_1X8F",   lcsc: "C124379",  desc: "1x8P 2.54mm Female Header" },
  { role: "HDR_1X6F",   lcsc: "C124377",  desc: "1x6P 2.54mm Female Header" },
  { role: "HDR_2X3M",   lcsc: "C2337",    desc: "2x3P 2.54mm Male Pin Header (ICSP)" }
];

const results = [];
for (const item of candidates) {
  try {
    const dev = await eda.lib_Device.getByLcscIds(item.lcsc);
    if (!dev) {
      results.push({ ...item, found: false });
      continue;
    }
    const op = dev.otherProperty || {};
    results.push({
      ...item,
      found: true,
      name: dev.name,
      uuid: dev.uuid,
      libraryUuid: dev.libraryUuid,
      footprintName: dev.footprintName || op["Footprint"] || null,
      jlcpcbClass: op["JLCPCB Part Class"] || null,
      stock: op["Stock"] ?? null,
      manufacturer: op["Manufacturer"] || null,
      mpn: op["Manufacturer Part"] || null,
      package: op["Supplier Footprint"] || op["Package"] || null
    });
  } catch (e) {
    results.push({ ...item, found: false, error: String(e?.message || e) });
  }
}

return {
  checkedAt: new Date().toISOString(),
  total: results.length,
  foundCount: results.filter((r) => r.found).length,
  results
};
