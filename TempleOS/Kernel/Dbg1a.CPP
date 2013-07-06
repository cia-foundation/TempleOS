Bool IsDbgTask()
{
  CTask *task=Fs->parent_task;
  if (TaskValidate(task) && task->dbg_task==Fs)
    return TRUE;
  else
    return FALSE;
}

Bool ChkPtr(U8 *ptr)
{
  if (sys_heap_base<=ptr<=MAPPED_MEM_SPACE) {
    if (*MemPageTable(ptr)&1)
      return TRUE;
    else
      return FALSE;
  } else if (ptr<sys_boot_base)
    return FALSE;
  else if (ptr<VGA_GRAPHIC_MEM_BASE)
    return TRUE;
  else
    return FALSE;
}

Bool ChkCodePtr(U8 *ptr)
{
  if (sys_heap_base<=ptr<=sys_heap_limit) {
    if (*MemPageTable(ptr)&1)
      return TRUE;
    else
      return FALSE;
  } else if (ptr<sys_boot_base)
    return FALSE;
  else if (ptr<VGA_GRAPHIC_MEM_BASE)
    return TRUE;
  else
    return FALSE;
}

Bool ChkOnStk(U8 *ptr,CTask *task=NULL)
{
  Bool result=FALSE;
  PushFD;
  Cli;
  if (task) {
    if (&task->stk->stk_base<=ptr<=(&task->stk->stk_base)(U8 *)+task->stk->stk_size)
      result=TRUE;
  } else if (sys_heap_base<=ptr<=sys_heap_limit)
    result=TRUE;
  PopFD;
  return result;
}

I64 UnusedStk(CTask *task=NULL)
{
  I64 result;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (task==Fs)
    result=GetStkPtr()(U8 *)-(&task->stk->stk_base)(U8 *);
  else
    result=task->rsp(U8 *)-(&task->stk->stk_base)(U8 *);
  PopFD;
  return result;
}

U8 *Caller(I64 num=1)
//Returns the addr of the fun
//which called this one, or the caller
//of the caller, etc.
{
  U8 **rbp=GetBasePtr,**ptr;
  while (num--) {
    if (rbp>=*rbp)
      return NULL;
    rbp=*rbp;
    if (!ChkOnStk(rbp,Fs))
      return NULL;
  }
  ptr=rbp+1;
  return *ptr;
}

U8 *TaskCaller(CTask *task=NULL,I64 num=0,Bool saved_context=FALSE)
{
  U8 **ptr,**rbp,**rsp;
  if (!task) task=Fs;
  if (!saved_context && task==Fs)
    return Caller(num+1);
  if (!TaskValidate(task))
    return NULL;
  rbp=task->rbp;
  rsp=task->rsp;
  if (num) {
    while (ChkOnStk(rbp,task)) {
      ptr=rbp+1;
      if (! --num)
	return *ptr;
      if (rbp>=*rbp)
	break;
      rbp=*rbp;
    }
    return NULL;
  } else {
    if (task->rip==SYS_RET)
      return *rsp;
    else
      return task->rip;
  }
}
#define STK_REP_LEN	32

U0 StkRep(CTask *task=NULL)
//Reports whats on the stk
{
  I64 i,j,add,
      **rbp,**rsp,*my_rsp[STK_REP_LEN];
  CHashTable *old_hash=Fs->hash_table;
  if (!task) task=Fs;
  if (!TaskValidate(task))
    return;
  PushFD;
  Cli;
  if (task==Fs) {
    rbp=GetBasePtr;
    rsp=rbp+3;
    rbp=*rbp;
  } else {
    rsp=task->rsp;
    rbp=task->rbp;
  }
  if (task->rip==SYS_RET)
    add=*rsp;
  else
    add=task->rip;
  MemCpy(my_rsp,rsp,STK_REP_LEN*sizeof(U8 *));
  PopFD;
  Fs->hash_table=task->hash_table;
  for (i=0;i<STK_REP_LEN;i++) {
    "%08X [RSP+%04X]: %016X ",rsp+i,
      i*sizeof(U8 *),my_rsp[i];
    while (TRUE) {
      if (!(&task->stk->stk_base<=rbp<(&task->stk->stk_base)(U8 *)+task->stk->stk_size))
	break;
      j=rbp-rsp;
      if (j>=i)
	break;
      add=my_rsp[j+1];
      if (rbp>=my_rsp[j])
	break;
      rbp=my_rsp[j];
    }
    if (my_rsp[i]==add)
      "$$FG,RED$$";
    "%P$$FG$$\n",my_rsp[i];
  }
  NewLine;
  Fs->hash_table=old_hash;
}

