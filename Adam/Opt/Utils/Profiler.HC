#help_index "Debugging/Profiler;Profiler;Cmd Line (Typically)/Profiler"
#help_file "::/Doc/Profiler"

#define PF_ARRAY_CNT 0x100000
I64 pf_jiffy_start,pf_jiffy_end;
I64 *pf_array=NULL;
I64 pf_cpu=0;
I64 pf_buf_in_ptr=0,pf_depth;
I64 pf_prof_active=0;

U0 ProfTimerInt(CTask *task)
{//See $LK,"profiler_timer_irq",A="FF:::/Kernel/KInts.HC,profiler_timer_irq"$.
  I64 i,k;
  if (Bt(&pf_prof_active,0))
    for (k=0;k<=pf_depth;k++) {
      if (task==Gs->idle_task)
	i=SYS_IDLE_PT;
      else
	i=TaskCaller(task,k,TRUE);
      if (pf_buf_in_ptr<PF_ARRAY_CNT) {
	pf_array[pf_buf_in_ptr++]=i;
	pf_jiffy_end=cnts.jiffies;
      }
    }
}

public U0 Prof(I64 depth=0,I64 cpu_num=0)
{/*Start collecting profiler statistics.
Profilers report where time is spent
by sampling RIP during the $TX,"1000Hz",D="DD_JIFFY_HZ"$
timer interrupt.

Do a $LK,"ProfRep",A="MN:ProfRep"$(), (profiler report)
after you have collected data.
*/
  if (!(0<=cpu_num<mp_cnt))
    ST_ERR_ST "Invalid CPU\n";
  else {
    cpu_structs[pf_cpu].profiler_timer_irq=NULL;
    pf_cpu=cpu_num;

    pf_depth=depth;
    pf_buf_in_ptr=0;
    if (!pf_array)
      pf_array=AMAlloc(sizeof(I64)*PF_ARRAY_CNT);
    pf_jiffy_end=pf_jiffy_start=cnts.jiffies;
    LBts(&pf_prof_active,0);
    cpu_structs[pf_cpu].profiler_timer_irq=&ProfTimerInt;
  }
}

I64 ProfCompare(U8 *i1,U8 *i2)
{
  return i1-i2;
}

public U0 ProfRep(I64 filter_cnt=1,Bool leave_it=OFF)
{//Profiler report. Call $LK,"Prof",A="MN:Prof"$() first and collect data.
  I64 i,hits,rip,last_rip=0,routine_total=0;
  F64 total_time;
  U8 buf[256],buf2[256],last_buf[256];
  if (!LBtr(&pf_prof_active,0))
    "Profiler Not Active\n";
  if (!pf_buf_in_ptr)
    "No Profiler Statistic\n";
  else {
    if (!(total_time=pf_jiffy_end-pf_jiffy_start))
      total_time=1;
    QSortI64(pf_array,pf_buf_in_ptr,&ProfCompare);
    *last_buf=0;
    for (i=0;i<pf_buf_in_ptr;i+=hits) {
      rip=pf_array[i];
      hits=0;
      do hits++;
      while (i+hits<pf_buf_in_ptr && pf_array[i+hits]==rip);

      StrPrint(buf,"%p",rip);
      StrFirstRem(buf,"+",buf2);
      if (StrCmp(buf2,last_buf)) {
	if (*last_buf && routine_total>=filter_cnt)
	  "$$GREEN$$%6.2f %08X:%s\n$$FG$$",100*routine_total/total_time,
		routine_total,last_buf;
	StrCpy(last_buf,buf2);
	routine_total=0;
      }
      routine_total+=hits;
      if (hits>=filter_cnt) {
	"%6.2f %08X:%P\n",100*hits/total_time,hits,rip;
	last_rip=rip;
      }
    }
    if (*last_buf && routine_total>=filter_cnt)
      "$$GREEN$$%6.2f %08X:%s\n$$FG$$",100*routine_total/total_time,
	    routine_total,last_buf;
    "Total Time:%0.6fs\n",total_time/JIFFY_FREQ;
    if (leave_it) {
      cpu_structs[pf_cpu].profiler_timer_irq=&ProfTimerInt;
      LBts(&pf_prof_active,0);
    } else
      cpu_structs[pf_cpu].profiler_timer_irq=NULL;
  }
}
