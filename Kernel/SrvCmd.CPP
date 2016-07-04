U0 SrvCmdDel(CSrvCmd *tempc)
{//Free one cmd node.
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

CSrvCmd *TaskExe(CTask *srv,CTask *master,U8 *data,I64 flags)
{//Queues a request to compile and execute src code text.
  CSrvCmd *res;
  if (!data || !TaskValidate(srv) || master && !TaskValidate(master) ||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  res=ACAlloc(sizeof(CSrvCmd));
  res->master_task=master;
  res->cmd_code=SVCT_EXE_STR;
  res->flags=flags;
  res->aux_str=AStrNew(data);
  res->ctrl=&srv->srv_ctrl;

  PUSHFD
  CLI
  while (LBts(&srv->srv_ctrl.flags,SVCRf_LOCKED))
    PAUSE
  if (!TaskValidate(srv)) {
    LBtr(&srv->srv_ctrl.flags,SVCRf_LOCKED);
    POPFD
    SrvCmdDel(res);
    return NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(res,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,SVCRf_LOCKED);
    if (Bt(&flags,SVCf_WAKE_MASTER)) {
      Suspend(master);
      Yield;
    }
  }
  POPFD
  return res;
}

CSrvCmd *TaskText(CTask *srv,CTask *master,U8 *data,I64 flags)
{//Post StdIn text to servant task. Tell who the master task is.
  CSrvCmd *res;
  CTask *task;
  if (!data || !TaskValidate(srv) || master && !TaskValidate(master) ||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  res=ACAlloc(sizeof(CSrvCmd));
  res->master_task=master; //in case somebody cares
  res->cmd_code=SVCT_TEXT_INPUT;
  res->flags=flags;
  res->aux_str=AStrNew(data);

  PUSHFD
  task=srv->last_input_filter_task;
  if (Bt(&flags,SVCf_HIGHEST_PRIORITY) || task==srv) {
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
  while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
    PAUSE
  if (!TaskValidate(task)) {
    SrvCmdDel(res);
    res=NULL;
  } else {
    LBtr(&task->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(task);
    QueIns(res,task->srv_ctrl.last_waiting);
    LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
  }
  POPFD
  return res;
}

CSrvCmd *TaskMsg(CTask *_srv,CTask *master,I64 msg_code,I64 a1,I64 a2,I64 flags)
{//Post message to servant task.  Tell who the master task is.
//See $LK,"flags",A="MN:SVCf_WAKE_MASTER"$ and $LK,"msg_code",A="MN:MSG_CMD"$.
  CSrvCmd *tempc1,*tempc;
  CTask *srv=_srv;
  if (!TaskValidate(srv) || master && !TaskValidate(master)||
	srv->popup_task && !Bt(&srv->task_flags,TASKf_FILTER_INPUT))
    return NULL;
  tempc=ACAlloc(sizeof(CSrvCmd));
  tempc->master_task=master;
  tempc->cmd_code=SVCT_MSG;
  tempc->msg_code=AbsI64(msg_code); //negative means do a down and up
  tempc->aux1=a1;
  tempc->aux2=a2;
  tempc->flags=flags;
  PUSHFD
  if (Bt(&sys_semas[SYS_SEMA_RECORD_MACRO],0) &&
	srv!=sys_macro_task && msg_code==MSG_KEY_DOWN) {
    tempc1=AMAllocIdent(tempc);
    CLI
    QueIns(tempc1,sys_macro_root.last);
  }
  CLI
  while (Bt(&srv->task_flags,TASKf_FILTER_INPUT) &&
	!Bt(&flags,SVCf_DONT_FILTER))
    srv=srv->next_input_filter_task;
  tempc->ctrl=&srv->srv_ctrl;
  while (LBts(&srv->srv_ctrl.flags,SVCRf_LOCKED))
    PAUSE
  if (!TaskValidate(srv)) {
    SrvCmdDel(tempc);
    tempc=NULL;
  } else {
    LBtr(&srv->task_flags,TASKf_IDLE);
    TaskRstAwaitingMsg(srv);
    QueIns(tempc,srv->srv_ctrl.last_waiting);
    LBtr(&srv->srv_ctrl.flags,SVCRf_LOCKED);
  }
  POPFD
  if (msg_code<0) //Down-Up
    TaskMsg(_srv,master,-msg_code+1,a1,a2,flags);
  return tempc;
}

Bool JobResScan(CSrvCmd *rqst=NULL,I64 *_res=NULL)
{//Check rqst complete, return with or without.
  CSrvCtrl *ctrl;
  CSrvCmd *tempc,*tempc1;
  if (!rqst || Bt(&rqst->flags,SVCf_DONE)) {
    if (!rqst || rqst->master_task)
      ctrl=&Fs->srv_ctrl;
    else
      ctrl=rqst->ctrl;
    PUSHFD
    CLI
    while (LBts(&ctrl->flags,SVCRf_LOCKED))
      PAUSE
    tempc1=&ctrl->next_done;
    tempc=tempc1->next;
    while (tempc!=tempc1) {
      if (!rqst || rqst==tempc) {
	QueRem(tempc);
	LBtr(&ctrl->flags,SVCRf_LOCKED);
	POPFD
	if (_res)
	  *_res=tempc->res;
	SrvCmdDel(tempc);
	return TRUE;
      }
      tempc=tempc->next;
    }
    LBtr(&ctrl->flags,SVCRf_LOCKED);
    POPFD
  }
  if (_res)
    *_res=0;
  return FALSE;
}

I64 JobResGet(CSrvCmd *rqst=NULL)
{//See $LK,"::/Demo/MultiCore/Lock.CPP"$
  I64 res;
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
  JobResScan(rqst,&res);
  return res;
}

U0 TaskWait(CTask *task=NULL)
{//Wait for idle.
  CTask *task1;
  CSrvCmd *tempc1;
  if (!task) task=Fs;
  if (TaskValidate(task)) {
    PUSHFD
    CLI
    while (TRUE) {
      task1=task->last_input_filter_task;
      tempc1=&task1->srv_ctrl.next_waiting;
      if (task1==Fs || !TaskValidate(task1) ||
	    tempc1==tempc1->next && Bt(&task1->task_flags,TASKf_IDLE))
	break;
      Yield;
    }
    POPFD
  }
}

U0 PostMsg(CTask *task,I64 msg_code,I64 a1,I64 a2,I64 flags=0)
{//Post message to a task and return immediately.  See $LK,"msg_code",A="MN:MSG_CMD"$.
  if (TaskValidate(task)) {
    if (Bt(&task->task_flags,TASKf_INPUT_FILTER_TASK))
      TaskMsg(task->last_input_filter_task,NULL,msg_code,a1,a2,
	    flags|1<<SVCf_DONT_FILTER);
    else
      TaskMsg(task,NULL,msg_code,a1,a2,flags);
  }
}

U0 PostMsgWait(CTask *task,I64 msg_code,I64 a1,I64 a2,I64 flags=0)
{//Post message to a task and wait until task is idle.See $LK,"msg_code",A="MN:MSG_CMD"$.
  PostMsg(task,msg_code,a1,a2,flags);
  TaskWait(task);
}

U0 Msg(I64 msg_code,I64 a1,I64 a2,I64 flags=0)
{//Post message to current task and return immediately.
//See $LK,"msg_code",A="MN:MSG_CMD"$.
  PostMsg(Fs,msg_code,a1,a2,flags);
}

#define SCR_DONE	0
#define SCR_CONT	1
#define SCR_EXIT	2

I64 SrvCmdRunOne(I64 run_flags,CSrvCtrl *ctrl)
{//Called with ctrl->flags,SVCRf_LOCKED.
  CSrvCmd *tempc=ctrl->next_waiting;
  CTask   *master;
  I64 res,flags=tempc->flags,old_flags=GetRFlags;
  if (Bt(&flags,SVCf_EXIT_ON_COMPLETE))
    res=SCR_EXIT;
  else
    res=SCR_CONT;
  switch (tempc->cmd_code) {
    case SVCT_SPAWN_TASK:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      if (tempc->aux_str)
	tempc->spawned_task=Spawn(tempc->addr,tempc->fun_arg,
	      tempc->aux_str,,tempc->aux1,tempc->aux2,tempc->flags);
      else
	tempc->spawned_task=Spawn(tempc->addr,tempc->fun_arg,
	      "Unnamed",,tempc->aux1,tempc->aux2,tempc->flags);
      break;
    case SVCT_CALL:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetRFlags(run_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	      tempc->res=(*tempc->addr)(tempc->fun_arg);
      catch
	Fs->catch_except=TRUE;
      SetRFlags(old_flags);
      break;
    case SVCT_EXE_STR:
      QueRem(tempc);
      LBts(&tempc->flags,SVCf_DISPATCHED);
      tempc->servant_cpu_num=Gs->num;
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetRFlags(run_flags);
      LBtr(&Fs->task_flags,TASKf_IDLE);
      try
	      tempc->res=ExePrint("%s",tempc->aux_str);
      catch
	Fs->catch_except=TRUE;
      SetRFlags(old_flags);
      break;
    default:
      res=SCR_DONE;
  }
  if (res) {
    if (master=tempc->master_task) {
      if (!Bt(&flags,SVCf_FREE_ON_COMPLETE)) {
	CLI
	while (LBts(&master->srv_ctrl.flags,SVCRf_LOCKED))
	  PAUSE
	QueIns(tempc,master->srv_ctrl.last_done);
	LBts(&tempc->flags,SVCf_DONE);
	LBtr(&master->srv_ctrl.flags,SVCRf_LOCKED);
	SetRFlags(old_flags);
      }
      if (Bt(&flags,SVCf_FOCUS_MASTER) &&
	    !Bt(&master->win_inhibit,WIf_SELF_FOCUS))
	sys_focus_task=master;
      if (Bt(&flags,SVCf_WAKE_MASTER))
	Suspend(master,FALSE);
    }
    if (Bt(&flags,SVCf_FREE_ON_COMPLETE))
      SrvCmdDel(tempc);
    else if (!master) {
      CLI
      while (LBts(&ctrl->flags,SVCRf_LOCKED))
	Yield;
      QueIns(tempc,ctrl->last_done);
      LBts(&tempc->flags,SVCf_DONE);
      LBtr(&ctrl->flags,SVCRf_LOCKED);
      SetRFlags(old_flags);
    }
  }
  return res;
}

I64 SrvCmdsHandler(I64 run_flags,CTask *task=NULL)
{//Handle all waiting cmds and return.
  I64 cnt=0,old_flags=GetRFlags;
  if (!task) task=Fs;
  while (TRUE) {
    CLI
    while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
      PAUSE
    if (task->srv_ctrl.next_waiting!=&task->srv_ctrl)
      switch (SrvCmdRunOne(run_flags,&task->srv_ctrl)) {
	case SCR_CONT:
	  cnt++;
	  break;
	case SCR_EXIT:
	  Exit;
	case SCR_DONE:
	  goto sch_done;
      }
    else
      goto sch_done;
  }
sch_done:
  LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
  SetRFlags(old_flags);
  return cnt;
}

I64 PopUp(U8 *buf,CTask *parent=NULL,CTask **_pu_task=NULL)
{//Execute code in $LK,"PopUp",A="MN:PopUp"$ task.
  I64 res;
  CSrvCmd *tempc;
  CTask *task=Spawn(&SrvCmdLine,NULL,"Servant",,parent);
  if (!parent) {
    TaskExe(task,parent,buf,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
    if (_pu_task) *_pu_task=task;
    return 0;
  } else {
    Fs->popup_task=task;
    tempc=TaskExe(task,parent,buf,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
    if (_pu_task) *_pu_task=task;
    JobResScan(tempc,&res);
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
  CSrvCmd *tempc;
  if (Fs==adam_task) {
    tempc=TaskExe(adam_task,Fs,buf,0);
    SrvCmdsHandler(GetRFlags);
  } else {
    TaskWait(adam_task);
    tempc=TaskExe(adam_task,Fs,buf,1<<SVCf_WAKE_MASTER);
  }
  JobResScan(tempc,&res);
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
{//Sends text to other task. See $LK,"::/Misc/OSTestSuite.CPP"$.
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

U0 AutoStr(U8 *fmt,...)
{//Send AutoFile code to self.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  if (Bt(&Fs->task_flags,TASKf_INPUT_FILTER_TASK))
    ExePrint("%s",buf);
  else
    TaskText(Fs,NULL,buf,1<<SVCf_HIGHEST_PRIORITY);
  Free(buf);
}

U0 AutoFile(U8 *filename)
{//Send AutoFile code file to self.
  U8 *name=DftExt(filename,"AUT.Z");
  AutoStr("Cd(\"%C:%s\");;#include \"%s\"",
	Drv2Let(Fs->cur_dv),Fs->cur_dir,name);
  Free(name);
}

U0 Auto(U8 *fmt,...)
{//Send text to own input buffer. See $LK,"::/Demo/AcctExample/TOSDistro.CPP"$.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st=AStrNew(buf);
  AutoStr("\"%%s\",%d;Free(%d);",st,st);
  Free(buf);
}

U0 XTalkStr(CTask *task,U8 *fmt,...)
{//Send AutoFile code to other task.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TaskText(task,NULL,buf,0);
  Free(buf);
}

U0 XTalkStrWait(CTask *task,U8 *fmt,...)
{//Send AutoFile code to other task and wait for it to idle.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TaskText(task,NULL,buf,0);
  Free(buf);
  TaskWait(task);
}
