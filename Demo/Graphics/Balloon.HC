// Classic Commodore 64 Sprite

U8 a[60]={0,127,0,1,255,192,3,255,224,3,231,224,
	  7,217,240,7,223,240,7,217,240,3,231,224,
	  3,255,224,3,255,224,2,255,160,1,127,64,
	  1,62,64,0,156,128,0,156,128,0,73,0,0,73,0,
	  0,62,0,0,62,0,0,28,0};

U0 Balloon()
{
  I64 i,j,k;

  CLI

  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,0xF);
  MemSetI64(text.vga_alias,0,640*480/64);

  OutU8(VGAP_IDX,VGAR_MAP_MASK);
  OutU8(VGAP_DATA,RED+GREEN+8);
  for (k=100;k<150;k++) {

    for (i=0;i<20;i++)
      for (j=0;j<3;j++)
	*(text.vga_alias(I64)+0x1000+(i+k)*640/8+j)(U8 *)=a[i*3+j];

    Busy(40000);

    for (i=0;i<20;i++)
      for (j=0;j<3;j++)
	*(text.vga_alias(I64)+0x1000+(i+k)*640/8+j)(U8 *)=0;
  }

  STI
  VGAFlush;
}

Balloon;
