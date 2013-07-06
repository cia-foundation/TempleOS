U0 ViewComics()
{
  I64 i=0;
  U8 *st;
  CDirEntry *tempm,*tempm1;
  CDoc *doc=DocNew;
  DocPutS(doc,"$$FG,LTBLUE$$");
  tempm=FilesFind("Comics/*.TXT*");
  tempm1=tempm;
  while (tempm1) {
    st=StrNew(tempm1->name);
    FileExtRem(st);
    tempm1->user_data=DocPrintF(doc,"$$MU-UL,\"%-10ts\",%d$$ ",st,tempm1);
    Free(st);
    tempm1=tempm1->next;
    if ((i++&3)==3)
      DocPutS(doc,"\n");
  }
  while (TRUE) {
    if ((tempm1=PopUpMenu(doc))>0)
      Ed(tempm1->full_name);
    else
      break;
  }
  DocDel(doc);
  CDirLstDel(tempm);
}

