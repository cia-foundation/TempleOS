U0 Job(CDoc *doc)
{
  I64 i;
  for (i=0;i<2;i++)
    DocPrint(doc,"$$FG,%d$$CPU%d ",Gs->num&15,Gs->num);
}

U0 MPPrintDemo()
{
  I64 i;
  for (i=0;i<500;i++)
    JobQue(&Job,DocPut,i%mp_cnt);
  Sleep(500);
  "$$FG$$\n";
}

MPPrintDemo;
