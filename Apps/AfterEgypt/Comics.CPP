U0 ViewComics()
{
  I64 i=0;
  U8 *st;
  CDirEntry *tempde=FilesFind("Comics/*.TXT*"),*tempde1=tempde;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$LTBLUE$$");
  while (tempde) {
    st=StrNew(tempde->name);
    FileExtRem(st);
    tempde->user_data=DocPrint(doc,"$$MU-UL,\"%-10ts\",LE=%d$$ ",st,tempde);
    Free(st);
    tempde=tempde->next;
    if ((i++&3)==3)
      DocPrint(doc,"\n");
  }
  while (TRUE) {
    if ((tempde=PopUpMenu(doc))>0)
      Ed(tempde->full_name);
    else
      break;
  }
  DocDel(doc);
  DirLstDel(tempde1);
}