U0 CallerRep(U8 **rbp=NULL,CTask *task=NULL)
//Prints a report of calling routines
{
  I64 **ptr;
  if (!task) task=Fs;
  if (!rbp) {
    if (task==Fs)
      rbp=GetBasePtr;
    else
      rbp=task->rbp;
  }
  "CallerRep:\n";
  while (ChkOnStk(rbp,task)) {
    ptr=rbp+1;
    "%08X:%08tX:%P\n",ptr,*ptr,*ptr;
    if (rbp>=*rbp)
      break;
    rbp=*rbp;
  }
}

U0 DbgStr(U8 *st,I64 mS=100)
//Shows a string using
//$LK,"Raw","MN:Raw"$ screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgStr(";
  "" st;
  ')';
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgI64(I64 num,I64 mS=100)
//Shows a hex num using
//$LK,"Raw","MN:Raw"$ screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgI64(%016X)",num;
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgF64(F64 d,I64 mS=100)
//Shows a floating-point val using
//$LK,"Raw","MN:Raw"$ screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgF64(%15e)",d;
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgAdd(U8 *add,I64 mS=100)
//Shows an addr using $LK,"Raw","MN:Raw"$ screen
//output for a length of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgAdd(%P)",add;
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgD(U8 *add,I64 cnt=0x80,I64 mS=100)
//Dumps a block of mem using $LK,"Raw","MN:Raw"$
//screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  D(add,cnt);
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgDm(U8 *add,I64 cnt=0x80,I64 mS=100)
//Dumps a block of mem using $LK,"Raw","MN:Raw"$
//screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  Dm(add,cnt);
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 D(U8 *add,I64 cnt=0x80,Bool show_offset=TRUE)
//Dump mem, showing offsets.
//See $LK,"DocD","MN:DocD"$() for a live dump.
{
  I64 i,j,ch;
  U8 *ptr=add;
  while (cnt) {
    if (show_offset)
      "%08X ",ptr-add;
    else
      "%010X ",ptr;
    if (cnt>16)
      j=16;
    else
      j=cnt;
    for (i=0;i<j;i++)
      "%02X ",ptr[i];
    for (;i<16;i++)
      "   ";
    for (i=0;i<j;i++) {
      ch=ptr[i];
      if (ch<CH_SHIFT_SPACE || ch==CH_BACKSPACE)
	ch='.';
      '' ch;
      if (ch=='$$')
	'' ch;
    }
    NewLine;
    cnt-=j;
    ptr+=j;
  }
}

U0 Dm(U8 *add,I64 cnt=0x80)
//Show mem addr, not offsets
{
  D(add,cnt,FALSE);
}

U0 Da(U8 **add,I64 cnt=0x10)
//Dump mem, showing symbolic addresses
{
  while (cnt-->0) {
    "%08X:%08X,%P\n",add,*add,*add;
    add++;
  }
}

I64 *TaskRegAdd(CTask *task,I64 reg_num)
{
  switch (reg_num) {
    case CREG_RAX: return &task->rax;
    case CREG_RCX: return &task->rcx;
    case CREG_RDX: return &task->rdx;
    case CREG_RBX: return &task->rbx;
    case CREG_RSP: return &task->rsp;
    case CREG_RBP: return &task->rbp;
    case CREG_RSI: return &task->rsi;
    case CREG_RDI: return &task->rdi;
    case 8 : return &task->r8;
    case 9 : return &task->r9;
    case 10: return &task->r10;
    case 11: return &task->r11;
    case 12: return &task->r12;
    case 13: return &task->r13;
    case 14: return &task->r14;
    case 15: return &task->r15;
  }
  return NULL;
}

#define RAWDR_COL	40

