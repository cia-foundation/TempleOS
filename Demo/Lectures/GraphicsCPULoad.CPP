//This is the fastest you can update the whole screen.
//See $LK,"::/Demo/Lectures/MiniGrLib.CPP"$ if this interests you.

F64 VGAPattern(I64 p)
{//This returns the time to update in seconds.
  F64 start=tS;
  I64 plane;
  for (plane=1;plane<0x10;plane<<=1) {
    OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
    OutU8(VGA_SC_DATA,plane);
    MemSetI64(0xA0000,p,640*480/64);
  }
  return tS-start;
}
 
 
U0 GrDemo()
{
  I64 i;
  F64 t;
  Bool old_preempt=Preempt(OFF);
  CPURep;
  for (i=0;i<16;i++) {
    t=VGAPattern(0x5555555555555555);
    "$$GREEN$$Rate:$$RED$$%5.1f FPS$$FG$$"
    "  $$GREEN$$Load@60Hz:$$RED$$%5.1f%%$$FG$$\n",1/t,100*t*60.0;
    t=VGAPattern(0x3333333333333333);
    "$$GREEN$$Rate:$$RED$$%5.1f FPS$$FG$$"
    "  $$GREEN$$Load@60Hz:$$RED$$%5.1f%%$$FG$$\n",1/t,100*t*60.0;
  }
  Preempt(old_preempt);
//<CTRL-ALT-f> will flush screen VGA cache.
  VGAFlush;
}

GrDemo;
