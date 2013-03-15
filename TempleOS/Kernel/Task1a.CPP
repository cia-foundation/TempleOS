U0 Exit()
{
  if (Fs==sys_focus_task && IsInDbg) {
    LBts(&Fs->task_flags,TASKf_KILL_AFTER_DBG);
    G;
  } else {
    if (!Gs->num && !IsInDbg)
      SingleUser(OFF);
    Fs->rflags=GetFlags;
    Fs->rsp=GetStkPtr;
    Fs->rbp=GetBasePtr;
    Fs->rip=$$;
    Cli;
    LBts(&Fs->task_flags,TASKf_KILL_TASK);
    TaskEndNow;
  }
}

Bool TaskValidate(CTask *task)
{
  if (!(0<task<=MAX_I32) || task->addr!=task ||
    task->task_signature(I64)!=TASK_COMBINED_SIGNATURE_VAL)
    return FALSE;
  else
    return TRUE;
}

U0 Kill(CTask *task,Bool wait=TRUE)
{
  I64 i;
  CCPU *c;
  Bool found,old_preempt=Preempt(OFF);
  if (TaskValidate(task)) {
    if (task==sys_winmgr_task)
      found=TRUE;
    else
      found=FALSE;
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      if (task==c->seth_task)
	found=TRUE;
    }
    if (!found) {
      LBts(&task->task_flags,TASKf_KILL_TASK);
      if (wait) {
	do Yield;
	while (TaskValidate(task) && Bt(&task->task_flags,TASKf_KILL_TASK));
      }
    }
  }
  Preempt(old_preempt);
}

Bool Suspend(CTask *task=NULL,Bool state=TRUE)
{
  Bool result;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (TaskValidate(task))
    result=LBEqu(&task->task_flags,TASKf_SUSPENDED,state);
  else
    result=FALSE;
  PopFD;
  return result;
}

Bool IsSuspended(CTask *task=NULL)
{
  if (!task) task=Fs;
  if (TaskValidate(task))
    return Bt(&task->task_flags,TASKf_SUSPENDED);
  else
    return FALSE;
}

CTaskStk *TaskStkNew(I64 stk_size,CTask *task)
{
  CTaskStk *temps=MAlloc(stk_size+offset(CTaskStk.stk_base),task);
  temps->next_stk=NULL;
  temps->stk_ptr=NULL;
  temps->stk_size=MSize(temps)-offset(CTaskStk.stk_base);
  return temps;
}


#exe {OptOn(OPTf_NO_REG_VAR);};
//fp_add can have any fixed number of arguments.
argpop I64 CallStkGrow(I64 stk_size_threshold,I64 stk_size,/*argpop*/I64 (*fp_add)(...),...)
{
  CTaskStk *temps,*temps2,**_stk;
  I64 result,*rsp,*rsp2,*old_stk;

  if (UnusedStk>=stk_size_threshold) {

asm {
	LEAVE
	POP	RAX	//return addr
	ADD	RSP,16	//pop threshold,stk_size
	POP	RBX	// *f
	ADD	RSP,8	//pop ARGC
	PUSH	RAX
	JMP	RBX	//CALL fp_add()
};

  } else {

    temps2=TaskStkNew(stk_size,Fs);
    temps2->next_stk=temps=Fs->stk;
    rsp2=(&temps2->stk_base)(U8 *)+temps2->stk_size;
    old_stk=rsp=&argv[argc];
    while (argc-->0)
      *--rsp2=*--rsp;
    _stk=&Fs->stk;
    temps->stk_ptr=rsp=GetStkPtr;
asm {
	IMPORT	_FREE;	//We are in a function, not at glbl scope.
			//The compiler treats these in isolation.

	PUSHFD
	POP	RDX	//flags
	CLI
	MOV	RBX,U64 &temps2[RBP]
	MOV	RAX,&_stk[RBP]
	MOV	U64 [RAX],RBX	//Fs->stk=temps2
	MOV	RSP,U64 &rsp2[RBP]
	PUSH	RDX
	POPFD

	CALL	U64 &fp_add[RBP]
	MOV	U64 &result[RBP],RAX

	PUSHFD
	POP	RDX	//flags
	CLI
	MOV	RBX,U64 &temps[RBP]
	MOV	RAX,&_stk[RBP]
	MOV	U64 [RAX],RBX	//Fs->stk=temps
	MOV	RSP,U64 &rsp[RBP]
	PUSH	RDX
	POPFD

	PUSH	U64 &temps2[RBP]
	CALL	_FREE

	MOV	RDX,U64 &old_stk[RBP]
	MOV	RBX,U64 8[RBP]
	MOV	RAX,U64 &result[RBP]
	MOV	RBP,U64 [RBP]
	MOV	RSP,RDX
	JMP	RBX	//return
};
  }
  return 0; //dummy to get rid of warning
}
;
#exe {OptOff(OPTf_NO_REG_VAR);};

