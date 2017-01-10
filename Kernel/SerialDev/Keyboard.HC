asm {
NORMAL_KEY_SCAN_DECODE_TABLE::
	DU8	0,CH_ESC,"1234567890-=",CH_BACKSPACE,'\t';
	DU8	"qwertyuiop[]",'\n',0,"as";
	DU8	"dfghjkl;'\`",0,"\\zxcv";
	DU8	"bnm,./",0,'*',0,CH_SPACE,0,0,0,0,0,0;
	DU8	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0;
SHIFT_KEY_SCAN_DECODE_TABLE::
	DU8	0,CH_SHIFT_ESC,"!@#$$%^&*()_+",CH_BACKSPACE,'\t';
	DU8	"QWERTYUIOP{}",'\n',0,"AS";
	DU8	"DFGHJKL:\"~",0,"|ZXCV";
	DU8	"BNM<>?",0,'*',0,CH_SHIFT_SPACE,0,0,0,0,0,0;
	DU8	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0;
CTRL_KEY_SCAN_DECODE_TABLE::
	DU8	0,CH_ESC,"1234567890-=",CH_BACKSPACE,'\t';
	DU8	CH_CTRLQ,CH_CTRLW,CH_CTRLE,CH_CTRLR,CH_CTRLT,CH_CTRLY,CH_CTRLU,
		CH_CTRLI,CH_CTRLO,CH_CTRLP,"[]",'\n',0,CH_CTRLA,CH_CTRLS;
	DU8	CH_CTRLD,CH_CTRLF,CH_CTRLG,CH_CTRLH,CH_CTRLJ,CH_CTRLK,CH_CTRLL,
		";'\`",0,"\\",CH_CTRLZ,CH_CTRLX,CH_CTRLC,CH_CTRLV;
	DU8	CH_CTRLB,CH_CTRLN,CH_CTRLM,",./",0,'*',0,CH_SPACE,0,0,0,0,0,0;
	DU8	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0;
}

U0 KbdCmdSend(I64 port, U8 val)
{
  F64 timeout=tS+0.125;
  while (tS<timeout) {
    if (!(InU8(KBD_CTRL)&2)) {
      OutU8(port,val);
      return;
    }
  }
  throw;
}

I64 KbdCmdRead()
{
  F64 timeout=tS+0.125;
  while (tS<timeout)
    if (InU8(KBD_CTRL)&1)
      return InU8(KBD_PORT);
  throw;
}

U0 KbdCmdFlush()
{
  F64 timeout=tS+0.03;
  while (tS<timeout)
    InU8(KBD_PORT);
}

U0 KbdLEDsSet(I64 sc)
{
  U8 v=0;
  BEqu(&v,0,Bt(&sc,SCf_SCROLL));
  BEqu(&v,1,Bt(&sc,SCf_NUM));
  BEqu(&v,2,Bt(&sc,SCf_CAPS));
  try {
    KbdCmdSend(KBD_PORT,0xED);
    KbdCmdSend(KBD_PORT,v);
  } catch
    Fs->catch_except=TRUE;
}

U0 KbdMsCmdAck(...)
{
  I64 i,ack,timeout;
  for (i=0;i<argc;i++) {
    timeout=5;
    do {
      ack=0;
      try {
	KbdCmdSend(KBD_CTRL,0xD4);
	KbdCmdSend(KBD_PORT,argv[i]);
	ack=KbdCmdRead;
      } catch {
	KbdCmdFlush;
	Fs->catch_except=TRUE;
      }
    } while (ack!=0xFA && --timeout);
    if (!timeout)
      throw;
  }
}

U0 KbdTypeMatic(U8 delay)
{//Set speed of repeated keys.
  try {
    KbdCmdSend(KBD_CTRL,0xA7); //Disable Mouse
    KbdCmdSend(KBD_CTRL,0xAE); //Enable Keyboard
    KbdCmdSend(KBD_PORT,0xF3);
    KbdCmdSend(KBD_PORT,delay); //Typematic rate
    KbdCmdSend(KBD_CTRL,0xA8); //Enable Mouse
  } catch {
    KbdCmdFlush;
    Fs->catch_except=TRUE;
  }
}

