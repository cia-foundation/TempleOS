#help_index "Debugging/Profiler;Utils/Profiler;Profiler"
#help_file "::/Doc/Profiler"

#define PF_ARRAY_CNT 0x100000
I64 pf_time_start,pf_time_end;
I64 pf_low=0,pf_high=SYS_KERNEL_END,pf_size=PF_ARRAY_CNT;
I64 *pf_array=NULL;
I64 pf_cpu=0;
U0 (*fp_chained_timer_routine)(CTask *task)=cpu_structs[pf_cpu].timer_irq_chain;
I64 pf_ptr,pf_depth;
#define PFM_OFF		0
#define PFM_BIN		1
#define PFM_ALL		2
I64 prof_mode=PFM_OFF;
F64 pf_t0;

U0 ProfTimerInt(CTask *task)
{
  I64 i,j=task->rip,k;
  nobound_switch (prof_mode) {
    case PFM_OFF:
      break;
    case PFM_BIN:
      i=(j-pf_low) * pf_size/(pf_high-pf_low);
      if (0<=i<pf_size)
	pf_array[i]++;
      break;
    case PFM_ALL:
      for (k=0;k<=pf_depth;k++) {
	if (task==Gs->idle_task)
	  j=SYS_IDLE_PT;
	else
	  j=TaskCaller(task,k,TRUE);
	if (pf_ptr<PF_ARRAY_CNT) {
	  pf_array[pf_ptr++]=j;
	  if (pf_ptr==PF_ARRAY_CNT)
	    pf_time_end=sys_jiffies;
	}
      }
      break;
  }
  (*fp_chained_timer_routine)(task);
}
cpu_structs[pf_cpu].timer_irq_chain=&ProfTimerInt;

public U0 ProfBin(U8 *start,I64 cnt=PF_ARRAY_CNT*3)
//Start profiler in bin mode.  Profilers
//report where time is spent by checking
//the instruction ptr during the
//timer interrupt.  Do a $LK,"ProfRep",A="MN:ProfRep"$(), (profiler report)
//after you have collected data.
{
  if (cnt<PF_ARRAY_CNT)
    pf_size=cnt;
  else
    pf_size=PF_ARRAY_CNT;
  pf_low=start;
  pf_high=start+cnt;
  if (!pf_array)
    pf_array=AMAlloc(sizeof(I64)*PF_ARRAY_CNT);
  MemSet(pf_array,0,PF_ARRAY_CNT*sizeof(I64));
  pf_time_end=0;
  pf_time_start=sys_jiffies;
  prof_mode=PFM_BIN;
  pf_t0=tS;
}

public U0 ProfCPUSet(I64 num)
{
  if (num>=mp_cnt)
    ST_ERR_ST "Invalid CPU\n";
  else if (cpu_structs[pf_cpu].timer_irq_chain!=&ProfTimerInt)
    ST_ERR_ST "Unchaining not implemented!\n";
  else {
    cpu_structs[pf_cpu].timer_irq_chain=fp_chained_timer_routine;
    pf_cpu=num;
    fp_chained_timer_routine=cpu_structs[pf_cpu].timer_irq_chain;
    cpu_structs[pf_cpu].timer_irq_chain=&ProfTimerInt;
  }
}

public U0 ProfAll(I64 depth=0)
//Start profiler in record-all mode.  Profilers
//report where time is spent by checking
//the instruction ptr during the
//timer interrupt.  Do a $LK,"ProfRep",A="MN:ProfRep"$(), (profiler report)
//after you have collected data.
{
  pf_depth=depth;
  pf_ptr=0;
  if (!pf_array)
    pf_array=AMAlloc(sizeof(I64)*PF_ARRAY_CNT);
  pf_time_end=0;
  pf_time_start=sys_jiffies;
  prof_mode=PFM_ALL;
  pf_t0=tS;
}

I64 PFACompare(U8 *i1,U8 *i2)
{
  if (i1<i2)
    return -1;
  else if (i1>i2)
    return 1;
  else
    return 0;
}

public U0 ProfRep(I64 filter_cnt=1)
{
  F64 pf_tf=tS;
  I64 i,ip,last_ip=pf_low,hits,
      total_time,routine_total=0,old_mode=prof_mode;
  U8 buf[128],buf2[128],buf3[128],last_buf[128];
  prof_mode=PFM_OFF;
  if (pf_time_end)
    total_time=pf_time_end-pf_time_start;
  else
    total_time=sys_jiffies-pf_time_start;
  if (!total_time) total_time=1;
  switch (old_mode) {
    case PFM_BIN:
      *last_buf=0;
      for (i=0;i<pf_size;i++) {
	ip=pf_low+i*(pf_high-pf_low)/pf_size;
	SPrint(buf,"%p",ip);
	StrCpy(buf3,buf);
	StrFirstRem(buf3,"+",buf2);
	if (StrCmp(buf2,last_buf)) {
	  if (*last_buf && routine_total>=filter_cnt) {
	    "$$GREEN$$%3d.%02d%% %08X:",routine_total*100/total_time,
	      routine_total*10000/total_time%100,routine_total;
	    "%s\n$$FG$$",last_buf;
	  }
	  StrCpy(last_buf,buf2);
	  routine_total=0;
	}
	hits=pf_array[i];
	routine_total+=hits;
	if (hits>=filter_cnt) {
	  "%3d.%02d%% %08X:",hits*100/total_time,
	  hits*10000/total_time%100,hits;
	  if (!PutSrcLink(ip,ip-last_ip))
	    "%s\n",buf;
	  else
	    NewLine;
	  last_ip=ip;
	}
      }
      if (*last_buf && routine_total>=filter_cnt) {
	"$$GREEN$$%3d.%02d%% %08X:",routine_total*100/total_time,
	  routine_total*10000/total_time%100,routine_total;
	"%s\n$$FG$$",last_buf;
      }
      break;
    case PFM_ALL:
      QSortI64(pf_array,pf_ptr,&PFACompare);
      *last_buf=0;
      for (i=0;i<pf_ptr;i++) {
	ip=pf_array[i];
	hits=0;
	while (pf_array[i]==ip) {
	  hits++;
	  i++;
	  if (i>=pf_ptr) break;
	}
	SPrint(buf,"%p",ip);
	StrCpy(buf3,buf);
	StrFirstRem(buf3,"+",buf2);
	if (StrCmp(buf2,last_buf)) {
	  if (*last_buf && routine_total>=filter_cnt) {
	    "$$GREEN$$%3d.%02d%% %08X:",routine_total*100/total_time,
	      routine_total*10000/total_time%100,routine_total;
	    "%s\n$$FG$$",last_buf;
	  }
	  StrCpy(last_buf,buf2);
	  routine_total=0;
	}
	routine_total+=hits;
	if (hits>=filter_cnt) {
	  "%3d.%02d%% %08X:",hits*100/total_time,
	  hits*10000/total_time%100,hits;
	  if (!PutSrcLink(ip,ip-last_ip))
	    "%s\n",buf;
	  else
	    NewLine;
	  last_ip=ip;
	}
      }
      if (*last_buf && routine_total>=filter_cnt) {
	"$$GREEN$$%3d.%02d%% %08X:",routine_total*100/total_time,
	  routine_total*10000/total_time%100,routine_total;
	"%s\n$$FG$$",last_buf;
      }
      break;
  }
  "Total Time:%12.6fs\n",pf_tf-pf_t0;
}

#help_index ""
