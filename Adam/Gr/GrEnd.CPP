#help_index "Graphics/Screen"

U0 CtrlAltZ(I64 sc)
{
  if (sc&SCF_SHIFT)
    GrScaleZoom(0.5);
  else
    GrScaleZoom(2.0);
}
CtrlAltLetCBSet('Z',&CtrlAltZ,"Sys/Zoom In or Out");

U0 Screen2Clipboard(I64)
{
  CDC *dc=DCScreenCapture;
  U8 *elems=DC2Sprite(dc);
  Bool unlock_doc=DocLock(sys_clipboard_doc);
  ClipboardDel;
  DocSprite(sys_clipboard_doc,elems);
  if (unlock_doc)
    DocUnlock(sys_clipboard_doc);
  Free(elems);
  DCDel(dc);
}
U0 CtrlAltS(I64)
{
  Spawn(&Screen2Clipboard);
}
CtrlAltLetCBSet('S',&CtrlAltS,"Sys/Screen Shot to Clipboard");
