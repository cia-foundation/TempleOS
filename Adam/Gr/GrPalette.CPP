#help_index "Graphics/Color"
public I32 gr_rainbow_10[10]=
{
BLACK,BROWN,RED,LTRED,YELLOW,
GREEN,BLUE,PURPLE,LTGRAY,WHITE
};

DefineLstLoad("ST_RAINBOW_10",
"BLACK\0BROWN\0RED\0LTRED\0YELLOW\0"
"GREEN\0BLUE\0PURPLE\0LTGRAY\0WHITE\0");

U0 SetVGAPalette4Indices()
{/*4-bit (16 color)
There is a level of indirection we wish to identity map.
This sets the 16-colors to use
the first 16 DAC rgb entries, so
we never need to worry about indexes,
just DAC palette settings.
*/
  I64 i;
  if (!sys_bios_gr_mode) return;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  InU8(VGA_INPUT_STAT); //Rsts attr index/data
  for (i=0;i<16;i++) {
    OutU8(VGA_ATTR_INDEX,i);
    OutU8(VGA_ATTR_DATA_WRITE,i);
  }
  OutU8(VGA_ATTR_INDEX,0x20);
  OutU8(VGA_ATTR_DATA_WRITE,0); //Dummy write
  InU8(VGA_INPUT_STAT); //Rsts attr index/data
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  POPFD
}

public U0 SetVGAPaletteColor(I64 color_num,CBGR bgr)
{//6-bits per b, g and r.
  if (!sys_bios_gr_mode) return;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  OutU8(VGA_PALETTE_MASK,0xFF);
  OutU8(VGA_REG_WRITE,color_num);
  OutU8(VGA_PALETTE_DATA,bgr.r>>2);
  OutU8(VGA_PALETTE_DATA,bgr.g>>2);
  OutU8(VGA_PALETTE_DATA,bgr.b>>2);
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  POPFD
}

public CBGR GetVGAPaletteColor(I64 color_num)
{//6-bits per b, g and r.
  CBGR result=0;
  if (!sys_bios_gr_mode) return 0;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  OutU8(VGA_PALETTE_MASK,0xFF);
  OutU8(VGA_REG_READ,color_num);
  result.r=InU8(VGA_PALETTE_DATA)<<2;
  result.g=InU8(VGA_PALETTE_DATA)<<2;
  result.b=InU8(VGA_PALETTE_DATA)<<2;
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  POPFD
  return result;
}

public CBGR vga_palette4_std[16]={
0x000000,0x0000AA,0x00AA00,0x00AAAA,
0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
0x555555,0x5555FF,0x55FF55,0x55FFFF,
0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF};

public CBGR vga_palette4_gray[16]={
0x000000,0x111111,0x222222,0x333333,
0x444444,0x555555,0x666666,0x777777,
0x888888,0x999999,0xAAAAAA,0xBBBBBB,
0xCCCCCC,0xDDDDDD,0xEEEEEE,0xFFFFFF};

public U0 GetVGAPalette4(CBGR *bgr)
{//4-bit (16 color)
  I64 i;
  for (i=0;i<16;i++)
    bgr[i]=GetVGAPaletteColor(i);
}

public U0 SetVGAPalette4(CBGR *bgr)
{//4-bit (16 color)
  I64 i;
  for (i=0;i<16;i++)
    SetVGAPaletteColor(i,bgr[i]);
}

public U0 SetStdPalette()
{//Activate std palette.
  SetVGAPalette4(vga_palette4_std);
}
