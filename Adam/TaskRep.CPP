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
