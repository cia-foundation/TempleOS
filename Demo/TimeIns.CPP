/*Allows you to time instructions.
Results vary because the CPU does
parallel and out-of-order execution.
*/

#define SAMPLE_SIZE	100000000

U0 TimeIns()
{
  I64 i,start,end,overhead_time,test_time;
  F64 d=5.0;
  Bool old_preempt=Preempt;

  //Measure Loop Overhead
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++);
  end=GetTSC;
  overhead_time=end-start;
  "\nOverhead Cycles:%10.5f\n",ToF64(overhead_time)/SAMPLE_SIZE;

  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++) {
    #exe {OTraceMask(0b10000000000);OTrace(ON);};
//Time these instructions
    d*=1.00001;
    #exe {OTrace(OFF);};
  }
  end=GetTSC;
  test_time=end-start;

  "Measured Cycles:%10.5f\n",ToF64(test_time-overhead_time)/SAMPLE_SIZE;
  Preempt(old_preempt);
}

TimeIns;
