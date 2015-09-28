interrupt U0 IntBeepOn()
{
  *LAPIC_EOI(U32 *)=0; //signal end of interrupt
  Snd(100);
}

interrupt U0 IntBeepOff()
{
  *LAPIC_EOI(U32 *)=0;
  Snd(0);
}

U0 IntTest()
{//Modify interrupt descriptor table use by all cores.
  IntEntrySet(I_USER,&IntBeepOn,IDTET_TRAP);
  IntEntrySet(I_USER+1,&IntBeepOff,IDTET_TRAP);
//Issue interupt to core#1
  MPInt(I_USER,1);
  Sleep(1000);
  MPInt(I_USER+1,1);
}

IntTest;
