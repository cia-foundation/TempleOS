//This just shows how screen memory works.
//See $LK,"::/Demo/Lectures/MiniGrLib.CPP"$

U0 PlotXY(I64 x,I64 y)
{
  //Screen bits are reversed
  LBts(VGA_GRAPHIC_MEM_BASE,y*GR_WIDTH+x^7);
}



U0 Main()
{
  I64 i;
  Bool old_preempt=Preempt(OFF);

//This makes all 4 color planes active.
  OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
  OutU8(VGA_SC_DATA,WHITE);
  MemSet(VGA_GRAPHIC_MEM_BASE,0,GR_WIDTH*GR_HEIGHT/8);

  OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
  OutU8(VGA_SC_DATA,RED);
  for (i=0;i<200;i++)
    PlotXY(i,i);

  OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
  OutU8(VGA_SC_DATA,GREEN);
  for (i=0;i<200;i++)
    PlotXY(100,i);

  OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
  OutU8(VGA_SC_DATA,BLUE);
  for (i=0;i<200;i++)
    PlotXY(200-i,i);
/*
If you want a mixed color select multiple planes
but you have to be sure the unselected planes
are zero, so select them and make them zero.
You can't do reads on VGA memory, by the way.
That means no read-modify-writes, too.
*/
  BusyWait(4000000);

  Preempt(old_preempt);

//TempleOS has a 4 plane memory duplicate of the screen, $LK,"gr_screen_image",A="MN:gr_screen_image"$,
//and only writes actual changes.  See $LK,"GrUpdateVGAGraphics",A="MN:GrUpdateVGAGraphics"$().
//<CTRL-ALT-F> will flush screen VGA cache.
  VGAFlush;
}

Main;

//See $LK,"::/Demo/Lectures/GraphicsCPULoad.CPP"$.
