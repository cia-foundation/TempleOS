interrupt U0 BeepOn()
{
  *LAPIC_EOI(U32 *)=0; //signal end of interrupt
  Snd(100);
}

interrupt U0 BeepOff()
{
  *LAPIC_EOI(U32 *)=0;
  Snd(0);
}

U0 IntTest()
{
  //Modify interrupt descriptor table
  //for CPU #1.
  IntEntrySet(I_USER,&BeepOn,IDTET_TRAP);
  IntEntrySet(I_USER+1,&BeepOff,IDTET_TRAP);
  MPInt(I_USER,1);
  Sleep(1000);
  MPInt(I_USER+1,1);
}

IntTest;
