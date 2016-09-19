/*Allows you to time insts.
Results vary because the CPU does
parallel and out-of-order execution.
*/

#define SAMPLE_SIZE	10000000

U0 TimeIns()
{
  I64 i,start,end,overhead_time,test_time;
  F64 d=5.0;

  //Measure Loop Overhead
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++);
  end=GetTSC;
  overhead_time=end-start;
  "\nOverhead Cycles:%12.6f\n",ToF64(overhead_time)/SAMPLE_SIZE;

  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++) {
    #exe {PassTrace(0b10000000000);};
//Time these insts
    d*=1.00001;
    #exe {PassTrace(0);};
  }
  end=GetTSC;
  test_time=end-start;

  "Measured Cycles:%12.6f\n",ToF64(test_time-overhead_time)/SAMPLE_SIZE;
}

TimeIns;
TimeIns;
TimeIns;
