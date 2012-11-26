/*

$WW,1$You may wonder why I use "&" instead of "%".
$WW,0$
*/

#define SAMPLE_SIZE	100000000

U0 TimeIns()
{
  I64 start1,end1,start2,end2,overhead_time,test_time;
  I64 reg i,reg temp;
  Bool old_preempt=Preempt(OFF);

  CPURep;

//Measure Loop Overhead
  start1=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++) {
  }
  end1=GetTimeStamp;
  overhead_time=end1-start1;

//Measure remainder...
  start2=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++)
     temp=i%0x400;
  end2=GetTimeStamp;
  test_time=end2-start2;
  "Remainder Version #1 Cycles\t: %10.5f\n",
    ToF64(test_time-overhead_time)/SAMPLE_SIZE;

//Measure remainder...
  start2=GetTimeStamp;
  for (i=0;i<SAMPLE_SIZE;i++)
     temp=i&0x3FF;
  end2=GetTimeStamp;
  test_time=end2-start2;
  "Remainder Version #2 Cycles\t: %10.5f\n",
    ToF64(test_time-overhead_time)/SAMPLE_SIZE;

  Preempt(old_preempt);
}

TimeIns;

/*  Program Output$HL,0$$WW+H,1$$FD,1$

8 Cores 2.660GHz
Remainder Version #1 Cycles	:   26.85345
Remainder Version #2 Cycles	:   -0.00800

$HL,1$*/