U0 RawDr(CTask *task=NULL)
{
  I64 i,j,old_col=sys_raw_dc_col;
  Bool old_raw=Raw(ON);
  U8 buf[200];

  if (!task) task=Fs;

  for (i=0;i<16;i++) {
    sys_raw_dc_col=i*text_cols+RAWDR_COL;
    "º%3Z:%016X\n",i,"ST_U64_REGS",*TaskRegAdd(task,i);
  }

  sys_raw_dc_col=i++*text_cols+RAWDR_COL;
  "ºRIP:%016X\n",task->rip;

  sys_raw_dc_col=i++*text_cols+RAWDR_COL;
  "º%-*tT\n",text_cols-(RAWDR_COL+1)-1,Fs->rip;

  sys_raw_dc_col=i++*text_cols+RAWDR_COL;
  'º';
  if (Bt(&sys_run_level,RLf_COMPILER)) {
    j=Fs->rip;
    Ui(buf,&j,,,TRUE);
    "" buf;
  } else
    NewLine;

  sys_raw_dc_col=i*text_cols+RAWDR_COL;
  'È';
  for (j=0;j<text_cols-RAWDR_COL-1;j++)
    'Í';

  sys_raw_dc_col=old_col;
  Raw(old_raw);
}

U0 Dr(CTask *task=NULL)
{//Dump registers
  I64 i;
  if (!task) task=Fs;
  for (i=0;i<16;i++)
    "%3Z:%016X\n",i,"ST_U64_REGS",*TaskRegAdd(task,i);
  "RIP:%016X\n",task->rip;
}

U0 OverRideSingleUserSwap()
{
  Bool old_single=SingleUser(OFF);
  Yield;
  SingleUser(old_single);
}

U8 *SysMGetS(I64)
{
  U8 buf[512];
  GetS(buf,512,FALSE);
  return StrNew(buf);
}

CBpt *BptFind(U8 *add,CTask *task=NULL,Bool rem=FALSE)
{
  CBpt *result=NULL,*tempb,*tempb1,*tempb2;
  if (!task) task=Fs;
  PushFD;
  Cli;
  tempb1=&task->bpt_lst;
  tempb=task->bpt_lst;
  while (tempb) {
    tempb2=tempb->next;
    if (tempb->addr==add) {
      result=tempb;
      if (rem)
	tempb1->next=tempb2;
      else
	tempb1=&tempb->next;
    } else
      tempb1=&tempb->next;
    tempb=tempb2;
  }
  PopFD;
  return result;
}

Bool BptS(U8 *add,CTask *task=NULL,Bool live=TRUE)
{//Set breakpoint
  CBpt *tempb;
  Bool result=TRUE;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (!(tempb=BptFind(add,task,FALSE))) {
    tempb=CAlloc(sizeof(CBpt),task);
    tempb->addr=add;
    tempb->val=*add;
    result=FALSE;
    tempb->next=task->bpt_lst;
    task->bpt_lst=tempb;
    if (task==Fs && live)
      *add=OC_BPT;
  }
  PopFD;
  return result;
}

Bool BptR(U8 *add,CTask *task=NULL,Bool live=TRUE,Bool rem=TRUE)
{//Rem breakpoint
  CBpt *tempb;
  Bool result=FALSE;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (tempb=BptFind(add,task,rem)) {
    if (task==Fs && live)
      *tempb->addr=tempb->val;
    result=TRUE;
    if (rem)
      Free(tempb);
  }
  PopFD;
  return result;
}

Bool B(U8 *add,CTask *task=NULL,Bool live=TRUE)
{//Toggle breakpoint
 //Returns TRUE if removed
  Bool result=FALSE;
  PushFD;
  Cli;
  if (BptFind(add,task,FALSE)) {
    BptR(add,task,live,TRUE);
    result=TRUE;
  } else
    BptS(add,task,live);
  PopFD;
  return result;
}

I64 B2(CTask *task=NULL,Bool live=TRUE)
{ //Rem all breakpoints
  //Returns cnt of removed
  I64 result=0;
  CBpt *tempb,*tempb1;
  if (!task) task=Fs;
  PushFD;
  Cli;
  tempb=task->bpt_lst;
  task->bpt_lst=NULL;
  while (tempb) {
    tempb1=tempb->next;
    if (task==Fs && live)
      *tempb->addr=tempb->val;
    Free(tempb);
    tempb=tempb1;
    result++;
  }
  PopFD;
  return result;
}

