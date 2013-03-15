//$BK,1$WARNING:$BK,0$ Don't return out of a catch{}.  This
//might get fixed.

//You can use $LK,"CatchAll","MN:CatchAll"$() in a catch statement
//and it will report args.

#define EXCEPT_1	1
#define EXCEPT_2	2
#define EXCEPT_3	3
#define EXCEPT_4	4

U0 Show()
{
  I64 i;
  "Fs->except_argc=%X\n",Fs->except_argc;
  for (i=0;i<Fs->except_argc;i++)
    "Fs->except_argv[%d]=%08X\n",i,Fs->except_argv[i];
}



Bool Pmt(I64 i)
{
  "%d ",i;
  return YorN;
}

U0 D1()
{
  "D1\n";
  if (Pmt(1)) throw(EXCEPT_1);
  if (Pmt(2)) throw(EXCEPT_2,33,55,66);
  if (Pmt(5)) throw;
}

U0 D2()
{
  "D2\n";
  try {
    D1;
    if (Pmt(4)) throw(EXCEPT_4);
  } catch {
    "D2 handler\n";
    Show;
    if (Fs->except_argc>0) {
      if (Fs->except_argv[0]==EXCEPT_1) {
	"Caught in D2\n";
	Fs->catch_except=TRUE;
      }
    }
  }
}

U0 Demo()
{
  try {
    D2;
    if (Pmt(3)) throw;
  } catch {
    "Demo handler\n";
    Show;
    Fs->catch_except=TRUE;
  }
}


Demo;
