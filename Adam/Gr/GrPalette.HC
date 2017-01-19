#help_index "Graphics/Color"
public U8 gr_rainbow_10[10]={
  BLACK,BROWN,RED,LTRED,YELLOW,GREEN,BLUE,PURPLE,LTGRAY,WHITE};

DefineLstLoad("ST_RAINBOW_10",
  "BLACK\0BROWN\0RED\0LTRED\0YELLOW\0GREEN\0BLUE\0PURPLE\0LTGRAY\0WHITE\0");

U0 GrPaletteIndicesSet()
{//There is a level of indirection that we identity map.
// Set 16-colors to use first 16 DAC BGR entries, so we
// never need to worry about indexes, just DAC palette settings.
  I64 i;
  if (!Bt(&sys_run_level,RLf_VGA)) return;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_VGA],0))
    Yield;
  InU8(VGAP_INPUT_STAT); //Rsts attr index/data
  for (i=0;i<COLORS_NUM;i++) {
    OutU8(VGAP_ATTR_INDEX,i);
    OutU8(VGAP_ATTR_DATA_WRITE,i);
  }
  OutU8(VGAP_ATTR_INDEX,0x20);
  OutU8(VGAP_ATTR_DATA_WRITE,0); //Dummy write
  InU8(VGAP_INPUT_STAT); //Rsts attr index/data
  LBtr(&sys_semas[SEMA_VGA],0);
  POPFD
}

public U0 GrPaletteColorSet(I64 color_num,CBGR48 bgr48)
{//VGA has 6-bits for blue, for green, and for red.
  if (!Bt(&sys_run_level,RLf_VGA)) return;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_VGA],0))
    Yield;
  OutU8(VGAP_PALETTE_MASK,0xFF);
  OutU8(VGAP_REG_WRITE,color_num);
  OutU8(VGAP_PALETTE_DATA,bgr48.r>>10);
  OutU8(VGAP_PALETTE_DATA,bgr48.g>>10);
  OutU8(VGAP_PALETTE_DATA,bgr48.b>>10);
  LBtr(&sys_semas[SEMA_VGA],0);
  POPFD
}

public CBGR48 GrPaletteColorGet(I64 color_num)
{//VGA has 6-bits for blue, for green, and for red.
  CBGR48 res=0;
  if (!Bt(&sys_run_level,RLf_VGA)) return 0;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_VGA],0))
    Yield;
  OutU8(VGAP_PALETTE_MASK,0xFF);
  OutU8(VGAP_REG_READ,color_num);
  res.r=0xFFFF<<10*InU8(VGAP_PALETTE_DATA)/0xFC00;
  res.g=0xFFFF<<10*InU8(VGAP_PALETTE_DATA)/0xFC00;
  res.b=0xFFFF<<10*InU8(VGAP_PALETTE_DATA)/0xFC00;
  LBtr(&sys_semas[SEMA_VGA],0);
  POPFD
  return res;
}

public CBGR48 gr_palette_std[COLORS_NUM]={
0x000000000000,0x00000000AAAA,0x0000AAAA0000,0x0000AAAAAAAA,
0xAAAA00000000,0xAAAA0000AAAA,0xAAAA55550000,0xAAAAAAAAAAAA,
0x555555555555,0x55555555FFFF,0x5555FFFF5555,0x5555FFFFFFFF,
0xFFFF55555555,0xFFFF5555FFFF,0xFFFFFFFF5555,0xFFFFFFFFFFFF};

public CBGR48 gr_palette_gray[COLORS_NUM]={
0x000000000000,0x111111111111,0x222222222222,0x333333333333,
0x444444444444,0x555555555555,0x666666666666,0x777777777777,
0x888888888888,0x999999999999,0xAAAAAAAAAAAA,0xBBBBBBBBBBBB,
0xCCCCCCCCCCCC,0xDDDDDDDDDDDD,0xEEEEEEEEEEEE,0xFFFFFFFFFFFF};

public U0 GrPaletteGet(CBGR48 *bgr48)
{//16 colors
  I64 i;
  for (i=0;i<COLORS_NUM;i++)
    bgr48[i]=GrPaletteColorGet(i);
}

public U0 GrPaletteSet(CBGR48 *bgr48)
{//16 colors
  I64 i;
  for (i=0;i<COLORS_NUM;i++)
    GrPaletteColorSet(i,bgr48[i]);
}

public U0 PaletteSetStd()
{//Activate std palette.
  GrPaletteSet(gr_palette_std);
}
