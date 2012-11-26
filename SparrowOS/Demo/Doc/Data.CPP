/*
  $$DA	  is the data widget.
     -T  flag is for var
	 width fields with a terminator
	 character.
     +RD refreshes the data.
     +UD updates the val when you edit it.
     +U32 means the val is 4 bytes.
     -I64 I64 is the default so we might
	 want to turn-it-off.  Strings
	 use I64.
    ,32  sets the tag string width.
	 See $LK,"Data Tag Width","FA:::/SparrowOS/Adam/Doc/DocPlain.CPP.Z,DataTagWidth"$.
     -P  Means it is a string var, basically.
*/



I32 i;
U8 buf[8];

U0 UpdateGlblTask(I64)
{
  while (TRUE) {
    i++;
    Sleep(1);
  }
}


U0 DataDemo()
{
  CDocEntry *doc_e;

//This is the command line document.
  CDoc *doc=DocCur;

//We do this to prevent access to
//doc_e->data before it is set.
  Bool unlock=DocLock(doc);

//You might set the $LK,"DOCF_FORM","MN:DOCF_FORM"$ flag.
//  doc->flags|=DOCF_FORM
//if you wish.

  Spawn(&UpdateGlblTask,NULL,"Update Glbl",Fs);

  "Enter editor overstrike mode\n"
  "and you can modify the val.\n"
  "However, changes happen immediately,\n"
  "so it's tricky.\n\n";
  //Use <CTRL-L> for the $$DA,$$ format.
  doc_e=DocPutS(doc,"$$DA-T+RD+UD-I64+U32,32,\"%12d\"$$\n");
  doc_e->data=&i;

  StrCpy(buf,"Terry");
  doc_e=DocPutS(doc,"$$DA-P+RD+UD,7,\"Str:%s\"$$\n");
  doc_e->data=buf;

  if (unlock)
    DocUnlock(doc);
}

DataDemo;
