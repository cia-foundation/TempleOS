//This example shows bttns.  Bttns return a menu val
//unless you rig them to send macros.  Menu entries are
//about the same as bttns, but don't have a border.

I64 PopUpFreq()
{//See also $LK,"PopUpRangeI64",A="MN:PopUpRangeI64"$()
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$TX+CX,\"Set Freq\"$$\n" //Centered text
	"$$CM+LX,2,4$$$$BT,\"100 Hz\",LE=100$$"
	"$$CM+LX,18,0$$$$BT,\"200 Hz\",LE=200$$"
	"$$CM+LX,2,4$$$$BT,\"400 Hz\",LE=400$$"
	"$$CM+LX,18,0$$$$BT,\"800 Hz\",LE=800$$\n");
  i=PopUpMenu(doc);
  if (i<0) i=0; // <SHIFT-ESC>
  DocDel(doc);
  return i;
}

#define MU_NOTHING	0
#define MU_SET_FREQ	1
#define MU_SND_ON	2
#define MU_SND_OFF	3

U0 MenuBttn()
{
  Bool done=FALSE;
  I64 i,j=0,freq=100;
  I64 old_flags=DocPut->flags;
//This allows keyboard navigation to skip nonselible entries.
  DocPut->flags|=DOCF_FORM;
  do {
    DocClear;
//Use <CTRL-l> to generate cursor movement expressions and check "Quote".
    "$$CM+CX-RE,-4$$Menu Demo\n\n";
    "$$LM,8$$"; //Set left margin
//These are bttns that return a val from a menu selection.
    if (!j)
      "\n$$BT,\"Snd On\",LE=MU_SND_ON$$\n\n\n";
    else
      "\n$$BT,\"Snd Off\",LE=MU_SND_OFF$$\n\n\n";
    "\n$$BT,\"Set Freq\",LE=MU_SET_FREQ$$\n\n\n"
	  "\n$$BT,\"Nothing\",LE=MU_NOTHING$$\n\n\n"
	  "\n$$BT,\"Done\",LE=DOCM_CANCEL$$\n\n\n";
    i=DocMenu(DocPut);
    DocBottom;
    switch (i) {
      case MU_NOTHING:
	break;
      case MU_SND_ON:
	j=freq;
	Snd(Freq2Ona(j));
	break;
      case MU_SND_OFF:
	j=0;
	Snd;
	break;
      case MU_SET_FREQ:
	freq=PopUpFreq;
	if (j) {
	  j=freq;
	  Snd(Freq2Ona(j));
	}
	break;
      default:
	done=TRUE;
    }
  } while (!done);
  DocPut->flags=DocPut->flags&~DOCF_FORM |
	old_flags & DOCF_FORM;
  DocClear;
  Snd;
}

MenuBttn;
