#help_index "Info;Time/CPU Cycles;Processor"
class CCPURep
{
  Bool mp_start,mp_end;
  I64 mp_not_done_flags,
      **swaps,
      **cycles;
};

U0 MPCPURep(CCPURep *cr)
{
  I64 swaps=0,cycles_0,cycles_f;
  while (!cr->mp_start)
    Yield;
  cycles_0=GetTSC;
  while (!cr->mp_end) {
    swaps++;
    Yield;
  }
  cycles_f=GetTSC;
  cr->swaps[Gs->num]=swaps;
  cr->cycles[Gs->num]=cycles_f-cycles_0;
  LBtr(&cr->mp_not_done_flags,Gs->num);
}

public U0 CPURep(Bool full=FALSE)
{//Report number of cores and clock freq.
  I64 i,total_swaps,total_cycles;
  F64 t0,tf;
  CCPURep cr;

  if (!full)
    "%03X Cores %6h?nHz\n",mp_cnt,ToF64(cnts.time_stamp_freq);
  else {
    cr.swaps=CAlloc(sizeof(I64)*mp_cnt);
    cr.cycles=CAlloc(sizeof(I64)*mp_cnt);
    cr.mp_start=cr.mp_end=FALSE;
    cr.mp_not_done_flags=1<<mp_cnt-1;
    for (i=0;i<mp_cnt;i++)
      Spawn(&MPCPURep,&cr,NULL,i);

    t0=tS;
    cr.mp_start=TRUE;
    Sleep(2000);
    cr.mp_end=TRUE;
    while (cr.mp_not_done_flags)
      Yield;
    tf=tS;

    "\n%03X Cores %6h?nHz\n",mp_cnt,ToF64(cnts.time_stamp_freq);
    "\n         Context\n"
	  "CPU     Swaps/s         Cycles\n"
	  "--- ------------- -----------------\n";
    total_swaps=0;
    total_cycles=0;
    for (i=0;i<mp_cnt;i++) {
      " %02X %13,d %17,d\n",i,
	    ToI64(cr.swaps[i]/(tf-t0)),ToI64(cr.cycles[i]/(tf-t0));
      total_swaps+=cr.swaps[i];
      total_cycles+=cr.cycles[i];
    }
    "--- ------------- -----------------\n";
    "%03X %13,d %17,d\n",i,
	  ToI64(total_swaps/(tf-t0)),ToI64(total_cycles/(tf-t0));
    "Avg %13,d %17,d\n\n",
	  ToI64(total_swaps/(tf-t0)/i),ToI64(total_cycles/(tf-t0)/i);
    "Avg Cycles/Swap: %12.6f\n",ToF64(total_cycles)/total_swaps;
    "Avg Time/Swap  : %12.6fns\n\n",(tf-t0)*1000000000.0*i/total_swaps;
    Free(cr.swaps);
    Free(cr.cycles);
  }
}
