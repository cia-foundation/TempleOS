U0 JobDel(CJob *tmpc)
{//Free one cmd node.
  Free(tmpc->aux_str);
  Free(tmpc);
}

U0 JobQueDel(CJob *head)
{
  CJob *tmpc=head->next,*tmpc1;
  while (tmpc!=head) {
    tmpc1=tmpc->next;
    QueRem(tmpc);
    JobDel(tmpc);
    tmpc=tmpc1;
  }
}

U0 JobCtrlInit(CJobCtrl *ctrl)
{
  QueInit(&ctrl->next_waiting);
  QueInit(&ctrl->next_done);
  ctrl->flags=0;
}

U0 TaskRstAwaitingMsg(CTask *task=NULL)
{//Pop-ups get parent messages so wake-up our pop-ups if we got a msg.
  if (!task) task=Fs;
  PUSHFD
  CLI
  do {
    if (TaskValidate(task))
      LBtr(&task->task_flags,TASKf_AWAITING_MSG);
    else
      break;
  } while (task=task->popup_task);
  POPFD
}

CJob *TaskExe(CTask *srv,CTask *master,U8 *data,I64 flags)
{//Queues a request to compile and execute src code text.
  CJob *res;
  if (!data || !TaskValidate(srv) || master && !TaskValidate(master) ||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  res=ACAlloc(sizeof(CJob));
  res->master_task=master;
  res->job_code=JOBT_EXE_STR;
  res->flags=flags;
  res->aux_str=AStrNew(data);
  res->ctrl=&srv->srv_ctrl;

  PUSHFD
  CLI
  while (LBts(&srv->srv_ctrl.flags,JOBCf_LOCKED))
    PAUSE
  if (!TaskValidate(srv)) {
    LBtr(&srv->srv_ctrl.flags,JOBCf_LOCKED);
    POPFD
    JobDel(res);
    return NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(res,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,JOBCf_LOCKED);
    if (Bt(&flags,JOBf_WAKE_MASTER)) {
      Suspend(master);
      Yield;
    }
  }
  POPFD
  return res;
}

CJob *TaskText(CTask *srv,CTask *master,U8 *data,I64 flags)
{//Post StdIn text to servant task. Tell who the master task is.
  CJob *res;
  CTask *task;
  if (!data || !TaskValidate(srv) || master && !TaskValidate(master) ||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  res=ACAlloc(sizeof(CJob));
  res->master_task=master; //in case somebody cares
  res->job_code=JOBT_TEXT_INPUT;
  res->flags=flags;
  res->aux_str=AStrNew(data);

  PUSHFD
  task=srv->last_input_filter_task;
  if (Bt(&flags,JOBf_HIGHEST_PRIORITY) || task==srv) {
    if (task!=srv)
      TaskWait(srv);
    task=Spawn(&InputFilterTask,NULL,"Input Filter",,srv);
    CLI
    task->next_input_filter_task=srv->next_input_filter_task;
    task->last_input_filter_task=srv;
    srv->next_input_filter_task=task;
    task->next_input_filter_task->last_input_filter_task=task;
  } else {
    CLI
    task=srv->next_input_filter_task;
  }
  res->ctrl=&task->srv_ctrl;
  while (LBts(&task->srv_ctrl.flags,JOBCf_LOCKED))
    PAUSE
  if (!TaskValidate(task)) {
    JobDel(res);
    res=NULL;
  } else {
    LBtr(&task->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(task);
    QueIns(res,task->srv_ctrl.last_waiting);
    LBtr(&task->srv_ctrl.flags,JOBCf_LOCKED);
  }
  POPFD
  return res;
}

CJob *TaskMsg(CTask *_srv,CTask *master,
	I64 msg_code,I64 arg1,I64 arg2,I64 flags)
{//Post message to servant task.  Tell who the master task is.
//See $LK,"flags",A="MN:JOBf_WAKE_MASTER"$ and $LK,"msg_code",A="MN:MSG_CMD"$.
  CJob *tmpc1,*tmpc;
  CTask *srv=_srv;
  if (!TaskValidate(srv) || master && !TaskValidate(master)||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  tmpc=ACAlloc(sizeof(CJob));
  tmpc->master_task=master;
  tmpc->job_code=JOBT_MSG;
  tmpc->msg_code=AbsI64(msg_code); //negative means do a down and up
  tmpc->aux1=arg1;
  tmpc->aux2=arg2;
  tmpc->flags=flags;
  PUSHFD
  if (Bt(&sys_semas[SEMA_RECORD_MACRO],0) &&
	srv!=sys_macro_task && msg_code==MSG_KEY_DOWN) {
    tmpc1=AMAllocIdent(tmpc);
    CLI
    QueIns(tmpc1,sys_macro_head.last);
  }
  CLI
  while (Bt(&srv->task_flags,TASKf_FILTER_INPUT) &&
	!Bt(&flags,JOBf_DONT_FILTER))
    srv=srv->next_input_filter_task;
  tmpc->ctrl=&srv->srv_ctrl;
  while (LBts(&srv->srv_ctrl.flags,JOBCf_LOCKED))
    PAUSE
  if (!TaskValidate(srv)) {
    JobDel(tmpc);
    tmpc=NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(tmpc,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,JOBCf_LOCKED);
  }
  POPFD
  if (msg_code<0) //Down-Up
    TaskMsg(_srv,master,-msg_code+1,arg1,arg2,flags);
  return tmpc;
}

Bool JobResScan(CJob *rqst=NULL,I64 *_res=NULL)
{//Check rqst complete, return with or without.
  CJobCtrl *ctrl;
  CJob *tmpc,*tmpc1;
  if (!rqst || Bt(&rqst->flags,JOBf_DONE)) {
    if (!rqst || rqst->master_task)
      ctrl=&Fs->srv_ctrl;
    else
      ctrl=rqst->ctrl;
    PUSHFD
    CLI
    while (LBts(&ctrl->flags,JOBCf_LOCKED))
      PAUSE
    tmpc1=&ctrl->next_done;
    tmpc=tmpc1->next;
    while (tmpc!=tmpc1) {
      if (!rqst || rqst==tmpc) {
	QueRem(tmpc);
	LBtr(&ctrl->flags,JOBCf_LOCKED);
	POPFD
	if (_res)
	  *_res=tmpc->res;
	JobDel(tmpc);
	return TRUE;
      }
      tmpc=tmpc->next;
    }
    LBtr(&ctrl->flags,JOBCf_LOCKED);
    POPFD
  }
  if (_res)
    *_res=0;
  return FALSE;
}

I64 JobResGet(CJob *rqst=NULL)
{//See $LK,"::/Demo/MultiCore/Lock.HC"$
  I64 res;
  CJob *tmpc1;
  if (!rqst) {
    tmpc1=&Fs->srv_ctrl.next_done;
    while (tmpc1==tmpc1->next) {
      LBts(&Fs->task_flags,TASKf_IDLE);
      Yield;
    }
  } else {
    while (!Bt(&rqst->flags,JOBf_DONE)) {
      LBts(&Fs->task_flags,TASKf_IDLE);
      Yield;
    }
  }
  LBtr(&Fs->task_flags,TASKf_IDLE);
//Could get taken by someone else.
  JobResScan(rqst,&res);
  return res;
}

U0 TaskWait(CTask *task=NULL,Bool cmd_line_pmt=FALSE)
{//Wait for idle.
  CTask *task1;
  CJob *tmpc1;
  if (!task) task=Fs;
  if (TaskValidate(task)) {
    PUSHFD
    CLI
    while (TRUE) {
      task1=task->last_input_filter_task;
      tmpc1=&task1->srv_ctrl.next_waiting;
      if (task1==Fs || !TaskValidate(task1) ||
	    tmpc1==tmpc1->next && Bt(&task1->task_flags,TASKf_IDLE) &&
	    (!cmd_line_pmt || Bt(&task1->task_flags,TASKf_CMD_LINE_PMT)))
	break;
      Yield;
    }
    POPFD
  }
}

U0 PostMsg(CTask *task,I64 msg_code,I64 arg1,I64 arg2,I64 flags=0)
{//Post message to a task and return immediately.  See $LK,"msg_code",A="MN:MSG_CMD"$.
  if (TaskValidate(task)) {
    if (Bt(&task->task_flags,TASKf_INPUT_FILTER_TASK))
      TaskMsg(task->last_input_filter_task,NULL,msg_code,arg1,arg2,
	    flags|1<<JOBf_DONT_FILTER);
    else
      TaskMsg(task,NULL,msg_code,arg1,arg2,flags);
  }
}

U0 PostMsgWait(CTask *task,I64 msg_code,I64 arg1,I64 arg2,I64 flags=0)
{//Post message to a task and wait until task is idle.See $LK,"msg_code",A="MN:MSG_CMD"$.
  PostMsg(task,msg_code,arg1,arg2,flags);
  TaskWait(task);
}

U0 Msg(I64 msg_code,I64 arg1,I64 arg2,I64 flags=0)
{//Post message to current task and return immediately.
//See $LK,"msg_code",A="MN:MSG_CMD"$.
  PostMsg(Fs,msg_code,arg1,arg2,flags);
}

#define JOB_DONE	0
#define JOB_CONT	1
#define JOB_EXIT	2

I64 JobRunOne(I64 run_flags,CJobCtrl *ctrl)
{//Called with ctrl->flags,JOBCf_LOCKED.
  CJob *tmpc=ctrl->next_waiting;
  CTask   *master;
  I64 res,flags=tmpc->flags,old_flags=GetRFlags;
  if (Bt(&flags,JOBf_EXIT_ON_COMPLETE))
    res=JOB_EXIT;
  else
    res=JOB_CONT;
  switch (tmpc->job_code) {
    case JOBT_SPAWN_TASK:
      QueRem(tmpc);
      LBts(&tmpc->flags,JOBf_DISPATCHED);
      LBtr(&ctrl->flags,JOBCf_LOCKED);
      if (tmpc->aux_str)
	tmpc->spawned_task=Spawn(tmpc->addr,tmpc->fun_arg,
	      tmpc->aux_str,,tmpc->aux1,tmpc->aux2,tmpc->flags);
      else
	tmpc->spawned_task=Spawn(tmpc->addr,tmpc->fun_arg,
	      "Unnamed",,tmpc->aux1,tmpc->aux2,tmpc->flags);
      break;
    case JOBT_CALL:
      QueRem(tmpc);
      LBts(&tmpc->flags,JOBf_DISPATCHED);
      LBtr(&ctrl->flags,JOBCf_LOCKED);
      SetRFlags(run_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	      tmpc->res=(*tmpc->addr)(tmpc->fun_arg);
      catch
	Fs->catch_except=TRUE;
      SetRFlags(old_flags);
      break;
    case JOBT_EXE_STR:
      QueRem(tmpc);
      LBts(&tmpc->flags,JOBf_DISPATCHED);
      LBtr(&ctrl->flags,JOBCf_LOCKED);
      SetRFlags(run_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	      tmpc->res=ExePrint("%s",tmpc->aux_str);
      catch
	Fs->catch_except=TRUE;
      SetRFlags(old_flags);
      break;
    default:
      res=JOB_DONE;
  }
  if (res) {
    if (master=tmpc->master_task) {
      if (!Bt(&flags,JOBf_FREE_ON_COMPLETE)) {
	CLI
	while (LBts(&master->srv_ctrl.flags,JOBCf_LOCKED))
	  PAUSE
	QueIns(tmpc,master->srv_ctrl.last_done);
	LBts(&tmpc->flags,JOBf_DONE);
	LBtr(&master->srv_ctrl.flags,JOBCf_LOCKED);
	SetRFlags(old_flags);
      }
      if (Bt(&flags,JOBf_FOCUS_MASTER) &&
	    !Bt(&master->win_inhibit,WIf_SELF_FOCUS))
	sys_focus_task=master;
      if (Bt(&flags,JOBf_WAKE_MASTER))
	Suspend(master,FALSE);
    }
    if (Bt(&flags,JOBf_FREE_ON_COMPLETE))
      JobDel(tmpc);
    else if (!master) {
      CLI
      while (LBts(&ctrl->flags,JOBCf_LOCKED))
	Yield;
      QueIns(tmpc,ctrl->last_done);
      LBts(&tmpc->flags,JOBf_DONE);
      LBtr(&ctrl->flags,JOBCf_LOCKED);
      SetRFlags(old_flags);
    }
  }
  return res;
}

I64 JobsHndlr(I64 run_flags,CTask *task=NULL)
{//Handle all waiting cmds and return.
  I64 cnt=0,old_flags=GetRFlags;
  if (!task) task=Fs;
  while (TRUE) {
    CLI
    while (LBts(&task->srv_ctrl.flags,JOBCf_LOCKED))
      PAUSE
    if (task->srv_ctrl.next_waiting!=&task->srv_ctrl)
      switch (JobRunOne(run_flags,&task->srv_ctrl)) {
	case JOB_CONT:
	  cnt++;
	  break;
	case JOB_EXIT:
	  Exit;
	case JOB_DONE:
	  goto jh_done;
      }
    else
      goto jh_done;
  }
jh_done:
  LBtr(&task->srv_ctrl.flags,JOBCf_LOCKED);
  SetRFlags(old_flags);
  return cnt;
}

I64 PopUp(U8 *buf,CTask *parent=NULL,CTask **_pu_task=NULL)
{//Execute code in $LK,"PopUp",A="MN:PopUp"$ task.
  I64 res;
  CJob *tmpc;
  CTask *task=Spawn(&SrvCmdLine,NULL,"Servant",,parent);
  if (!parent) {
    TaskExe(task,parent,buf,1<<JOBf_EXIT_ON_COMPLETE|1<<JOBf_FREE_ON_COMPLETE);
    if (_pu_task) *_pu_task=task;
    return 0;
  } else {
    Fs->popup_task=task;
    tmpc=TaskExe(task,parent,buf,1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
    if (_pu_task) *_pu_task=task;
    JobResScan(tmpc,&res);
    Fs->popup_task=NULL;
    Kill(task);
    if (_pu_task) *_pu_task=NULL;
    return res;
  }
}

I64 PopUpPrint(U8 *fmt,...)
{//Execute code in $LK,"PopUp",A="MN:PopUp"$ task.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  I64 res;
  res=PopUp(buf,Fs);
  Free(buf);
  return res;
}

I64 Adam(U8 *fmt,...)
{//Make adam_task execute code.
  I64 res;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  CJob *tmpc;
  if (Fs==adam_task) {
    tmpc=TaskExe(adam_task,Fs,buf,0);
    JobsHndlr(GetRFlags);
  } else {
    TaskWait(adam_task);
    tmpc=TaskExe(adam_task,Fs,buf,1<<JOBf_WAKE_MASTER);
  }
  JobResScan(tmpc,&res);
  Free(buf);
  return res;
}

U0 AdamLog(U8 *fmt,...)
{//Display text in adam_task.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  if (Fs==adam_task)
    "%s",buf;
  else if (!IsSingleUser)
    Adam("\"%%s\",%d;",buf);
  Free(buf);
}

U0 AdamErr(U8 *fmt,...)
{//Display red blinking Err text in adam_task.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),
	*st=MStrPrint(ST_ERR_ST "%s",buf);
  if (Fs==adam_task)
    "%s",st;
  else if (!IsSingleUser)
    Adam("\"%%s\",%d;",st);
  Free(st);
  Free(buf);
}

U0 XTalk(CTask *task,U8 *fmt,...)
{//Sends text to other task. See $LK,"::/Misc/OSTestSuite.HC"$.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st=AStrNew(buf),
	*st2=MStrPrint("\"%%s\",%d;Free(%d);",st,st);
  TaskText(task,NULL,st2,0);
  Free(st2);
  Free(buf);
}

U0 XTalkWait(CTask *task,U8 *fmt,...)
{//Send text to other task and wait for it to idle.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st=AStrNew(buf),
	*st2=MStrPrint("\"%%s\",%d;Free(%d);",st,st);
  TaskText(task,NULL,st2,0);
  Free(st2);
  Free(buf);
  TaskWait(task);
}

U0 InStr(U8 *fmt,...)
{//Send InFile code to self.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK))
    ExePrint("%s",buf);
  else
    TaskText(Fs,NULL,buf,1<<JOBf_HIGHEST_PRIORITY);
  Free(buf);
}

U0 InFile(U8 *filename)
{//Send InFile code file to self.
  U8 *name=ExtDft(filename,"IN.Z");
  InStr("Cd(\"%C:%s\");;#include \"%s\"",
	Drv2Let(Fs->cur_dv),Fs->cur_dir,name);
  Free(name);
}

U0 In(U8 *fmt,...)
{//Send text to own input buffer. See $LK,"::/Demo/AcctExample/TOS/TOSDistro.HC"$.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st=AStrNew(buf);
  InStr("\"%%s\",%d;Free(%d);",st,st);
  Free(buf);
}

U0 XTalkStr(CTask *task,U8 *fmt,...)
{//Send InFile code to other task.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TaskText(task,NULL,buf,0);
  Free(buf);
}

U0 XTalkStrWait(CTask *task,U8 *fmt,...)
{//Send InFile code to other task and wait for it to idle.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TaskText(task,NULL,buf,0);
  Free(buf);
  TaskWait(task);
}
