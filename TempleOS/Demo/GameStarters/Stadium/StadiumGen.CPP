//Generate "Stadium.GRA.Z" from "Stadium.TXT.Z"


U0 StadiumGen()
{
  U0 (*fp_old_update)(CDC *dc);
  CDC *dc;

  WinMax;
  WinBorder;
  DocClear;
  Type("::/TempleOS/Demo/GameStarters/Stadium/Stadium.TXT");
  "$$CM,0,%d$$",TEXT_ROWS-2;
  DocCursor(OFF);
  fp_old_update=fp_final_screen_update; //hide mouse cursor
  fp_final_screen_update=NULL;

  WinMgrSync(2,TRUE);
  dc=DCExtract(gr_screen_image,0,FONT_HEIGHT,GR_WIDTH,GR_HEIGHT);

  GRAWrite("::/TempleOS/Demo/GameStarters/Stadium/Stadium",dc);
  DCDel(dc);
 
  DocCursor(ON);
  fp_final_screen_update=fp_old_update;
  DocClear;
}

StadiumGen;

