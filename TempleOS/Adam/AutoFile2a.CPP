#help_index "AutoFile;Training"

public U0 AFPmtStr(U8 *st)
{
  I64 ch,sc;
  U8 buf[136];
  while (*st) {
    ch=GetKey(&sc,FALSE);
    if (sc.u8[0]!=SC_SHIFT &&
	sc.u8[0]!=SC_ALT &&
	sc.u8[0]!=SC_CTRL) {
      if (ch==*st) {
	'' ch;
	st++;
      } else {
	if (*st=='\n')
	  SPrintF(buf,"Press the '$$FG,GREEN$$<ENTER>$$FG$$' key");
	else if (*st==CH_SPACE)
	  SPrintF(buf,"Press the '$$FG,GREEN$$<SPACE BAR>$$FG$$' key");
	else
	  SPrintF(buf,"Press the '$$FG,GREEN$$%c$$FG$$' key",*st);
	PopUpOk(buf);
      }
    }
  }
}

public U0 AFDelayedPutS(U8 *st,I64 mS=100)
{
  I64 ch;
  while (ch=*st++) {
    '' ch;
    Sleep(mS);
  }
}

public U0 AFGetKey(I64 scancode,I64 sc_mask=0xFF|SCF_SHIFT|SCF_CTRL|SCF_ALT)
{
  I64 sc,ch;
  U8 buf[128],*st;
  do {
    ch=GetKey(&sc);
    if (sc.u8[0]!=SC_PRTSCRN1 &&
	!(sc.u8[0]==SC_SHIFT && scancode&SCF_SHIFT) &&
	!(sc.u8[0]==SC_CTRL && scancode&SCF_CTRL) &&
	!(sc.u8[0]==SC_ALT && scancode&SCF_ALT) ) {
      if (sc&sc_mask!=scancode&sc_mask) {
	st=ScanCode2KeyName(scancode);
	SPrintF(buf,"Press the '$$FG,GREEN$$%s$$FG$$' key",st);
	Free(st);
	PopUpOk(buf);
      }
    }
  } while (sc&sc_mask!=scancode&sc_mask);
  Msg(MSG_KEY_DOWN,ch,sc);
}

public U0 AFGetChar(I64 ch1,I64 ch2=0)
{
  I64 sc,ch;
  U8 buf[256],*st1,*st2;
  while (TRUE) {
    ch=GetKey(&sc);
    if (sc.u8[0]!=SC_SHIFT &&
	sc.u8[0]!=SC_ALT &&
	sc.u8[0]!=SC_CTRL) {
      if (ch!=ch1 && (!ch2 || ch!=ch2)) {
	if (!ch2) {
	  st1=A2KeyName(ch1);
	  SPrintF(buf,"Press the '$$FG,GREEN$$%s$$FG$$' key",st1);
	  Free(st1);
	} else {
	  st1=A2KeyName(ch1);
	  st2=A2KeyName(ch2);
	  SPrintF(buf,"Press the '$$FG,GREEN$$%s$$FG$$' key or\n"
		      "the '$$FG,GREEN$$%s$$FG$$' key.",st1,st2);
	  Free(st1);
	  Free(st2);
	}
	PopUpOk(buf);
      } else
	break;
    }
  }
  Msg(MSG_KEY_DOWN,ch,sc);
}

public U0 AFUntilKey(I64 scancode,I64 sc_mask=0xFF|SCF_SHIFT|SCF_CTRL|SCF_ALT)
{
  I64 sc,ch;
  do {
    ch=GetKey(&sc);
    Msg(MSG_KEY_DOWN,ch,sc);
  } while (sc&sc_mask!=scancode&sc_mask);
}

public U0 AFUntilChar(I64 ch1,I64 ch2=0)
{
  I64 sc,ch;
  do {
    ch=GetKey(&sc);
    Msg(MSG_KEY_DOWN,ch,sc);
  } while (ch!=ch1 && (!ch2 || ch!=ch2));
}

I64 af_l,af_r,af_c;

Bool AFSetIPPlot(I64 mS,I64 x,I64 y,I64 z)
{
  SetIP(x,y,z,af_l,af_r,af_c);
  Sleep(mS);
  return TRUE;
}

public U0 AFSetIP(I64 mS=7,I64 x=MAX_I64,I64 y=MAX_I64,I64 z=MAX_I64,I64 l=MAX_I64,I64 r=MAX_I64,I64 c=MAX_I64)
{
  if (!(0<=x<gr_width))
    x=ipx;
  if (!(0<=y<gr_height))
    y=ipy;
  if (z==MAX_I64)
    z=ipz;
  if (!(FALSE<=l<=TRUE))
    l=ip_lb;
  if (!(FALSE<=c<=TRUE))
    c=ip_cb;
  if (!(FALSE<=r<=TRUE))
    r=ip_rb;
  af_l=l; af_r=r; af_c=c;
  Line(mS,ipx,ipy,ipz,x,y,z,&AFSetIPPlot);
}

public Bool AFView()
{
  Bool result=View;
  DocBottom;
  return result;
}
#help_index ""
