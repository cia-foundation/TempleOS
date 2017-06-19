U0 Exit()
{//Terminate own task.
  if (Fs==sys_focus_task && IsDbgMode) {
    LBts(&Fs->task_flags,TASKf_KILL_AFTER_DBG);
    G;
  } else {
    if (sys_staff_mode_flag)
      AdamLog("%p:%p:%p:%p:%p:%p\n",Caller(0),Caller(1),Caller(2),Caller(3),
	    Caller(4),Caller(5),Caller(6),Caller(7));
    if (!Gs->num && !IsDbgMode)
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
  if (!(0<task<=I32_MAX) || task->addr!=task ||
	task->task_signature!=TASK_SIGNATURE_VAL)
    return FALSE;
  else
    return TRUE;
}

I64 BirthWait(CTask **_task,I64 task_num=-1)
{//Wait for task valid and not task_num.
  while (!TaskValidate(*_task)||(*_task)->task_num==task_num)
    Yield;
  return (*_task)->task_num;
}

U0 DeathWait(CTask **_task,Bool send_exit=FALSE)
{//Wait for task death.
  if (send_exit && TaskValidate(*_task)) {
    TaskWait(*_task,TRUE);
    XTalk(*_task,"Exit;\n");
  }
  while (TaskValidate(*_task))
    Yield;
}

Bool Kill(CTask *task,Bool wait=TRUE,Bool just_break=FALSE)
{//Terminate other task.
  I64 i;
  if (TaskValidate(task)) {
    if (just_break) {
      if (task!=Fs)
	Break;
      else {//TODO wait
	sys_focus_task=task;
	LBts(sys_ctrl_alt_flags,CTRL_ALT_C);
	return TRUE;
      }
    } else {
      if (task!=sys_winmgr_task) {
	for (i=0;i<mp_cnt;i++)
	  if (task==cpu_structs[i].seth_task)
	    return FALSE;
	LBts(&task->task_flags,TASKf_KILL_TASK);
	if (wait) {
	  do Yield;
	  while (TaskValidate(task) && Bt(&task->task_flags,TASKf_KILL_TASK));
	}
	return TRUE;
      }
    }
  }
  return FALSE;
}

Bool Suspend(CTask *task=NULL,Bool state=TRUE)
{//Tell scheduler to skip task.
  Bool res;
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (TaskValidate(task))
    res=LBEqu(&task->task_flags,TASKf_SUSPENDED,state);
  else
    res=FALSE;
  POPFD
  return res;
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
  CTaskStk *tmps=MAlloc(stk_size+offset(CTaskStk.stk_base),task);
  tmps->next_stk=NULL;
  tmps->stk_ptr=NULL;
  tmps->stk_size=MSize(tmps)-offset(CTaskStk.stk_base);
  return tmps;
}

#exe {Option(OPTf_NO_REG_VAR,ON);};
argpop I64 CallStkGrow(I64 stk_size_threshold,I64 stk_size,
			/*argpop*/I64 (*fp_addr)(...),...)
{//Grow stk in call with any fixed num of args.
//See $LK,"::/Demo/StkGrow.HC"$.
  CTaskStk *tmps,*tmps2,**_stk;
  I64 res,*rsp,*rsp2,*old_stk;

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

    tmps2=TaskStkNew(stk_size,Fs);
    tmps2->next_stk=tmps=Fs->stk;
    rsp2=(&tmps2->stk_base)(U8 *)+tmps2->stk_size;
    old_stk=rsp=&argv[argc];
    while (argc-->0)
      *--rsp2=*--rsp;
    _stk=&Fs->stk;
    tmps->stk_ptr=rsp=GetRSP;
    asm {
      IMPORT	_FREE;	//We are in a function, not at glbl scope.
//The compiler treats these in isolation.

      PUSHFD
      POP	RDX	//flags
      CLI
      MOV	RBX,U64 &tmps2[RBP]
      MOV	RAX,&_stk[RBP]
      MOV	U64 [RAX],RBX	//Fs->stk=tmps2
      MOV	RSP,U64 &rsp2[RBP]
      PUSH	RDX
      POPFD

      CALL	U64 &fp_addr[RBP]
      MOV	U64 &res[RBP],RAX

      PUSHFD
      POP	RDX	//flags
      CLI
      MOV	RBX,U64 &tmps[RBP]
      MOV	RAX,&_stk[RBP]
      MOV	U64 [RAX],RBX	//Fs->stk=tmps
      MOV	RSP,U64 &rsp[RBP]
      PUSH	RDX
      POPFD

      PUSH	U64 &tmps2[RBP]
      CALL	_FREE

      MOV	RDX,U64 &old_stk[RBP]
      MOV	RBX,U64 8[RBP]
      MOV	RAX,U64 &res[RBP]
      MOV	RBP,U64 [RBP]
      MOV	RSP,RDX
      JMP	RBX	//return
    };
  }
  return 0; //dummy to get rid of warning
}
;
#exe {Option(OPTf_NO_REG_VAR,OFF);};

