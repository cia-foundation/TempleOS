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
	DU8	CH_CTRLQ,CH_CTRLW,CH_CTRLE,CH_CTRLR,CH_CTRLT,CH_CTRLY,CH_CTRLU,CH_CTRLI,CH_CTRLO,CH_CTRLP,"[]",'\n',0,CH_CTRLA,CH_CTRLS;
	DU8	CH_CTRLD,CH_CTRLF,CH_CTRLG,CH_CTRLH,CH_CTRLJ,CH_CTRLK,CH_CTRLL,";'\`",0,"\\",CH_CTRLZ,CH_CTRLX,CH_CTRLC,CH_CTRLV;
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
  throw(EXCEPT_LOCAL);
}

I64 KbdCmdRead()
{
  F64 timeout=tS+0.125;
  while (tS<timeout)
    if (InU8(KBD_CTRL)&1)
      return InU8(KBD_PORT);
  throw(EXCEPT_LOCAL);
}

U0 KbdCmdFlush()
{
  F64 timeout=tS+0.03;
  while (tS<timeout)
    InU8(KBD_PORT);
}

U0 KbdSetLEDs(I64 sc)
{
  Bool old_preempt=Preempt(OFF);
  U8 v=0;
  BEqu(&v,0,Bt(&sc,SCf_SCROLL));
  BEqu(&v,1,Bt(&sc,SCf_NUM));
  BEqu(&v,2,Bt(&sc,SCf_CAPS));
  try {
    KbdCmdSend(KBD_PORT,0xED);
    KbdCmdSend(KBD_PORT,v);
  } catch
    Fs->catch_except=TRUE;
  Preempt(old_preempt);
}

U0 KbdMouseCmdAck(U8 val)
{
  I64 ack,timeout=5;
  do {
    ack=0;
    try {
      KbdCmdSend(KBD_CTRL,0xD4);
      KbdCmdSend(KBD_PORT,val);
      ack=KbdCmdRead;
    } catch {
      KbdCmdFlush;
      Fs->catch_except=TRUE;
    }
  } while (ack!=0xFA && --timeout);
  if (!timeout)
    throw(EXCEPT_LOCAL);
}