I64 TaskInit(CTask *task,I64 stk_size)
{ //Returns Fs of task
  CTaskStk *temps;

  QueInit(&task->code_heap->next_mem_blk);
  if (task->code_heap!=task->data_heap)
    QueInit(&task->data_heap->next_mem_blk);

  task->addr=
    task->next_task=
    task->last_task=
    task->next_input_filter_task=
    task->last_input_filter_task=
    task;

  task->task_num=sys_num_task_inits++;

  task->rflags=SYS_NORMAL_RFLAGS;
  task->win_max_refresh=WIN_DFT_MAX_REFRESH;
  SetSnap(&task->snap,1,1.0);

  task->next_child_task=
    task->last_child_task=
    (&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task);

  task->next_doc=
    task->last_doc=
    (&task->next_doc)(U8 *)-offset(CDoc.next_doc);

  QueInit(&task->next_servant_cmd);
  QueInit(&task->next_master_cmd);
  QueInit(&task->next_lex);
  QueInit(&task->next_except);
  QueInit(&task->next_ctrl);
  QueInit(&task->next_ode);

  task->fpu_mmx=MAllocAligned(sizeof(CFPU),0x10,task);
  MemCpy(task->fpu_mmx,&sys_init_fpu_mmx,sizeof(CFPU));

  if (task==SYS_ADAM_TASK) {
// Delayed until heap expanded.  See $LK,"Expand Heap","FA:::/TempleOS/Kernel/Memory.CPP,ExpandHeap"$
//    task->hash_table=HashTableNew(ADAM_HASH_SIZE,task);
  } else {
    if (!Gs->num)
      task->hash_table=HashTableNew(STD_HASH_SIZE,task);
    else
      task->hash_table=HashTableNew(MP_STD_HASH_SIZE,task);
  }

  if (!stk_size) {
    if (!Gs->num)
      stk_size=DFT_STK;
    else
      stk_size=MP_DFT_STK;
  }
  task->stk=temps=TaskStkNew(stk_size,task);
  task->rsp=(&temps->stk_base)(U8 *)+temps->stk_size;

  task->answers_displayed=1;  //from 0-8
  task->text_attr  =WHITE<<4+BLUE;
  task->border_attr=WHITE<<4+LTBLUE;
  task->win_left   =1;
  task->win_right  =text_cols-2;
  task->win_top    =1;
  task->win_bottom =text_rows-2;

  return task;
}

CTask *Spawn(U0 (*fp_start_add)(U8 *data),U8 *data=NULL,
	    U8 *task_name=NULL,
	    CTask *parent=NULL, //NULL means adam
	    I64 stk_size=0, //0=default
	    I64 flags=SPAWNF_ADD_TO_QUE)
{
//We alloc from code heap so addr will be short
//Could be allocated off of data heap
  CTask *task=CAlloc(sizeof(CTask),adam_task->code_heap);
  task->task_signature=TASK_SIGNATURE_VAL;
  if (!task_name) task_name="Unnamed Task";
  if (!parent) parent=Gs->seth_task;
  task->parent_task=parent;
  task->gs=parent->gs;
  if (sys_code_bp)
    task->code_heap=TaskHeapCtrlNew(task,sys_code_bp);
  if (sys_data_bp)
    task->data_heap=TaskHeapCtrlNew(task,sys_data_bp);
  else
    task->data_heap=task->code_heap;
  TaskInit(task,stk_size);
  task->rip=fp_start_add;
  task->rsp(U8 *)-=8;
  *task->rsp=data;
  task->rsp(U8 *)-=8;
  *task->rsp=&Exit;
  task->hash_table->next=parent->hash_table;
  MemCpy(task->task_title,task_name,TASK_TITLE_LEN+1);
  MemCpy(task->task_name,task_name,TASK_NAME_LEN+1);
  PushFD;
  Cli;
  if (flags&SPAWNF_ADD_TO_QUE) {
    TaskQueInsChild(task);
    TaskQueIns(task);
  }
  PopFD;
  return task;
}

U0 ServantTaskCont()
{
  FlushMsgs;
  while (TRUE) {
    if (ServantCmdsHandler)
      if (!Bt(&Fs->task_flags,TASKf_LOCK_TITLE))
	MemCpy(Fs->task_title,Fs->task_name,TASK_NAME_LEN+1);
    FlushMsgs;
    LBts(&Fs->task_flags,TASKf_IDLE);
    LBts(&Fs->task_flags,TASKf_AWAITING_MSG);
    Yield;
  }
}

