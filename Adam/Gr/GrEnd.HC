#help_index "Graphics/Scrn"

U0 CtrlAltZ(I64 sc)
{
  if (sc&SCF_SHIFT)
    GrScaleZoom(0.5);
  else
    GrScaleZoom(2.0);
}
CtrlAltCBSet('Z',&CtrlAltZ,"Cmd /Zoom In",
	"Cmd /Zoom Out");

U0 Scrn2Clip(I64)
{
  CDC *dc=DCScrnCapture;
  U8 *elems=DC2Sprite(dc);
  Bool unlock_doc=DocLock(sys_clip_doc);
  ClipDel;
  DocSprite(sys_clip_doc,elems);
  if (unlock_doc)
    DocUnlock(sys_clip_doc);
  Free(elems);
  DCDel(dc);
}
U0 CtrlAltS(I64)
{
  Spawn(&Scrn2Clip);
}
CtrlAltCBSet('S',&CtrlAltS,"Cmd /Scrn Shot to Clip");
