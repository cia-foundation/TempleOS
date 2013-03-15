I64 ExceptDepth(CTask *task=NULL)
{
  CExcept *tempt;
  I64 result=0;
  if (!task) task=Fs;
  tempt=task->next_except;
  while (tempt!=&task->next_except) {
    result++;
    tempt=tempt->next;
  }
  return result;
}

asm {
USE64
#assert !((CREG_NON_PTR_MASK|CREG_VARS_MASK)&~0xFCC0)
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
	ENTER	0
	MOV	RAX,U64 SF_ARG1[RBP]
	MOV	U64 CExcept.rsi[RAX],RSI
	MOV	U64 CExcept.rdi[RAX],RDI
	MOV	U64 CExcept.r10[RAX],R10
	MOV	U64 CExcept.r11[RAX],R11
	MOV	U64 CExcept.r12[RAX],R12
	MOV	U64 CExcept.r13[RAX],R13
	MOV	U64 CExcept.r14[RAX],R14
	MOV	U64 CExcept.r15[RAX],R15
	LEAVE
	RET1	8
}

_extern _TEST_EXCEPT U0 TestExcept();
_extern _TAKE_EXCEPT U0 TakeExcept();
_extern _SAVE_EXCEPT_REGS U0 SaveExceptRegs(CExcept *t);

#exe {OptOn(OPTf_NO_REG_VAR);};

class CTryStk
{
  I64 rbp;
  I64 ret_rip;
  I64 arg1;
  I64 arg2;
};

U0 SysTry(U8 *catch_start,U8 *untry_start)
{
  I64 *rbp=GetBasePtr,old_flags=GetFlags;
  CExcept *tempt=MAlloc(sizeof(CExcept));
  tempt->handler_catch=catch_start;
  tempt->handler_untry=untry_start;
  tempt->rsp=rbp(U8 *)+sizeof(CTryStk);
  tempt->rbp=*rbp;
  tempt->rflags=old_flags;
  SaveExceptRegs(tempt);
  Cli;
  QueIns(tempt,Fs->last_except);
  SetFlags(old_flags);
}

U0 SysUntry()
{
  CExcept *tempt=Fs->last_except;
  PushFD;
//The adam task sometimes probes except depth,
//so we lock it out.  A probe from a multicore
//would fail, but who'd want to do that?
  Cli;
  QueRem(tempt);
  PopFD;
  Free(tempt);
}

U0 AdamLogExcept(I64 argc,I64 *argv)
{
  I64 i;
  if (!Gs->num) { //TODO: Other cores
    AdamLog("Depth:%02X Task:%08X throw(",
      ExceptDepth(Fs),Fs);
    for (i=0;i<argc;i++) {
      AdamLog("%X",argv[i]);
      if (i<argc-1)
	AdamLog(",");
    }
    AdamLog(");\n");
  }
}

U0 PutExcept()
{
  I64 i;
  if (Fs->except_argc>0) {
    if (Fs->except_argv[0]<DefineSubCnt("ST_EXCEPT_TYPES")) {
      "Exception Type :%Z\n",Fs->except_argv[0],"ST_EXCEPT_TYPES";
      if (Fs->except_argv[0]==EXCEPT_UNDEF_DEFINE)
	"Name:'%s'\n",Fs->except_argv[1];
      else if (Fs->except_argv[0]==EXCEPT_DIVIDE_BY_ZERO)
	"Addr:%P\n",Fs->except_argv[1];
      else {
	for (i=1;i<Fs->except_argc;i++)
	  "%016X ",Fs->except_argv[i];
	NewLine;
      }
    } else {
      "Exception:";
      for (i=0;i<Fs->except_argc;i++)
	"%016X ",Fs->except_argv[i];
      NewLine;
    }
  } else
    "Generic Exception\n";
}

U0 throw_join(I64 argc,I64 *argv)
{
  CExcept *tempt=Fs->last_except;
  Bool was_raw;

  Fs->except_argc=argc;
  Fs->except_argv=argv;
  Fs->except_rbp=GetBasePtr;
  Fs->catch_except=FALSE;
  while (Fs->next_except!=&Fs->next_except) {
    TestExcept;
    if (Fs->catch_except)
      TakeExcept;
    SetBasePtr(Fs->except_rbp);
    tempt=Fs->last_except;
    QueRem(tempt);
    Free(tempt);
  }
  was_raw=Raw(ON);
  PutExcept;
  Dbg("Unhandled Exception");
  Raw(was_raw);
}

U0 throw_no_log(...)
{
  throw_join(argc,argv);
}

U0 throw(...)
{
  AdamLogExcept(argc,argv);
  throw_join(argc,argv);
}

#exe {OptOff(OPTf_NO_REG_VAR);};

U0 SysThrowBreak()
{
  if (Bt(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC))
    Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0x20100000201);
  else { //TODO: Might want lock
    Fs->wake_jiffy=0;
    TaskRstAwaitingMsg;
    PrtsRelease();
    BlkDevsRelease();
    FlushMsgs;
    throw(EXCEPT_BREAK);
  }
}

//This can be used to exit an app, but not a task.
U0 ThrowBreak(CTask *task=NULL)
{
  if (task && task!=Fs) {
    if (!TaskValidate(task)) return;
    //TODO: Might want lock
    sys_focus_task=task;
    LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_C);
  } else
    SysThrowBreak;
}

Bool BreakLock(CTask *task=NULL)
{
  if (!task) task=Fs;
  return !LBts(&task->task_flags,TASKf_BREAK_LOCKED);
}

Bool BreakUnlock(CTask *task=NULL)
{
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

U0 CatchAll()
{
  PutExcept;
  Fs->catch_except=TRUE;
}
