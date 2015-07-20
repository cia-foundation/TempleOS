U0 PutKey(I64 ch=0,I64 sc=0)
{//See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
  CKeyDevEntry *tempp;
  if (ch||sc) {
    tempp=keydev.put_key_root.next;
    if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
      if (kbd.scan_code & SCF_SCROLL && sys_focus_task==Fs)
	while (kbd.scan_code & SCF_SCROLL)
	  Yield; //Wait on SCROLL LOCK Key
      while (tempp!=&keydev.put_key_root) {
	if ((!(sc & SCF_KEY_DESC) || tempp->flags & KDF_HAS_DESCS) &&
	      (*tempp->put_key)(ch,sc))
	  break;
	tempp=tempp->next;
      }
    }
  }
}

U0 PutChars(U64 ch)
{//Output chars. Up to 8 chars in a single U64.
//Don't use this.  $LK,"See Print() shortcut.",A="FF:::/Doc/HolyC.TXT.Z,DemoHolyC"$
  while (ch) {
    PutKey(ch&255,0);
    ch>>=8;
  }
}

U0 PutS(U8 *st)
{//Use $LK,"Print",A="MN:Print"$(). See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
//Don't use this.  $LK,"See Print() shortcut.",A="FF:::/Doc/HolyC.TXT.Z,DemoHolyC"$
  I64 ch;
  U8 *ptr;
  Bool cont=TRUE;
  if (!st) return;
  CKeyDevEntry *tempp=keydev.put_key_root.next;
  if (!Bt(&Fs->display_flags,DISPLAYf_SILENT)) {
    if (kbd.scan_code & SCF_SCROLL && sys_focus_task==Fs)
      while (kbd.scan_code & SCF_SCROLL)
	Yield;
    while (cont && tempp!=&keydev.put_key_root) {
      if (tempp->put_s) {
	if ((*tempp->put_s)(st))
	  break;
      } else {
	ptr=st;
	while (ch=*ptr++)
	  if ((*tempp->put_key)(ch,0))
	    cont=FALSE;
      }
      tempp=tempp->next;
    }
  }
}

U0 KeyDescSet(U8 *fmt,...)
{//Call this from key handler to report desc in $LK,"KeyMap",A="MN:KeyMap"$().
  I64 i;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  StrCpy(keydev.desc,buf);
  keydev.handler=Caller;
  Free(buf);
  for (i=0;i<5;i++)
    Yield;  //Makes KeyMap work better.
}

CKeyDevEntry *KeyDevAdd(Bool (*fp_put_key)(I64 ch,I64 sc),
	Bool (*fp_puts)(U8 *st),I64 priority,Bool key_descs=FALSE)
{//Places hook in StdOut chain. See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
  Bool old_preempt=Preempt;
  CKeyDevEntry *tempp,*tempp1=keydev.put_key_root.last;
  tempp=ACAlloc(sizeof(CKeyDevEntry));
  tempp->put_key=fp_put_key;
  tempp->put_s=fp_puts;
  tempp->priority=priority;
  if (key_descs)
    tempp->flags|=KDF_HAS_DESCS;
  while (tempp1->priority>priority)
    tempp1=tempp1->last;
  QueIns(tempp,tempp1);
  Preempt(old_preempt);
  return tempp;
}

U0 KeyDevRem(CKeyDevEntry *tempp)
{//Remove StdOut hook and free.
  Bool old_preempt=Preempt;
  QueRem(tempp);
  Preempt(old_preempt);
  Free(tempp);
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
  Bool old_preempt;
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    old_preempt=Preempt;
    Msg(MSG_KEY_DOWN,ch,scan_code);
    Preempt(old_preempt);
    return TRUE;
  } else
    return FALSE;
}

Bool KDInputFilterPutS(U8 *st)
{
  I64 ch;
  Bool old_preempt;
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK)) {
    old_preempt=Preempt;
    while (ch=*st++)
      Msg(MSG_KEY_DOWN,ch,0);
    Preempt(old_preempt);
    return TRUE;
  } else
    return FALSE;
}

U0 CtrlAltDel(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_DEL);
}

U0 CtrlAltC(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_C);
}

U0 CtrlAltD(I64)
{
  if (!IsDbg) {
    if (Fs==Gs->idle_task)
      BptS(sys_winmgr_task->rip,sys_winmgr_task);
    else
      BptS(*keydev.ctrl_alt_let_ret_addr);
  }
}

U0 CtrlAltE(I64)
{
  User;
}

U0 CtrlAltF(I64)
{
  VGAFlush;
}

U0 CtrlAltM(I64)
{
  Mute(!IsMute);
}

U0 CtrlAltT(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_TAB);
}

U0 CtrlAltX(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_X);
}

U0 CtrlAltLetCBSet(U8 ch,U0 (*fp_handler)(I64 sc),U8 *desc,Bool in_irq=FALSE)
{//Set callback for <CTRL-ALT-letter>.
  ch=ToUpper(ch)-'A';
  if (ch<26) {
    keydev.fp_ctrl_alt_let_cbs[ch]=fp_handler;
    Free(keydev.ctrl_alt_let_descs[ch]);
    keydev.ctrl_alt_let_descs[ch]=AStrNew(desc);
    BEqu(&keydev.ctrl_alt_let_in_irq_flags,ch,in_irq);
  }
}

U0 KeyDevInit()
{
  keydev.fp_ctrl_alt_let_cbs	=CAlloc(26*sizeof(U8 *));
  keydev.ctrl_alt_let_descs	=CAlloc(26*sizeof(U8 *));
  keydev.ctrl_alt_let_in_irq_flags=0;
  MemSet(&keydev.put_key_root,0,sizeof(CKeyDevEntry));
  QueInit(&keydev.put_key_root);
  KeyDevAdd(&KDInputFilterPutKey,&KDInputFilterPutS,0x40000000,FALSE);
  KeyDevAdd(&KDRawPutKey,&KDRawPutS,0x60000000,FALSE);
  CtrlAltLetCBSet('C',&CtrlAltC,"Sys/Break Execution",TRUE);
  CtrlAltLetCBSet('D',&CtrlAltD,"Sys/Enter Debugger",TRUE);
  CtrlAltLetCBSet('E',&CtrlAltE,"Sys/Spawn New User Task");
  CtrlAltLetCBSet('F',&CtrlAltF,"Sys/VGA Flush",TRUE);
  CtrlAltLetCBSet('M',&CtrlAltM,"Sys/Toggle Mute");
  CtrlAltLetCBSet('T',&CtrlAltT,"Sys/Switch Focus to Next Task",TRUE);
  CtrlAltLetCBSet('X',&CtrlAltX,"Sys/Kill Focused Task",TRUE);
}
