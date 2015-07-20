#help_index "Memory/Task"
public I64 TaskMemAlloced(CTask *task=NULL,Bool override_validate=FALSE)
{//Count of bytes alloced to a task, used+unused.
  I64 result;
  if (!task) task=Fs;
  if (override_validate || TaskValidate(task)) {
    result=task->code_heap->alloced_u8s;
    if (task->code_heap!=task->data_heap)
      result+=task->data_heap->alloced_u8s;
    return result;
  } else
    return 0;
}

public I64 TaskMemUsed(CTask *task=NULL,Bool override_validate=FALSE)
{//Count of bytes alloced to a task and in use.
  I64 result;
  if (!task) task=Fs;
  if (override_validate || TaskValidate(task)) {
    result=task->code_heap->used_u8s;
    if (task->data_heap!=task->code_heap)
      result+=task->data_heap->used_u8s;
    return result;
  } else
    return 0;
}

#help_index "Memory/Task;Debugging/Heap"
public Bool HeapRep(CTask *task)
{//Report status of task's heap.
  I64 i,cnt;
  CMemUnused *uum;

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

#help_index "Memory/HeapCtrl;Debugging/Heap"
public Bool IsInHeapCtrl(U8 *a,CHeapCtrl *hc,Bool lock=TRUE)
{//Check addr if in HeapCtrl.
  CMemBlk *m;
  PUSHFD
  CLI
  if (lock)
    while (LBts(&hc->locked_flags,HClf_LOCKED))
      PAUSE
  m=hc->next_mem_blk;
  while (m!=&hc->next_mem_blk) {
    if (a>=m && a<m(U8 *)+m->pages*PAGE_SIZE) {
      if (lock)
	LBtr(&hc->locked_flags,HClf_LOCKED);
      POPFD
      return TRUE;
    }
    m=m->next;
  }
  if (lock)
    LBtr(&hc->locked_flags,HClf_LOCKED);
  POPFD
  return FALSE;
}

public Bool HeapCtrlWalk(CHeapCtrl *hc)
{//Check integrity of HeapCtrl.
  I64 i;
  CMemUnused *uum;

  PUSHFD
  CLI
  while (LBts(&hc->locked_flags,HClf_LOCKED))
    PAUSE

  for (i=0;i<HEAP_HASH_SIZE>>3;i++) {
    uum=hc->heap_hash[i];
    while (uum) {
      if (!IsInHeapCtrl(uum,hc,FALSE))
	goto hc_false;
      uum=uum->next;
    }
  }
  uum=hc->malloc_free_lst;
  while (uum) {
    if (!IsInHeapCtrl(uum,hc,FALSE))
      goto hc_false;
    uum=uum->next;
  }

  #if _CFG_HEAP_DBG
  CMemUsed *um,*um1;
  um1=(&hc->next_um)(U8 *)-offset(CMemUsed.next);
  um=um1->next;
  while (um!=um1) {
    if (!IsInHeapCtrl(um,hc,FALSE))
      goto hc_false;
    um=um->next;
  }
#endif

  LBtr(&hc->locked_flags,HClf_LOCKED);
  POPFD
  return TRUE;

  hc_false:
  LBtr(&hc->locked_flags,HClf_LOCKED);
  POPFD
  return FALSE;
}

#help_index "Memory/Task;Debugging/Heap"
public Bool IsInHeap(U8 *a,CTask *task=NULL,Bool lock=TRUE)
{//Check addr if in task's heaps.
  if (!task) task=Fs;
  if (TaskValidate(task) && (IsInHeapCtrl(a,task->code_heap,lock)||
	task->data_heap!=task->code_heap &&
	IsInHeapCtrl(a,task->data_heap,lock)))
    return TRUE;
  else
    return FALSE;
}

public Bool HeapWalk(CTask *task=NULL)
{//Check integrity of task's heaps.
  if (!task) task=Fs;
  if (!TaskValidate(task) || !HeapCtrlWalk(task->code_heap) ||
	task->data_heap!=task->code_heap && !HeapCtrlWalk(task->data_heap))
    return FALSE;
  else
    return TRUE;
}

#help_index "Info;Task"
U0 TaskRepTask(CTask *task,I64 indent)
{
  CTask *task1;
  U8 *st,*desc=MStrUtil(task->task_title,SUF_SAFE_DOLLAR);
  st=MStrPrint("$$MA,T=\"%08X\",LM=\"Kill(0x%X);\n\",$$",task,task,task);
  "%h*c%s $$TX,\"%Q...\",SCX=16$$ Mem:%08X\n",indent,CH_SPACE,
	st,desc,TaskMemAlloced(task);
  "%h*cFlags:%04X:%04X Time:%0.2fm\n",indent+2,CH_SPACE,
	task->task_flags,task->display_flags,
	task->total_time/60.0/cnts.time_stamp_freq_initial;
  Free(st);
  Free(desc);
  task1=task->next_child_task;
  while (task1!=(&task->next_child_task)(U8 *)
	-offset(CTask.next_sibling_task)) {
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