U0 SetTypeMatic(U8 delay)
{
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

I64 A2ScanCode(I64 ch,I64 sc_flags=0)
{ //ASCII val to scan code (Slow)
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


U8 ScanCode2A(I64 sc)
{//Scan Code to ASCII val
  U8 *table=NORMAL_KEY_SCAN_DECODE_TABLE;
  if (sc & SCF_E0_PREFIX)
    return 0;
  if (sc & SCF_CTRL)
    table=CTRL_KEY_SCAN_DECODE_TABLE;
  else if (sc & SCF_SHIFT) {
    if (!(sc & SCF_CAPS))
      table=SHIFT_KEY_SCAN_DECODE_TABLE;
  } else {
    if (sc & SCF_CAPS)
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
  0,0,0,0,0,0,0,SC_HOME,SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,0,0,0,0,SC_GUI,0,0,0,
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
  0,0,0,0,0,0,0,SC_HOME,SC_CURSOR_UP,SC_PAGE_UP,0,SC_CURSOR_LEFT,0,SC_CURSOR_RIGHT,0,SC_END,
  SC_CURSOR_DOWN,SC_PAGE_DOWN,SC_INS,SC_DELETE,0,0,0,0,0,0,0,0,SC_GUI,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

U8 *A2KeyName(I64 ch,Bool include_ctrl=TRUE)
{//ASCII val to key name
  I64 i;
  U8 buf[136];
  if (ch<=CH_SPACE) {
    nobound_switch (ch) {
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
      case 0: //nobound_switch
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
  } else if (Bt(printable_chars_bitmap,ch)) {
    *buf=ch;
    buf[1]=0;
  } else
    *buf=0;
  return StrNew(buf);
}

U8 *ScanCode2KeyName(I64 sc)
{
  I64 ch;
  U8 buf[136],*st;
  *buf=0;
  if (sc&SCF_CTRL)
    StrCat(buf,"CTRL ");
  if (sc&SCF_ALT)
    StrCat(buf,"ALT ");
  if (sc&SCF_SHIFT)
    StrCat(buf,"SHIFT ");
  if (ch=ScanCode2A(sc&255)) {
    st=A2KeyName(ch,FALSE);
    StrCpy(buf+StrLen(buf),st);
    Free(st);
  } else {
    switch (sc&255) {
      case SC_BACKSPACE:StrCat(buf,"BACK"); break;
      case SC_CAPS:	StrCat(buf,"CAPS"); break;
      case SC_NUM:	StrCat(buf,"NUM"); break;
      case SC_SCROLL:	StrCat(buf,"SCROLL"); break;
      case SC_CURSOR_UP:StrCat(buf,"UP"); break;
      case SC_CURSOR_DOWN:StrCat(buf,"DOWN"); break;
      case SC_CURSOR_LEFT:StrCat(buf,"LEFT"); break;
      case SC_CURSOR_RIGHT:StrCat(buf,"RIGHT"); break;
      case SC_PAGE_UP:StrCat(buf,"PAGE_UP"); break;
      case SC_PAGE_DOWN:StrCat(buf,"PAGE_DOWN"); break;
      case SC_HOME:	StrCat(buf,"HOME"); break;
      case SC_END:	StrCat(buf,"END"); break;
      case SC_INS:	StrCat(buf,"INS"); break;
      case SC_DELETE:	StrCat(buf,"DELETE"); break;
      case SC_F1:	StrCat(buf,"F1"); break;
      case SC_F2:	StrCat(buf,"F2"); break;
      case SC_F3:	StrCat(buf,"F3"); break;
      case SC_F4:	StrCat(buf,"F4"); break;
      case SC_F5:	StrCat(buf,"F5"); break;
      case SC_F6:	StrCat(buf,"F6"); break;
      case SC_F7:	StrCat(buf,"F7"); break;
      case SC_F8:	StrCat(buf,"F8"); break;
      case SC_F9:	StrCat(buf,"F9"); break;
      case SC_F10:	StrCat(buf,"F10"); break;
      case SC_F11:	StrCat(buf,"F11"); break;
      case SC_F12:	StrCat(buf,"F12"); break;
      case SC_GUI:	StrCat(buf,"WINDOWS"); break;
      case SC_PRTSCRN1:	StrCat(buf,"PRTSCRN1"); break;
      case SC_PRTSCRN2:	StrCat(buf,"PRTSCRN2"); break;
    }
  }
  return StrNew(buf);
}

I64 KbdBuildSC(U8 raw_byte,U8 *_last_raw_byte,I64 *_last_sc,
  Bool *_set_LEDs,Bool *_ctrl_alt)
{
  I64 sc_flags,sc,sc2,sc_raw;
  *_set_LEDs=FALSE;
  *_ctrl_alt=FALSE;
  if (raw_byte==0xE0) {
    *_last_raw_byte=raw_byte;
    return 0;
  } else {
    sc=raw_byte;
    BEqu(&sc,SCf_E0_PREFIX,*_last_raw_byte==0xE0);
    BEqu(&sc,SCf_KEY_UP,raw_byte & 0x80);
    *_last_raw_byte=raw_byte;
  }
  sc_flags=_last_sc->u32[0]&~0x1FF;
  sc_raw=sc;

  if (sc_flags & SCF_NUM) {
    if (sc2=num_lock_map[sc.u8[0]])
      sc.u8[0]=sc2;
  } else {
    if (sc2=scan_code_map[sc.u8[0]])
      sc.u8[0]=sc2;
  }
  if (sc==SC_SHIFT) {
    sc_flags|=SCF_SHIFT;
    goto new_key_done;
  }
  if (sc==SC_SHIFT+SCF_KEY_UP) {
    sc_flags&=~SCF_SHIFT;
    goto new_key_done;
  }

  if (sc==SC_CTRL) {
    sc_flags|=SCF_CTRL;
    goto new_key_done;
  }
   if (sc==SC_CTRL+SCF_KEY_UP) {
    sc_flags&=~SCF_CTRL;
    goto new_key_done;
  }

  if (sc==SC_ALT) {
    sc_flags|=SCF_ALT;
    goto new_key_done;
  }
  if (sc==SC_ALT+SCF_KEY_UP) {
    sc_flags&=~SCF_ALT;
    goto new_key_done;
  }

  if (sc==SC_DELETE)
    sc_flags|=SCF_DELETE;
  else if (sc==SC_DELETE+SCF_KEY_UP)
    sc_flags&=~SCF_DELETE;

  if (sc==SC_INS)
    sc_flags|=SCF_INS;
  else if (sc==SC_INS+SCF_KEY_UP)
    sc_flags&=~SCF_INS;

  if (sc==SC_CAPS)
    goto new_key_done;
  if (sc==SC_CAPS+SCF_KEY_UP) {
    sc_flags^=SCF_CAPS;
    *_set_LEDs=TRUE;
    goto new_key_done;
  }

  if (sc==SC_NUM)
    goto new_key_done;
  if (sc==SC_NUM+SCF_KEY_UP) {
    sc_flags^=SCF_NUM;
    *_set_LEDs=TRUE;
    goto new_key_done;
  }

  if (sc==SC_SCROLL)
    goto new_key_done;
  if (sc==SC_SCROLL+SCF_KEY_UP) {
    sc_flags^=SCF_SCROLL;
    *_set_LEDs=TRUE;
    goto new_key_done;
  }

  if (sc_flags & SCF_CTRL && sc_flags & SCF_ALT) {
    sc_flags&=~SCF_NEW_KEY;
    *_ctrl_alt=TRUE;
  } else {
new_key_done:
    sc_flags|=SCF_NEW_KEY;
  }
  return *_last_sc=sc_flags|sc|(sc_flags|sc_raw)<<32;
}

U0 KbdPktRead()
{
  static U8 last_raw_byte=0;
  static I64 last_sc=0;
  U8 raw_byte;
  I64 ch,sc;
  Bool set_LEDs,ctrl_alt;

  if (GetTSC>kbd_evt_time+time_stamp_freq>>3)
    FifoU8Flush(kbd_fifo);
  kbd_evt_time=GetTSC;
  raw_byte=InU8(KBD_PORT);
  sc=KbdBuildSC(raw_byte,&last_raw_byte,&last_sc,&set_LEDs,&ctrl_alt);
  if (ctrl_alt && sc) {
    if (!(sc & SCF_KEY_UP)) {
      if (sc&255==SC_DELETE)
	CtrlAltDel(last_sc);
      else {
	if (sc&255==SC_ESC)
	  ch='e';
	else if (sc&255==SC_TAB)
	  ch='t';
	else
	  ch=ScanCode2A(sc&255);
	if ('a'<=ch<='z' && fp_ctrl_alt_let_cbs[ch-'a'] &&
	    Bt(&ctrl_alt_let_in_irq_flags,ch-'a')) {
	  (*fp_ctrl_alt_let_cbs[ch-'a'])(last_sc);
	  return;
	}
      }
    }
  }
  if (!FifoU8Cnt(kbd_fifo)) {
    FifoU8Ins(kbd_fifo,raw_byte);
    if (raw_byte!=0xE0) {
      while (FifoU8Rem(kbd_fifo,&raw_byte))
	FifoU8Ins(kbd2_fifo,raw_byte);
    }
  } else {
    FifoU8Ins(kbd_fifo,raw_byte);
    while (FifoU8Rem(kbd_fifo,&raw_byte))
      FifoU8Ins(kbd2_fifo,raw_byte);
  }
}

interrupt U0 KbdIntHandler()
{
  Cld;
  OutU8(0x20,0x20);
  kbd_irqs_working=TRUE;
  if (mouse_install_attempt_in_progress) {
    kbdmouse_rst=TRUE;
    return;
  }
  ctrl_alt_let_ret_addr=GetRBP()(I64)+8;
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
    KbdSetLEDs(sys_cur_scan_code);
  } catch {
    KbdCmdFlush;
    Fs->catch_except=TRUE;
  }
  IntEntrySet(0x21,&KbdIntHandler);
  OutU8(0x21,InU8(0x21)&~2);

  SetCtrlAltLetCB('C',&CtrlAltC,"Sys/Break Execution",TRUE);
  SetCtrlAltLetCB('D',&CtrlAltD,"Sys/Enter Debugger",TRUE);
  SetCtrlAltLetCB('E',&CtrlAltE,"Sys/Spawn New User Task");
  SetCtrlAltLetCB('F',&CtrlAltF,"Sys/VGA Flush",TRUE);
  SetCtrlAltLetCB('M',&CtrlAltM,"Sys/Toggle Mute");
  SetCtrlAltLetCB('T',&CtrlAltT,"Sys/Switch Focus to Next Task",TRUE);
  SetCtrlAltLetCB('X',&CtrlAltX,"Sys/Kill Focused Task",TRUE);
}

U0 KbdHandler()
{
  static U8 last_raw_byte=0;
  U8  raw_byte;
  Bool set_LEDs,ctrl_alt;
  I64 ch,sc;

  FifoU8Rem(kbd2_fifo,&raw_byte);
  sc=KbdBuildSC(raw_byte,&last_raw_byte,&sys_cur_scan_code,&set_LEDs,&ctrl_alt);
  if (set_LEDs)
    KbdSetLEDs(sc);
  if (raw_byte==0xE0) {
    FifoU8Rem(kbd2_fifo,&raw_byte);
    sc=KbdBuildSC(raw_byte,&last_raw_byte,&sys_cur_scan_code,&set_LEDs,&ctrl_alt);
    if (set_LEDs)
      KbdSetLEDs(sc);
  }
  if (ctrl_alt) {
    if (!(sc & SCF_KEY_UP)) {
      if (sc&255==SC_ESC)
	ch='e';
      else if (sc&255==SC_TAB)
	ch='t';
      else
	ch=ScanCode2A(sc&255);
      if ('a'<=ch<='z' && fp_ctrl_alt_let_cbs[ch-'a'] &&
	!Bt(&ctrl_alt_let_in_irq_flags,ch-'a'))
	(*fp_ctrl_alt_let_cbs[ch-'a'])(sys_cur_scan_code);
    }
    return;
  }
  if (Btr(&sys_cur_scan_code,SCf_NEW_KEY)) {
    Btr(&sys_cur_scan_code,32+SCf_NEW_KEY);
    FifoI64Ins(scan_code_fifo,sys_cur_scan_code);
    sys_key_cnt++;
    if (!(sys_cur_scan_code&SCF_KEY_UP)) {
      sys_last_down_scan_code=sys_cur_scan_code;
      Bts(key_down_bitmap,sys_cur_scan_code.u8[0]);
      Bts(key_down_bitmap2,sys_cur_scan_code.u8[4]);
    } else {
      Btr(key_down_bitmap,sys_cur_scan_code.u8[0]);
      Btr(key_down_bitmap2,sys_cur_scan_code.u8[4]);
    }
  }
}

I64 KbdMsgsQue()
{
  I64 p1,p2,msg_code=MSG_NULL;
  CTask *task_focus;
  if (task_focus=sys_focus_task) {
    while (FifoI64Rem(scan_code_fifo,&p2)) {
      p1=ScanCode2A(p2);
      if (p2 & SCF_KEY_UP) {
	TaskMsgQue(task_focus,0,
	  MSG_KEY_UP,p1,p2,0);
	msg_code=MSG_KEY_UP;
      } else {
	TaskMsgQue(task_focus,0,
	  MSG_KEY_DOWN,p1,p2,0);
	msg_code=MSG_KEY_DOWN;
      }
    }
  }
  return msg_code;
}

I64 KbdMouseEvtTime()
{
  if (mouse_evt_time>kbd_evt_time)
    return mouse_evt_time;
  else
    return kbd_evt_time;
}
