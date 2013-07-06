U0 Job(CDoc *doc)
{
  I64 i;
  for (i=0;i<2;i++)
    DocPrintF(doc,"$$FG,%d$$CPU%d ",Gs->num,Gs->num);
}


U0 MPPrintFDemo()
{
  I64 i;
  for (i=0;i<500;i++)
    JobQue(&Job,DocPut,i%mp_cnt);
  Sleep(500);
  "$$FG$$\n";
}

MPPrintFDemo;
