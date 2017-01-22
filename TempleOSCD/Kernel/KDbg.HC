Bool ChkPtr(U8 *ptr)
{//Check if addr is valid ptr.
  if (mem_heap_base<=ptr<=mem_mapped_space) {
    if (*MemPageTable(ptr)&1)
      return TRUE;
    else
      return FALSE;
  } else if (ptr<mem_boot_base)
    return FALSE;
  else if (ptr<VGAM_GRAPHICS)
    return TRUE;
  else
    return FALSE;
}

Bool ChkCodePtr(U8 *ptr)
{//Check if addr is valid code addr.
  if (mem_heap_base<=ptr<=mem_heap_limit) {
    if (*MemPageTable(ptr)&1)
      return TRUE;
    else
      return FALSE;
  } else if (ptr<mem_boot_base)
    return FALSE;
  else if (ptr<VGAM_GRAPHICS)
    return TRUE;
  else
    return FALSE;
}

Bool ChkOnStk(U8 *ptr,CTask *task=NULL)
{//Check if addr is valid stk addr.
  Bool res=FALSE;
  PUSHFD
  CLI
  if (task) {
    if (&task->stk->stk_base<=ptr<=
	  (&task->stk->stk_base)(U8 *)+task->stk->stk_size)
      res=TRUE;
  } else if (mem_heap_base<=ptr<=mem_heap_limit)
    res=TRUE;
  POPFD
  return res;
}

I64 UnusedStk(CTask *task=NULL)
{//Count of usused bytes in task's stk.
  I64 res;
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (task==Fs)
    res=GetRSP()(U8 *)-(&task->stk->stk_base)(U8 *);
  else
    res=task->rsp(U8 *)-(&task->stk->stk_base)(U8 *);
  POPFD
  return res;
}

U8 *Caller(I64 num=1)
{//Returns the addr of the fun which called this one,
//or the caller of the caller, etc.
  U8 **rbp=GetRBP,**ptr;
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
{//Fetches addr of Nth caller on task's stk.
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
    if (task->rip==_RET)
      return *rsp;
    else
      return task->rip;
  }
}
#define STK_REP_LEN	32

U0 StkRep(CTask *task=NULL)
{//Reports whats on the stk.
  I64 i,j,addr,
	**rbp,**rsp,*my_rsp[STK_REP_LEN];
  CHashTable *old_hash=Fs->hash_table;
  if (!task) task=Fs;
  if (!TaskValidate(task))
    return;
  PUSHFD
  CLI
  if (task==Fs) {
    rbp=GetRBP;
    rsp=rbp+3;
    rbp=*rbp;
  } else {
    rsp=task->rsp;
    rbp=task->rbp;
  }
  if (task->rip==_RET)
    addr=*rsp;
  else
    addr=task->rip;
  MemCpy(my_rsp,rsp,STK_REP_LEN*sizeof(U8 *));
  POPFD
  Fs->hash_table=task->hash_table;
  for (i=0;i<STK_REP_LEN;i++) {
    "%08X [RSP+%04X]: %016X ",rsp+i,
	  i*sizeof(U8 *),my_rsp[i];
    while (TRUE) {
      if (!(&task->stk->stk_base<=rbp<
	    (&task->stk->stk_base)(U8 *)+task->stk->stk_size))
	break;
      j=rbp-rsp;
      if (j>=i)
	break;
      addr=my_rsp[j+1];
      if (rbp>=my_rsp[j])
	break;
      rbp=my_rsp[j];
    }
    if (my_rsp[i]==addr)
      "$$RED$$";
    "%P$$FG$$\n",my_rsp[i];
  }
  '\n';
  Fs->hash_table=old_hash;
}

