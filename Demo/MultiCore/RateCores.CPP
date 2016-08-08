Bool mp_start,mp_end;
I64 mp_not_done_flags,
    **glbl_cnts;

U0 MPRateCores(I64 )
{
  I64 i=0;
  while (!mp_start)
    Yield;
  while (!mp_end) {
    i++;
    Yield;
  }
  glbl_cnts[Gs->num]=i;
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 RateCores()
{
  I64 i;
  F64 t0,tf;
  glbl_cnts=CAlloc(sizeof(I64)*mp_cnt);

  mp_start=mp_end=FALSE;
  mp_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    Spawn(&MPRateCores,,NULL,i);

  t0=tS;
  mp_start=TRUE;
  Sleep(2000);
  mp_end=TRUE;
  while (mp_not_done_flags)
    Yield;
  tf=tS;

  "\n        Context\n"
	"Core    Swaps/s\n"
	"---- --------------\n";
  for (i=0;i<mp_cnt;i++)
    "%4d %14.2,f\n",i,glbl_cnts[i]/(tf-t0);
  "---- --------------\n\n";
  Free(glbl_cnts);
}

RateCores;
