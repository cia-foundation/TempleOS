/*
  lock{} is a compiler statement
  which places asm LOCK instruction
  prefixes on code.  Only use it on
  read-modify-write statements.
  It will generate invalid instructions
  if you misuse it and it will crash.
  $LK,"U","MN:U"$() to see what code was generated.
*/

#define CNT	0x1000000

I64 glbl;


U0 MPUnlockedInc(I64)
{
  I64 i;
  for (i=0;i<CNT;i++)
    glbl++;
}

U0 UnlockedInc()
{
  CSrvCmd *tempm0,*tempm1;
  glbl=0;
  tempm0=JobQue(&MPUnlockedInc,NULL,0,0);
  tempm1=JobQue(&MPUnlockedInc,NULL,1,0);
  GetResult(tempm0);
  GetResult(tempm1);
  "Correct Cnt:%X Actual Cnt:%X\n",CNT*2,glbl;
}



U0 MPLockedInc(I64)
{
  I64 i;
  for (i=0;i<CNT;i++)
    lock  //Can be used without {}
      glbl++;
}

U0 LockedInc()
{
  CSrvCmd *tempm0,*tempm1;
  glbl=0;
  tempm0=JobQue(&MPLockedInc,NULL,0,0);
  tempm1=JobQue(&MPLockedInc,NULL,1,0);
  GetResult(tempm0);
  GetResult(tempm1);
  "Correct Cnt:%X Actual Cnt:%X\n",CNT*2,glbl;
}

UnlockedInc;
LockedInc;