U0 G(U8 *ip=INVALID_PTR,CTask *task=NULL)
//Go
{
  if (!task) task=Fs;
  if (ip!=INVALID_PTR) task->rip=ip;
  if (BptFind(task->rip,task))
    "\nDo one of the following, first:\n"
    ">S;\t\t\t//Single step\n"
    ">B2;\t\t\t//Clear all break points\n"
    ">G2;\t\t\t//Clear all break points and Go\n\n"
    "After resuming, <CTRL-ALT-T> changes focus task\n"
    "After resuming, <CTRL-ALT-F> flushes screen VGA cache\n";
  else {
    LBtr(&task->task_flags,TASKf_DISABLE_BPTS);
    LBtr(&task->rflags,RFLAGf_TRAP);//No single step
    Suspend(task,FALSE);
    if (task==Fs) {
      if (IsInDbg && task->next_lex!=&task->next_lex) {
	"Exit Dbg\n";
	Btr(&task->last_lex->flags,Lf_PMT);
      }
    } else
      Exit;
  }
}

U0 G2(U8 *ip=INVALID_PTR,CTask *task=NULL)
//Rem all breakpoints and Go.
{
  if (!task) task=Fs;
  B2(task);
  if (ext[EXT_WIN_FOCUS])
    CallExtNum(EXT_WIN_FOCUS,sys_dbg_focus_task);
  VGAFlush;
  G(ip,task);
}

public U0 S(U8 *ip=INVALID_PTR,CTask *task=NULL) //Single-step.
{ //Single step
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (ip!=INVALID_PTR) task->rip=ip;
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  LBts(&task->rflags,RFLAGf_TRAP);
  Suspend(task,FALSE);
  if (task==Fs) {
    if (IsInDbg) {
      if (task->next_lex!=&task->next_lex)
	Btr(&task->last_lex->flags,Lf_PMT);
    }
  } else
    Exit;
  PopFD;
}

U0 DbgHelp()
{
  "\n"
  "The cmd line is basically the same as normal.  Here are some common\n"
  "debugging commands.\n\n"
  ">EdLite(\"FileName\");\t\t//Edit file.\n"
  ">D(0x100000);\t\t\t//Dump page tables.\n"
  ">Dm(0x100000);\t\t\t//Dump page tables.\n"
  ">Dm(Fs,sizeof(CTask));\t\t//Dump current task record.\n"
  ">ClassRep(\"CTask\",Fs,1);\t//Dump current task record.\n"
  ">CallerRep;\t\t\t//Stack trace report.\n"
  ">Da(_RSP);\t\t\t//Dump stk.\n"
  ">Dr;\t\t\t\t//Dump Registers.\n"
  ">1+2*3+&PutS;\t\t\t//Show calculation result.\n"
  ">*(0x70000)(I64 *)=0x123456789;\t//Assign value to 0x70000-0x70007.\n"
  ">_RAX=0x1234;\t\t\t//Set RAX to 0x1234.\n"
  ">_RIP=&SysThrowBreak;\t\t//Set RIP.\n"
  ">I64 i;\t\t\t\t//Declare variable.\n"
  ">i=_RCX+_RDX;\t\t\t//Assign to variable.\n"
  ">U(&PrintF+0x8);\t\t//Unassemble PrintF.\n"
  ">Uf(\"PutS\");\t\t\t//Unassembler function \"PutS\".\n"
  ">Man(\"PutS\");\t\t\t//Edit Src for \"PutS\".\n"
  ">E(_RIP);\t\t\t//Edit Src Code.\n"
  ">Fix;\t\t\t\t//Edit Last Err Src Code.\n"
  ">B(&Main+0x20);\t\t\t//Toggle break point.\n"
  ">B2;\t\t\t\t//Clear all break points.\n"
  ">S;\t\t\t\t//Single step.\n"
  ">G;\t\t\t\t//Resume execution.\n"
  ">G2;\t\t\t\t//B2;VGAFlush;WinFocus;G;\n"
  ">Exit;\t\t\t\t//Exit (kill) task.\n\n"
  "After resuming, <CTRL-ALT-T> changes focus task.\n"
  "After resuming, <CTRL-ALT-F> flushes screen VGA cache.\n\n";
}

