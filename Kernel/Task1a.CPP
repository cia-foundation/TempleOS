U0 Exit()
{//Terminate own task.
  if (Fs==sys_focus_task && IsDbg) {
    LBts(&Fs->task_flags,TASKf_KILL_AFTER_DBG);
    G;
  } else {
    if (!Gs->num && !IsDbg)
      SingleUser(OFF);
    Fs->rflags=GetRFlags;
    Fs->rsp=GetRSP;
    Fs->rbp=GetRBP;
    Fs->rip=$$;
    CLI
    LBts(&Fs->task_flags,TASKf_KILL_TASK);
    TaskEndNow;
  }
}

Bool TaskValidate(CTask *task)
{//return TRUE if task looks valid.
  if (!(0<task<=MAX_I32) || task->addr!=task ||
    task->task_signature(I64)!=TASK_COMBINED_SIGNATURE_VAL)
    return FALSE;
  else
    return TRUE;
}

U0 Kill(CTask *task,Bool wait=TRUE)
{//Terminate other task.
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
{//Tell scheduler to skip task.
  Bool result;
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (TaskValidate(task))
    result=LBEqu(&task->task_flags,TASKf_SUSPENDED,state);
  else
    result=FALSE;
  POPFD
  return result;
}

Bool IsSuspended(CTask *task=NULL)
{//You might use this in a DrawIt() or Animatetask().
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
argpop I64 CallStkGrow(I64 stk_size_threshold,I64 stk_size,/*argpop*/I64 (*fp_addr)(...),...)
{//Grow stk in call with any fixed num of args.
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
	JMP	RBX	//CALL fp_addr()
};

  } else {

    temps2=TaskStkNew(stk_size,Fs);
    temps2->next_stk=temps=Fs->stk;
    rsp2=(&temps2->stk_base)(U8 *)+temps2->stk_size;
    old_stk=rsp=&argv[argc];
    while (argc-->0)
      *--rsp2=*--rsp;
    _stk=&Fs->stk;
    temps->stk_ptr=rsp=GetRSP;
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

	CALL	U64 &fp_addr[RBP]
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
{//Returns Fs of task
  CTaskStk *temps;

  QueInit(&task->code_heap->next_mem_blk);
  if (task->code_heap!=task->data_heap)
    QueInit(&task->data_heap->next_mem_blk);

  task->addr=task->next_task=task->last_task=
    task->next_input_filter_task=task->last_input_filter_task=
    task;

  task->task_num=sys_num_spawned_tasks++;

  task->rflags=SYS_NORMAL_RFLAGS;
  task->win_inhibit=WIG_TASK_DFT;

  task->next_child_task=task->last_child_task=
    (&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task);

  SrvCtrlInit(&task->srv_ctrl);
  QueInit(&task->next_lex);
  QueInit(&task->next_except);
  QueInit(&task->next_ctrl);
  QueInit(&task->next_ode);

  task->fpu_mmx=MAllocAligned(sizeof(CFPU),0x10,task);
  MemCpy(task->fpu_mmx,SYS_FIXED_AREA+offset(CSysFixedArea.init_fpu_mmx),sizeof(CFPU));

  if (task==SYS_FIXED_AREA+offset(CSysFixedArea.adam)) {
// Delayed until heap expanded.  See $LK,"Expand Heap",A="FA:::/Kernel/Mem1a.CPP,ExpandHeap"$
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

  task->text_attr	=TERM_ATTR_TEXT;
  task->border_attr	=TERM_ATTR_BORDER;
  task->border_src	=BDS_CONST;
  task->title_src	=TTS_CONST;
  task->win_left	=1;
  task->win_right	=text.cols-2;
  task->win_top		=13;
  task->win_bottom	=text.rows-2;

  //Beware Adam $LK,"TaskInit",A="FF:::/Kernel/KStart.CPP,TaskInit"$ gets NULL.  Okay, I guess, until $LK,"ChgDsk",A="FF:::/Kernel/KEnd.CPP,ChgDsk"$().
  task->cur_prt=dsk.drv_to_prt[ToUpper(*sys_acct)-'A'];
  task->cur_dir=StrNew(sys_acct+2,task);

  return task;
}

CTask *Spawn(U0 (*fp_start_addr)(U8 *data),U8 *data=NULL,
		U8 *task_name=NULL,
		I64 target_cpu=-1,	//-1 means current CPU. See $LK,"multi-core",A="FI:::/Demo/MultiCore/LoadTest.CPP"$.
		CTask *parent=NULL,	//NULL means adam
		I64 stk_size=0,		//0=default
		I64 flags=1<<SVCf_ADD_TO_QUE)
{//Create task on core running at address.
//Alloc $LK,"CTask",A="MN:CTask"$ structure from code heap so addr will be short.
//Could be allocated off of data heap.
  CTask *task;
  if (target_cpu>=0)
    return SpawnQue(fp_start_addr,data,task_name,target_cpu,parent,stk_size,flags);
  task=CAlloc(sizeof(CTask),adam_task->code_heap);
  task->task_signature=TASK_SIGNATURE_VAL;
  if (!task_name) task_name="Unnamed Task";
  if (!parent) parent=Gs->seth_task;
  task->parent_task=parent;
  task->gs=parent->gs;
  if (sys_code_bp)
    task->code_heap=HeapCtrlInit(,task,sys_code_bp);
  if (sys_data_bp)
    task->data_heap=HeapCtrlInit(,task,sys_data_bp);
  else
    task->data_heap=task->code_heap;
  TaskInit(task,stk_size);
  task->rip=fp_start_addr;
  task->rsp(U8 *)-=8;
  *task->rsp=data;
  task->rsp(U8 *)-=8;
  *task->rsp=&Exit;
  task->hash_table->next=parent->hash_table;
  MemCpy(task->task_name,task_name,TASK_NAME_LEN+1);
  StrCpy(task->task_title,task->task_name);
  task->title_src=TTS_TASK_NAME;
  PUSHFD
  CLI
  if (Bt(&flags,SVCf_ADD_TO_QUE)) {
    TaskQueInsChild(task);
    TaskQueIns(task);
  }
  POPFD
  return task;
}

U0 TaskDerivedValsUpdate(CTask *task=NULL,Bool update_z_buf=TRUE)
{//Those things calculated from other variables.
  if (!task) task=Fs;
  PUSHFD
  CLI
  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
    PAUSE
  WinDerivedValsUpdate(task);
  if (fp_update_ctrls)
    (*fp_update_ctrls)(task);
  if (update_z_buf && Bt(&task->display_flags,DISPLAYf_SHOW))
    LBts(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0);
  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
  POPFD
}

I64 ExeCmdLine(CLex *lx)
{//Terminal JIT-compile-and-execute loop for CLex.
  I64 result=0,type,old_title_src=Fs->title_src;
  U8 *ptr,*ptr2,*ptr3,*ex,*old_task_title=StrNew(Fs->task_title);
  F64 t0;
  CDocEntry *doc_e;
  CDoc *doc;
  if (Fs->title_src!=TTS_LOCKED_CONST)
    Fs->title_src=TTS_CUR_LEX;
  while (lx->token &&
    (lx->token!='}' || !(lx->flags & LF_EXE_BLK)) ) {
    if (Fs->title_src==TTS_CUR_LEX) {
      ptr2=&Fs->task_title;
      ptr3=ptr2+TASK_TITLE_LEN;
      if (lx->cur_lfn->flags & LFSF_DOC) {
	doc_e=lx->cur_lfn->cur_entry;
	doc=lx->cur_lfn->doc;
	while (doc_e!=doc && ptr2<ptr3) {
	  switch (doc_e->type_u8) {
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
	if (ptr2<ptr3) *ptr2=0;
      } else
	if ((ptr=lx->cur_lfn->line_start) && *ptr)
	  MemCpy(ptr2,ptr,TASK_TITLE_LEN);
    }
    lx->flags&=~LF_HAS_MISC_DATA;
    ex=LexStatement2Bin(lx,&type);
    if (ex!=INVALID_PTR) {
      if (!(lx->flags&LF_JUST_LOAD)) {
	t0=tS;
	result=Call(ex);
	Fs->answer=result;
	Fs->answer_type=type;
	Fs->answer_time=tS-t0;
      }
      if (!(lx->flags&LF_HAS_MISC_DATA))
	Free(ex);
    }
  }
  if (Fs->title_src!=TTS_LOCKED_CONST) {
    Fs->title_src=old_title_src;
    StrCpy(Fs->task_title,old_task_title);
  }
  Free(old_task_title);
  if (lx->flags&LF_JUST_LOAD) {
    if (lx->error_cnt)
      return FALSE;
    else
      return TRUE;
  } else
    return result;
}

U0 UserTaskCont()
{//Terminal key-input-execute loop.
  CLex *lx;
  CDoc *doc;
  Bool cont=TRUE;
  do {
    lx=LexNew(,LF_PMT);
    QueIns(lx,Fs->last_lex);
    try {
      Lex(lx);
      ExeCmdLine(lx);
      cont=Bt(&lx->flags,Lf_PMT);
      QueRem(lx);
      LexDel(lx);
    } catch {
      if ((doc=Fs->put_doc) && doc->doc_signature==DOC_SIGNATURE_VAL)
	DocUnlock(doc);
      PutExcept;
    }
  } while (cont);
}

U0 UserCmdLine(I64 dummy=0)
{//A user task ends-up calling this.
  nounusedwarn dummy;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  if (HashFind("UserStartUp",adam_task->hash_table,HTT_FUN))
    CallExtStr("UserStartUp");
  if (!LBts(&Fs->display_flags,DISPLAYf_SHOW))
    Dbg;
  UserTaskCont;
}

CTask *SpawnUser(U8 *fmt=NULL,...)
{//Create user term task.
  U8 *st;
  CTask *task=Spawn(&UserCmdLine);
  TaskWait(task);
  if (fmt) {
    st=StrPrintJoin(NULL,fmt,argc,argv);
    XTalk(task,st);
    Free(st);
  }
  return task;
}

U0 SrvTaskCont()
{//Act as server task in a loop handling commands.
  FlushMsgs;
  while (TRUE) {
    if (SrvCmdsHandler && Fs->title_src==TTS_TASK_NAME)
      MemCpy(Fs->task_title,Fs->task_name,TASK_NAME_LEN+1);
    FlushMsgs;
    LBts(&Fs->task_flags,TASKf_IDLE);
    LBts(&Fs->task_flags,TASKf_AWAITING_MSG);
    Yield;
  }
}

U0 SrvUserCmdLine(I64 dummy=0)
{
  nounusedwarn dummy;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  if (HashFind("SrvStartUp",adam_task->hash_table,HTT_FUN))
    CallExtStr("SrvStartUp");
  SrvTaskCont;
}

U0 TaskFinishOff(CTask *task)
{//We delay freeing in case lingering ptr to reincarnated.
  HeapCtrlDel(task->code_heap);
  if (task->data_heap!=task->code_heap)
    HeapCtrlDel(task->data_heap);
  Free(task);
}

I64 TaskEnd()
{//Called with irq's off.
  CTask *task=Fs,*tempt,*tempt1;
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
  if (task->parent_task && task->parent_task->popup_task==task) {
    task->parent_task->popup_task=NULL;
    Kill(task->parent_task,FALSE);
    return task->parent_task;
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
    PAUSE
  while (LBts(&task->srv_ctrl.flags,SVCRf_LOCKED))
    PAUSE

  SrvCmdQueDel(&task->srv_ctrl.next_waiting);
  SrvCmdQueDel(&task->srv_ctrl.next_done);

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

  LBtr(&task->srv_ctrl.flags,SVCRf_LOCKED);
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
{//Called by Seth tasks. Delay freeing in case lingering ptr to reincarnated.
  CTask *task1,*task2;
  if (task1=XchgI64(&Gs->dying_task_lst,NULL)) {
    do {
      task2=task1->next_task;
      TaskFinishOff(task1);
      task1=task2;
    } while (task1);
  }
}