I64 TaskInit(CTask *task,I64 stk_size)
{//Returns Fs of task
  CTaskStk *tmps;

  QueInit(&task->code_heap->next_mem_blk);
  task->code_heap->last_mergable=NULL;
  if (task->code_heap!=task->data_heap) {
    QueInit(&task->data_heap->next_mem_blk);
    task->data_heap->last_mergable=NULL;
  }

  task->addr=task->next_task=task->last_task=
	task->next_input_filter_task=task->last_input_filter_task=
	task;

  task->task_num=sys_num_spawned_tasks++;

  task->rflags=RFLAGG_NORMAL;
  task->win_inhibit=WIG_TASK_DFT;

  task->next_child_task=task->last_child_task=
	(&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task);

  JobCtrlInit(&task->srv_ctrl);
  QueInit(&task->next_cc);
  QueInit(&task->next_except);
  QueInit(&task->next_ctrl);
  QueInit(&task->next_ode);

  task->fpu_mmx=MAllocAligned(sizeof(CFPU),0x10,task);
  MemCpy(task->fpu_mmx,
	SYS_FIXED_AREA+offset(CSysFixedArea.init_fpu_mmx),sizeof(CFPU));

  task->hash_table=HashTableNew(TASK_HASH_TABLE_SIZE,task);

  if (!stk_size)
    stk_size=MEM_DFT_STK;
  task->stk=tmps=TaskStkNew(stk_size,task);
  task->rsp=(&tmps->stk_base)(U8 *)+tmps->stk_size;

  task->text_attr	=WHITE<<4+BLUE;
  task->border_src	=BDS_CONST;
  task->border_attr	=DrvTextAttrGet(':');
  task->title_src	=TTS_CONST;
  task->win_left	=1;
  task->win_right	=text.cols-2;
  task->win_top		=13;
  task->win_bottom	=text.rows-2;

  if (blkdev.home_dir) {//Beware Adam $LK,"TaskInit",A="FF:::/Kernel/KStart64.HC,TaskInit"$. I guess ok until $LK,"DskChg",A="FF:::/Kernel/KMain.HC,DskChg"$().
    task->cur_dv=blkdev.let_to_drv[*blkdev.home_dir-'A'];
    task->cur_dir=StrNew(blkdev.home_dir+2,task);
  } else
    task->cur_dir=StrNew("/Home",task);

  Seed(,task);

  return task;
}

