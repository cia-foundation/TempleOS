#help_index "Debugging/Heap"

class CHeapLog
{
  CHeapLog *next,*last;
  U8 *addr;
  U8 *caller1;
  U8 *caller2;
  U8 *caller3;
  U8 *caller4;
};

class CHeapLogHash
{
  CHeapLog *next,*last;
};

#define HEAPLOG_HASH_SIZE	0x1000

CHeapCtrl *heaplog_hc_watched,*heaplog_hc=NULL;
CHeapLogHash *heaplog_root=NULL;

U0 HeapLogMAlloc(U8 *addr)
{
  CHeapLog *temphl;
  I64 i;
  if (MHeapCtrl(addr)==heaplog_hc_watched) {
    temphl=MAlloc(sizeof(CHeapLog),heaplog_hc);
    temphl->addr=addr;
    temphl->caller1=Caller(2);
    temphl->caller2=Caller(3);
    temphl->caller3=Caller(4);
    temphl->caller4=Caller(5);
    i=addr>>3 &(HEAPLOG_HASH_SIZE-1);
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
    i=addr>>3 &(HEAPLOG_HASH_SIZE-1);
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

public U0 HeapLogRep()
{
  I64 i,total=0;
  CHeapLog *temphl,hl;
  if (!LBtr(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0)) {
    "HeapLog Not Active\n";
    return;
  }
  "$$WW,0$$";
  while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
    PAUSE
  for (i=0;i<HEAPLOG_HASH_SIZE;i++) {
    temphl=heaplog_root[i].next;
    while (temphl!=&heaplog_root[i]) {
      //Take snapshot in case modified. (while we work)
      MemCpy(&hl,temphl,sizeof(CHeapLog));
      "$$PURPLE$$%08X$$FG$$ %08X %P %P %P %P\n",MSize(hl.addr),hl.addr,
	hl.caller1,hl.caller2,hl.caller3,hl.caller4;
      total+=MSize(hl.addr);
      temphl=hl.next;
    }
  }
  LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
  "\n$$LTRED$$Total:%08X$$FG$$\n",total;
  LBts(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0);
}

public U0 HeapLog(Bool val=TRUE,CTask *task=NULL)
{
  I64 i;
  if (val) {
    if (Bt(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0))
      "HeapLog Already Active\n";
    else {
      if (!task) task=Fs;
      if (TaskValidate(task))
	heaplog_hc_watched=task->data_heap;
      else
	heaplog_hc_watched=task;
      PUSHFD
      CLI
      while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
	PAUSE
      heaplog_hc=HeapCtrlInit(,,sys_data_bp);
      ext[EXT_HEAPLOG_MALLOC]=&HeapLogMAlloc;
      ext[EXT_HEAPLOG_FREE]=&HeapLogFree;
      heaplog_root=MAlloc(sizeof(CHeapLogHash)*HEAPLOG_HASH_SIZE,heaplog_hc);
      for (i=0;i<HEAPLOG_HASH_SIZE;i++)
	QueInit(&heaplog_root[i]);
      LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);
      POPFD
      LBts(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0);
    }
  } else {
    if (!LBtr(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0))
      "HeapLog Not Active\n";
    else {
      HeapCtrlDel(heaplog_hc);
      heaplog_root=heaplog_hc=NULL;
      ext[EXT_HEAPLOG_MALLOC]=NULL;
      ext[EXT_HEAPLOG_FREE]=NULL;
    }
  }
}

#help_index ""
