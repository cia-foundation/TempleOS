asm {
#assert !((REGG_C_LOCAL_NON_PTR_VARS|REGG_C_LOCAL_VARS)&~0xFCC0)
_TEST_EXCEPT::
	XOR	RAX,RAX
	MOV	RAX,FS:U64 CTask.last_except[RAX]
	MOV	RBP,U64 CExcept.rbp[RAX]
	MOV	RSI,U64 CExcept.rsi[RAX]
	MOV	RDI,U64 CExcept.rdi[RAX]
	MOV	R10,U64 CExcept.r10[RAX]
	MOV	R11,U64 CExcept.r11[RAX]
	MOV	R12,U64 CExcept.r12[RAX]
	MOV	R13,U64 CExcept.r13[RAX]
	MOV	R14,U64 CExcept.r14[RAX]
	MOV	R15,U64 CExcept.r15[RAX]
	PUSH	U64 CExcept.rflags[RAX]
	POPFD
	JMP	U64 CExcept.handler_catch[RAX]

_TAKE_EXCEPT::
	XOR	RAX,RAX
	MOV	RAX,FS:U64 CTask.last_except[RAX]
	MOV	RSP,U64 CExcept.rsp[RAX]
	JMP	U64 CExcept.handler_untry[RAX]

_SAVE_EXCEPT_REGS::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RAX,U64 SF_ARG1[RBP]
	MOV	U64 CExcept.rsi[RAX],RSI
	MOV	U64 CExcept.rdi[RAX],RDI
	MOV	U64 CExcept.r10[RAX],R10
	MOV	U64 CExcept.r11[RAX],R11
	MOV	U64 CExcept.r12[RAX],R12
	MOV	U64 CExcept.r13[RAX],R13
	MOV	U64 CExcept.r14[RAX],R14
	MOV	U64 CExcept.r15[RAX],R15
	POP	RBP
	RET1	8
}

_extern _TEST_EXCEPT U0 TestExcept();
_extern _TAKE_EXCEPT U0 TakeExcept();
_extern _SAVE_EXCEPT_REGS U0 SaveExceptRegs(CExcept *t);

U0 PutExcept(Bool catch_it=TRUE)
{//Print exception msg and catch exception.
  "Except:%c:",Fs->except_ch;
  "%P:%P\n",Fs->except_caller1,Fs->except_caller2;
  Fs->catch_except=catch_it;
}

#exe {Option(OPTf_NO_REG_VAR,ON);};

class CTryStk
{
  I64 rbp;
  I64 ret_rip;
  I64 arg1;
  I64 arg2;
};

U0 SysTry(U8 *catch_start,U8 *untry_start)
{
  I64 *rbp=GetRBP;
  CExcept *tempt=MAlloc(sizeof(CExcept));
  tempt->handler_catch=catch_start;
  tempt->handler_untry=untry_start;
  tempt->rsp=rbp(U8 *)+sizeof(CTryStk);
  tempt->rbp=*rbp;
  tempt->rflags=GetRFlags;
  SaveExceptRegs(tempt);
  QueIns(tempt,Fs->last_except);
}

U0 SysUntry()
{
  CExcept *tempt=Fs->last_except;
  QueRem(tempt);
  Free(tempt);
}

U0 throw(I64 ch=0,Bool no_log=FALSE)
{//ch can be up to 8 chars like $LK,"PutChars",A="MN:PutChars"$().
//In the catcher, fetch ch from Fs->except_ch.
  CExcept *tempt=Fs->last_except;
  Bool was_raw;
  Fs->except_ch=ch;
  Fs->except_caller1=Caller(1);
  Fs->except_caller2=Caller(2);
  Fs->except_rbp=GetRBP;
  Fs->catch_except=FALSE;
  if (!no_log)
    AdamLog("Except:%c:%p:%p\n",ch,Fs->except_caller1,Fs->except_caller2);
  while (Fs->next_except!=&Fs->next_except) {
    TestExcept;
    if (Fs->catch_except)
      TakeExcept;
    SetRBP(Fs->except_rbp);
    tempt=Fs->last_except;
    QueRem(tempt);
    Free(tempt);
  }
  was_raw=Raw(ON);
  PutExcept(FALSE);
  Dbg("Unhandled Exception");
  Raw(was_raw);
}

#exe {Option(OPTf_NO_REG_VAR,OFF);};

U0 SysThrowBreak()
{
  if (Bt(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC))
    Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0x20100000201);
  else {//TODO: Might want lock
    Fs->wake_jiffy=0;
    TaskRstAwaitingMsg;
    DrvsRelease();
    BlkDevsRelease();
    FlushMsgs;
    throw('Break');
  }
}

U0 ThrowBreak(CTask *task=NULL)
{//Can be used to exit an app, but not a task.
  if (task && task!=Fs) {
    if (!TaskValidate(task)) return;
//TODO: Might want lock
    sys_focus_task=task;
    LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_C);
  } else
    SysThrowBreak;
}

Bool BreakLock(CTask *task=NULL)
{//Disables <CTRL-ALT-c>.
  if (!task) task=Fs;
  return !LBts(&task->task_flags,TASKf_BREAK_LOCKED);
}

Bool BreakUnlock(CTask *task=NULL)
{//Reenables <CTRL-ALT-c> and issues any pending breaks.
  Bool result;
  if (!task) task=Fs;
  result=LBtr(&task->task_flags,TASKf_BREAK_LOCKED);
  if (LBtr(&task->task_flags,TASKf_PENDING_BREAK)) {
    if (task==Fs)
      ThrowBreak;
    else
      task->rip=&ThrowBreak;
  }
  return result;
}
