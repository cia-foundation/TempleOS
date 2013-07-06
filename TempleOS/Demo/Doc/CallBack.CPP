U8 *tSCB(CDoc *,CDocEntry *,CTask *mem_task)
{
//This routine is called by the window
//mgr things would get corrupted
//if the window mgr used it's own
//heap, so we use the owning task's heap.
  U8 *st=MAlloc(64,mem_task);
  CDate cdt=tS*CDATE_FREQ;
  //Doesn't have to be fixed width!!
  SPrintF(st,"%d=%h11T",cdt,cdt);
  return st;
}

U8 *CurTimeCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  CDate cdt=GetCurTimeDate;
  SPrintF(st,"%h0D %h0T",cdt,cdt);
  return st;
}

U0 DoIt()
{
  CDoc *bdoc=DocBorder,
       *pdoc=DocPut;
  CDocEntry *doc_e;

// We don't want an update between
// creating the CDocEntry and defining the
// callback, so we turn-off preemption.

  Bool old_preempt=Preempt(OFF);

  DocBottom(bdoc);  //Ins at the bottom
  DocPutS(bdoc,"$$FG,RED$$$$CM+BY+LX,5,-3$$");

// The $LK,"DocPutS","MN:DocPutS"$() routine returns the addr of the last entry.
  doc_e=DocPutS(bdoc,"$$TX+TC,\" \"$$");
// The TC flag is "has tag callback".

// Flags are explained here:
//	$LK-A,"::/TempleOS/Doc/Widget.TXT"$
//	$LK-A,"::/TempleOS/Doc/Dollar.TXT"$
//	$LK,"Dollar Flags","MN:DOCEf1_AUX_STR"$
//	$LK,"ST_DOC_FLAGS","FF:::/TempleOS/Adam/Doc/Doc.CPP,ST_DOC_FLAGS"$

  doc_e->tag_cb=&tSCB;
  DocPutS(bdoc,"$$FG$$");

//$BK,1$WARNING:$BK,0$ If you use the put_doc you
//run the risk of the user pressing
//<CTRL-T> or using the clipboard, both
//of which will crash.	So, you might want
//to use the border_doc.

  DocPutS(pdoc,"$$FG,LTRED$$");
  doc_e=DocPutS(pdoc,"$$TX+TC,\" \"$$");
  doc_e->tag_cb=&CurTimeCB;
  DocPutS(pdoc,"$$FG$$");

  Preempt(old_preempt);

//Send carriage return, new line, so
//that the timer string is not part
//of the next cmd on the cmd line.
  NewLine;
}



U0 UndoIt()
{
//Clear-out entries without a +H hold flag.
  DocClear(Fs->border_doc);
}


DoIt;
