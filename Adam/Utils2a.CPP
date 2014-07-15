#help_index "Memory/Blk Pool"
public I64 UnusedBPMem(CBlkPool *bp)
{//Count of unused bytes in bp.
  I64 result=0,i;
  CMemBlk *tempm;

  if (!bp) return 0;

  PUSHFD
  CLI
  while (LBts(&bp->locked_flags,BPlf_LOCKED))
    PAUSE

  tempm=bp->mem_free_lst;
  while (tempm) {
    if (tempm->mb_signature!=MBS_UNUSED_SIGNATURE_VAL)
      Dbg("Corrupt Sys Memory");
    result+=tempm->pages;
    tempm=tempm->next;
  }
  for (i=0;i<FREE_PAGE_HASH_SIZE;i++) {
    tempm=bp->free_page_hash[i];
    while (tempm) {
      if (tempm->mb_signature!=MBS_UNUSED_SIGNATURE_VAL)
	Dbg("Corrupt Sys Memory");
      result+=tempm->pages;
      tempm=tempm->next;
    }
  }

  for (i=0;i<64-PAGE_BITS;i++) {
    tempm=bp->free_page_hash2[i];
    while (tempm) {
      if (tempm->mb_signature!=MBS_UNUSED_SIGNATURE_VAL)
	Dbg("Corrupt Sys Memory");
      result+=tempm->pages;
      tempm=tempm->next;
    }
  }
  LBtr(&bp->locked_flags,BPlf_LOCKED);
  POPFD
  return result*PAGE_SIZE;
}

#help_index "Memory/Heap Ctrl"
I64 MemBlkLstSize(CHeapCtrl *hc)
{
  CMemBlk *m;
  I64 result=0;
  m=hc->next_mem_blk;
  while (m!=&hc->next_mem_blk) {
    result+=sizeof(CMemBlk);
    m=m->next;
  }
  return result;
}

public I64 UnusedHCMem(CHeapCtrl *hc)
{//Unused bytes alloced to a heap ctrl.
  I64 result,i;
  CMemUnusedAllocated *u;

  if (!hc) return 0;

  result=UnusedBPMem(hc->bp);

  PUSHFD
  CLI
  while (LBts(&hc->locked_flags,HClf_LOCKED))
    PAUSE

  for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
    u=hc->heap_hash[i];
    while (u) {
      result+=u->size;
      u=u->next;
    }
  }
  u=hc->malloc_free_lst;
  while (u) {
    result+=u->size;
    u=u->next;
  }

  LBtr(&hc->locked_flags,HClf_LOCKED);
  POPFD
  return result;
}

#help_index "Memory/Task"
public I64 TaskAllocatedMem(CTask *task=NULL,Bool override_validate=FALSE)
{//Count of bytes alloced to a task, used+unused.
  CMemBlk *m,*m1;
  I64 result=0;

  if (!task) task=Fs;
  if (!override_validate && !TaskValidate(task)) return 0;

  PUSHFD
  CLI
  while (LBts(&task->code_heap->locked_flags,HClf_LOCKED))
    PAUSE
  if (task->code_heap!=task->data_heap)
    while (LBts(&task->data_heap->locked_flags,HClf_LOCKED))
      PAUSE

  m=task->code_heap->next_mem_blk;
  m1=&task->code_heap->next_mem_blk;
  while (m!=m1) {
    if (m->mb_signature!=MBS_USED_SIGNATURE_VAL)
      Dbg("Corrupt Task Memory");
    result+=m->pages*PAGE_SIZE;
    m=m->next;
  }

  if (task->code_heap!=task->data_heap) {
    m=task->data_heap->next_mem_blk;
    m1=&task->data_heap->next_mem_blk;
    while (m!=m1) {
      if (m->mb_signature!=MBS_USED_SIGNATURE_VAL)
	Dbg("Corrupt Task Memory");
      result+=m->pages*PAGE_SIZE;
      m=m->next;
    }
  }

  if (task->code_heap!=task->data_heap)
    LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
  LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
  POPFD
  return result;
}

