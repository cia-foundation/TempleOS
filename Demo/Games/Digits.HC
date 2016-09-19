//This loads a lst of zero terminated
//strings into the sym table and
//gives a name to it.  Strings like
//this are known as Define entries.

//See $LK,"ST_RAINBOW_10",A="FF:::/Adam/Gr/GrPalette.HC,ST_RAINBOW_10"$

U0 PrintDigit(U8 ch)
{
//The %Z code displays a Define subentry
  if ('0'<=ch<='9')
    "$$FG,%Z$$%d$$FG$$",ch-'0',"ST_RAINBOW_10",ch-'0';
}

U0 PrintPattern(U8 *st)
{
  I64 ch;
  while (ch=*st++)
    PrintDigit(ch);
  '\n';
}

U0 Digits()
{
  I64 num,ch,i;
  U8 answer[1024];

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=DKGRAY<<4+WHITE;
  DocClear;
//Set default background and foreground
  "$$WW,1$$$$BG,DKGRAY$$$$FD,WHITE$$"
	"\nThis is a memory game.  "
	"Try to remember the digits and enter them.  "
	"They are colored based on electrical "
	"engineering color codes.\n";
  for (i=0;i<10;i++)
    "$$FG,%Z$$%d: %Z\n",i,"ST_RAINBOW_10",i,i,"ST_RAINBOW_10";
  "$$FG$$\n"; //set to default background
  ch=PressAKey;
  if (ch==CH_ESC||ch==CH_SHIFT_ESC) goto dg_done;
  while (TRUE) {
restart:
    num=0;
//Set to zeros so we have terminators
    MemSet(answer,0,sizeof(answer));
    while (TRUE) {
      DocClear;
      "$$WW,1$$$$BG,DKGRAY$$$$FD,WHITE$$\n";
      "Pattern\nLength:%d\n\n",num+1;
      answer[num++]=RandU32%10+'0';
      PrintPattern(answer);
      ch=PressAKey;
      if (ch==CH_ESC||ch==CH_SHIFT_ESC) goto dg_done;
      DocClear;
      "$$WW,1$$$$BG,DKGRAY$$$$FD,WHITE$$\n";
      "Guess\nLength:%d\n\n",num;
      for (i=0;i<num;i++) {
	ch=GetChar(,FALSE);
	if (ch==CH_ESC||ch==CH_SHIFT_ESC) goto dg_done;
	PrintDigit(ch);
	if (ch!=answer[i]) {
	  "\nScore:%d\n",num;
	  PrintPattern(answer);
	  Beep; Beep;
	  ch=PressAKey;
	  if (ch==CH_ESC||ch==CH_SHIFT_ESC)
	    goto dg_done;
	  else
	    goto restart;
	}
      }
    }
  }
dg_done:
  DocClear;
  SettingsPop;
}

Digits;
