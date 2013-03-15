U0 TaskRstAwaitingMsg(CTask *task=NULL)
{ //Pop-ups get parent messages so wake-up our pop-ups if we got a msg.
  Bool old_preempt=Preempt(OFF);
  if (!task) task=Fs;
  if (TaskValidate(task)) {
    LBtr(&task->task_flags,TASKf_AWAITING_MSG);
    while (task=task->popup_task)
      if (TaskValidate(task))
	LBtr(&task->task_flags,TASKf_AWAITING_MSG);
      else
	break;
  }
  Preempt(old_preempt);
}

CTaskCmd *TaskRqstQue(CTask *servant,CTask *master,
		     U8 *data,I64 flags)
{
  CTaskCmd *result;
  if (!data || !TaskValidate(servant) ||
      master && !TaskValidate(master) ||
      servant->popup_task && !Bt(&servant->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  result=AMAlloc(sizeof(CTaskCmd));
  result->master_task=master;
  result->cmd_code=TASKCT_EXECUTE_STR;
  result->flags=flags;
  result->data=AStrNew(data);
  result->result=NULL;

  PushFD;
  Cli;
  while (LBts(&servant->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  if (!TaskValidate(servant)) {
    PopFD;
    Free(result->data);
    Free(result);
    return NULL;
  } else {
    LBtr(&servant->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(servant);
    QueIns(result,servant->last_servant_cmd);
    LBtr(&servant->locked_flags,TASKLf_TASK_LOCK);
    if (Bt(&flags,TASKCf_WAKE_MASTER)) {
      Suspend(master);
      Yield;
    }
  }
  PopFD;
  return result;
}

CTaskCmd *TextInputQue(CTask *servant,CTask *master,
		     U8 *data,I64 flags)
{
  CTaskCmd *result;
  Bool old_preempt=Preempt(OFF);
  CTask *task;
  if (!data || !TaskValidate(servant) ||
      master && !TaskValidate(master) ||
      servant->popup_task && !Bt(&servant->task_flags,TASKf_FILTER_INPUT)) {
    Preempt(old_preempt);
    return NULL;
  }
  result=AMAlloc(sizeof(CTaskCmd));
  result->master_task=master; //in case somebody cares
  result->cmd_code=TASKCT_TEXT_INPUT;
  result->flags=flags;
  result->data=AStrNew(data);
  result->result=NULL;

  PushFD;
  if (flags&1<<TASKCf_HIGHEST_PRIORITY ||
      servant->next_input_filter_task==servant) {
    task=Spawn(&InputFilterTask,NULL,
	"Input Filter",servant);
    Cli;
    task->next_input_filter_task=servant->next_input_filter_task;
    task->last_input_filter_task=servant;
    servant->next_input_filter_task=task;
    task->next_input_filter_task->last_input_filter_task=task;
  } else
    task=servant->next_input_filter_task;
  Cli;
  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  if (!TaskValidate(task)) {
    Free(result->data);
    Free(result);
    result=NULL;
  } else {
    LBtr(&task->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(task);
    QueIns(result,task->last_servant_cmd);
    LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
  }
  PopFD;
  Preempt(old_preempt);
  return result;
}

CTaskCmd *QueMsg(CTask *_servant,CTask *master,
		     I64 msg_code,I64 p1,I64 p2,I64 flags)
{
  CTaskCmd *tempc1,*tempc;
  CTask *servant=_servant;
  if (!TaskValidate(servant) ||
      master && !TaskValidate(master)||
      servant->popup_task && !Bt(&servant->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  tempc=AMAlloc(sizeof(CTaskCmd));
  tempc->master_task=master;
  tempc->cmd_code=TASKCT_MSG;
  tempc->msg_code=AbsI64(msg_code); //negative means do a down and up
  tempc->p1=p1;
  tempc->p2=p2;
  tempc->flags=flags;
  tempc->data=NULL;
  tempc->result=NULL;
  PushFD;
  if (Bt(&sys_semas[SYS_SEMA_RECORD_MACRO],0) &&
      servant!=sys_macro_util_task &&
      msg_code==MSG_KEY_DOWN) {
    tempc1=AMAllocIdentical(tempc);
    Cli;
    QueIns(tempc1,sys_macro_root.last);
  }
  Cli;
  while (Bt(&servant->task_flags,TASKf_FILTER_INPUT) &&
      !Bt(&flags,TASKCf_DONT_FILTER))
    servant=servant->next_input_filter_task;
  while (LBts(&servant->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  if (!TaskValidate(servant)) {
    Free(tempc);
    tempc=NULL;
  } else {
    LBtr(&servant->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(servant);
    QueIns(tempc,servant->last_servant_cmd);
    LBtr(&servant->locked_flags,TASKLf_TASK_LOCK);
  }
  PopFD;
  if (msg_code<0) //Down-Up
    QueMsg(_servant,master,-msg_code+1,p1,p2,flags);
  return tempc;
}

Bool GetRqstResult(CTaskCmd *rqst=NULL,I64 *_result=NULL)
{
  CTaskCmd *tempc,*tempc1;
  PushFD;
  Cli;
  while (LBts(&Fs->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  tempc1=&Fs->next_master_cmd;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    if (!rqst || rqst==tempc) {
      QueRem(tempc);
      LBtr(&Fs->locked_flags,TASKLf_TASK_LOCK);
      PopFD;
      if (_result)
	*_result=tempc->result;
      Free(tempc->data);
      Free(tempc);
      return TRUE;
    }
    tempc=tempc->next;
  }
  LBtr(&Fs->locked_flags,TASKLf_TASK_LOCK);
  PopFD;
  return FALSE;
}

U0 TaskWait(CTask *task=NULL)
{
  CTask *task1;
  CTaskCmd *tempc;
  if (!task) task=Fs;
  if (TaskValidate(task)) {
    PushFD;
    Cli;
    while (TRUE) {
      task1=task->last_input_filter_task;
      tempc=&task1->next_servant_cmd;
      if (TaskValidate(task1) && tempc==*tempc &&
	  (task1==Fs || Bt(&task1->task_flags,TASKf_IDLE)))
	break;
      Yield;
    }
    PopFD;
  }
}

U0 PostMsg(CTask *task,I64 msg_code,I64 p1,I64 p2,I64 flags=0)
{
  if (TaskValidate(task)) {
    if (Bt(&task->task_flags,TASKf_INPUT_FILTER_TASK))
      QueMsg(task->last_input_filter_task,NULL,msg_code,p1,p2,
	  flags| 1<<TASKCf_DONT_FILTER);
    else
      QueMsg(task,NULL,msg_code,p1,p2,flags);
  }
}

U0 SendMsg(CTask *task,I64 msg_code,I64 p1,I64 p2,I64 flags=0)
{
  PostMsg(task,msg_code,p1,p2,flags);
  TaskWait(task);
}

U0 Msg(I64 msg_code,I64 p1,I64 p2,I64 flags=0)
{
  PostMsg(Fs,msg_code,p1,p2,flags);
}

I64 ServantCmdsHandler(CTask *task=NULL)
{
  I64 cnt=0,old_flags=GetFlags;
  CTaskCmd *tempc,*tempc1;
  CTask *master;
  if (!task) task=Fs;
start_over:
  Cli;
  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  tempc1=&task->next_servant_cmd;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    tempc=tempc1->next;
    if (tempc->cmd_code==TASKCT_EXECUTE_STR ||
    tempc->cmd_code==TASKCT_EXECUTE_FILE) {
      cnt++;
      QueRem(tempc);
      LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
      SetFlags(old_flags);
      try {
	if (tempc->cmd_code==TASKCT_EXECUTE_STR)
	  tempc->result=ExePutS(tempc->data,sys_temp_filename);
	else
	  tempc->result=ExeFile(tempc->data);
      } catch
	Fs->catch_except=TRUE;
      if (master=tempc->master_task) {
	if (!Bt(&tempc->flags,TASKCf_EXIT_ON_COMPLETE)) {
	  Cli;
	  while (LBts(&master->locked_flags,TASKLf_TASK_LOCK))
	    Pause;
	  QueIns(tempc,master->last_master_cmd);
	  LBtr(&master->locked_flags,TASKLf_TASK_LOCK);
	  SetFlags(old_flags);
	}
	if (Bt(&tempc->flags,TASKCf_FOCUS_MASTER) &&
	Bt(&master->task_flags,TASKf_USER))
	  sys_focus_task=master;
	if (Bt(&tempc->flags,TASKCf_WAKE_MASTER))
	  Suspend(master,FALSE);
	if (!Bt(&tempc->flags,TASKCf_EXIT_ON_COMPLETE))
	  goto start_over;
	else {
	  Free(tempc->data);
	  Free(tempc);
	  Exit;
	}
      } else {
	if (!Bt(&tempc->flags,TASKCf_EXIT_ON_COMPLETE)) {
	  Free(tempc->data);
	  Free(tempc);
	  goto start_over;
	} else {
	  Free(tempc->data);
	  Free(tempc);
	  Exit;
	}
      }
    } else
      break;
  }
  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
  SetFlags(old_flags);
  return cnt;
}

I64 Adam(U8 *fmt,...)
//Sends text to the adam task which gets
//compiled and run.
{
  I64 result;
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  Bool old_preempt=Preempt(OFF);
  CTaskCmd *tempc;
  if (Fs==adam_task) {
    tempc=TaskRqstQue(adam_task,Fs,st,0);
    ServantCmdsHandler;
  } else {
    TaskWait(adam_task);
    tempc=TaskRqstQue(adam_task,Fs,st,1<<TASKCf_WAKE_MASTER);
  }
  Preempt(old_preempt);
  GetRqstResult(tempc,&result);
  Free(st);
  return result;
}

U0 AdamLog(U8 *fmt,...)
//Sends text to the adam task which gets
//displayed.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  if (Fs==adam_task)
    "" st;
  else if (!IsSingleUser)
    Adam("\"%Q\";",st);
  Free(st);
}

U0 AdamErr(U8 *fmt,...)
//Sends text to the adam task which gets
//displayed as red and blinking.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv),
     *st2=MSPrintF("%S%s","ST_ERR",st);
  if (Fs==adam_task)
    "" st2;
  else if (!IsSingleUser)
    Adam("\"%Q\";",st2);
  Free(st2);
  Free(st);
}

U0 XTalk(CTask *task,U8 *fmt,...)
//Sends text to another task's input
//buffer.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv),
     *st2=MSPrintF("\"%Q\";",st);
  TextInputQue(task,0,st2,0);
  Free(st2);
  Free(st);
}

U0 XTalkWait(CTask *task,U8 *fmt,...)
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv),
     *st2=MSPrintF("\"%Q\";",st);
  TextInputQue(task,0,st2,0);
  Free(st2);
  Free(st);
  TaskWait(task);
}

U0 Auto(U8 *fmt,...)
//Sends text to one's own input buffer
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv),
     *st2=MSPrintF("\"%Q\";",st);
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK))
    ExePutS(st2);
  else
    TextInputQue(Fs,0,st2,1<<TASKCf_HIGHEST_PRIORITY);
  Free(st2);
  Free(st);
}

I64 ServantUser(U8 *fmt,...)
//Spawns a visible task and sends text
//which gets compiled and run.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  I64 result=0;
  CTaskCmd *tempc;
  CTask *task=Spawn(&ServantUserCmdLine,NULL,"Servant",Fs);
  Fs->popup_task=task;
  tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
  GetRqstResult(tempc,&result);
  Fs->popup_task=NULL;
  Kill(task,FALSE);
  Free(st);
  return result;
}

U0 AutoStr(U8 *fmt,...)
//Sends text in the nature of an AutoFile to one's
//own input buffer.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK))
    ExePutS(st);
  else
    TextInputQue(Fs,0,st,1<<TASKCf_HIGHEST_PRIORITY);
  Free(st);
}

U0 AutoFile(U8 *filename)
{
  U8 *name2=DftExt(filename,"AUT.Z");
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK))
    ExeFile(name2);
  else
    AutoStr("%F",name2);
  Free(name2);
}


//This sends text of an AutoFile nature to a
//task's input buffer.
U0 XTalkStr(CTask *task,U8 *fmt,...)
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  TextInputQue(task,0,st,0);
  Free(st);
}

U0 XTalkStrWait(CTask *task,U8 *fmt,...)
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  TextInputQue(task,0,st,0);
  Free(st);
  TaskWait(task);
}
