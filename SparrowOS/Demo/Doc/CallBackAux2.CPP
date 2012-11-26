//See $LK,"::/SparrowOS/Demo/Doc/CallBack.CPP.Z","FI:::/SparrowOS/Demo/Doc/CallBack.CPP.Z"$ first.

U8 *tSCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  CDate ltdt=tS*CDATE_FREQ;
  SPrintF(st,"%d=%h11T",ltdt,ltdt);
  return st;
}

U0 DoIt()
{
  CDocEntry *doc_e;
  CDoc *aux2=DocNew;

  aux2->flags|=DOCF_AUX_DOC;

  DocPutS(aux2,"$$FG,RED$$$$CM+BY+LX,5,-3$$");
  doc_e=DocPutS(aux2,"$$TX+TC,\" \"$$");
  doc_e->tag_cb=&tSCB;
  DocPutS(aux2,"$$FG$$");

  DocQueInsRev(aux2,DocAux);

  NewLine;
}

U0 UndoIt()
{
  CDoc *aux2=DocAux;
  DocQueRem(aux2);
  DocDel(aux2);
}

DoIt;
