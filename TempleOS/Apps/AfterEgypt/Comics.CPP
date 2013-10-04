U0 ViewComics()
{
  I64 i=0;
  U8 *st;
  CDirEntry *tempde,*tempde1;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,LTBLUE$$");
  tempde=FilesFind("Comics/*.TXT*");
  tempde1=tempde;
  while (tempde1) {
    st=StrNew(tempde1->name);
    FileExtRem(st);
    tempde1->user_data=DocPrint(doc,"$$MU-UL,\"%-10ts\",%d$$ ",st,tempde1);
    Free(st);
    tempde1=tempde1->next;
    if ((i++&3)==3)
      DocPrint(doc,"\n");
  }
  while (TRUE) {
    if ((tempde1=PopUpMenu(doc))>0)
      Ed(tempde1->full_name);
    else
      break;
  }
  DocDel(doc);
  DirLstDel(tempde);
}

