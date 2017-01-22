//Generate "StadiumBG.GR.Z" from "StadiumBG.DD.Z"

U0 StadiumGen()
{
  U0 (*fp_old_update)(CDC *dc);
  CDC *dc,*dc2;

  Cd(__DIR__);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor(OFF);
  DocClear;
  Type("StadiumBG.DD");
  "$$CM,0,%d$$",TEXT_ROWS-3;
  fp_old_update=gr.fp_final_scrn_update; //hide mouse cursor
  gr.fp_final_scrn_update=NULL;

  Refresh(2,TRUE);
  dc2=DCScrnCapture;
  dc=DCExt(dc2,0,FONT_HEIGHT,GR_WIDTH,GR_HEIGHT);
  DCDel(dc2);

  GRWrite("StadiumBG",dc,DCSF_COMPRESSED|DCSF_PALETTE_GET);
  DCDel(dc);

  DocCursor(ON);
  gr.fp_final_scrn_update=fp_old_update;
  DocClear;
}

StadiumGen;