U0 TaskDerivedValsUpdate(CTask *task=NULL,Bool update_z_buf=TRUE)
{
  if (!task) task=Fs;
  PushFD;
  Cli;
  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  WinDerivedValsUpdate(task);
  if (fp_update_ctrls)
    (*fp_update_ctrls)(task);
  if (update_z_buf && Bt(&task->display_flags,DISPLAYf_SHOW))
    LBts(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0);
  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
  PopFD;
}

U0 TaskInitExt(Bool from_dbg=FALSE)
{
  if (LBts(&Fs->task_flags,TASKf_INIT_EXT))
    return;
  if (!from_dbg) {
    LBts(&Fs->display_flags,DISPLAYf_HAS_CLOSE_WIN);
    Fs->text_attr  =WHITE<<4+BLUE;
    Fs->border_attr=WHITE<<4+LTBLUE;
    Fs->win_top   =13;
    Fs->win_bottom=text_rows-2;
    TaskDerivedValsUpdate;
  }
  Cd("HOME");
  CallExtNum(EXT_WIN_TO_TOP,Fs,Bt(&Fs->display_flags,DISPLAYf_SHOW));
}

I64 ExeCmdLine(CLex *lx)
{
  I64 result=0,i,type;
  U8 *ptr,*ptr2,*ptr3,*code;
  F64 t0;
  CDocEntry *doc_e;
  CDoc *doc;
  while (lx->token &&
    (lx->token!='}' || !(lx->flags & LF_EXE_BLK)) ) {
    if (!Bt(&Fs->task_flags,TASKf_LOCK_TITLE)) {
      ptr2=&Fs->task_title;
      ptr3=ptr2+TASK_TITLE_LEN;
      if (lx->cur_lfn->flags & LFSF_IS_DOC) {
	doc_e=lx->cur_lfn->line_start;
	doc=lx->cur_lfn->doc;
	while (doc_e!=doc && ptr2<ptr3) {
	  switch (doc_e->type_u8 && ptr2<ptr3) {
	    case DOCT_TEXT:
	      ptr=doc_e->tag;
	      while (*ptr && ptr2<ptr3)
		*ptr2++=*ptr++;
	      break;
	    case DOCT_TAB:
	    case DOCT_NEW_LINE:
	      *ptr2++='.';
	      break;
	  }
	  doc_e=doc_e->next;
	}
      } else
	if (ptr=lx->cur_lfn->line_start)
	  MemCpy(ptr2,ptr,TASK_TITLE_LEN);
    }
    lx->flags&=~LF_HAS_MISC_DATA;
    code=CmpStatement(lx,&type);
    if (code!=INVALID_PTR) {
      if (!(lx->flags&LF_JUST_LOAD)) {
	t0=tT;
	result=Call(code);
	for (i=2;i>=0;i--) {
	  Fs->answers[i+1]=Fs->answers[i];
	  Fs->answer_types[i+1]=Fs->answer_types[i];
	  Fs->answer_times[i+1]=Fs->answer_times[i];
	}
	Fs->answers[0]=result;
	Fs->answer_types[0]=type;
	Fs->answer_times[0]=tT-t0;
      }
      if (!(lx->flags&LF_HAS_MISC_DATA))
	Free(code);
    }
  }
  if (lx->flags&LF_JUST_LOAD) {
    if (lx->error_cnt)
      return FALSE;
    else
      return TRUE;
  } else
    return result;
}

U0 UserTaskCont()
{
  CLex *lx;
  CDoc *doc;
  Bool cont=TRUE;
  do {
    lx=LexNew(NULL,LF_PMT,NULL);
    QueIns(lx,Fs->last_lex);
    try {
      Lex(lx);
      ExeCmdLine(lx);
      cont=Bt(&lx->flags,Lf_PMT);
      QueRem(lx);
      LexDel(lx);
    } catch {
      doc=Fs->last_doc;
      if (doc!=(&Fs->next_doc)(U8 *)-offset(CDoc.next_doc) &&
	  doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL)
	DocUnlock(doc);
      CatchAll;
    }
  } while (cont);
}

U0 UserCmdLine(I64 dummy=0)
{
  nounusedwarn dummy;
  LBts(&Fs->task_flags,TASKf_USER);
  TaskInitExt;
  Fs->user_num=++sys_num_spawned_users;
  if (HashFind("UserStartUp",adam_task->hash_table,HTT_FUN))
    CallExtStr("UserStartUp");
  if (!LBts(&Fs->display_flags,DISPLAYf_SHOW))
    Dbg;
  UserTaskCont;
}

