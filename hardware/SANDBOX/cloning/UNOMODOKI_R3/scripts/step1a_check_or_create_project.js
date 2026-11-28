const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
let info = null;
try { info = await eda.dmt_Project.getCurrentProjectInfo(); } catch {}
let created = false;
if (!info || info.name !== "Sandbox_UNOMODOKI_R3") {
  const projUuid = await eda.dmt_Project.createProject(
    "Sandbox_UNOMODOKI_R3",
    "sandbox-unomodoki-r3",
    undefined,
    undefined,
    "UNOMODOKI R3 (Arduino UNO R3 clean-room clone) - EasyEDA AI API sandbox",
    undefined,
    EDMT_ProjectFileFormat.EPRJ3
  );
  created = true;
  await eda.dmt_Project.openProject(projUuid);
  await sleep(3000);
  if (eda.dmt_Project.waitForReady) await eda.dmt_Project.waitForReady();
  info = await eda.dmt_Project.getCurrentProjectInfo();
}
let pages = await eda.dmt_Schematic.getAllSchematicPagesInfo();
if (!pages || pages.length === 0) {
  await eda.dmt_Board.createBoard();
  await sleep(3000);
  pages = await eda.dmt_Schematic.getAllSchematicPagesInfo();
}
await eda.dmt_EditorControl.openDocument(pages[0].uuid);
await sleep(2000);
// Clean up any leftover temporary components on sheet
const existing = await eda.sch_PrimitiveComponent.getAll() || [];
let deleted = 0;
for (const c of existing) {
  try { await eda.sch_PrimitiveComponent.delete(c.getState_PrimitiveId()); deleted++; } catch {}
}
if (deleted > 0) await eda.sch_Document.save();
const boards = await eda.dmt_Board.getAllBoardsInfo?.() || [];
const pcbs = await eda.dmt_Pcb.getAllPcbsInfo?.() || [];
return { created, info, boardsCount: boards.length, pagesCount: pages.length, pcbsCount: pcbs.length, pageUuid: pages[0].uuid, deletedLeftovers: deleted };
