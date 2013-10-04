U0 InputFilterTask()
{
  CSrvCmd *tempc,*tempc1;
  Bool old_filter;
  I64 old_flags=GetRFlags;
  LBts(&Fs->task_flags,TASKf_USER);
  TaskInitExt;
  LBts(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  old_filter=LBts(&Fs->last_input_filter_task->task_flags,TASKf_FILTER_INPUT);
  LBEqu(&Fs->task_flags,TASKf_FILTER_INPUT,old_filter);
  while (TRUE) {
    SrvCmdsHandler;
    Cli;
    tempc1=&Fs->srv_ctrl.next_waiting;
    tempc=tempc1->next;
    if (tempc==tempc1)
      break;
    else {
      if (tempc->cmd_code==SVCT_TEXT_INPUT) {
	QueRem(tempc);
	SetRFlags(old_flags);
	if (!IsSysDeterministic)
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

I64 ScanMsg(I64 *_p1=NULL,I64 *_p2=NULL,I64 mask=~1,CTask *task=NULL)
{
  I64 result,old_flags;
  CSrvCmd *tempc,*tempc1;
  if (!task) task=Fs;
  old_flags=GetRFlags;
  tempc1=&task->srv_ctrl.next_waiting;
  while (TRUE) {
    if (task==Fs)
      SrvCmdsHandler;
    Cli;
    tempc=tempc1->next;
    if (tempc==tempc1)
      break;
    else {
      if (tempc->cmd_code==SVCT_MSG) {
	QueRem(tempc);
	SetRFlags(old_flags);
	result=tempc->msg_code;
	if (_p1)
	  *_p1=tempc->aux1;
	if (_p2)
	  *_p2=tempc->aux2;
	SrvCmdDel(tempc);
	if ((result!=MSG_KEY_DOWN || !(tempc->aux2&SCF_KEY_DESC) ||
	  Bt(&mask,MSGF_ALLOW_KEY_DESC)) && Bt(&mask,result))
	    goto sm_done;
      }
    }
    SetRFlags(old_flags);
  }
  result=MSG_NULL;
  if (_p1)
    *_p1=0;
  if (_p2)
    *_p2=0;
  if (task->parent_task&&task->parent_task->popup_task==task) {
    SetRFlags(old_flags);
    return ScanMsg(_p1,_p2,mask,task->parent_task);
  }
sm_done:
  SetRFlags(old_flags);
  return result;
}

I64 FlushMsgs(CTask *task=NULL)
{
  I64 result=0,p1,p2;
  while (ScanMsg(&p1,&p2,~1,task))
    result++;
  return result;
}

I64 GetMsg(I64 *_p1=NULL,I64 *_p2=NULL,I64 mask=~1,CTask *task=NULL)
{
  I64 result;
  Bool old_preempt=Preempt(OFF);
  if (!task) task=Fs;
  LBtr(&task->task_flags,TASKf_IDLE);
  while (!(result=ScanMsg(_p1,_p2,mask,task))) {
    LBts(&task->task_flags,TASKf_IDLE);
    Yield;
  }
  LBtr(&task->task_flags,TASKf_IDLE);
  Preempt(old_preempt);
  return result;
}

I64 ScanChar()
{
  I64 p1a,p2a,p1b,p2b;
  if (!ScanMsg(&p1a,&p2a,1<<MSG_KEY_DOWN)||!p1a)
    return 0;
  else
    do GetMsg(&p1b,&p2b,1<<MSG_KEY_UP);
    while (!p1b);
  return p1a;
}

Bool ScanKey(I64 *_ch=NULL,I64 *_scancode=NULL,Bool echo=FALSE)
{
  I64 ch=0,sc=0;
  if (ScanMsg(&ch,&sc,1<<MSG_KEY_DOWN)) {
    if (_ch) *_ch=ch;
    if (_scancode) *_scancode=sc;
    if (echo)
      PutKey(ch,sc);
    return TRUE;
  } else {
    if (_ch) *_ch=0;
    if (_scancode) *_scancode=0;
    return FALSE;
  }
}

I64 GetKey(I64 *_scancode=NULL,Bool echo=FALSE,Bool raw_cursor=FALSE)
{
  I64 ch,sc;
  Bool old_preempt=Preempt(OFF),cursor_on=FALSE;
  while (!ScanKey(&ch,&sc,FALSE)) {
    if (IsRaw && raw_cursor) {
      if (!cursor_on && ToI64(GetTSC*5/time_stamp_freq)&1) {
	'Û';
	cursor_on=TRUE;
      } else if (cursor_on && !(ToI64(GetTSC*5/time_stamp_freq)&1)) {
	'' CH_BACKSPACE;
	cursor_on=FALSE;
      }
    }
    LBts(&Fs->task_flags,TASKf_IDLE);
    if (IsInDbg) {
      //We don't want interrupt-driven keyboard when in debugger
      //because that could have side-effects or crash, so we poll
      //keyboard when in debugger with interrupts off.
      PushFD;
      Cli;
      KbdMouseHandler(TRUE,FALSE);
      KbdMsgsQue;
      PopFD;
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
  if (_scancode) *_scancode=sc;
  return ch;
}

I64 GetChar(I64 *_scancode=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE)
{
  I64 ch1;
  do ch1=GetKey(_scancode,FALSE,raw_cursor);
  while (!ch1);
  if (echo)
    "$$PT$$%c$$FG$$",ch1;
  return ch1;
}

U8 *MGetS(I64 flags=0)
//$LK,"Flags","MN:GETSF_SHIFT_ESC_EXIT"$
{
//Returns a $LK,"MAlloc","MN:MAlloc"$()ed prompted string.
  return (*fp_mgets)(flags);
}

I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE)
{
  U8 *st;
  I64 ch,i=0;
  if (!size || !buf) return 0;
  if (allow_ext) {
    st=MGetS;
    if (StrLen(st)>size-1) {
      MemCpy(buf,st,size-1);
      buf[size-1]=0;
    } else
      StrCpy(buf,st);
    i=StrLen(buf);
    Free(st);
  } else {
    while (TRUE) {
      ch=GetChar(,FALSE,IsInDbg);
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
