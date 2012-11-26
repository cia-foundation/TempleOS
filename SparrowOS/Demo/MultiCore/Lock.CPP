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
  I64 i;
  CMPCmd *tempm;
  glbl=0;

  tempm=MPJob(&MPUnlockedInc,NULL,1,0);
  for (i=0;i<CNT;i++)
    glbl++;

  MPJobResult(tempm);
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
  I64 i;
  CMPCmd *tempm;
  glbl=0;

  tempm=MPJob(&MPLockedInc,NULL,1,0);
  for (i=0;i<CNT;i++)
    lock {
      glbl++;
    }

  MPJobResult(tempm);
  "Correct Cnt:%X Actual Cnt:%X\n",CNT*2,glbl;
}

UnlockedInc;
LockedInc;