public I64 TaskUnusedAllocatedMem(CTask *task=NULL,Bool override_validate=FALSE)
{//Count of bytes alloced to a task but unused.
  I64 result=0,i;
  CMemUnusedAllocated *uum;
  if (!task) task=Fs;
  if (!override_validate && !TaskValidate(task)) return 0;

  PUSHFD
  CLI
  while (LBts(&task->code_heap->locked_flags,HClf_LOCKED))
    PAUSE
  if (task->data_heap!=task->code_heap)
    while (LBts(&task->data_heap->locked_flags,HClf_LOCKED))
      PAUSE

  result+=MemBlkLstSize(task->code_heap);
  for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
    uum=task->code_heap->heap_hash[i];
    while (uum) {
      result+=uum->size;
      uum=uum->next;
    }
  }
  uum=task->code_heap->malloc_free_lst;
  while (uum) {
    result+=uum->size;
    uum=uum->next;
  }

  if (task->data_heap!=task->code_heap) {
    result+=MemBlkLstSize(task->data_heap);
    for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
      uum=task->data_heap->heap_hash[i];
      while (uum) {
	result+=uum->size;
	uum=uum->next;
      }
    }
    uum=task->data_heap->malloc_free_lst;
    while (uum) {
      result+=uum->size;
      uum=uum->next;
    }
  }

  if (task->data_heap!=task->code_heap)
    LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
  LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
  POPFD
  return result;
}

public I64 TaskUsedMem(CTask *task=NULL,Bool override_validate=FALSE)
{//Count of bytes alloced to a task and in use.
  I64 result=0;
  if (!task) task=Fs;
  if (!override_validate && !TaskValidate(task)) return 0;
  result=task->code_heap->used_u8s;
  if (task->data_heap!=task->code_heap)
    result+=task->data_heap->used_u8s;
  return result;
}

public Bool HeapRep(CTask *task)
{//Report status of task's heap.
  I64 i,cnt;
  CMemUnusedAllocated *uum;

  if (!task || task==Fs) {
    "Task can't HeapRep on self.\n";
    return FALSE;
  }
  if (!TaskValidate(task)) return FALSE;

  PUSHFD
  CLI
  while (LBts(&task->code_heap->locked_flags,HClf_LOCKED))
    PAUSE
  if (task->data_heap!=task->code_heap)
    while (LBts(&task->data_heap->locked_flags,HClf_LOCKED))
      PAUSE

  for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
    cnt=0;
    uum=task->code_heap->heap_hash[i];
    while (uum) {
      cnt+=uum->size;
      uum=uum->next;
    }
    if (task->data_heap!=task->code_heap) {
      uum=task->data_heap->heap_hash[i];
      while (uum) {
	cnt+=uum->size;
	uum=uum->next;
      }
    }
    if (cnt)
      "%03X:%08X\n",i<<3,cnt;
  }
  '\n';

  uum=task->code_heap->malloc_free_lst;
  while (uum) {
    "%X, ",uum->size;
    uum=uum->next;
  }
  if (task->data_heap!=task->code_heap) {
    uum=task->data_heap->malloc_free_lst;
    while (uum) {
      "%X, ",uum->size;
      uum=uum->next;
    }
  }

  if (task->data_heap!=task->code_heap)
    LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
  LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
  POPFD

  '\n';
}

#help_index "Memory/Task;Debugging/Heap"
public Bool IsInHeap(U8 *a,CTask *task=NULL,Bool lock=TRUE)
{//Check addr if on heap.
  CMemBlk *m;

  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;

  PUSHFD
  CLI
  if (lock) {
    while (LBts(&task->code_heap->locked_flags,HClf_LOCKED))
      PAUSE
    if (task->data_heap!=task->code_heap)
      while (LBts(&task->data_heap->locked_flags,HClf_LOCKED))
	PAUSE
  }

  m=task->code_heap->next_mem_blk;
  while (m!=&task->code_heap->next_mem_blk) {
    if (a>=m && a<m(U8 *)+m->pages*PAGE_SIZE) {
      if (lock)
	LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
      POPFD
      return TRUE;
    }
    m=m->next;
  }
  if (task->data_heap!=task->code_heap) {
    m=task->data_heap->next_mem_blk;
    while (m!=&task->code_heap->next_mem_blk) {
      if (a>=m && a<m(U8 *)+m->pages*PAGE_SIZE) {
	if (lock)
	  LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
	POPFD
	return TRUE;
      }
      m=m->next;
    }
  }

  if (lock) {
    LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
    if (task->data_heap!=task->code_heap)
      LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
  }
  POPFD

  return FALSE;
}

