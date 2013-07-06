U0 SrvCmdDel(CSrvCmd *tempc)
{
  Free(tempc->aux_str);
  Free(tempc);
}

U0 SrvCmdQueDel(CSrvCmd *root)
{
  CSrvCmd *tempc=root->next,*tempc1;
  while (tempc!=root) {
    tempc1=tempc->next;
    QueRem(tempc);
    SrvCmdDel(tempc);
    tempc=tempc1;
  }
}

U0 SrvCtrlInit(CSrvCtrl *ctrl)
{
  QueInit(&ctrl->next_waiting);
  QueInit(&ctrl->next_done);
  ctrl->flags=0;
}

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

CSrvCmd *TaskExeStrQue(CTask *srv,CTask *master,
		     U8 *data,I64 flags)
{
  CSrvCmd *result;
  if (!data || !TaskValidate(srv) ||
      master && !TaskValidate(master) ||
      srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  result=ACAlloc(sizeof(CSrvCmd));
  result->master_task=master;
  result->cmd_code=SVCT_EXE_STR;
  result->flags=flags;
  result->aux_str=AStrNew(data);
  result->ctrl=&srv->srv_ctrl;

  PushFD;
  Cli;
  while (LBts(&srv->srv_ctrl.flags,SVCRf_LOCKED))
    Pause;
  if (!TaskValidate(srv)) {
    PopFD;
    SrvCmdDel(result);
    return NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(result,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,SVCRf_LOCKED);
    if (Bt(&flags,SVCf_WAKE_MASTER)) {
      Suspend(master);
      Yield;
    }
  }
  PopFD;
  return result;
}

CSrvCmd *TaskTextInputQue(CTask *srv,CTask *master,
		     U8 *data,I64 flags)
{
  CSrvCmd *result;
  Bool old_preempt=Preempt(OFF);
  CTask *task;
  if (!data || !TaskValidate(srv) ||
      master && !TaskValidate(master) ||
      srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT)) {
    Preempt(old_preempt);
    return NULL;
  }
  result=ACAlloc(sizeof(CSrvCmd));
  result->master_task=master; //in case somebody cares
  result->cmd_code=SVCT_TEXT_INPUT;
  result->flags=flags;
  result->aux_str=AStrNew(data);

  PushFD;
  if (Bt(&flags,SVCf_HIGHEST_PRIORITY) ||
      srv->next_input_filter_task==srv) {
    task=Spawn(&InputFilterTask,NULL,"Input Filter",,srv);
    Cli;
    task->next_input_filter_task=srv->next_input_filter_task;
    task->last_input_filter_task=srv;
    srv->next_input_filter_task=task;
    task->next_input_filter_task->last_input_filter_task=task;
  } else
    task=srv->next_input_filter_task;
  result->ctrl=&task->srv_ctrl;
  Cli;
  while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
    Pause;
  if (!TaskValidate(task)) {
    SrvCmdDel(result);
    result=NULL;
  } else {
    LBtr(&task->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(task);
    QueIns(result,task->srv_ctrl.last_waiting);
    LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
  }
  PopFD;
  Preempt(old_preempt);
  return result;
}

CSrvCmd *TaskMsgQue(CTask *_srv,CTask *master,
		     I64 msg_code,I64 p1,I64 p2,I64 flags)
{
  CSrvCmd *tempc1,*tempc;
  CTask *srv=_srv;
  if (!TaskValidate(srv) ||
      master && !TaskValidate(master)||
      srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  tempc=ACAlloc(sizeof(CSrvCmd));
  tempc->master_task=master;
  tempc->cmd_code=SVCT_MSG;
  tempc->msg_code=AbsI64(msg_code); //negative means do a down and up
  tempc->aux1=p1;
  tempc->aux2=p2;
  tempc->flags=flags;
  PushFD;
  if (Bt(&sys_semas[SYS_SEMA_RECORD_MACRO],0) &&
      srv!=sys_macro_util_task &&
      msg_code==MSG_KEY_DOWN) {
    tempc1=AMAllocIdentical(tempc);
    Cli;
    QueIns(tempc1,sys_macro_root.last);
  }
  Cli;
  while (Bt(&srv->task_flags,TASKf_FILTER_INPUT) &&
      !Bt(&flags,SVCf_DONT_FILTER))
    srv=srv->next_input_filter_task;
  tempc->ctrl=&srv->srv_ctrl;
  while (LBts(&srv->srv_ctrl.flags,SVCRf_LOCKED))
    Pause;
  if (!TaskValidate(srv)) {
    SrvCmdDel(tempc);
    tempc=NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(tempc,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,SVCRf_LOCKED);
  }
  PopFD;
  if (msg_code<0) //Down-Up
    TaskMsgQue(_srv,master,-msg_code+1,p1,p2,flags);
  return tempc;
}

Bool ScanResult(CSrvCmd *rqst=NULL,I64 *_result=NULL)
{
  CSrvCtrl *ctrl;
  CSrvCmd *tempc,*tempc1;
  if (!rqst || Bt(&rqst->flags,SVCf_DONE)) {
    if (!rqst || rqst->master_task)
      ctrl=&Fs->srv_ctrl;
    else
      ctrl=rqst->ctrl;
    PushFD;
    Cli;
    while (LBts(&ctrl->flags,SVCRf_LOCKED))
      Pause;
    tempc1=&ctrl->next_done;
    tempc=tempc1->next;
    while (tempc!=tempc1) {
      if (!rqst || rqst==tempc) {
	QueRem(tempc);
	LBtr(&ctrl->flags,SVCRf_LOCKED);
	PopFD;
	if (_result)
	  *_result=tempc->result;
	SrvCmdDel(tempc);
	return TRUE;
      }
      tempc=tempc->next;
    }
    LBtr(&ctrl->flags,SVCRf_LOCKED);
    PopFD;
  }
  if (_result)
    *_result=0;
  return FALSE;
}

I64 GetResult(CSrvCmd *rqst=NULL)
{
  I64 result;
  CSrvCmd *tempc1;
  if (!rqst) {
    tempc1=&Fs->srv_ctrl.next_done;
    while (tempc1==tempc1->next) {
      LBts(&Fs->task_flags,TASKf_IDLE);
      Yield;
    }
  } else {
    while (!Bt(&rqst->flags,SVCf_DONE)) {
      LBts(&Fs->task_flags,TASKf_IDLE);
      Yield;
    }
  }
  LBtr(&Fs->task_flags,TASKf_IDLE);
//Could get taken by someone else.
  ScanResult(rqst,&result);
  return result;
}

U0 TaskWait(CTask *task=NULL)
{
  CTask *task1;
  CSrvCmd *tempc1;
  if (!task) task=Fs;
  if (TaskValidate(task)) {
    PushFD;
    Cli;
    while (TRUE) {
      task1=task->last_input_filter_task;
      tempc1=&task1->srv_ctrl.next_waiting;
      if (TaskValidate(task1) && tempc1==tempc1->next &&
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
      TaskMsgQue(task->last_input_filter_task,NULL,msg_code,p1,p2,
	  flags|1<<SVCf_DONT_FILTER);
    else
      TaskMsgQue(task,NULL,msg_code,p1,p2,flags);
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

#define SCR_DONE	0
#define SCR_CONT	1
#define SCR_EXIT	2

I64 SrvCmdRun(CSrvCmd *tempc,I64 old_flags)
{
  CSrvCtrl *ctrl=tempc->ctrl;
  CTask    *master;
  Bool old_preempt=IsPreempt;
  I64 result,flags=tempc->flags;
  if (Bt(&flags,SVCf_EXIT_ON_COMPLETE))
    result=SCR_EXIT;
  else
    result=SCR_CONT;
  switch (tempc->cmd_code) {
    case SVCT_SPAWN_TASK:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      if (tempc->aux_str)
	tempc->spawned_task=Spawn(tempc->add,tempc->fun_arg,tempc->aux_str,,tempc->aux1,tempc->aux2,tempc->flags);
      else
	tempc->spawned_task=Spawn(tempc->add,tempc->fun_arg,"Unnamed",,tempc->aux1,tempc->aux2,tempc->flags);
      break;
    case SVCT_CALL:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetFlags(old_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	tempc->result=(*tempc->add)(tempc->fun_arg);
      catch
	Fs->catch_except=TRUE;
      SetFlags(old_flags);
      break;
    case SVCT_EXE_STR:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetFlags(old_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	tempc->result=ExePutS(tempc->aux_str);
      catch
	Fs->catch_except=TRUE;
      SetFlags(old_flags);
      break;
    default:
      result=SCR_DONE;
  }
  if (result) {
    if (master=tempc->master_task) {
      if (!Bt(&flags,SVCf_FREE_ON_COMPLETE)) {
	Cli;
	while (LBts(&master->srv_ctrl.flags,SVCRf_LOCKED))
	  Pause;
	QueIns(tempc,master->srv_ctrl.last_done);
	LBts(&tempc->flags,SVCf_DONE);
	LBtr(&master->srv_ctrl.flags,SVCRf_LOCKED);
	SetFlags(old_flags);
      }
      if (Bt(&flags,SVCf_FOCUS_MASTER) &&
	    Bt(&master->task_flags,TASKf_USER))
	sys_focus_task=master;
      if (Bt(&flags,SVCf_WAKE_MASTER))
	Suspend(master,FALSE);
    }
    if (Bt(&flags,SVCf_FREE_ON_COMPLETE))
      SrvCmdDel(tempc);
    else if (!master) {
      Cli;
      while (LBts(&ctrl->flags,SVCRf_LOCKED))
	Yield;
      QueIns(tempc,ctrl->last_done);
      LBts(&tempc->flags,SVCf_DONE);
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetFlags(old_flags);
    }
  }
  Preempt(old_preempt);
  return result;
}

I64 SrvCmdsHandler(CTask *task=NULL)
{
  I64 cnt=0,old_flags=GetFlags;
  CSrvCmd *tempc,*tempc1;
  if (!task) task=Fs;
  while (TRUE) {
    Cli;
    while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
      Pause;
    tempc1=&task->srv_ctrl.next_waiting;
    tempc=tempc1->next;
    if (tempc==tempc1)
      goto sch_done;
    else
      switch (SrvCmdRun(tempc,old_flags)) {
	case SCR_CONT:
	  cnt++;
	  break;
	case SCR_EXIT:
	  Exit;
	case SCR_DONE:
	  goto sch_done;
      }
  }
sch_done:
  LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
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
  CSrvCmd *tempc;
  if (Fs==adam_task) {
    tempc=TaskExeStrQue(adam_task,Fs,st,0);
    SrvCmdsHandler;
  } else {
    TaskWait(adam_task);
    tempc=TaskExeStrQue(adam_task,Fs,st,1<<SVCf_WAKE_MASTER);
  }
  Preempt(old_preempt);
  ScanResult(tempc,&result);
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
  TaskTextInputQue(task,0,st2,0);
  Free(st2);
  Free(st);
}

U0 XTalkWait(CTask *task,U8 *fmt,...)
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv),
     *st2=MSPrintF("\"%Q\";",st);
  TaskTextInputQue(task,0,st2,0);
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
    TaskTextInputQue(Fs,0,st2,1<<SVCf_HIGHEST_PRIORITY);
  Free(st2);
  Free(st);
}

I64 SrvUser(U8 *fmt,...)
//Spawns a visible task and sends text
//which gets compiled and run.
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  I64 result=0;
  CSrvCmd *tempc;
  CTask *task=Spawn(&SrvUserCmdLine,NULL,"Servant",,Fs);
  Fs->popup_task=task;
  tempc=TaskExeStrQue(task,Fs,st,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
  ScanResult(tempc,&result);
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
    TaskTextInputQue(Fs,0,st,1<<SVCf_HIGHEST_PRIORITY);
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
  TaskTextInputQue(task,0,st,0);
  Free(st);
}

U0 XTalkStrWait(CTask *task,U8 *fmt,...)
{
  U8 *st=SPrintFJoin(NULL,fmt,argc,argv);
  TaskTextInputQue(task,0,st,0);
  Free(st);
  TaskWait(task);
}
