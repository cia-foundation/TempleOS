interrupt U0 IntBeepOn()
{
  *(dev.uncached_alias+LAPIC_EOI)(U32 *)=0; //signal end of interrupt
  Snd(34);
}

interrupt U0 IntBeepOff()
{
  *(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
  Snd;
}

U0 IntTest()
{//Modify interrupt descriptor table use by all cores.
  IntEntrySet(I_USER,&IntBeepOn,IDTET_TRAP);
  IntEntrySet(I_USER+1,&IntBeepOff,IDTET_TRAP);
//Issue interupt to core1
  MPInt(I_USER,1);
  Sleep(1000);
  MPInt(I_USER+1,1);
}

IntTest;
