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
{//Output chars. Can pack multiple chars into a single U64.
  while (ch) {
    PutKey(ch&255,0);
    ch>>=8;
  }
}

U0 PutS(U8 *st)
{//Use $LK,"Print",A="MN:Print"$().  See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
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

U0 SetKeyDesc(U8 *fmt,...)
{//Call this from key handler to report desc in $LK,"KeyMap",A="MN:KeyMap"$().
  I64 i;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  StrCpy(keydev.desc,buf);
  keydev.handler=Caller;
  Free(buf);
  for (i=0;i<5;i++)
    Yield;  //Makes KeyMap work better.
}

CKeyDevEntry *KeyDevAdd(Bool (*fp_put_key)(I64 ch,I64 sc),Bool (*fp_puts)(U8 *st),I64 priority,Bool key_descs=FALSE)
{//Places hook in StdOut chain. See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.
  Bool old_preempt=Preempt(OFF);
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
  Bool old_preempt=Preempt(OFF);
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
    old_preempt=Preempt(OFF);
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
    old_preempt=Preempt(OFF);
    while (ch=*st++)
      Msg(MSG_KEY_DOWN,ch,0);
    Preempt(old_preempt);
    return TRUE;
  } else
    return FALSE;
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
}
