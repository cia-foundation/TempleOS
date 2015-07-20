//Generate "StadiumBG.GRA.Z" from "StadiumBG.TXT.Z"

U0 StadiumGen()
{
  U0 (*fp_old_update)(CDC *dc);
  CDC *dc,*dc2;

  Cd(__DIR__);
  WinMax;
  WinBorder;
  AutoComplete;
  DocClear;
  Type("StadiumBG.TXT");
  "$$CM,0,%d$$",TEXT_ROWS-3;
  DocCursor(OFF);
  fp_old_update=gr.fp_final_screen_update; //hide mouse cursor
  gr.fp_final_screen_update=NULL;

  WinMgrSync(2,TRUE);
  dc2=DCScreenCapture;
  dc=DCExtract(dc2,0,FONT_HEIGHT,GR_WIDTH,GR_HEIGHT);
  DCDel(dc2);

  GRAWrite("StadiumBG",dc,DCSF_COMPRESSED|DCSF_PALETTE_GET);
  DCDel(dc);

  DocCursor(ON);
  gr.fp_final_screen_update=fp_old_update;
  DocClear;
}

StadiumGen;

