#help_index "Debugging/Heap"

#define HL_CALLER_DEPTH	4
#define HL_HASH_SIZE	0x1000

class CHeapLog
{
  CHeapLog *next,*last;
  U8 *addr;
  U8 *caller[HL_CALLER_DEPTH];
};

class CHeapLogHash
{
  CHeapLog *next,*last;
};

CHeapCtrl *heaplog_hc_watched,*heaplog_hc=NULL;
CHeapLogHash *heaplog_root=NULL;

U0 HeapLogMAlloc(U8 *addr)
{
  CHeapLog *temphl;
  I64 i;
  if (MHeapCtrl(addr)==heaplog_hc_watched) {
    temphl=MAlloc(sizeof(CHeapLog),heaplog_hc);
    temphl->addr=addr;
    for (i=0;i<HL_CALLER_DEPTH;i++)
      temphl->caller[i]=Caller(i+2);
    i=addr>>3 &(HL_HASH_SIZE-1);
    PUSHFD
    CLI
    while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
      PAUSE
    QueIns(temphl,heaplog_root[i].last);
    LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
    POPFD
  }
}

U0 HeapLogFree(U8 *addr)
{
  I64 i;
  CHeapLog *temphl;
  if (!addr) return;
  if (MHeapCtrl(addr)==heaplog_hc_watched) {
    i=addr>>3 &(HL_HASH_SIZE-1);
    PUSHFD
    CLI
    while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
      PAUSE
    temphl=heaplog_root[i].next;
    while (temphl!=&heaplog_root[i]) {
      if (addr==temphl->addr) {
	QueRem(temphl);
	LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
	POPFD
	Free(temphl);
	return;
      }
      temphl=temphl->next;
    }
    LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
    POPFD
  }
}

public Bool HeapLog(Bool val=ON,CTask *task=NULL)
{//Turn on.  Collect data.  Call $LK,"HeapLogRep",A="MN:HeapLogRep"$().
  I64 i;
  if (val) {
    if (Bt(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0)) {
      "HeapLog Already Active\n";
      return TRUE;
    } else {
      if (!task) task=Fs;
      if (TaskValidate(task))
	heaplog_hc_watched=task->data_heap;
      else
	heaplog_hc_watched=task;//Actually, not a task, must be a HeapCtrl.
      PUSHFD
      CLI
      while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
	PAUSE
      heaplog_hc=HeapCtrlInit(,,sys_data_bp);
      ext[EXT_HEAPLOG_MALLOC]=&HeapLogMAlloc;
      ext[EXT_HEAPLOG_FREE]=&HeapLogFree;
      heaplog_root=MAlloc(sizeof(CHeapLogHash)*HL_HASH_SIZE,heaplog_hc);
      for (i=0;i<HL_HASH_SIZE;i++)
	QueInit(&heaplog_root[i]);
      LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
      POPFD
      LBts(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0);
      return FALSE;
    }
  } else {
    if (!LBtr(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0)) {
      "HeapLog Not Active\n";
      return FALSE;
    } else {
      HeapCtrlDel(heaplog_hc);
      heaplog_root=heaplog_hc=NULL;
      ext[EXT_HEAPLOG_MALLOC]=NULL;
      ext[EXT_HEAPLOG_FREE]=NULL;
      return TRUE;
    }
  }
}

public U0 HeapLogRep(Bool leave_it=OFF)
{//Call $LK,"HeapLog",A="MN:HeapLog"$() first and collect data.
  I64 i,j,total=0;
  CHeapLog *temphl,hl;
  if (!LBtr(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0)) {
    "HeapLog Not Active\n";
    return;
  }
  "$$WW,0$$";
  while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
    PAUSE
  for (i=0;i<HL_HASH_SIZE;i++) {
    temphl=heaplog_root[i].next;
    while (temphl!=&heaplog_root[i]) {
      //Take snapshot in case modified. (while we work)
      MemCpy(&hl,temphl,sizeof(CHeapLog));
      "$$PURPLE$$%08X$$FG$$ %08X",MSize(hl.addr),hl.addr;
      for (j=0;j<HL_CALLER_DEPTH;j++)
	" %P",hl.caller[j];
      '\n';
      total+=MSize(hl.addr);
      temphl=hl.next;
    }
  }
  LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
  "\n$$LTRED$$Total:%08X$$FG$$\n",total;
  LBts(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0);
  if (!leave_it)
    HeapLog(OFF);
}