public Bool HeapWalk(CTask *task=NULL)
{//Check integrity of task's heap.
  I64 i;
  CMemUnusedAllocated *uum;
  Bool result=TRUE;

  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;

  PUSHFD
  CLI
  while (LBts(&task->code_heap->locked_flags,HClf_LOCKED))
    PAUSE
  if (task->data_heap!=task->code_heap)
    while (LBts(&task->data_heap->locked_flags,HClf_LOCKED))
      PAUSE

  for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
    uum=task->code_heap->heap_hash[i];
    while (uum) {
      if (!IsInHeap(uum,task,FALSE))
	result=FALSE;
      uum=uum->next;
    }
    if (task->data_heap!=task->code_heap) {
      uum=task->data_heap->heap_hash[i];
      while (uum) {
	if (!IsInHeap(uum,task,FALSE))
	  result=FALSE;
	uum=uum->next;
      }
    }
  }

  uum=task->code_heap->malloc_free_lst;
  while (uum) {
    if (!IsInHeap(uum,task,FALSE))
      result=FALSE;
    uum=uum->next;
  }

  if (task->data_heap!=task->code_heap) {
    uum=task->data_heap->malloc_free_lst;
    while (uum) {
      if (!IsInHeap(uum,task,FALSE))
	result=FALSE;
      uum=uum->next;
    }
  }

#if _CFG_HEAP_DBG
  CMemUsedAllocated *um,*um1;
  um1=(&task->code_heap->next_um)(U8 *)-offset(CMemUsedAllocated.next);
  um=um1->next;
  while (um!=um1) {
    if (!IsInHeap(um,task,FALSE))
      result=FALSE;
    um=um->next;
  }
  if (task->data_heap!=task->code_heap) {
    um1=(&task->data_heap->next_um)(U8 *)-offset(CMemUsedAllocated.next);
    um=um1->next;
    while (um!=um1) {
      if (!IsInHeap(um,task,FALSE))
	result=FALSE;
      um=um->next;
    }
  }
#endif

  if (task->data_heap!=task->code_heap)
    LBtr(&task->data_heap->locked_flags,HClf_LOCKED);
  LBtr(&task->code_heap->locked_flags,HClf_LOCKED);
  POPFD
  return result;
}

#help_index "Info;Task"
U0 TaskRepTask(CTask *task,I64 indent)
{
  CTask *task1;
  U8 *st,*desc=MStrUtil(task->task_title,SUF_SAFE_DOLLAR);
  st=MStrPrint("$$MA,T=\"%08X\",LM=\"Kill(0x%X);\n\",$$",task,task,task);
  "%h*c%s $$TX,\"%Q...\",SCX=16$$ Mem:%08X\n",indent,CH_SPACE,
    st,desc,TaskAllocatedMem(task);
  "%h*cFlags:%04X:%04X Time:%0.2fm\n",indent+2,CH_SPACE,
    task->task_flags,task->display_flags,
    task->total_time/60.0/cnts.time_stamp_freq_initial;
  Free(st);
  Free(desc);
  task1=task->next_child_task;
  while (task1!=(&task->next_child_task)(U8 *)-offset(CTask.next_sibling_task)) {
    TaskRepTask(task1,indent+2);
    task1=task1->next_sibling_task;
  }
}

public U0 TaskRep()
{//Report current tasks on all cores.
  I64 i;
  CCPU *c;
  PUSHFD
  CLI
  for (i=0;i<mp_cnt;i++) {
    c=&cpu_structs[i];
    "$$RED$$CPU:%d$$FG$$\n",i;
    TaskRepTask(c->seth_task,2);
  }
  POPFD
}
