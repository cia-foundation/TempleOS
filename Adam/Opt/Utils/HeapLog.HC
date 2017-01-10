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
CHeapLogHash *heaplog_head=NULL;

U0 HeapLogMAlloc(U8 *addr)
{
  CHeapLog *tmphl;
  I64 i;
  if (MHeapCtrl(addr)==heaplog_hc_watched) {
    tmphl=MAlloc(sizeof(CHeapLog),heaplog_hc);
    tmphl->addr=addr;
    for (i=0;i<HL_CALLER_DEPTH;i++)
      tmphl->caller[i]=Caller(i+2);
    i=addr>>3 &(HL_HASH_SIZE-1);
    PUSHFD
    CLI
    while (LBts(&sys_semas[SEMA_HEAPLOG_LOCK],0))
      PAUSE
    QueIns(tmphl,heaplog_head[i].last);
    LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);
    POPFD
  }
}

U0 HeapLogFree(U8 *addr)
{
  I64 i;
  CHeapLog *tmphl;
  if (!addr) return;
  if (MHeapCtrl(addr)==heaplog_hc_watched) {
    i=addr>>3 &(HL_HASH_SIZE-1);
    PUSHFD
    CLI
    while (LBts(&sys_semas[SEMA_HEAPLOG_LOCK],0))
      PAUSE
    tmphl=heaplog_head[i].next;
    while (tmphl!=&heaplog_head[i]) {
      if (addr==tmphl->addr) {
	QueRem(tmphl);
	LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);
	POPFD
	Free(tmphl);
	return;
      }
      tmphl=tmphl->next;
    }
    LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);
    POPFD
  }
}

public Bool HeapLog(Bool val=ON,CTask *task=NULL)
{//Turn on.  Collect data.  Call $LK,"HeapLogAddrRep",A="MN:HeapLogAddrRep"$() or  $LK,"HeapLogSizeRep",A="MN:HeapLogSizeRep"$().
  I64 i;
  if (val) {
    if (Bt(&sys_semas[SEMA_HEAPLOG_ACTIVE],0)) {
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
      while (LBts(&sys_semas[SEMA_HEAPLOG_LOCK],0))
	PAUSE
      heaplog_hc=HeapCtrlInit(,,sys_data_bp);
      ext[EXT_HEAPLOG_MALLOC]=&HeapLogMAlloc;
      ext[EXT_HEAPLOG_FREE]=&HeapLogFree;
      heaplog_head=MAlloc(sizeof(CHeapLogHash)*HL_HASH_SIZE,heaplog_hc);
      for (i=0;i<HL_HASH_SIZE;i++)
	QueInit(&heaplog_head[i]);
      LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);
      POPFD
      LBts(&sys_semas[SEMA_HEAPLOG_ACTIVE],0);
      return FALSE;
    }
  } else {
    if (!LBtr(&sys_semas[SEMA_HEAPLOG_ACTIVE],0)) {
      "HeapLog Not Active\n";
      return FALSE;
    } else {
      HeapCtrlDel(heaplog_hc);
      heaplog_head=heaplog_hc=NULL;
      ext[EXT_HEAPLOG_MALLOC]=NULL;
      ext[EXT_HEAPLOG_FREE]=NULL;
      return TRUE;
    }
  }
}

public U0 HeapLogAddrRep(Bool leave_it=OFF)
{//Call $LK,"HeapLog",A="MN:HeapLog"$() first and collect data.
  I64 i,j,total=0;
  CHeapLog *tmphl,hl;
  if (!LBtr(&sys_semas[SEMA_HEAPLOG_ACTIVE],0)) {
    "HeapLog Not Active\n";
    return;
  }
  "$$WW,0$$";
  while (LBts(&sys_semas[SEMA_HEAPLOG_LOCK],0))
    PAUSE
  for (i=0;i<HL_HASH_SIZE;i++) {
    tmphl=heaplog_head[i].next;
    while (tmphl!=&heaplog_head[i]) {
//Take snapshot in case modified. (while we work)
      MemCpy(&hl,tmphl,sizeof(CHeapLog));
      "$$PURPLE$$%08X$$FG$$ %08X",MSize(hl.addr),hl.addr;
      for (j=0;j<HL_CALLER_DEPTH;j++)
	" %P",hl.caller[j];
      '\n';
      total+=MSize(hl.addr);
      tmphl=hl.next;
    }
  }
  LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);
  "\n$$LTRED$$Total:%08X$$FG$$\n",total;
  LBts(&sys_semas[SEMA_HEAPLOG_ACTIVE],0);
  if (!leave_it)
    HeapLog(OFF);
}

public U0 HeapLogSizeRep(Bool leave_it=OFF)
{//Call $LK,"HeapLog",A="MN:HeapLog"$() first and collect data.
  I64 i,j,k,total=0;
  CHeapLog *tmphla,hla,*tmphls,*tmphls1;
  CHeapLogHash *size_head;
  if (!LBtr(&sys_semas[SEMA_HEAPLOG_ACTIVE],0)) {
    "HeapLog Not Active\n";
    return;
  }

  size_head=MAlloc(sizeof(CHeapLogHash)*HL_HASH_SIZE,heaplog_hc);
  for (i=0;i<HL_HASH_SIZE;i++)
    QueInit(&size_head[i]);

  "$$WW,0$$";
  while (LBts(&sys_semas[SEMA_HEAPLOG_LOCK],0))
    PAUSE
  for (i=0;i<HL_HASH_SIZE;i++) {
    tmphla=heaplog_head[i].next;
    while (tmphla!=&heaplog_head[i]) {
//Take snapshot in case modified. (while we work)
      MemCpy(&hla,tmphla,sizeof(CHeapLog));
      k=(MSize(hla.addr)>>3+hla.caller[0])&(HL_HASH_SIZE-1);
      tmphls=size_head[k].next;
      while (tmphls!=&size_head[k]) {
	if (MSize(hla.addr)==tmphls->size) {
	  for (j=0;j<HL_CALLER_DEPTH;j++)
	    if (hla.caller[j]!=tmphls->caller[j])
	      goto hl_next;
	  tmphls->cnt++;
	  goto hl_found;
	}
hl_next:
	tmphls=tmphls->next;
      }
      tmphls=MAlloc(sizeof(CHeapLog),heaplog_hc);
      MemCpy(tmphls,&hla,sizeof(CHeapLog));
      tmphls->cnt=1;
      tmphls->size=MSize(hla.addr);
      QueIns(tmphls,size_head[k].last);
hl_found:
      tmphla=hla.next;
    }
  }
  LBtr(&sys_semas[SEMA_HEAPLOG_LOCK],0);

  for (i=0;i<HL_HASH_SIZE;i++) {
    tmphls=size_head[i].next;
    while (tmphls!=&size_head[i]) {
      tmphls1=tmphls->next;
      "%08X*%08X=%08X",tmphls->size,tmphls->cnt,tmphls->size*tmphls->cnt;
      for (j=0;j<HL_CALLER_DEPTH;j++)
	" %P",tmphls->caller[j];
      '\n';
      total+=tmphls->size*tmphls->cnt;
      Free(tmphls);
      tmphls=tmphls1;
    }
  }
  Free(size_head);

  "\n$$LTRED$$Total:%08X$$FG$$\n",total;
  LBts(&sys_semas[SEMA_HEAPLOG_ACTIVE],0);
  if (!leave_it)
    HeapLog(OFF);
}
