#help_index "InFile;Help System/Training"
#help_file "::/Doc/InFile"

public U0 InGetStr(U8 *st)
{//Wait for user to type certain str.
  I64 ch,sc;
  U8 buf[256],*st2;
  while (*st) {
    ch=GetKey(&sc,FALSE);
    if (sc.u8[0]!=SC_SHIFT &&
	  sc.u8[0]!=SC_ALT &&
	  sc.u8[0]!=SC_CTRL) {
      if (ch==*st) {
	'' ch;
	st++;
      } else {
	st2=Char2KeyName(*st);
	StrPrint(buf,"Press the $$GREEN$$<%s>$$FG$$ key.",st2);
	Free(st2);
	PopUpOk(buf);
      }
    }
  }
}

public U0 InPrint(I64 mS=100,U8 *fmt,...)
{//Print message with delay after each char.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st=buf;
  I64 ch;
  while (ch=*st++) {
    '' ch;
    Sleep(mS);
  }
  Free(buf);
}

public U0 InGetKey(I64 scan_code,I64 sc_mask=0xFF|SCF_SHIFT|SCF_CTRL|SCF_ALT)
{//Wait for user to press certain key.
  I64 sc,ch;
  U8 buf[STR_LEN],*st;
  do {
    ch=GetKey(&sc);
    if (sc.u8[0]!=SC_PRTSCRN1 &&
	  !(sc.u8[0]==SC_SHIFT && scan_code&SCF_SHIFT) &&
	  !(sc.u8[0]==SC_CTRL && scan_code&SCF_CTRL) &&
	  !(sc.u8[0]==SC_ALT && scan_code&SCF_ALT) ) {
      if (sc&sc_mask!=scan_code&sc_mask) {
	st=ScanCode2KeyName(scan_code);
	StrPrint(buf,"Press the $$GREEN$$<%s>$$FG$$ key",st);
	Free(st);
	PopUpOk(buf);
      }
    }
  } while (sc&sc_mask!=scan_code&sc_mask);
  Msg(MSG_KEY_DOWN,ch,sc);
}

public I64 InGetChar(...)
{//Wait for user to press one of set of chars.
  I64 i,sc,ch;
  U8 buf[512],*st;
  while (TRUE) {
    ch=GetKey(&sc);
    if (sc.u8[0]!=SC_SHIFT && sc.u8[0]!=SC_ALT && sc.u8[0]!=SC_CTRL) {
      for (i=0;i<argc;i++)
	if (ch==argv[i]) {
	  Msg(MSG_KEY_DOWN,ch,sc);
	  return ch;
	}
      StrPrint(buf,"Press ");
      for (i=0;i<argc;i++) {
	st=Char2KeyName(argv[i]);
	CatPrint(buf,"$$GREEN$$<%s>$$FG$$",st);
	Free(st);
	if (argc==i+1)
	  CatPrint(buf,".");
	else if (argc==i+2)
	  CatPrint(buf," or ");
	else
	  CatPrint(buf,", ");
      }
      PopUpOk(buf);
    }
  }
}

public U0 InUntilKey(I64 scan_code,I64 sc_mask=0xFF|SCF_SHIFT|SCF_CTRL|SCF_ALT)
{//Let user type until he presses certain key.
  I64 sc,ch;
  do {
    ch=GetKey(&sc);
    Msg(MSG_KEY_DOWN,ch,sc);
  } while (sc&sc_mask!=scan_code&sc_mask);
}

public I64 InUntilChar(...)
{//Let user type until he presses one of set of chars.
  I64 i,sc,ch;
  while (TRUE) {
    ch=GetKey(&sc);
    Msg(MSG_KEY_DOWN,ch,sc);
    for (i=0;i<argc;i++)
      if (ch==argv[i])
	return ch;
  }
}

public Bool InView()
{//Let user type until <ESC> or <SHIFT-ESC>.
  Bool res=View;
  DocBottom;
  return res;
}

#help_index "InFile;Help System/Training;Mouse"
I64 in_plot_l,in_plot_r;

Bool InSetMsPlot(I64 mS,I64 x,I64 y,I64 z)
{
  MsSet(x,y,z,in_plot_l,in_plot_r);
  Sleep(mS);
  return TRUE;
}

public U0 InSetMs(I64 mS=7,I64 x=I64_MAX,I64 y=I64_MAX,I64 z=I64_MAX,
		I64 l=I64_MAX,I64 r=I64_MAX)
{//Move mouse to spot at certain speed.
  if (!(0<=x<GR_WIDTH))
    x=ms.pos.x;
  if (!(0<=y<GR_HEIGHT))
    y=ms.pos.y;
  if (z==I64_MAX)
    z=ms.pos.z;
  if (!(FALSE<=l<=TRUE))
    l=ms.lb;
  if (!(FALSE<=r<=TRUE))
    r=ms.rb;
  in_plot_l=l; in_plot_r=r;
  Line(mS,ms.pos.x,ms.pos.y,ms.pos.z,x,y,z,&InSetMsPlot);
}
