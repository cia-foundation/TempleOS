#help_index "Debugging/Heap;Memory/Debugging"
#help_file "::/Doc/HeapDbg"

#define HL_CALLER_DEPTH	5 //Feel free to change this.
#define HL_HASH_SIZE	0x1000

class CHeapLog
{
  CHeapLog *next,*last;
  union {
    U8 *addr;
    I64 size;
  }
  I64 cnt;
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
{//Turn on.  Collect data.  Call $LK,"HeapLogAddrRep",A="MN:HeapLogAddrRep"$() or  $LK,"HeapLogSizeRep",A="MN:HeapLogSizeRep"$().
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

public U0 HeapLogAddrRep(Bool leave_it=OFF)
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

public U0 HeapLogSizeRep(Bool leave_it=OFF)
{//Call $LK,"HeapLog",A="MN:HeapLog"$() first and collect data.
  I64 i,j,k,total=0;
  CHeapLog *temphla,hla,*temphls,*temphls1;
  CHeapLogHash *size_root;
  if (!LBtr(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0)) {
    "HeapLog Not Active\n";
    return;
  }

  size_root=MAlloc(sizeof(CHeapLogHash)*HL_HASH_SIZE,heaplog_hc);
  for (i=0;i<HL_HASH_SIZE;i++)
    QueInit(&size_root[i]);

  "$$WW,0$$";
  while (LBts(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0))
    PAUSE
  for (i=0;i<HL_HASH_SIZE;i++) {
    temphla=heaplog_root[i].next;
    while (temphla!=&heaplog_root[i]) {
//Take snapshot in case modified. (while we work)
      MemCpy(&hla,temphla,sizeof(CHeapLog));
      k=(MSize(hla.addr)>>3+hla.caller[0])&(HL_HASH_SIZE-1);
      temphls=size_root[k].next;
      while (temphls!=&size_root[k]) {
	if (MSize(hla.addr)==temphls->size) {
	  for (j=0;j<HL_CALLER_DEPTH;j++)
	    if (hla.caller[j]!=temphls->caller[j])
	      goto hl_next;
	  temphls->cnt++;
	  goto hl_found;
	}
hl_next:
	temphls=temphls->next;
      }
      temphls=MAlloc(sizeof(CHeapLog),heaplog_hc);
      MemCpy(temphls,&hla,sizeof(CHeapLog));
      temphls->cnt=1;
      temphls->size=MSize(hla.addr);
      QueIns(temphls,size_root[k].last);
hl_found:
      temphla=hla.next;
    }
  }
  LBtr(&sys_semas[SYS_SEMA_HEAPLOG_LOCK],0);

  for (i=0;i<HL_HASH_SIZE;i++) {
    temphls=size_root[i].next;
    while (temphls!=&size_root[i]) {
      temphls1=temphls->next;
      "%08X*%08X=%08X",temphls->size,temphls->cnt,temphls->size*temphls->cnt;
      for (j=0;j<HL_CALLER_DEPTH;j++)
	" %P",temphls->caller[j];
      '\n';
      total+=temphls->size*temphls->cnt;
      Free(temphls);
      temphls=temphls1;
    }
  }
  Free(size_root);

  "\n$$LTRED$$Total:%08X$$FG$$\n",total;
  LBts(&sys_semas[SYS_SEMA_HEAPLOG_ACTIVE],0);
  if (!leave_it)
    HeapLog(OFF);
}