I64 Char2ScanCode(I64 ch,I64 sc_flags=0)
{//ASCII val to scan code (Slow).
  I64 i;
  U8 *table;
  if (sc_flags) {
    table=NORMAL_KEY_SCAN_DECODE_TABLE;
    if (sc_flags & SCF_CTRL || ch<26)
      table=CTRL_KEY_SCAN_DECODE_TABLE;
    else if (sc_flags & SCF_SHIFT || 'A'<=ch<='Z') {
      if (!(sc_flags & SCF_CAPS))
	table=SHIFT_KEY_SCAN_DECODE_TABLE;
    } else {
      if (sc_flags & SCF_CAPS)
	table=SHIFT_KEY_SCAN_DECODE_TABLE;
    }
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|sc_flags;
    return sc_flags;
  } else {
    table=NORMAL_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i;
    table=SHIFT_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|SCF_SHIFT;
    table=CTRL_KEY_SCAN_DECODE_TABLE;
    for (i=0;i<0x50;i++)
      if (table[i]==ch)
	return i|SCF_CTRL;
    return 0;
  }
}

U8 ScanCode2Char(I64 sc)
{//Scan code to ASCII val.
  U8 *table=NORMAL_KEY_SCAN_DECODE_TABLE;
  if (sc&SCF_E0_PREFIX)
    return 0;
  if (sc&SCF_CTRL)
    table=CTRL_KEY_SCAN_DECODE_TABLE;
  else if (sc&SCF_SHIFT) {
    if (!(sc&SCF_CAPS))
      table=SHIFT_KEY_SCAN_DECODE_TABLE;
  } else {
    if (sc&SCF_CAPS)
      table=SHIFT_KEY_SCAN_DECODE_TABLE;
  }
  sc&=0x7F;
  if (sc>=0x50)
    return 0;
  else
    return table[sc];
}

