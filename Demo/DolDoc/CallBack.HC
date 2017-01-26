U8 *tSCB(CDoc *,CDocEntry *,CTask *mem_task)
{//This is called by the window mgr.
//Things would get corrupted
  //if the window mgr used it's own
  //heap, so we use the owning task's heap.
  U8 *st=MAlloc(64,mem_task);
  CDate cdt=tS*CDATE_FREQ;
//Doesn't have to be fixed width!!
  StrPrint(st,"%d=%T",cdt,cdt);
  return st;
}

U8 *CurTimeCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  CDate cdt=Now;
  StrPrint(st,"%D %T",cdt,cdt);
  return st;
}

U0 DoIt()
{
  CDoc *bdoc=DocBorder,*pdoc=DocPut;
  CDocEntry *doc_e;

  DocLock(bdoc);
  DocBottom(bdoc);  //Ins at the bottom
  DocPrint(bdoc,"$$RED$$$$CM+BY+LX,5,-3$$");
//The $LK,"DocPrint",A="MN:DocPrint"$() routine returns the addr of the last entry.
  doc_e=DocPrint(bdoc,"$$TX+TC,\" \"$$");
//The TC flag is "has tag callback".

  //Flags are explained here:
  //$LK,"::/Doc/DolDocOverview.DD"$	$LK,"::/Doc/Widget.DD"$
  //$LK,"Dollar Flags",A="MN:DOCEf_TAG"$		$LK,"ST_DOC_FLAGS",A="FF:::/Adam/DolDoc/DocInit.HC,ST_DOC_FLAGS"$
  doc_e->tag_cb=&tSCB;
  DocPrint(bdoc,"$$FG$$");
  DocUnlock(bdoc);

  //$BK,1$WARNING:$BK,0$ If you use the put_doc you
  //run the risk of the user pressing
  //<CTRL-t> or using the clip, both
  //of which will crash.	So, you might want
  //to use the border_doc.

  DocLock(pdoc);
  DocPrint(pdoc,"$$LTRED$$");
  doc_e=DocPrint(pdoc,"$$TX+TC,\" \"$$");
  doc_e->tag_cb=&CurTimeCB;
  DocPrint(pdoc,"$$FG$$");
  DocUnlock(pdoc);

  //Send carriage return, new line, so
  //that the timer string is not part
  //of the next cmd on the cmd line.
  '\n';
}

U0 UndoIt()
{//Clear-out entries without a +H hold flag.
  DocClear(Fs->border_doc);
}

DoIt;
