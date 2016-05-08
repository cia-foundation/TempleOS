I64 mp_not_done_flags;

U0 MPRoutine(CDoc *doc)
{
  DocPrint(doc,"Core:%d\n",Gs->num);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 Main()
{
  I64 i;
  mp_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    Spawn(&MPRoutine,DocPut,NULL,i);
  while (mp_not_done_flags)
    Yield;
}

Main;