U8 scan_code_map[0x100]={
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,SC_SHIFT,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,SC_ENTER,SC_CTRL,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0x35,0,0,SC_ALT,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,SC_HOME,
	SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,
	0,0,0,0,SC_GUI,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

U8 num_lock_map[0x100]={
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,8,9,10,0,5,6,7,0,2,
  3,4,11,0x34,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,SC_ENTER,SC_CTRL,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0x35,0,0,SC_ALT,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,SC_HOME,
	SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,
	0,0,0,0,SC_GUI,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

U8 *Char2KeyName(I64 ch,Bool include_ctrl=TRUE)
{//ASCII val to key name.
  I64 i;
  U8 buf[STR_LEN];
  if (ch<=CH_SPACE) {
    switch [ch] {
      case '\n':
	StrCpy(buf,"ENTER");
	break;
      case CH_BACKSPACE:
	StrCpy(buf,"BACKSPACE");
	break;
      case '\t':
	StrCpy(buf,"TAB");
	break;
      case CH_ESC:
	StrCpy(buf,"ESC");
	break;
      case CH_SHIFT_ESC:
	StrCpy(buf,"SHIFT_ESC");
	break;
      case 0: //nobound switch
      case 29:
      case 30:
	*buf=0;
	break;
      case CH_SHIFT_SPACE:
	StrCpy(buf,"SHIFT_SPACE");
	break;
      case CH_SPACE:
	StrCpy(buf,"SPACE");
	break;
      default:
	if (include_ctrl)
	  StrCpy(buf,"CTRL ");
	buf[i=StrLen(buf)]=ch-1+'a';
	buf[i+1]=0;
	break;
    }
  } else if (Bt(char_bmp_printable,ch)) {
    *buf=ch;
    buf[1]=0;
  } else
    *buf=0;
  return StrNew(buf);
}

U8 *ScanCode2KeyName(I64 sc)
{//Scan code to key name.
  I64 ch;
  U8 buf[STR_LEN],*st;
  *buf=0;
  if (sc&SCF_CTRL)
    CatPrint(buf,"CTRL ");
  if (sc&SCF_ALT)
    CatPrint(buf,"ALT ");
  if (sc&SCF_SHIFT)
    CatPrint(buf,"SHIFT ");
  if (sc&SCF_NO_SHIFT)
    CatPrint(buf,"      ");
  if (ch=ScanCode2Char(sc&255)) {
    st=Char2KeyName(ch,FALSE);
    StrCpy(buf+StrLen(buf),st);
    Free(st);
  } else {
    switch (sc&255) {
      case SC_BACKSPACE:CatPrint(buf,"BACK");	break;
      case SC_CAPS:	CatPrint(buf,"CAPS");	break;
      case SC_NUM:	CatPrint(buf,"NUM");	break;
      case SC_SCROLL:	CatPrint(buf,"SCROLL");	break;
      case SC_CURSOR_UP:CatPrint(buf,"UP");	break;
      case SC_CURSOR_DOWN:CatPrint(buf,"DOWN");	break;
      case SC_CURSOR_LEFT:CatPrint(buf,"LEFT");	break;
      case SC_CURSOR_RIGHT:CatPrint(buf,"RIGHT"); break;
      case SC_PAGE_UP:	CatPrint(buf,"PAGE_UP");  break;
      case SC_PAGE_DOWN:CatPrint(buf,"PAGE_DOWN");break;
      case SC_HOME:	CatPrint(buf,"HOME");	break;
      case SC_END:	CatPrint(buf,"END");	break;
      case SC_INS:	CatPrint(buf,"INS");	break;
      case SC_DELETE:	CatPrint(buf,"DELETE");	break;
      case SC_F1:	CatPrint(buf,"F1");	break;
      case SC_F2:	CatPrint(buf,"F2");	break;
      case SC_F3:	CatPrint(buf,"F3");	break;
      case SC_F4:	CatPrint(buf,"F4");	break;
      case SC_F5:	CatPrint(buf,"F5");	break;
      case SC_F6:	CatPrint(buf,"F6");	break;
      case SC_F7:	CatPrint(buf,"F7");	break;
      case SC_F8:	CatPrint(buf,"F8");	break;
      case SC_F9:	CatPrint(buf,"F9");	break;
      case SC_F10:	CatPrint(buf,"F10");	break;
      case SC_F11:	CatPrint(buf,"F11");	break;
      case SC_F12:	CatPrint(buf,"F12");	break;
      case SC_GUI:	CatPrint(buf,"WINDOWS");  break;
      case SC_PRTSCRN1:	CatPrint(buf,"PRTSCRN1"); break;
      case SC_PRTSCRN2:	CatPrint(buf,"PRTSCRN2"); break;
    }
  }
  return StrNew(buf);
}

U0 KbdBuildSC(U8 raw_byte,Bool in_irq,U8 *_last_raw_byte,I64 *_last_sc)
{
  I64 ch,sc_flags,sc,sc2,sc_raw,new_key_f;
  Bool set_LEDs=FALSE;
  if (raw_byte==0xE0) {
    *_last_sc&=~0x1FF;
    *_last_raw_byte=raw_byte;
    return;
  }
  sc=raw_byte;
  BEqu(&sc,SCf_E0_PREFIX,*_last_raw_byte==0xE0);
  BEqu(&sc,SCf_KEY_UP,raw_byte & 0x80);
  *_last_raw_byte=raw_byte;

  sc_flags=_last_sc->u32[0]&~0x1FF;
  sc_raw=sc;

  if (sc_flags & SCF_NUM) {
    if (sc2=num_lock_map[sc.u8[0]])
      sc.u8[0]=sc2;
  } else {
    if (sc2=scan_code_map[sc.u8[0]])
      sc.u8[0]=sc2;
  }

  new_key_f=SCF_NEW_KEY;
  if (sc&SCF_KEY_UP)
    switch (sc&~SCF_KEY_UP) {
      case SC_SHIFT:	sc_flags&=~SCF_SHIFT;	break;
      case SC_CTRL:	sc_flags&=~SCF_CTRL;	break;
      case SC_ALT:	sc_flags&=~SCF_ALT;	break;
      case SC_DELETE:	sc_flags&=~SCF_DELETE;	break;
      case SC_INS:	sc_flags&=~SCF_INS;	break;
      case SC_CAPS:	sc_flags^=SCF_CAPS;	set_LEDs=TRUE;	break;
      case SC_NUM:	sc_flags^=SCF_NUM;	set_LEDs=TRUE;	break;
      case SC_SCROLL:	sc_flags^=SCF_SCROLL;	set_LEDs=TRUE;	break;
    }
  else
    switch (sc) {
      case SC_SHIFT:
	if (Bts(&sc_flags,SCf_SHIFT)) new_key_f=0;
	break;
      case SC_CTRL:
	if (Bts(&sc_flags,SCf_CTRL)) new_key_f=0;
	break;
      case SC_ALT:
	if (Bts(&sc_flags,SCf_ALT)) new_key_f=0;
	break;
      case SC_DELETE:
	sc_flags|=SCF_DELETE;
	break;
      case SC_INS:
	sc_flags|=SCF_INS;
	break;
    }

  sc_flags|=new_key_f;
  sc=sc_flags|sc|(sc_flags|sc_raw)<<32;
  if (sc_flags & SCF_CTRL && sc_flags & SCF_ALT) {
    if (!(sc&SCF_KEY_UP)) {
      if (sc&255==SC_DELETE && !(sc_flags & SCF_SHIFT))
	CtrlAltDel(sc);
      else {
	if (sc&255==SC_ESC)
	  ch='t';
	else if (sc&255==SC_TAB)
	  ch='n';
	else
	  ch=ScanCode2Char(sc&255);
	if ('a'<=ch<='z') {
	  sc&=~(SCF_NEW_KEY|SCF_NEW_KEY<<32);
	  ch-='a';
	  kbd.last_down_scan_code=sc;
	  if (keydev.fp_ctrl_alt_cbs[ch] &&
		Bt(&keydev.ctrl_alt_in_irq_flags,ch)==in_irq &&
		(!(sc_flags & SCF_SHIFT)&&keydev.ctrl_alt_no_shift_descs[ch]) ||
		sc_flags & SCF_SHIFT && keydev.ctrl_alt_shift_descs[ch])
	    (*keydev.fp_ctrl_alt_cbs[ch])(sc);
	}
      }
    }
  }
  if (set_LEDs && !in_irq)
    KbdLEDsSet(sc);
  *_last_sc=sc;
}

U0 KbdPktRead()
{
  static U8 last_raw_byte=0;
  static I64 last_sc=0;
  U8 raw_byte;
  if (GetTSC>kbd.timestamp+cnts.time_stamp_freq>>3)
    FifoU8Flush(kbd.fifo);
  kbd.timestamp=GetTSC;
  raw_byte=InU8(KBD_PORT);
  KbdBuildSC(raw_byte,TRUE,&last_raw_byte,&last_sc);
  if (!FifoU8Cnt(kbd.fifo)) {
    FifoU8Ins(kbd.fifo,raw_byte);
    if (raw_byte!=0xE0) {
      while (FifoU8Rem(kbd.fifo,&raw_byte))
	FifoU8Ins(kbd.fifo2,raw_byte);
    }
  } else {
    FifoU8Ins(kbd.fifo,raw_byte);
    while (FifoU8Rem(kbd.fifo,&raw_byte))
      FifoU8Ins(kbd.fifo2,raw_byte);
  }
}

interrupt U0 IRQKbd()
{
  CLD
  OutU8(0x20,0x20);
  kbd.irqs_working=TRUE;
  if (ms_hard.install_in_progress) {
    kbd.rst=TRUE;
    return;
  }
  keydev.ctrl_alt_ret_addr=GetRBP()(I64)+8;
  KbdPktRead;
}

U0 KbdInit()
{
  try {
    KbdCmdFlush;
    KbdCmdSend(KBD_CTRL,0xA7); //Disable Mouse
    KbdCmdSend(KBD_CTRL,0xAE); //Enable Keyboard
    KbdCmdSend(KBD_PORT,0xF0);
    KbdCmdSend(KBD_PORT,0x02);
    KbdLEDsSet(kbd.scan_code);
  } catch {
    KbdCmdFlush;
    Fs->catch_except=TRUE;
  }
  IntEntrySet(0x21,&IRQKbd);
  OutU8(0x21,InU8(0x21)&~2);
}

U0 KbdHndlr()
{
  static U8 last_raw_byte=0;
  U8  raw_byte;
  FifoU8Rem(kbd.fifo2,&raw_byte);
  KbdBuildSC(raw_byte,FALSE,&last_raw_byte,&kbd.scan_code);
  if (raw_byte==0xE0) {
    FifoU8Rem(kbd.fifo2,&raw_byte);
    KbdBuildSC(raw_byte,FALSE,&last_raw_byte,&kbd.scan_code);
  }
  if (Btr(&kbd.scan_code,SCf_NEW_KEY)) {
    kbd.new_key_timestamp=kbd.timestamp;
    Btr(&kbd.scan_code,32+SCf_NEW_KEY);
    FifoI64Ins(kbd.scan_code_fifo,kbd.scan_code);
    kbd.cnt++;
    if (!(kbd.scan_code&SCF_KEY_UP)) {
      kbd.last_down_scan_code=kbd.scan_code;
      Bts(kbd.down_bitmap,kbd.scan_code.u8[0]);
      Bts(kbd.down_bitmap2,kbd.scan_code.u8[4]);
    } else {
      Btr(kbd.down_bitmap,kbd.scan_code.u8[0]);
      Btr(kbd.down_bitmap2,kbd.scan_code.u8[4]);
    }
  }
}

I64 KbdMsgsQue()
{
  I64 arg1,arg2,msg_code=MSG_NULL;
  CTask *task_focus;
  if (task_focus=sys_focus_task) {
    while (FifoI64Rem(kbd.scan_code_fifo,&arg2)) {
      arg1=ScanCode2Char(arg2);
      if (arg2 & SCF_KEY_UP) {
	TaskMsg(task_focus,0,MSG_KEY_UP,arg1,arg2,0);
	msg_code=MSG_KEY_UP;
      } else {
	TaskMsg(task_focus,0,MSG_KEY_DOWN,arg1,arg2,0);
	msg_code=MSG_KEY_DOWN;
      }
    }
  }
  return msg_code;
}

I64 KbdMsEvtTime()
{//Timestamp of last key or mouse event.
  if (ms_hard.timestamp>kbd.timestamp)
    return ms_hard.timestamp;
  else
    return kbd.new_key_timestamp;
}
