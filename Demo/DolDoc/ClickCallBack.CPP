I64 MyLeftCB1(CDoc *,CDocEntry *)
{
  I64 i;
  for (i=500;i<1000;i+=25) {
    Snd(i);
    Sleep(10);
  }
  Snd(0);
  return 0;
}

U0 ClickCallBack()
{
  CDocEntry *doc_e;
  DocLock(DocPut);
//The $LK,"DocPrint",A="MN:DocPrint"$() routine returns the addr of the last entry.
  doc_e=DocPrint(DocPut,"\n$$MA+LC,\"Click Me\"$$ ");
//The LC flag is "has left callback".

//Flags are explained here:
//$LK,"::/Doc/DolDocOverview.TXT"$	$LK,"::/Doc/Widget.TXT"$
//$LK,"Dollar Flags",A="MN:DOCEf_TAG"$		$LK,"ST_DOC_FLAGS",A="FF:::/Adam/DolDoc/DocInit.CPP,ST_DOC_FLAGS"$
  "(Not preserved when <CTRL-t> and back.)\n\n";
  doc_e->left_cb=&MyLeftCB1;
  DocUnlock(DocPut);
}

ClickCallBack;

I64 MyLeftCB2(CDoc *,CDocEntry *)
{
  return cnts.jiffies;
}

U0 PopUpJiffiesCallBack()
{
  CDoc *doc=DocNew;
  CDocEntry *doc_e=DocPrint(doc,"\n\n\n$$BT+LC,\"Jiffies\"$$\n\n\n");
  doc_e->left_cb=&MyLeftCB2;
  "Current Jiffies:%d\n",PopUpMenu(doc);
  DocDel(doc);
}

PopUpJiffiesCallBack;
PopUpJiffiesCallBack;
PopUpJiffiesCallBack;
