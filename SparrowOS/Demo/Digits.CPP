//This loads a list of zero terminated
//strings into the sym table and
//gives a name to it.  Strings like
//this are known as Define entries.

//See $LK,"ST_RAINBOW_10","FF:::/SparrowOS/Adam/Gr/GrScreen.CPP.Z,ST_RAINBOW_10"$

U0 ShowDigit(U8 ch)
{
//The %Z code displays a Define subentry
  "$$FG,%Z$$%d$$FG$$",ch-'0',"ST_RAINBOW_10",ch-'0';
}

U0 ShowPattern(U8 *st)
{
  U8 ch;
  while (ch=*st++)
    ShowDigit(ch);
  NewLine;
}

U0 Digits()
{
  I64 num,ch,i;
  U8 answer[1024];

  DocClear;
  //Set default background and foreground
  "$$WW,1$$$$BD,DKGRAY$$$$FD,WHITE$$"
  "\nThis is a mem game.  "
  "Try to remember the digits and enter them.  "
  "They are colored based on electrical "
  "engineering color codes.\n";
  for (i=0;i<10;i++) {
    "$$FG,%Z$$%d: %Z\n",i,"ST_RAINBOW_10",i,i,"ST_RAINBOW_10";
  }
  "$$FG$$\n"; //set to default background
  if (!PressAKey) goto dg_done;

  while (TRUE) {
restart:
    num=0;
    //Set to zeros so we have terminators
    MemSet(answer,0,sizeof(answer));
    while (TRUE) {
      DocClear;
      "$$WW,1$$$$BD,DKGRAY$$$$FD,WHITE$$\n";
      "Pattern\nLength:%d\n\n",num+1;
      answer[num++]=RandU32%10+'0';
      ShowPattern(answer);
      if (!PressAKey) goto dg_done;

      DocClear;
      "$$WW,1$$$$BD,DKGRAY$$$$FD,WHITE$$\n";
      "Guess\nLength:%d\n\n",num;
      for (i=0;i<num;i++) {
	ch=GetChar(NULL,FALSE);
	if (ch==CH_SHIFT_ESC || ch==CH_ESC)
	  return;
	ShowDigit(ch);
	if (ch!=answer[i]) {
	  Snd(0);
	  "\nScore:%d\n",num;
	  ShowPattern(answer);
	  Beep;
	  Beep;
	  if (!PressAKey) goto dg_done;
	  goto restart;
	}
      }
    }
  }
dg_done:
  "$$CL$$\n";
}

Digits;
