/*$$DA	is the data widget.
   -TRM	flag is for var
	width fields with a terminator
	character.
   +RD	refreshes the data.
   +UD	updates the val when you edit it.

   -P	Means it is a string var, basically.

   ,32	sets the tag string width.
	See $LK,"Data Tag Width",A="FA:::/Adam/DolDoc/DocPlain.HC,DataTagWidth"$.

   ,RT=I16 means the val is 2 bytes.
	See $LK,"DocDataFmt",A="MN:DocDataFmt"$() and $LK,"DocDataScan",A="MN:DocDataScan"$().
*/

I16 i=0;
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
  CDoc *doc=DocPut;

  //We do this to prevent access to
  //doc_e->data before it is set.
  Bool unlock=DocLock(doc);

  //You might set the $LK,"DOCF_FORM",A="MN:DOCF_FORM"$ flag.
  //  doc->flags|=DOCF_FORM
  //if you wish.

  Spawn(&UpdateGlblTask,NULL,"Update Glbl",,Fs);

  "Enter editor overstrike mode\n"
	"and you can modify the val.\n"
	"However, changes happen immediately,\n"
	"so it's tricky.\n\n";
//Use <CTRL-l> for the $$DA...$$ format.
  doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,RT=I16,A=\"%%7d\"$$\n");
  doc_e->data=&i;

  StrCpy(buf,"Terry");
  doc_e=DocPrint(doc,"$$DA-P+RD+UD,LEN=7,A=\"Str:%%s\"$$\n");
  doc_e->data=buf;

  if (unlock)
    DocUnlock(doc);
}

DataDemo;

//See $LK,"PopUpExtents",A="MN:PopUpExtents"$().
