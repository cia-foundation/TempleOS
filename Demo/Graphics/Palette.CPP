U0 PaletteDemo()
{
  I64 i,j;
  CBGR bgr;
  for (i=0;i<16;i++)
    "$$BLACK$$$$BG,%d$$__%-8Z__$$FG$$$$BG$$\n",i,i,"ST_COLORS";

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
//Alternatively, use $LK,"SetVGAPalette4",A="MN:SetVGAPalette4"$(vga_palette4_gray);
  for (i=0;i<16;i++) {
    j=0xFF*i/15;
    bgr.b=j;
    bgr.g=j;
    bgr.r=j;
    SetVGAPaletteColor(i,bgr);
  }
  PressAKey;

  //You can do it this way, too.
  SetVGAPaletteColor(WHITE,0xFFFF00);
  PressAKey;

//$LK,"SettingsPop",A="MN:SettingsPop"$() restores original palette
  SettingsPop;
}

PaletteDemo;
