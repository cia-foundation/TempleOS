U0 PutKey(I64 ch=0,I64 sc=0)
{//See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
  CKeyDevEntry *tmpk;
  if (ch||sc) {
    tmpk=keydev.put_key_head.next;
    if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
      if (kbd.scan_code & SCF_SCROLL && sys_focus_task==Fs)
	while (kbd.scan_code & SCF_SCROLL)
	  Yield; //Wait on SCROLL LOCK Key
      while (tmpk!=&keydev.put_key_head) {
	if ((!(sc&SCF_KEY_DESC) || tmpk->flags & KDF_HAS_DESCS) &&
	      (*tmpk->put_key)(ch,sc))
	  break;
	tmpk=tmpk->next;
      }
    }
  }
}

U0 PutChars(U64 ch)
{//Output chars. Up to 8 chars in a single U64.
//Don't use this.  $LK,"See Print() shortcut.",A="FF:::/Doc/HolyC.DD,DemoHolyC"$
  while (ch) {
    PutKey(ch&255,0);
    ch>>=8;
  }
}

U0 PutS(U8 *st)
{//Use $LK,"Print",A="MN:Print"$(). See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
//Don't use this.  $LK,"See Print() shortcut.",A="FF:::/Doc/HolyC.DD,DemoHolyC"$
  I64 ch;
  U8 *ptr;
  Bool cont=TRUE;
  if (!st) return;
  CKeyDevEntry *tmpk=keydev.put_key_head.next;
  if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
    if (kbd.scan_code & SCF_SCROLL && sys_focus_task==Fs)
      while (kbd.scan_code & SCF_SCROLL)
	Yield;
    while (cont && tmpk!=&keydev.put_key_head) {
      if (tmpk->put_s) {
	if ((*tmpk->put_s)(st))
	  break;
      } else {
	ptr=st;
	while (ch=*ptr++)
	  if ((*tmpk->put_key)(ch,0))
	    cont=FALSE;
      }
      tmpk=tmpk->next;
    }
  }
}

U0 KeyDescSet(U8 *fmt,...)
{//Call this from key hndlr to report desc in $LK,"KeyMap",A="MN:KeyMap"$().
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  StrCpy(keydev.desc,buf);
  keydev.hndlr=Caller;
  Free(buf);
}

U0 KeyDevRem(CKeyDevEntry *tmpk)
{//Remove StdOut hook and free.
  QueRem(tmpk);
  Free(tmpk);
}

CKeyDevEntry *KeyDevAdd(Bool (*fp_put_key)(I64 ch,I64 sc),
	Bool (*fp_puts)(U8 *st),I64 priority,Bool key_descs=FALSE)
{//Places hook in StdOut chain. See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
  CKeyDevEntry *tmpk=keydev.put_key_head.last,
	*tmpk1=ACAlloc(sizeof(CKeyDevEntry));
  tmpk1->put_key=fp_put_key;
  tmpk1->put_s=fp_puts;
  tmpk1->priority=priority;
  if (key_descs)
    tmpk1->flags|=KDF_HAS_DESCS;
  while (tmpk->priority>priority)
    tmpk=tmpk->last;
  QueIns(tmpk1,tmpk);
  if (tmpk->priority==priority)
    KeyDevRem(tmpk);
  return tmpk1;
}

Bool KDRawPutKey(I64 ch,I64)
{
  if (IsRaw) {
    RawPutChar(ch);
    return TRUE;
  } else
    return FALSE;
}

Bool KDRawPutS(U8 *st)
{
  I64 ch;
  if (IsRaw) {
    while (ch=*st++)
      RawPutChar(ch);
    return TRUE;
  } else
    return FALSE;
}

Bool KDInputFilterPutKey(I64 ch,I64 scan_code)
{
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    Msg(MSG_KEY_DOWN,ch,scan_code);
    return TRUE;
  } else
    return FALSE;
}

