U0 Main()
{
  CDC *dc=DCAlias;
  dc->color=RED+BLACK<<16+ROPF_DITHER;
  GrCircle(dc,100,100,50);
  GrLine(dc,10,10,280,70);
  dc->color=RED+BLUE<<16+ROPF_DITHER;
  GrFloodFill(dc,100,100);
  dc->color=PURPLE+BLUE<<16+ROPF_DITHER;
  GrRect(dc,20,20,100,100);
  PressAKey;
  DCFill;
  DCDel(dc);
}

Main;