U0 Dbg2()
{
  Bool old_user_task,old_waiting_msg;
  I64 i,old_dbg_mgets;
  U8 buf[200];
  if (IsRaw) {
    i=Fs->rip;
    Ui(buf,&i);
    "" buf;
  } else
    U(Fs->rip,1);

  old_waiting_msg=LBtr(&Fs->task_flags,TASKf_AWAITING_MSG);
  old_user_task  =LBts(&Fs->task_flags,TASKf_USER);
  TaskInitExt(TRUE);
  sys_focus_task=Fs;
  sys_cur_scan_code=0;
  old_dbg_mgets=fp_mgets;
  fp_mgets=&SysMGetS;
  while (!mouse_install_attempts)
    OverRideSingleUserSwap;
  UserTaskCont;
  fp_mgets=old_dbg_mgets;
  LBEqu(&Fs->task_flags,TASKf_USER,old_user_task);
  LBEqu(&Fs->task_flags,TASKf_AWAITING_MSG,old_waiting_msg);
}

U0 Fault3(I64 fault_num,I64 fault_err_code)
{
  nounusedwarn fault_err_code;
  Bool was_in_dbg;
  PushFD;
  Cli;
  was_in_dbg=InDbg(ON);
  if (Gs->num && mp_crash) {
    mp_cnt=1;
    mp_crash->cpu_num=Gs->num;
    mp_crash->task=Fs;
    mp_crash->msg=sys_dbg_msg;
    mp_crash->msg_num=sys_dbg_msg_num;
    MPInt(I_MP_CRASH,0);
    SysHlt;
  }
  "\n\tTempleOS Debugger\n\n"
  ">Help;\t//For help.\n\n";
  Beep(500,TRUE);
  if (fault_num==I_DBG) {
    if (sys_dbg_msg) {
      "\n!!! %s",sys_dbg_msg;
      if (sys_dbg_msg_num)
	"%016X",sys_dbg_msg_num;
      " !!!\n\n";
    }
  }
  CallerRep;
  Dbg2;
  InDbg(was_in_dbg);
  PopFD;
}

U0 Fault2(I64 fault_num,I64 fault_err_code) //Called from $LK,"Fault2","FF:::/TempleOS/Kernel/Interrupts.CPP,Fault2"$
{
//$BK,1$Be careful not to swap-out and ruin the saved context$BK,0$
  Bool was_raw,was_single_user,was_silent;
  I64  i,old_dc_flags=sys_raw_dc_flags;
  if (fault_num==I_DIVIDE_BY_ZERO)
    throw(EXCEPT_DIVIDE_BY_ZERO,Fs->rip);
  was_single_user=SingleUser(ON);
  if (!IsInDbg)
    sys_dbg_focus_task=sys_focus_task;
  sys_focus_task=NULL;
  if (fault_num==I_BPT)
    Fs->rip--;
  if (Fs->dbg_task)
    CallExtNum(EXT_RESUME_DBG);  //See $LK,"IDEResumeDbg","FF:::/TempleOS/Adam/Doc/DocIDE.CPP,IDEResumeDbg"$
  else {
    was_raw=Raw(ON);
    was_silent=Silent(OFF);
    sys_raw_dc_flags|=RDCF_SHOW_DOLLAR|RDCF_SCROLL;
    "Task \"";
    "" Fs->task_title;
    "\"\n";
    "Fault:0x%02X %Z\t\tErr Code:%08X\n",fault_num,fault_num,"ST_INT_NAMES",fault_err_code;
    "RIP:%08X",Fs->rip; //Sometimes crashes on %P, so do this first
    ":%P  RSP:%08X\n",Fs->rip,Fs->rsp;
    if (fault_num==I_PAGE_FAULT) {
      asm {	MOV_RAX_CR2	 }
      i=GetRAX;
      "Fault Addr:%08X:%P\n",i,i;
    }
    Fault3(fault_num,fault_err_code);
    Silent(was_silent);
    Raw(was_raw);
    sys_raw_dc_flags=old_dc_flags;
  }
  SingleUser(was_single_user);
  if (LBtr(&Fs->task_flags,TASKf_KILL_AFTER_DBG))
    Exit;
}

U0 Dbg(U8 *msg=NULL,I64 msg_num=0)
//Enters the debugger.	Can be used as a panic.
{
  PushFD;
  Cli;
  sys_dbg_msg=msg;
  sys_dbg_msg_num=msg_num;
  asm {INT 0x32};
  #assert I_DBG==0x32
  PopFD;
}
