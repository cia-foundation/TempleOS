interrupt U0 BeepOn()
{
  IntEOI; //signal end of interrupt
  Snd(100);
}

interrupt U0 BeepOff()
{
  IntEOI;
  Snd(0);
}

U0 IntTest()
{
  //Modify interrupt descriptor table
  //for CPU #1.
  IntEntrySet(I_USER,&BeepOn,1,IDTET_TRAP);
  IntEntrySet(I_USER+1,&BeepOff,1,IDTET_TRAP);
  MPInt(I_USER,1);
  Sleep(1000);
  MPInt(I_USER+1,1);
}

IntTest;