U0 ServantUserCmdLine(I64 dummy=0)
{
  nounusedwarn dummy;
  LBts(&Fs->task_flags,TASKf_USER);
  TaskInitExt;
  if (HashFind("ServantStartUp",adam_task->hash_table,HTT_FUN))
    CallExtStr("ServantStartUp");
  ServantTaskCont;
}

CTask *SpawnUser(U8 *fmt=NULL,...)
{
  U8 *st;
  CTask *task=Spawn(&UserCmdLine);
  TaskWait(task);
  if (fmt) {
    st=SPrintFJoin(NULL,fmt,argc,argv);
    XTalk(task,st);
    Free(st);
  }
  return task;
}

U0 TaskFinishOff(CTask *task)
{ //We delay freeing in case lingering
  //references are made to task's.
  TaskHeapCtrlDel(task->code_heap);
  if (task->data_heap!=task->code_heap)
    TaskHeapCtrlDel(task->data_heap);
  Free(task);
}

I64 TaskEnd()
{ //called with irq's off
  CTask *task=Fs,*tempt,*tempt1;
  CTaskCmd *tempc,*tempc1,*tempc2;
  if (task==sys_task_being_screen_updated) {
    LBts(&task->task_flags,TASKf_KILL_TASK);
    return task->next_task;
  }
  if (task->task_end_cb) {
    task->wake_jiffy=0;
    LBtr(&task->task_flags,TASKf_KILL_TASK);
    TaskRstAwaitingMsg(task);
    Suspend(task,FALSE);
    task->rip=task->task_end_cb;
    task->task_end_cb=NULL;
    return task;
  }
  if (task->parent_task) {
    if (task->parent_task->popup_task==task) {
      task->parent_task->popup_task=NULL;
      Kill(task->parent_task,FALSE);
      return task->parent_task;
    }
    if (task->parent_task->dbg_task==task)
      task->parent_task->dbg_task=NULL;
  }

  PrtsRelease;
  BlkDevsRelease;
  tempt1=(&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task);
  tempt=tempt1->next_sibling_task;
  if (tempt!=tempt1) {
    do {
      LBts(&tempt->task_flags,TASKf_KILL_TASK);
      tempt=tempt->next_sibling_task;
    } while (tempt!=tempt1);
    return task->next_task;
  }
  if (LBtr(&task->display_flags,DISPLAYf_SHOW))
    LBts(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0);

  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
    Pause;
  tempc1=&task->next_servant_cmd;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    tempc2=tempc->next;
    Free(tempc->data);
    Free(tempc);
    tempc=tempc2;
  }

  tempc1=&task->next_master_cmd;
  tempc=tempc1->next;
  while (tempc!=tempc1) {
    tempc2=tempc->next;
    Free(tempc->data);
    Free(tempc);
    tempc=tempc2;
  }

  if (IsRaw)
    VGAFlush;

  if (sys_focus_task==task) {
    if (!Gs->num)
      SingleUser(OFF);
    sys_focus_task=NULL;
    if (fp_set_std_palette)
      (*fp_set_std_palette)();
  }

//QueRem
  task->task_signature(I64)=0;

  tempt =task->next_input_filter_task;
  tempt1=task->last_input_filter_task;
  tempt1->next_input_filter_task=tempt;
  tempt ->last_input_filter_task=tempt1;

  tempt =task->next_sibling_task;
  tempt1=task->last_sibling_task;
  tempt1->next_sibling_task=tempt;
  tempt ->last_sibling_task=tempt1;

  tempt =task->next_task; //save to return
  TaskQueRem(task);

  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);

  if (sys_winmgr_task) {
    task->next_task=Gs->dying_task_lst;
    Gs->dying_task_lst=task;
  } else
    TaskFinishOff(task); //dangerous (stk in use)
//Never happens--winmgr exists except during boot and no tasks die during boot
//currently, though this has been different and might be different in future.
//Would have to be multicore task getting this task's memory
//because IRQs are off, so no tasks on this core run.
  return tempt;
}

U0 TaskFinishOffDying()
{ //Called by Seth tasks with preemption off.
/*
Delay freeing a task in case lingering
references are made.  $LK-A,"::/TempleOS/Adam/WallPaper.CPP"$,
for example, might be processing a task record.

A dead task fails $LK,"TaskValidate","MN:TaskValidate"$(), but
what if a task dies and is reincarnated
as another task with the same task record!

A slight delay in freeing the task record
is just a precaution, in case code somewhere
doesn't use $LK,"Preempt","MN:Preempt"$(OFF) and could get burned
by a task being reincarnated.
*/
  CTask *task1,*task2;
  if (task1=Gs->dying_task_lst) {
    do {
      task2=task1->next_task;
      TaskFinishOff(task1);
      task1=task2;
    } while (task1);
    Gs->dying_task_lst=NULL;
  }
}