U0 CallerRep(U8 **rbp=NULL,CTask *task=NULL)
{//Prints a report of calling routines.
  I64 **ptr;
  if (!task) task=Fs;
  if (!rbp) {
    if (task==Fs)
      rbp=GetRBP;
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

U0 D(U8 *addr,I64 cnt=0x80,Bool show_offset=TRUE)
{//Dump mem, showing offsets.
//See $LK,"DocD",A="MN:DocD"$() for a live dump.
  I64 i,j,ch;
  U8 *ptr=addr;
  while (cnt) {
    if (show_offset)
      "%08X ",ptr-addr;
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
    '\n';
    cnt-=j;
    ptr+=j;
  }
}

U0 Dm(U8 *addr,I64 cnt=0x80)
{//Show mem addr, not offsets.
  D(addr,cnt,FALSE);
}

U0 Da(U8 **addr,I64 cnt=0x10)
{//Dump mem, showing symbolic addresses.
  while (cnt-->0) {
    "%08X:%08X,%P\n",addr,*addr,*addr;
    addr++;
  }
}

U0 RawPrint(I64 mS=100,U8 *fmt,...)
{//Print using $LK,"Raw",A="MN:Raw"$ scrn output for a length of time.
//$BK,1$Your heap must be good.$BK,0$
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  Bool old_raw,old_input_filter;
  PUSHFD
  CLI
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  "%s",buf;
  Busy(mS<<10);
  POPFD
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
  Free(buf);
}

U0 RawD(I64 mS=100,U8 *addr,I64 cnt=0x80)
{//Dumps a block of mem using $LK,"Raw",A="MN:Raw"$
//scrn output for a fixed length
  //of time.
  Bool old_raw,old_input_filter;
  PUSHFD
  CLI
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  D(addr,cnt);
  Busy(mS<<10);
  POPFD
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

U0 RawDm(I64 mS=100,U8 *addr,I64 cnt=0x80)
{//Dumps a block of mem using $LK,"Raw",A="MN:Raw"$
//scrn output for a fixed length
  //of time.
  Bool old_raw,old_input_filter;
  PUSHFD
  CLI
  old_raw=Raw(ON);
  old_input_filter=LBtr(&Fs->task_flags,TASKf_INPUT_FILTER_TASK);
  Dm(addr,cnt);
  Busy(mS<<10);
  POPFD
  LBEqu(&Fs->task_flags,TASKf_INPUT_FILTER_TASK,old_input_filter);
  Raw(old_raw);
}

I64 *TaskRegAddr(CTask *task,I64 reg_num)
{
  switch (reg_num) {
    case REG_RAX: return &task->rax;
    case REG_RCX: return &task->rcx;
    case REG_RDX: return &task->rdx;
    case REG_RBX: return &task->rbx;
    case REG_RSP: return &task->rsp;
    case REG_RBP: return &task->rbp;
    case REG_RSI: return &task->rsi;
    case REG_RDI: return &task->rdi;
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
  I64 i,j,old_col=text.raw_col;
  Bool old_raw=Raw(ON);
  U8 buf[200];

  if (!task) task=Fs;

  for (i=0;i<16;i++) {
    text.raw_col=i*text.cols+RAWDR_COL;
    "º%3Z:%016X\n",i,"ST_U64_REGS",*TaskRegAddr(task,i);
  }

  text.raw_col=i++*text.cols+RAWDR_COL;
  "ºRIP:%016X\n",task->rip;

  text.raw_col=i++*text.cols+RAWDR_COL;
  "º%-*tp\n",text.cols-(RAWDR_COL+1)-1,Fs->rip;

  text.raw_col=i++*text.cols+RAWDR_COL;
  'º';
  if (Bt(&sys_run_level,RLf_COMPILER)) {
    j=Fs->rip;
    Ui(buf,&j,,,TRUE);
    "%s",buf;
  } else
    '\n';

  text.raw_col=i*text.cols+RAWDR_COL;
  'È';
  for (j=0;j<text.cols-RAWDR_COL-1;j++)
    'Í';

  text.raw_col=old_col;
  Raw(old_raw);
}

U0 Dr(CTask *task=NULL)
{//Dump regs
  I64 i;
  if (!task) task=Fs;
  for (i=0;i<16;i++)
    "%3Z:%016X\n",i,"ST_U64_REGS",*TaskRegAddr(task,i);
  "RIP:%016X\n",task->rip;
}

U8 *SysGetStr2(I64)
{
  U8 buf[512];
  GetS(buf,512,FALSE);
  return StrNew(buf);
}

CBpt *BptFind(U8 *needle_addr,CTask *haystack_task=NULL,Bool rem=FALSE)
{
  CBpt *res=NULL,*tmpb,*tmpb1,*tmpb2;
  if (!haystack_task) haystack_task=Fs;
  PUSHFD
  CLI
  tmpb1=&haystack_task->bpt_lst;
  tmpb=haystack_task->bpt_lst;
  while (tmpb) {
    tmpb2=tmpb->next;
    if (tmpb->addr==needle_addr) {
      res=tmpb;
      if (rem)
	tmpb1->next=tmpb2;
      else
	tmpb1=&tmpb->next;
    } else
      tmpb1=&tmpb->next;
    tmpb=tmpb2;
  }
  POPFD
  return res;
}

Bool BptS(U8 *addr,CTask *task=NULL,Bool live=TRUE)
{//Set breakpoint.
  CBpt *tmpb;
  Bool res=TRUE;
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (!(tmpb=BptFind(addr,task,FALSE))) {
    tmpb=CAlloc(sizeof(CBpt),task);
    tmpb->addr=addr;
    tmpb->val=*addr;
    res=FALSE;
    tmpb->next=task->bpt_lst;
    task->bpt_lst=tmpb;
    if (task==Fs && live)
      *addr=OC_BPT;
  }
  POPFD
  return res;
}

Bool BptR(U8 *addr,CTask *task=NULL,Bool live=TRUE,Bool rem=TRUE)
{//Rem breakpoint.
  CBpt *tmpb;
  Bool res=FALSE;
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (tmpb=BptFind(addr,task,rem)) {
    if (task==Fs && live)
      *tmpb->addr=tmpb->val;
    res=TRUE;
    if (rem)
      Free(tmpb);
  }
  POPFD
  return res;
}

Bool B(U8 *addr,CTask *task=NULL,Bool live=TRUE)
{//Toggle breakpoint.
//Return: TRUE if removed.
  Bool res=FALSE;
  PUSHFD
  CLI
  if (BptFind(addr,task,FALSE)) {
    BptR(addr,task,live,TRUE);
    res=TRUE;
  } else
    BptS(addr,task,live);
  POPFD
  return res;
}

I64 B2(CTask *task=NULL,Bool live=TRUE)
{//Rem all breakpoints.
//Return: cnt of removed.
  I64 res=0;
  CBpt *tmpb,*tmpb1;
  if (!task) task=Fs;
  PUSHFD
  CLI
  tmpb=task->bpt_lst;
  task->bpt_lst=NULL;
  while (tmpb) {
    tmpb1=tmpb->next;
    if (task==Fs && live)
      *tmpb->addr=tmpb->val;
    Free(tmpb);
    tmpb=tmpb1;
    res++;
  }
  POPFD
  return res;
}

U0 G(U8 *ip=INVALID_PTR,CTask *task=NULL)
{//Go
  if (!task) task=Fs;
  if (ip!=INVALID_PTR) task->rip=ip;
  if (BptFind(task->rip,task))
    "\nDo one of the following, first:\n"
	  ">S;\t\t\t//Single step\n"
	  ">B2;\t\t\t//Clear all break points\n"
	  ">G2;\t\t\t//Clear all break points and Go\n\n"
	  "After resuming, <CTRL-ALT-n> next focus task\n"
	  "After resuming, <CTRL-ALT-v> flushes scrn VGA cache\n";
  else {
    LBtr(&task->task_flags,TASKf_DISABLE_BPTS);
    LBtr(&task->rflags,RFLAGf_TRAP);//No single step
    Suspend(task,FALSE);
    if (task==Fs) {
      if (IsDbgMode && task->next_cc!=&task->next_cc) {
	"Exit Dbg\n";
	Btr(&task->last_cc->flags,CCf_PMT);
      }
    } else
      Exit;
  }
}

U0 G2(U8 *ip=INVALID_PTR,CTask *task=NULL)
{//Rem all breakpoints and Go.
  if (!task) task=Fs;
  B2(task);
  if (ext[EXT_WIN_FOCUS])
    CallExtNum(EXT_WIN_FOCUS,dbg.focus_task);
  VGAFlush;
  G(ip,task);
}

public U0 S(U8 *ip=INVALID_PTR,CTask *task=NULL) //Single-step.
{//Single step.
  if (!task) task=Fs;
  PUSHFD
  CLI
  if (ip!=INVALID_PTR) task->rip=ip;
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  LBts(&task->rflags,RFLAGf_TRAP);
  Suspend(task,FALSE);
  if (task==Fs) {
    if (IsDbgMode) {
      if (task->next_cc!=&task->next_cc)
	Btr(&task->last_cc->flags,CCf_PMT);
    }
  } else
    Exit;
  POPFD
}

U0 DbgHelp()
{
  "\n"
	"The cmd line is basically the same as normal.	Here are some common\n"
	"debugging commands.\n\n"
	">EdLite(\"FileName\");\t\t//Edit file.\n"
	">D(0x100000);\t\t\t//Dump page tables.\n"
	">Dm(0x100000);\t\t\t//Dump page tables.\n"
	">Dm(Fs,sizeof(CTask));\t\t//Dump current task record.\n"
	">ClassRep(Fs,\"CTask\",1);\t//Dump current task record.\n"
	">ClassRep(Fs,,1);\t\t//(It knows lastclass.)\n"
	">CallerRep;\t\t\t//Stack trace report.\n"
	">Da(_RSP);\t\t\t//Dump stk.\n"
	">Dr;\t\t\t\t//Dump Regs.\n"
	">1+2*3+&Print;\t\t\t//Show calculation res.\n"
	">*(0x70000)(I64 *)=0x123456789;\t//Assign value to 0x70000-0x70007.\n"
	">_RAX=0x1234;\t\t\t//Set RAX to 0x1234.\n"
	">_RIP=&Break;\t\t//Set RIP.\n"
	">I64 i;\t\t\t\t//Declare variable.\n"
	">i=_RCX+_RDX;\t\t\t//Assign to variable.\n"
	">U(&Print+0x8);\t\t\t//Unassemble Print.\n"
	">Uf(\"Print\");\t\t\t//Unassembler function \"Print\".\n"
	">Man(\"Print\");\t\t\t//Edit Src for \"Print\".\n"
	">E(_RIP);\t\t\t//Edit Src Code.\n"
	">Fix;\t\t\t\t//Edit Last Err Src Code.\n"
	">B(&Main+0x20);\t\t\t//Toggle break point.\n"
	">B2;\t\t\t\t//Clear all break points.\n"
	">S;\t\t\t\t//Single step.\n"
	">G;\t\t\t\t//Resume execution.\n"
	">G2;\t\t\t\t//B2;VGAFlush;WinFocus;G;\n"
	">Exit;\t\t\t\t//Exit (kill) task.\n\n"
	"After resuming, <CTRL-ALT-n> next focus task.\n"
	"After resuming, <CTRL-ALT-v> flushes scrn VGA cache.\n\n";
}

U0 Dbg2()
{
  Bool old_win_inhibit,old_waiting_msg,old_single;
  I64 i,old_getstr2;
  U8 buf[200];
  if (dbg.panic) {
    if (IsRaw) {
      i=Fs->rip;
      Ui(buf,&i);
      "%s",buf;
    } else
      U(Fs->rip,1);
  } else
    dbg.panic=TRUE;
  old_waiting_msg=LBtr(&Fs->task_flags,TASKf_AWAITING_MSG);
  old_win_inhibit=Fs->win_inhibit;
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  sys_focus_task=Fs;
  kbd.scan_code=0;
  old_getstr2=fp_getstr2;
  fp_getstr2=&SysGetStr2;
  old_single=SingleUser(OFF);
  while (!ms_hard.install_attempts)
    Yield;
  SingleUser(old_single);
  UserTaskCont;
  fp_getstr2=old_getstr2;
  Fs->win_inhibit=old_win_inhibit;
  LBEqu(&Fs->task_flags,TASKf_AWAITING_MSG,old_waiting_msg);
}

U0 Fault3(I64 fault_num,I64 fault_err_code)
{
  no_warn fault_err_code;
  PUSHFD
  CLI
  if (Gs->num && dbg.mp_crash) {
    mp_cnt=1;
    dbg.mp_crash->cpu_num=Gs->num;
    dbg.mp_crash->task=Fs;
    dbg.mp_crash->rip=Fs->rip;
    dbg.mp_crash->msg=dbg.msg;
    dbg.mp_crash->msg_num=dbg.msg_num;
    MPInt(I_MP_CRASH,0);
    SysHlt;
  }
  "\n\tTempleOS Debugger\n\n"
	">Help;\t//For help.\n\n";
  Beep(62,TRUE);
  if (fault_num==I_DBG) {
    if (dbg.msg) {
      "\n!!! %s",dbg.msg;
      if (dbg.msg_num)
	"%016X",dbg.msg_num;
      " !!!\n\n";
    }
  }
  if (dbg.panic)
    CallerRep;
  Dbg2;
  POPFD
}

U0 Fault2(I64 fault_num,I64 fault_err_code)
{//Called from $LK,"Fault2",A="FF:::/Kernel/KInts.HC,Fault2"$.
//$BK,1$Be careful not to swap-out and ruin the saved context$BK,0$
  Bool was_raw,was_single_user,was_silent,was_in_dbg;
  I64  i,old_raw_flags=text.raw_flags;
  was_single_user=SingleUser(ON);
  if (!IsDbgMode)
    dbg.focus_task=sys_focus_task;
  sys_focus_task=NULL;
  if (fault_num==I_BPT)
    Fs->rip--;
  if (Fs->dbg_task)
    CallExtNum(EXT_DBG_RESUME,fault_num,fault_err_code);
  else {
    was_raw=Raw(ON);
    was_silent=Silent(OFF);
    text.raw_flags|=RWF_SHOW_DOLLAR|RWF_SCROLL;

    "Task \"";
    "%s",Fs->task_title;
    "\"\n";
    "Fault:0x%02X %Z\t\tErr Code:%08X\n",
	  fault_num,fault_num,"ST_INT_NAMES",fault_err_code;
    was_in_dbg=DbgMode(ON);
    "RIP:%08X",Fs->rip; //Sometimes crashes on %p, so do this first
    ":%p  RSP:%08X\n",Fs->rip,Fs->rsp;
    if (fault_num==I_PAGE_FAULT) {
      MOV_RAX_CR2
      i=GetRAX;
      "Fault Addr:%08X:%p\n",i,i;
    }
    Fault3(fault_num,fault_err_code);
    DbgMode(was_in_dbg);
    Silent(was_silent);
    Raw(was_raw);
    text.raw_flags=old_raw_flags;
  }
  SingleUser(was_single_user);
  if (LBtr(&Fs->task_flags,TASKf_KILL_AFTER_DBG))
    Exit;
}

U0 Panic(U8 *msg=NULL,I64 msg_num=0,Bool panic=TRUE)
{//Enter the debugger with panic?
  PUSHFD
  CLI
  dbg.msg=msg;
  dbg.msg_num=msg_num;
  dbg.panic=panic;
  INT I_DBG
  POPFD
}

U0 Dbg(U8 *msg=NULL,I64 msg_num=0)
{//Enter debugger, no panic.
  Panic(msg,msg_num,FALSE);
}