Bool KDInputFilterPutS(U8 *st)
{
  I64 ch;
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    while (ch=*st++)
      Msg(MSG_KEY_DOWN,ch,0);
    return TRUE;
  } else
    return FALSE;
}

U0 CtrlAltDel(I64)
{
  LBts(sys_ctrl_alt_flags,CTRL_ALT_DEL);
}

U0 CtrlAltC(I64)
{
  LBts(sys_ctrl_alt_flags,CTRL_ALT_C);
}

U0 CtrlAltD(I64)
{
  if (!IsDbgMode) {
    if (Fs==Gs->idle_task)
      BptS(sys_winmgr_task->rip,sys_winmgr_task);
    else
      BptS(*keydev.ctrl_alt_ret_addr);
  }
}

U0 CtrlAltF(I64)
{
  SwapI64(&text.font,&text.aux_font);
}

U0 CtrlAltM(I64)
{
  Mute(!IsMute);
}

U0 CtrlAltN(I64)
{
  LBts(sys_ctrl_alt_flags,CTRL_ALT_TAB);
}

U0 CtrlAltT(I64)
{
  User;
}

U0 CtrlAltV(I64)
{
  VGAFlush;
}

U0 CtrlAltX(I64)
{
  LBts(sys_ctrl_alt_flags,CTRL_ALT_X);
}

U0 CtrlAltCBSet(U8 ch,U0 (*fp_hndlr)(I64 sc),
	U8 *no_shift_desc=NULL,U8 *shift_desc=NULL,Bool in_irq=FALSE)
{//Set callback for <CTRL-ALT-letter>.
  ch=ToUpper(ch)-'A';
  if (ch<26) {
    keydev.fp_ctrl_alt_cbs[ch]=fp_hndlr;

    Free(keydev.ctrl_alt_no_shift_descs[ch]);
    if (no_shift_desc)
      keydev.ctrl_alt_no_shift_descs[ch]=AStrNew(no_shift_desc);
    else
      keydev.ctrl_alt_no_shift_descs[ch]=NULL;

    Free(keydev.ctrl_alt_shift_descs[ch]);
    if (shift_desc)
      keydev.ctrl_alt_shift_descs[ch]=AStrNew(shift_desc);
    else
      keydev.ctrl_alt_shift_descs[ch]=NULL;

    BEqu(&keydev.ctrl_alt_in_irq_flags,ch,in_irq);
  }
}

U0 KeyDevInit()
{
  keydev.fp_ctrl_alt_cbs	=CAlloc(26*sizeof(U8 *));
  keydev.ctrl_alt_no_shift_descs=CAlloc(26*sizeof(U8 *));
  keydev.ctrl_alt_shift_descs	=CAlloc(26*sizeof(U8 *));
  keydev.ctrl_alt_in_irq_flags	=0;
  MemSet(&keydev.put_key_head,0,sizeof(CKeyDevEntry));
  QueInit(&keydev.put_key_head);
  KeyDevAdd(&KDInputFilterPutKey,&KDInputFilterPutS,0x40000000,FALSE);
  KeyDevAdd(&KDRawPutKey,&KDRawPutS,0x60000000,FALSE);
  CtrlAltCBSet('C',&CtrlAltC,"Cmd /Break Execution",,TRUE);
  CtrlAltCBSet('D',&CtrlAltD,"Cmd /Enter Debugger",,TRUE);
  CtrlAltCBSet('F',&CtrlAltF,"Cmd /Toggle Aux Font");
  CtrlAltCBSet('M',&CtrlAltM,"Cmd /Toggle Mute");
  CtrlAltCBSet('N',&CtrlAltN,"Cmd /Next Focus Task",,TRUE);
  CtrlAltCBSet('T',&CtrlAltT,"Cmd /Terminal Window");
  CtrlAltCBSet('V',&CtrlAltV,"Cmd /VGA Flush",,TRUE);
  CtrlAltCBSet('X',&CtrlAltX,"Cmd /Kill Focused Task",,TRUE);
}
