U0 InputFilterTask()
{
  CSrvCmd *tempc,*tempc1;
  Bool old_filter;
  I64 old_flags=GetRFlags;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  LBts(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  old_filter=LBts(&Fs->last_input_filter_task->task_flags,TASKf_FILTER_INPUT);
  LBEqu(&Fs->task_flags,TASKf_FILTER_INPUT,old_filter);
  while (TRUE) {
    SrvCmdsHandler;
    CLI
    tempc1=&Fs->srv_ctrl.next_waiting;
    tempc=tempc1->next;
    if (tempc==tempc1)
      break;
    else {
      if (tempc->cmd_code==SVCT_TEXT_INPUT) {
	QueRem(tempc);
	SetRFlags(old_flags);
	Preempt(ON);
	try
	  ExePrint("%s",tempc->aux_str);
	catch
	  Fs->catch_except=TRUE;
	Preempt(OFF);
	SrvCmdDel(tempc);
      } else
	break;
    }
  }
  Fs->next_input_filter_task->last_input_filter_task=Fs->last_input_filter_task;
  Fs->last_input_filter_task->next_input_filter_task=Fs->next_input_filter_task;
  if (!old_filter)
    LBtr(&Fs->last_input_filter_task->task_flags,TASKf_FILTER_INPUT);
  SetRFlags(old_flags);
}

I64 ScanMsg(I64 *_a1=NULL,I64 *_a2=NULL,I64 mask=~1,CTask *task=NULL)
{/*Check for a message of type specified by a one in the mask.
Throw-out messages not in mask.
If no message fit mask, return NULL immediately.
Remove desired message, return $LK,"msg_code",A="MN:MSG_CMD"$.
Note: This delivers messages from parent down to pop-up.
*/
  I64 result,old_flags;
  CSrvCmd *tempc,*tempc1;
  if (!task) task=Fs;
  old_flags=GetRFlags;
  tempc1=&task->srv_ctrl.next_waiting;
  while (TRUE) {
    if (task==Fs)
      SrvCmdsHandler;
    CLI
    tempc=tempc1->next;
    if (tempc==tempc1)
      break;
    else {
      if (tempc->cmd_code==SVCT_MSG) {
	QueRem(tempc);
	SetRFlags(old_flags);
	result=tempc->msg_code;
	if (_a1)
	  *_a1=tempc->aux1;
	if (_a2)
	  *_a2=tempc->aux2;
	SrvCmdDel(tempc);
	if ((result!=MSG_KEY_DOWN || !(tempc->aux2&SCF_KEY_DESC) ||
	  Bt(&mask,MSGF_ALLOW_KEY_DESC)) && Bt(&mask,result))
	    goto sm_done;
      }
    }
    SetRFlags(old_flags);
  }
  result=MSG_NULL;
  if (_a1)
    *_a1=0;
  if (_a2)
    *_a2=0;
  if (task->parent_task&&task->parent_task->popup_task==task) {
    SetRFlags(old_flags);
    return ScanMsg(_a1,_a2,mask,task->parent_task);
  }
sm_done:
  SetRFlags(old_flags);
  return result;
}

I64 FlushMsgs(CTask *task=NULL)
{//Throw away all messages.  Return count.
  I64 result=0,a1,a2;
  while (ScanMsg(&a1,&a2,~1,task))
    result++;
  return result;
}

I64 GetMsg(I64 *_a1=NULL,I64 *_a2=NULL,I64 mask=~1,CTask *task=NULL)
{//Wait for a message of type specified by a one in the mask.
//Throw-out all messages not in mask.
//Returns $LK,"msg_code",A="MN:MSG_CMD"$. See $LK,"::/Demo/MsgLoop.CPP"$.
  I64 result;
  Bool old_preempt=Preempt(OFF);
  if (!task) task=Fs;
  LBtr(&task->task_flags,TASKf_IDLE);
  while (!(result=ScanMsg(_a1,_a2,mask,task))) {
    LBts(&task->task_flags,TASKf_IDLE);
    Yield;
  }
  LBtr(&task->task_flags,TASKf_IDLE);
  Preempt(old_preempt);
  return result;
}

I64 ScanChar()
{//Checks for $LK,"MSG_KEY_DOWN",A="MN:MSG_KEY_DOWN"$ and returns 0 immediately if no key.
//Waits for $LK,"MSG_KEY_UP",A="MN:MSG_KEY_UP"$ of non-zero $LK,"ASCII",A="MN:CH_CTRLA"$ key and returns $LK,"ASCII",A="MN:CH_CTRLA"$ if key.
//$LK,"ScanMsg",A="MN:ScanMsg"$() throws away other message types.
  I64 a1a,a2a,a1b,a2b;
  if (!ScanMsg(&a1a,&a2a,1<<MSG_KEY_DOWN)||!a1a)
    return 0;
  else
    do GetMsg(&a1b,&a2b,1<<MSG_KEY_UP);
    while (!a1b);  //Be careful of $LK,"SC_SHIFT",A="MN:SC_SHIFT"$ and $LK,"SC_CTRL",A="MN:SC_CTRL"$, etc.
  return a1a;
}

Bool ScanKey(I64 *_ch=NULL,I64 *_scan_code=NULL,Bool echo=FALSE)
{//Checks for $LK,"MSG_KEY_DOWN",A="MN:MSG_KEY_DOWN"$ and returns FALSE immediately if no key.
//Sets $LK,"ASCII",A="MN:CH_CTRLA"$ and $LK,"scan_code",A="FI:::/Doc/CharOverview.TXT"$.
//Removes key message and returns TRUE.
//$LK,"ScanMsg",A="MN:ScanMsg"$() throws away other message types.
  I64 ch=0,sc=0;
  if (ScanMsg(&ch,&sc,1<<MSG_KEY_DOWN)) {
    if (_ch) *_ch=ch;
    if (_scan_code) *_scan_code=sc;
    if (echo)
      PutKey(ch,sc);
    return TRUE;
  } else {
    if (_ch) *_ch=0;
    if (_scan_code) *_scan_code=0;
    return FALSE;
  }
}

I64 GetKey(I64 *_scan_code=NULL,Bool echo=FALSE,Bool raw_cursor=FALSE)
{//Waits for $LK,"MSG_KEY_DOWN",A="MN:MSG_KEY_DOWN"$ message and returns $LK,"ASCII",A="MN:CH_CTRLA"$.
//Sets $LK,"scan_code",A="FI:::/Doc/CharOverview.TXT"$.
//$LK,"ScanKey",A="MN:ScanKey"$() throws away other message types.
  I64 ch,sc;
  Bool old_preempt=Preempt(OFF),cursor_on=FALSE;
  while (!ScanKey(&ch,&sc,FALSE)) {
    if (IsRaw && raw_cursor) {
      if (!cursor_on && ToI64(GetTSC*5/cnts.time_stamp_freq)&1) {
	'Û';
	cursor_on=TRUE;
      } else if (cursor_on && !(ToI64(GetTSC*5/cnts.time_stamp_freq)&1)) {
	'' CH_BACKSPACE;
	cursor_on=FALSE;
      }
    }
    LBts(&Fs->task_flags,TASKf_IDLE);
    if (IsDbg) {
      //We don't want interrupt-driven keyboard when in debugger
      //because that could have side-effects or crash, so we poll
      //keyboard when in debugger with interrupts off.
      PUSHFD
      CLI
      KbdMouseHandler(TRUE,FALSE);
      KbdMsgsQue;
      POPFD
      if (old_preempt && GetRFlags&1<<RFLAGf_INT)
	Yield;
    } else {
      LBts(&Fs->task_flags,TASKf_AWAITING_MSG);
      Yield;
    }
    LBtr(&Fs->task_flags,TASKf_IDLE);
  }
  if (IsRaw && raw_cursor && cursor_on)
    '' CH_BACKSPACE;
  Preempt(old_preempt);
  if (echo)
    PutKey(ch,sc);
  if (_scan_code) *_scan_code=sc;
  return ch;
}

I64 GetChar(I64 *_scan_code=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE)
{//Waits for non-zero $LK,"ASCII",A="MN:CH_CTRLA"$ key.
//Sets $LK,"scan_code",A="FI:::/Doc/CharOverview.TXT"$.
  I64 ch1;
  do ch1=GetKey(_scan_code,FALSE,raw_cursor);
  while (!ch1);
  if (echo)
    "$$PT$$%c$$FG$$",ch1;
  return ch1;
}

U8 *GetStr(U8 *msg=NULL,U8 *dft=NULL,I64 flags=0)
{//Returns a $LK,"MAlloc",A="MN:MAlloc"$()ed prompted string.  See $LK,"Flags",A="MN:GSF_SHIFT_ESC_EXIT"$.
  U8 *st;
  if (msg)
    "" msg,dft;
  st=(*fp_getstr2)(flags);
  if (!*st) {
    Free(st);
    if (dft)
      return StrNew(dft);
    else
      return StrNew("");
  }
  return st;
}

I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE)
{//Prompt into fixed length string. Size must include terminator.
  U8 *st;
  I64 ch,i=0;
  if (!size || !buf) return 0;
  if (allow_ext) {
    st=GetStr;
    if (StrLen(st)>size-1) {
      MemCpy(buf,st,size-1);
      buf[size-1]=0;
    } else
      StrCpy(buf,st);
    i=StrLen(buf);
    Free(st);
  } else {
    while (TRUE) {
      ch=GetChar(,FALSE,IsDbg);
      if (ch=='\n') {
	'' ch;
	break;
      } else if (ch==CH_BACKSPACE) {
	if (i>0) {
	  i--;
	  '' ch;
	}
      } else {
	if (i<size-1) {
	  buf[i++]=ch;
	  '' ch;
	}
      }
    }
    buf[i]=0;
  }
  return i;
}
