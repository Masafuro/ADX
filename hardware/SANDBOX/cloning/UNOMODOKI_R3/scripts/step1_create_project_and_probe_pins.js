// U1 Step 1: Create isolated local project "Sandbox_UNOMODOKI_R3" (.eprj3),
// reuse default Board1's schematic sheet, temporarily instantiate each unique BOM part
// to measure exact schematic pin numbers/names & metadata, then delete temporary parts
// so the schematic stays clean for U2.

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

const projUuid = await eda.dmt_Project.createProject(
  "Sandbox_UNOMODOKI_R3",
  "sandbox-unomodoki-r3",
  undefined,
  undefined,
  "UNOMODOKI R3 (Arduino UNO R3 clean-room clone) - EasyEDA AI API sandbox",
  undefined,
  EDMT_ProjectFileFormat.EPRJ3
);
if (!projUuid) {
  throw new Error("createProject returned undefined");
}

await eda.dmt_Project.openProject(projUuid);
await sleep(3000);
if (eda.dmt_Project.waitForReady) {
  await eda.dmt_Project.waitForReady();
}

// Reuse existing default Board1 if created automatically; otherwise create one
let pages = await eda.dmt_Schematic.getAllSchematicPagesInfo();
if (!pages || pages.length === 0) {
  await eda.dmt_Board.createBoard();
  await sleep(3000);
  if (eda.dmt_Project.waitForReady) {
    await eda.dmt_Project.waitForReady();
  }
  pages = await eda.dmt_Schematic.getAllSchematicPagesInfo();
}
if (!pages || pages.length === 0) {
  throw new Error("No schematic pages found");
}

const boards = await eda.dmt_Board.getAllBoardsInfo?.() || [];
const pcbs = await eda.dmt_Pcb.getAllPcbsInfo?.() || [];
const pageUuid = pages[0].uuid;
await eda.dmt_EditorControl.openDocument(pageUuid);
await sleep(2500);

const bomDefs = [
  { key: "MCU",       lcsc: "C14877",   qty: 1, refs: ["U1"],                     role: "Main MCU (ATmega328P-AU TQFP-32)" },
  { key: "USB_UART",  lcsc: "C84681",   qty: 1, refs: ["U2"],                     role: "USB-UART Bridge (CH340C SOP-16)" },
  { key: "LDO_3V3",   lcsc: "C6186",    qty: 1, refs: ["U3"],                     role: "3.3V LDO (AMS1117-3.3 SOT-223)" },
  { key: "USB_C",     lcsc: "C165948",  qty: 1, refs: ["J_USB"],                  role: "USB Type-C 16P (TYPE-C-31-M-12)" },
  { key: "XTAL_16M",  lcsc: "C13738",   qty: 1, refs: ["Y1"],                     role: "16MHz Crystal (X322516MLB4SI, CL=9pF)" },
  { key: "PTC_500M",  lcsc: "C369159",  qty: 1, refs: ["F1"],                     role: "500mA PTC Resettable Fuse (1206)" },
  { key: "DIODE_RST", lcsc: "C2128",    qty: 1, refs: ["D1"],                     role: "RESET Clamp Diode (1N4148WS SOD-323)" },
  { key: "SW_RST",    lcsc: "C318884",  qty: 1, refs: ["SW1"],                    role: "Reset Tactile Switch (5.1x5.1mm SMD)" },
  { key: "LED_PWR",   lcsc: "C12624",   qty: 1, refs: ["LED_PWR"],                role: "PWR LED Green 0603 (KT-0603G)" },
  { key: "LED_L",     lcsc: "C2287",    qty: 1, refs: ["LED_L"],                  role: "D13 L LED Yellow 0603 (KT-0603Y)" },
  { key: "LED_TX",    lcsc: "C2286",    qty: 1, refs: ["LED_TX"],                 role: "TX LED Red 0603 (KT-0603R)" },
  { key: "LED_RX",    lcsc: "C2288",    qty: 1, refs: ["LED_RX"],                 role: "RX LED Blue 0603 (KT-0603B)" },
  { key: "R_5K1",     lcsc: "C23186",   qty: 2, refs: ["R1","R2"],                role: "5.1k 0603 (USB-C CC1/CC2)" },
  { key: "R_1K",      lcsc: "C21190",   qty: 5, refs: ["R3","R4","R5","R6","R7"], role: "1k 0603 (UART series + PWR/TX/RX LEDs)" },
  { key: "R_2K2",     lcsc: "C4190",    qty: 1, refs: ["R8"],                     role: "2.2k 0603 (D13 L LED light load)" },
  { key: "R_10K",     lcsc: "C25804",   qty: 1, refs: ["R9"],                     role: "10k 0603 (RESET pull-up)" },
  { key: "C_100N",    lcsc: "C14663",   qty: 6, refs: ["C1","C2","C3","C4","C5","C6"], role: "100nF 0603 (Bypass, V3, DTR, AREF)" },
  { key: "C_10U",     lcsc: "C19702",   qty: 3, refs: ["C7","C8","C9"],           role: "10uF 0603 (5V bulk + 3.3V LDO in/out)" },
  { key: "C_15P",     lcsc: "C1644",    qty: 2, refs: ["C10","C11"],              role: "15pF 0603 C0G (16MHz xtal load)" },
  { key: "HDR_1X10F", lcsc: "C5116489", qty: 1, refs: ["J_DIG_HI"],               role: "1x10P 2.54mm Female Header (SCL..D8)" },
  { key: "HDR_1X8F",  lcsc: "C5224027", qty: 2, refs: ["J_DIG_LO","J_POWER"],     role: "1x8P 2.54mm Female Header (D7..D0, POWER)" },
  { key: "HDR_1X6F",  lcsc: "C5224026", qty: 1, refs: ["J_ANALOG"],               role: "1x6P 2.54mm Female Header (A0..A5)" },
  { key: "HDR_2X3M",  lcsc: "C5116479", qty: 1, refs: ["J_ICSP"],                 role: "2x3P 2.54mm Male Header (ICSP)" }
];

const verified = [];
for (const item of bomDefs) {
  const dev = await eda.lib_Device.getByLcscIds(item.lcsc);
  if (!dev) {
    verified.push({ ...item, found: false });
    continue;
  }
  const op = dev.otherProperty || {};
  const refObj = { libraryType: "3", libraryUuid: dev.libraryUuid, uuid: dev.uuid };
  // Temporarily place at (500, 500) to measure exact pin numbers & names
  const comp = await eda.sch_PrimitiveComponent.create(refObj, 500, 500, undefined, 0, false, true, true);
  const pid = comp.getState_PrimitiveId();
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(pid);
  const pinList = pins.map((p) => ({
    number: String(p.pinNumber ?? p.getState_PinNumber?.() ?? ""),
    name: String(p.pinName ?? p.getState_PinName?.() ?? ""),
    dx: Math.round(((p.x ?? p.getState_X?.() ?? 0) - 500) * 100) / 100,
    dy: Math.round(((p.y ?? p.getState_Y?.() ?? 0) - 500) * 100) / 100
  }));
  await eda.sch_PrimitiveComponent.delete(pid);

  verified.push({
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
}

await eda.sch_Document.save();

return {
  project: {
    uuid: projUuid,
    info: await eda.dmt_Project.getCurrentProjectInfo(),
    boardsCount: boards.length,
    pagesCount: pages.length,
    pcbsCount: pcbs.length,
    pageUuid
  },
  uniqueTypes: verified.length,
  totalComponentCount: verified.reduce((a, b) => a + b.qty, 0),
  verified
};