CTask *Spawn(U0 (*fp_start_addr)(U8 *data),U8 *data=NULL,U8 *task_name=NULL,
	I64 target_cpu=-1,	//-1 for current CPU. See $LK,"multi-core",A="FI:::/Demo/MultiCore/LoadTest.HC"$.
	CTask *parent=NULL,	//NULL means adam
	I64 stk_size=0,		//0=default
	I64 flags=1<<JOBf_ADD_TO_QUE)
{//Create task on core running at address.
//Alloc $LK,"CTask",A="MN:CTask"$ structure from code heap so addr will be short.
  //Could be alloced off of data heap.
  CTask *task;
  if (target_cpu>=0)
    return SpawnQue(fp_start_addr,data,task_name,target_cpu,
	  parent,stk_size,flags);
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
  MemCpy(task->task_name,task_name,TASK_NAME_LEN);
  StrCpy(task->task_title,task->task_name);
  task->title_src=TTS_TASK_NAME;
  PUSHFD
  CLI
  if (Bt(&flags,JOBf_ADD_TO_QUE)) {
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
  while (LBts(&task->task_flags,TASKf_TASK_LOCK))
    PAUSE
  WinDerivedValsUpdate(task);
  if (fp_update_ctrls)
    (*fp_update_ctrls)(task);
  if (update_z_buf && Bt(&task->display_flags,DISPLAYf_SHOW))
    LBts(&sys_semas[SEMA_UPDATE_WIN_Z_BUF],0);
  LBtr(&task->task_flags,TASKf_TASK_LOCK);
  POPFD
}

I64 ExeCmdLine(CCmpCtrl *cc)
{//Terminal JIT-compile-and-execute loop for CCmpCtrl.
  I64 res=0,type,old_title_src=Fs->title_src;
  U8 *ptr,*ptr2,*ptr3,*machine_code,*old_task_title=StrNew(Fs->task_title);
  F64 t0;
  CDocEntry *doc_e;
  CDoc *doc;
  if (Fs->title_src!=TTS_LOCKED_CONST)
    Fs->title_src=TTS_CUR_LEX;
  while (cc->token &&
	(cc->token!='}' || !(cc->flags & CCF_EXE_BLK)) ) {
    if (Fs->title_src==TTS_CUR_LEX) {
      ptr2=&Fs->task_title;
      ptr3=ptr2+STR_LEN-1;
      if (cc->lex_include_stk->flags & LFSF_DOC) {
	doc_e=cc->lex_include_stk->cur_entry;
	doc=cc->lex_include_stk->doc;
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
	if ((ptr=cc->lex_include_stk->line_start) && *ptr)
	  MemCpy(ptr2,ptr,STR_LEN-1);
    }
    cc->flags&=~CCF_HAS_MISC_DATA;
    machine_code=LexStmt2Bin(cc,&type);
    if (machine_code!=INVALID_PTR) {
      if (!(cc->flags&CCF_JUST_LOAD)) {
	t0=tS;
	res=Call(machine_code);
	Fs->answer=res;
	Fs->answer_type=type;
	Fs->answer_time=tS-t0;
	Fs->new_answer=TRUE;
	cc->pmt_line=0;
      }
      if (!(cc->flags&CCF_HAS_MISC_DATA))
	Free(machine_code);
    }
  }
  if (Fs->title_src!=TTS_LOCKED_CONST) {
    Fs->title_src=old_title_src;
    StrCpy(Fs->task_title,old_task_title);
  }
  Free(old_task_title);
  if (cc->flags&CCF_JUST_LOAD) {
    if (cc->error_cnt)
      return FALSE;
    else
      return TRUE;
  } else
    return res;
}

U0 SrvTaskCont()
{//Act as server task in a loop handling commands.
  I64 old_flags=GetRFlags;
  FlushMsgs;
  while (TRUE) {
    CLI
    if (JobsHndlr(old_flags) && Fs->title_src==TTS_TASK_NAME)
      MemCpy(Fs->task_title,Fs->task_name,TASK_NAME_LEN);
    FlushMsgs;
    LBts(&Fs->task_flags,TASKf_IDLE);
    LBts(&Fs->task_flags,TASKf_AWAITING_MSG);
    Yield;
    SetRFlags(old_flags);
  }
}

U0 UserTaskCont()
{//Terminal key-input-execute loop.
  CCmpCtrl *cc;
  CDoc *doc;
  Bool cont=TRUE;
  do {
    cc=CmpCtrlNew(,CCF_CMD_LINE|CCF_PMT|CCF_QUESTION_HELP);
    QueIns(cc,Fs->last_cc);
    try {
      Lex(cc);
      ExeCmdLine(cc);
      cont=Bt(&cc->flags,CCf_PMT);
      QueRem(cc);
      CmpCtrlDel(cc);
    } catch {
      if ((doc=Fs->put_doc) && doc->doc_signature==DOC_SIGNATURE_VAL)
	DocUnlock(doc);
      PutExcept;
    }
  } while (cont);
}

U0 SrvCmdLine(I64 dummy=0)
{
  no_warn dummy;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  CallExtStr("SrvStartUp");
  SrvTaskCont;
}

U0 UserCmdLine(I64 dummy=0)
{//A user task ends-up calling this.
  no_warn dummy;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  CallExtStr("UserStartUp");
  if (!LBts(&Fs->display_flags,DISPLAYf_SHOW))
    Dbg;
  UserTaskCont;
}

CTask *User(U8 *fmt=NULL,...)
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

U0 TaskDel(CTask *task)
{//We delay freeing in case lingering ptr to reincarnated.
  HeapCtrlDel(task->code_heap);
  if (task->data_heap!=task->code_heap)
    HeapCtrlDel(task->data_heap);
  Free(task);
}

I64 TaskEnd()
{//Called with irq's off.
  CTask *task=Fs,*tmpt,*tmpt1;
  if (task==sys_task_being_scrn_updated) {
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

  DrvsRelease;
  BlkDevsRelease;
  tmpt1=(&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task);
  tmpt=tmpt1->next_sibling_task;
  if (tmpt!=tmpt1) {
    do {
      LBts(&tmpt->task_flags,TASKf_KILL_TASK);
      tmpt=tmpt->next_sibling_task;
    } while (tmpt!=tmpt1);
    return task->next_task;
  }
  if (LBtr(&task->display_flags,DISPLAYf_SHOW))
    LBts(&sys_semas[SEMA_UPDATE_WIN_Z_BUF],0);

  while (LBts(&task->task_flags,TASKf_TASK_LOCK))
    PAUSE
  while (LBts(&task->srv_ctrl.flags,JOBCf_LOCKED))
    PAUSE

  JobQueDel(&task->srv_ctrl.next_waiting);
  JobQueDel(&task->srv_ctrl.next_done);

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

  tmpt =task->next_input_filter_task;
  tmpt1=task->last_input_filter_task;
  tmpt1->next_input_filter_task=tmpt;
  tmpt ->last_input_filter_task=tmpt1;

  tmpt =task->next_sibling_task;
  tmpt1=task->last_sibling_task;
  tmpt1->next_sibling_task=tmpt;
  tmpt ->last_sibling_task=tmpt1;

  tmpt =task->next_task; //save to return
  TaskQueRem(task);

  LBtr(&task->srv_ctrl.flags,JOBCf_LOCKED);
  LBtr(&task->task_flags,TASKf_TASK_LOCK);

  task->wake_jiffy=cnts.jiffies+DYING_JIFFIES;
  while (LBts(&Gs->cpu_flags,CPUf_DYING_TASK_QUE))
    PAUSE
  QueIns(task,Gs->last_dying);
  LBtr(&Gs->cpu_flags,CPUf_DYING_TASK_QUE);

  return tmpt;
}

U0 TaskKillDying()
{//Delay freeing to prevent asking for trouble with quick reincarnations.
//What if the user is doing this: $LK,"DoTreeCheckers",A="FF:::/Misc/OSTestSuite.HC,DoTreeCheckers"$.
  CTaskDying *task,*task1;
  if (Gs->kill_jiffy<cnts.jiffies) {//Avoid doing as many lock operations.
    while (LBts(&Gs->cpu_flags,CPUf_DYING_TASK_QUE))
      PAUSE
    task=Gs->next_dying;
    while (task!=&Gs->next_dying && task->wake_jiffy<cnts.jiffies) {
      task1=task->next;
      QueRem(task);
      TaskDel(task);
      task=task1;
    }
    LBtr(&Gs->cpu_flags,CPUf_DYING_TASK_QUE);
    Gs->kill_jiffy=cnts.jiffies+DYING_JIFFIES;
  }
}
