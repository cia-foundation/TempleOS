//This just shows how scrn memory works.
//See $LK,"::/Demo/Lectures/MiniGrLib.HC"$

U0 PlotXY(I64 x,I64 y)
{
//Scrn bits are revd
  LBts(text.vga_alias,y*GR_WIDTH+x^7);
}

U0 Main()
{
  I64 i;
  //This makes all 4 color planes active.
  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,WHITE);
  MemSet(text.vga_alias,0,GR_WIDTH*GR_HEIGHT/8);

  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,RED);
  for (i=0;i<200;i++)
    PlotXY(i,i);

  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,GREEN);
  for (i=0;i<200;i++)
    PlotXY(100,i);

  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,BLUE);
  for (i=0;i<200;i++)
    PlotXY(200-i,i);
/*If you want a mixed color sel multiple planes
but you have to be sure the unseled planes
are zero, so sel them and make them zero.
You can't do reads on VGA memory, by the way.
That means no read-modify-writes, too.
*/
  Busy(4000000);

  //TempleOS has a 4 plane memory duplicate of the scrn, $LK,"gr.scrn_image",A="MN:CGrGlbls"$,
  //and only writes actual changes.  See $LK,"GrUpdateVGAGraphics",A="MN:GrUpdateVGAGraphics"$().
  //<CTRL-ALT-v> will flush scrn VGA cache.
  VGAFlush;
}

Main;

//See $LK,"::/Demo/Lectures/GraphicsCPULoad.HC"$.

