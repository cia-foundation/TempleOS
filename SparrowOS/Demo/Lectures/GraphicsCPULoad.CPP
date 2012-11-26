//This is the fastest you can update the whole screen.
//See $LK,"::/SparrowOS/Demo/Lectures/MiniGrLib.CPP.Z","FI:::/SparrowOS/Demo/Lectures/MiniGrLib.CPP.Z"$ if this interests you.

F64 VGAPattern(I64 p)
{//This returns the time to update in seconds.
  F64 start=tP;
  I64 i,plane,*ptr;
  for (plane=1;plane<0x10;plane<<=1) {
    OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
    OutU8(VGA_SC_DATA,plane);
    ptr=0xA0000;
    for (i=0;i<640*480/64;i++)
      *ptr++=p;
  }
  return tP-start;
}
 
 
U0 GrDemo()
{
  I64 i;
  F64 t;
  Bool old_preempt=Preempt(OFF);
  CPURep;
  for (i=0;i<16;i++) {
    t=VGAPattern(0x5555555555555555);
    "$$FG,GREEN$$Rate:$$FG,RED$$%5.1f FPS$$FG$$"
    "  $$FG,GREEN$$Load@60Hz:$$FG,RED$$%5.1f%%$$FG$$\n",1/t,100*t*60.0;
    t=VGAPattern(0x3333333333333333);
    "$$FG,GREEN$$Rate:$$FG,RED$$%5.1f FPS$$FG$$"
    "  $$FG,GREEN$$Load@60Hz:$$FG,RED$$%5.1f%%$$FG$$\n",1/t,100*t*60.0;
  }
  Preempt(old_preempt);
//<CTRL-ALT-F> will flush screen VGA cache.
  VGAFlush;
}

GrDemo;

/*  Program Output$HL,0$$WW+H,1$$FD,1$

8 Cores 2.660GHz
$FG,2$Rate:$FG,4$369.7 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$370.3 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$361.2 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.6%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$370.2 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$360.6 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.6%$FG$
$FG,2$Rate:$FG,4$370.4 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$353.1 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 17.0%$FG$
$FG,2$Rate:$FG,4$370.3 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$357.0 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.8%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$370.4 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$353.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 17.0%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$370.2 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$370.4 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.7 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.9 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$
$FG,2$Rate:$FG,4$369.8 FPS$FG$	$FG,2$Load@60Hz:$FG,4$ 16.2%$FG$

$HL,1$*/
