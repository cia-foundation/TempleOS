//This is the fastest you can update the whole scrn.
//See $LK,"::/Demo/Lectures/MiniGrLib.HC"$ if this interests you.

F64 VGAPattern(I64 p)
{//This returns the time to update in seconds.
  F64 start=tS;
  I64 plane;
  for (plane=1;plane<0x10;plane<<=1) {
    OutU8(VGAP_IDX,VGAR_MAP_MASK);
    OutU8(VGAP_DATA,plane);
    MemSetI64(text.vga_alias,p,640*480/64);
  }
  return tS-start;
}
 
 
U0 GrDemo()
{
  I64 i;
  F64 t;
  CPURep;
  for (i=0;i<16;i++) {
    t=VGAPattern(0x5555555555555555);
    "$$GREEN$$Rate:$$RED$$%5.1f FPS$$FG$$"
	  "  $$GREEN$$Load@30Hz:$$RED$$%5.1f%%$$FG$$\n",1/t,100*t*30.0;
    t=VGAPattern(0x3333333333333333);
    "$$GREEN$$Rate:$$RED$$%5.1f FPS$$FG$$"
	  "  $$GREEN$$Load@30Hz:$$RED$$%5.1f%%$$FG$$\n",1/t,100*t*30.0;
  }
//<CTRL-ALT-v> will flush scrn VGA cache.
  VGAFlush;
}

GrDemo;
