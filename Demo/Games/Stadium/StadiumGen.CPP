//Generate "Stadium.GRA.Z" from "Stadium.TXT.Z"

U0 StadiumGen()
{
  U0 (*fp_old_update)(CDC *dc);
  CDC *dc,*dc2;

  WinMax;
  WinBorder;
  WordStat;
  DocClear;
  Type("::/Demo/Games/Stadium/Stadium.TXT");
  "$$CM,0,%d$$",TEXT_ROWS-3;
  DocCursor(OFF);
  fp_old_update=gr.fp_final_screen_update; //hide mouse cursor
  gr.fp_final_screen_update=NULL;

  WinMgrSync(2,TRUE);
  dc2=DCCaptureScreen;
  dc=DCExtract(dc2,0,FONT_HEIGHT,GR_WIDTH,GR_HEIGHT);
  DCDel(dc2);

  GRAWrite("::/Demo/Games/Stadium/Stadium",dc);
  DCDel(dc);
 
  DocCursor(ON);
  gr.fp_final_screen_update=fp_old_update;
  DocClear;
}

StadiumGen;

