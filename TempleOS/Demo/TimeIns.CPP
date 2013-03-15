/*
This program allows you to measure the
timing of various instructions.
Don't be surprised by odd results
because the CPU does instructions in
parallel sometimes and a simple
instruction might take place in parallel
with the loop instruction and it will
report zero clock cycles.
*/


#define SAMPLE_SIZE	100000000


U0 TimeIns()
{
  I64 i;
  I64 start1,end1,start2,end2,overhead_time,test_time;
  F64 reg d=5.0; //Use the reg directive to designate register vars
  Bool old_preempt=Preempt(OFF);

//Measure Loop Overhead
  start1=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++);
  end1=GetTimeStamp;
  overhead_time=end1-start1;


  start2=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++) {
#exe {OPassTraceMask(0b1000000000);OTrace(ON);};
//Put your test instructions here.
    d*=1.00001;
#exe {OTrace(OFF);};
  }
  end2=GetTimeStamp;
  test_time=end2-start2;

  "\n\n\nCycles %10.5f\n",ToF64(test_time-overhead_time)/SAMPLE_SIZE;
  Preempt(old_preempt);
}

TimeIns;
