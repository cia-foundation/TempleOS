U0 ViewComics()
{
  I64 i=0;
  U8 *st;
  CDirEntry *tmpde=FilesFind("Comics/*.DD*"),*tmpde1=tmpde;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$LTBLUE$$");
  while (tmpde) {
    st=StrNew(tmpde->name);
    FileExtRem(st);
    tmpde->user_data=DocPrint(doc,"$$MU-UL,\"%-10ts\",LE=%d$$ ",st,tmpde);
    Free(st);
    tmpde=tmpde->next;
    if ((i++&3)==3)
      DocPrint(doc,"\n");
  }
  while (TRUE) {
    if ((tmpde=PopUpMenu(doc))>0)
      Ed(tmpde->full_name);
    else
      break;
  }
  DocDel(doc);
  DirTreeDel(tmpde1);
}

