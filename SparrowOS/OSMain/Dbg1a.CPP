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
    if (*MemPtAtPageTableEntry(ptr)&1)
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
    if (task->rip==SYS_IDLE_PT)
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
  if (task->rip==SYS_IDLE_PT)
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
  if (!rbp) rbp=GetBasePtr;
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

U0 DbgNum(I64 num,I64 mS=100)
//Shows a hex num using
//$LK,"Raw","MN:Raw"$ screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgNum(%016X)",num;
  BusyWait(mS<<10);
  PopFD;
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 DbgF(F64 d,I64 mS=100)
//Shows a floating-point val using
//$LK,"Raw","MN:Raw"$ screen output for a fixed length
//of time.
{
  Bool old_raw,old_input_filter;
  PushFD;
  Cli;
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "DbgF(%15e)",d;
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

U0 D(U8 *buf,I64 cnt=0x80,Bool show_offset=TRUE)
//Dump mem, showing offsets.
//See $LK,"DocD","MN:DocD"$() for a live dump.
{
  I64 i,j,ch;
  U8 *b=buf;
  while (cnt) {
    if (show_offset)
      "%08X ",b-buf;
    else
      "%010X ",b;
    j= cnt>16 ? 16:cnt;
    for (i=0;i<j;i++)
      "%02X ",b[i];
    for (;i<16;i++)
      "   ";
    for (i=0;i<j;i++) {
      ch=b[i];
      if (ch<CH_SHIFT_SPACE || ch==CH_BACKSPACE)
	ch='.';
      '' ch;
      if (ch=='$$')
	'' ch;
    }
    NewLine;
    cnt-=j;
    b+=j;
  }
}

U0 Dm(U8 *buf,I64 cnt=0x80)
//Show mem addr, not offsets
{
  D(buf,cnt,FALSE);
}

U0 Da(U8 **b,I64 cnt=0x10)
//Dump mem, showing symbolic addresses
{
  while (cnt-->0) {
    "%08X:%08X,%P\n",b,*b,*b;
    b++;
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

U0 DefineRegs() //A courtesy for dbgging
{
  DefineLoad("RAX","Fs->rax");
  DefineLoad("RBX","Fs->rbx");
  DefineLoad("RCX","Fs->rcx");
  DefineLoad("RDX","Fs->rdx");
  DefineLoad("RSI","Fs->rsi");
  DefineLoad("RDI","Fs->rdi");
  DefineLoad("RBP","Fs->rbp");
  DefineLoad("RSP","Fs->rsp");
  DefineLoad("RIP","Fs->rip");
  DefineLoad("R8" ,"Fs->r8");
  DefineLoad("R9" ,"Fs->r9");
  DefineLoad("R10","Fs->r10");
  DefineLoad("R11","Fs->r11");
  DefineLoad("R12","Fs->r12");
  DefineLoad("R13","Fs->r13");
  DefineLoad("R14","Fs->r14");
  DefineLoad("R15","Fs->r15");
}

U8 *SysMGetS(I64)
{
  U8 buf[512];
  GetS(buf,512,FALSE);
  return StrNew(buf);
}

U0 SysDbgExit()
{
  if (IsInDbg) {
    "Exit Dbg\n";
    Btr(&Fs->last_lex->flags,Lf_PROMPT);
  }
}

U0 Go1()
{
  if (IsInDbg && ext[EXT_EXIT_DEBUGGER])
    CallExtNum(EXT_EXIT_DEBUGGER);
}

U0 SysDbgEnter(Bool was_in_debugger)
{
  Bool old_user_task,old_waiting_msg;
  I64 i,old_dbg_mgets;
  U8 buf[200];
  CHashTable *tempht;

  if (IsRaw) {
    i=Fs->rip;
    Ui(buf,&i,64);
    PutS(buf);
  } else
    U(Fs->rip,1);

  if (!was_in_debugger) {
    old_waiting_msg=LBtr(&Fs->task_flags,TASKf_AWAITING_MSG);
    old_user_task  =LBts(&Fs->task_flags,TASKf_USER);
    TaskInitExt;
    sys_focus_task=Fs;
    sys_cur_scan_code=0;
    old_dbg_mgets=fp_mgets;
    fp_mgets=&SysMGetS;
    while (!mouse_install_attempts)
      OverRideSingleUserSwap;

    tempht=HashTableNew(STD_HASH_SIZE);
    tempht->next=Fs->hash_table;
    Fs->hash_table=tempht;
    DefineRegs;
    UserTaskCont;
    Fs->hash_table=tempht->next;
    HashTableDel(tempht);

    fp_mgets=old_dbg_mgets;
    LBEqu(&Fs->task_flags,TASKf_USER,old_user_task);
    LBEqu(&Fs->task_flags,TASKf_AWAITING_MSG,old_waiting_msg);
  }
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
  ">Da(RSP);\t\t\t//Dump stk.\n"
  ">Dr;\t\t\t\t//Dump Registers.\n"
  ">1+2*3+&PutS;\t\t\t//Show calculation result.\n"
  ">*(0x70000)(I64 *)=0x123456789;\t//Assign value to 0x70000-0x70007.\n"
  ">RAX=0x1234;\t\t\t//Set RAX to 0x1234.\n"
  ">RIP=&SysThrowBreak;\t\t//Set RIP.\n"
  ">I64 i;\t\t\t\t//Declare variable.\n"
  ">i=RCX+RDX;\t\t\t//Assign to variable.\n"
  ">U(&PrintF+0x8);\t\t//Unassemble PrintF.\n"
  ">Uf(\"PutS\");\t\t\t//Unassembler function \"PutS\".\n"
  ">BptS(&Main+0x20);\t\t//Set break point.\n"
  ">BptR(&Main+0x20);\t\t//Reset (clear) break point.\n"
  ">BptsR;\t\t\t\t//Reset (clear) all break points.\n"
  ">Step;\t\t\t\t//Single step.\n"
  ">Go1;\t\t\t\t//Resume execution.\n"
  ">Go2;\t\t\t\t//Better resume execution.\n"
  ">Go3;\t\t\t\t//BptsR;VGAFlush;WinFocus;Go2;\n"
  ">Exit;\t\t\t\t//Exit (kill) task.\n\n"
  "After resuming, <CTRL-ALT-T> changes focus task.\n"
  "After resuming, <CTRL-ALT-F> flushes screen VGA cache.\n\n";
}

U0 Dbg(U8 *msg=NULL,I64 num=0,Bool fault=FALSE)
//Enters the debugger.
//This can be used as a kernel panic.
{
  Bool was_in_dbg,was_single_user,was_raw;
  I64 old_dc_flags=sys_raw_dc_flags;
  PushFD;
  Cli;
  if (Gs->num && mp_crash) {
    mp_cnt=1;
    mp_crash->cpu_num=Gs->num;
    mp_crash->task=Fs;
    mp_crash->msg=msg;
    mp_crash->msg_num=num;
    MPInt(I_MP_CRASH,0);
    SysHlt;
  }
 
  was_single_user=SingleUser(ON);
  was_raw=Raw(ON);
  was_in_dbg=InDbg(ON);
  sys_raw_dc_flags|=RDCF_SHOW_DOLLAR|RDCF_SCROLL;

  if (!fault && !was_in_dbg)
    sys_dbg_focus_task=sys_focus_task;

  "\n\tSparrowOS Debugger\n\n"
  ">Help;\t//For help.\n\n";

  Beep(500,TRUE);
  if (msg) {
    "\n!!! %s",msg;
    if (num)
      "%016X",num;
    " !!!\n\n";
  }
  CallerRep;
  if (ext[EXT_DEBUGGER])
    CallExtNum(EXT_DEBUGGER,was_in_dbg);
  else {
    "No Debugger\n";
    SysHlt;
  }
  InDbg(was_in_dbg);
  SingleUser(was_single_user);
  Raw(was_raw);
  sys_raw_dc_flags=old_dc_flags;
  PopFD;
  if (!fault && LBtr(&Fs->task_flags,TASKf_KILL_AFTER_DBG))
    Exit;
}

U0 Fault2() //Called from $LK,"Fault2","FF:::/SparrowOS/OSMain/Interrupts.CPP.Z,Fault2"$
{
//$BK,1$Be careful not to swap-out and ruin the saved context$BK,0$
  Bool was_raw,was_single_user;
  I64  i,old_fault_num=sys_dbg_fault_num,
       old_dc_flags=sys_raw_dc_flags;
  U8 *old_fault_rip=sys_dbg_fault_rip;
  if (Fs->fault_num==I_DIVIDE_BY_ZERO)
    throw(EXCEPT_DIVIDE_BY_ZERO,Fs->rip);
  was_single_user=SingleUser(ON);
  if (!IsInDbg)
    sys_dbg_focus_task=sys_focus_task;
  sys_focus_task=NULL;
  sys_dbg_fault_num=Fs->fault_num;
  if (Fs->fault_num==I_BPT)
    Fs->rip--;
  sys_dbg_fault_rip=Fs->rip;
  if (Fs->dbg_task)
    CallExtNum(EXT_RESUME_DBG);  //See $LK,"IDEResumeDbg","FF:::/SparrowOS/Adam/Doc/DocIDE.CPP.Z,IDEResumeDbg"$
  else {
    was_raw=Raw(ON);
    sys_raw_dc_flags|=RDCF_SHOW_DOLLAR|RDCF_SCROLL;
    "Task \"";
    "" Fs->task_title;
    "\"\n";
    "Fault:0x%02X %Z\t\tErr Code:%08X\n",Fs->fault_num,Fs->fault_num,"ST_INT_NAMES",Fs->fault_err_code;
    "RIP:%08X",Fs->rip; //Sometimes crashes on %P, so do this first
    ":%P  RSP:%08X\n",Fs->rip,Fs->rsp;
    if (Fs->fault_num==I_PAGE_FAULT) {
      asm {	MOV_RAX_CR2	 }
      i=GetRAX;
      "Fault Addr:%08X:%P\n",i,i;
    }
    Dbg(NULL,0,TRUE);
    Raw(was_raw);
    sys_raw_dc_flags=old_dc_flags;
  }
  sys_dbg_fault_num=old_fault_num;
  sys_dbg_fault_rip=old_fault_rip;
  SingleUser(was_single_user);
  if (LBtr(&Fs->task_flags,TASKf_KILL_AFTER_DBG))
    Exit;
}
