U0 Job(CDoc *doc)
{
  I64 i;
  for (i=0;i<10;i++)
    DocPrintF(doc,"$$FG,%d$$CPU%d ",Gs->num,Gs->num);
}


U0 MPPrintFDemo()
{
  I64 i;
  for (i=0;i<1000;i++)
    MPJob(&Job,DocCur,1+i%(mp_cnt-1));
  Sleep(1000);
  "$$FG$$\n";
}

MPPrintFDemo;
