/*
On an 8-core machine, this takes the top 3-bits
of random numbers and distributes them to the 8 cores
for sorting.  Then, it merge sorts them.
*/

#define NUM	1000000

I64 my_mp_cnt=1<<Bsr(mp_cnt);//Power of 2

I32 *arg1,*arg2;
I32 *b[my_mp_cnt],bn[my_mp_cnt];
I64 mp_not_done_flags;

I64 Compare(I32 *e1,I32 *e2)
{
  return *e1-*e2;
}

U0 QSortU32(I32 *base,I64 num)
{//By customizing, we dramatically improve it!
//Cut and paste from $LK,"QSortI64",A="MN:QSortI64"$().
  I64 i;
  I32 *less,*greater,pivot;
  if (num>1) {
    do {
      less=base;
      greater=base+num;
      pivot=base[num/2];
      while (less<greater) {
	if (*less<=pivot)
	  less++;
	else {
	  greater--;
	  SwapU32(less,greater);
	}
      }
      i=less-base;
      if (i==num) {//All less or equ to pivot

	//Point greater to first less
	do greater--;
	while (--i && *greater==pivot);

	if (i) {
	  less=base+num/2; //Pivot was not moved, point to it
	  if (less<greater)
	    SwapU32(less,greater);
	  num=i;
	} else //All equ
	  break;
      } else if (i<num/2) {
	QSortU32(base,i);
	num-=i;
	base=greater;
      } else {
	QSortU32(greater,num-i);
	num=i;
      }
    } while (num>1);
  }
}

U0 MPSort(I64 dummy=0)
{
  no_warn dummy;
  QSortU32(b[Gs->num],bn[Gs->num]);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 MPRadixSortDemo(I64 dummy=0)
{
  no_warn dummy;
  I64 i,j,k1,k2;
  F64 t0;
  arg1=MAlloc(NUM*sizeof(I32));
  for (i=0;i<NUM;i++)
    arg1[i]=RandI32;

  arg2=MAlloc(NUM*sizeof(I32));

  "$$GREEN$$QSort$$FG$$\n";
  t0=tS;
  MemCpy(arg2,arg1,sizeof(I32)*NUM);
  QSort(arg2,NUM,sizeof(I32),&Compare);
  "Time:%9.6f\n",tS-t0;
  D(arg2+NUM/4);

  "$$GREEN$$QSortU32$$FG$$\n";
  t0=tS;
  MemCpy(arg2,arg1,sizeof(I32)*NUM);
  QSortU32(arg2,NUM);
  "Time:%9.6f\n",tS-t0;
  D(arg2+NUM/4);

  for (i=0;i<my_mp_cnt;i++) {
//$WW,0$We must do full size, just in case.
    //There will be uneven split between cores
    //depending on the distribution of rand numbers.
    b[i]=MAlloc(NUM*sizeof(I32));
    bn[i]=0;
  }

  if (my_mp_cnt<2) throw('MultCore');

  "$$GREEN$$MP Radix QSortU32$$FG$$\n";
  t0=tS;
  k1=32-Bsr(my_mp_cnt);
  k2=my_mp_cnt/2;
  for (i=0;i<NUM;i++) {
    j=arg1[i]>>k1+k2; //This is a preliminary radix sort.
    b[j][bn[j]++]=arg1[i];
  }
  mp_not_done_flags=1<<my_mp_cnt-1;
  for (i=0;i<my_mp_cnt;i++)
    Spawn(&MPSort,NULL,NULL,i);
  while (mp_not_done_flags)
    Yield;
  j=0;
  for (i=0;i<my_mp_cnt;i++) {
    MemCpy(&arg2[j],b[i],bn[i]*sizeof(I32));
    j+=bn[i];
  }
  "Time:%9.6f\n",tS-t0;
  D(arg2+NUM/4);

  Free(arg1);
  Free(arg2);
  for (i=0;i<my_mp_cnt;i++)
    Free(b[i]);
}

MPRadixSortDemo;

/*$HL,0$ Results on 8 Cores 3.397GHz Core i7:
$FG,2$QSort$FG$
Time: 0.759998
$FG,2$QSortU32$FG$
Time: 0.093684
$FG,2$MP Radix QSortU32$FG$
Time: 0.045450
$HL,1$*/
