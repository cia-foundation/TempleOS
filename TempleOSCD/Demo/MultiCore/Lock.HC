/*lock{} is a compiler stmt
which places asm LOCK inst
prefixes on code.  Only use it on
read-modify-write stmts.
It will generate invalid insts
if you misuse it and it will crash.
$LK,"U",A="MN:U"$() to see what code was generated.
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
  CJob *tmpm0,*tmpm1;
  glbl=0;
  tmpm0=JobQue(&MPUnlockedInc,NULL,0,0);
  tmpm1=JobQue(&MPUnlockedInc,NULL,1,0);
  JobResGet(tmpm0);
  JobResGet(tmpm1);
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
  CJob *tmpm0,*tmpm1;
  glbl=0;
  tmpm0=JobQue(&MPLockedInc,NULL,0,0);
  tmpm1=JobQue(&MPLockedInc,NULL,1,0);
  JobResGet(tmpm0);
  JobResGet(tmpm1);
  "Correct Cnt:%X Actual Cnt:%X\n",CNT*2,glbl;
}

UnlockedInc;
LockedInc;
