class MPCtrl
{
  Bool done[MP_MAX_PROCESSORS];
} mp;

I32 *a1=NULL, //This is the master copy of random nums
	     //so we have the same nums for both tests.
   *a2=NULL, //This is the temp copy.
   *a3=NULL; //This is a merged final array for dual core example.

U0 Init(I64 n)
{
  I64 i;
  Free(a1);
  a1=MAlloc(n*sizeof(I32));
  for (i=0;i<n;i++)
    a1[i]=RandI32;
  Free(a2);
  a2=MAlloc(n*sizeof(I32));
  Free(a3);
  a3=MAlloc(n*sizeof(I32));
}

U0 CleanUp()
{
  Free(a1);
  a1=NULL;
  Free(a2);
  a2=NULL;
  Free(a3);
  a3=NULL;
}

U0 Output(I32 *a,I64 n)
{
  I64 i;
  WinMax;
  for (i=0;i<n;i++) {
    "%08X ",a[i];
    if (i&7==3)
      NewLine;
  }
  NewLine;
}

I64 IntCompare(I32 *i1,I32 *i2)
{
  return *i1-*i2;
}

F64 Sort1(I64 n)
{
  F64 t0=tNP;
  MemCpy(a2,a1,n*sizeof(I32));
  QSort(a2,n,sizeof(I32),&IntCompare);
  mp.done[Gs->num]=TRUE;
  return tNP-t0;
}

F64 Sort2(I64 n)
{
  I64 i1=0,i2=n/2,i3=0;
  F64 t0=tNP;
  MemSet(&mp,0,sizeof(MPCtrl));
  MemCpy(&a2[n/2],&a1[n/2],n/2*sizeof(I32));
  MPJob(&Sort1,n/2,1);
  QSort(&a2[n/2],n/2,sizeof(I32),&IntCompare);
  while (!mp.done[1])
    Yield;
  while (i3<n) {
    if (i2==n)
      while (i3<n)
	a3[i3++]=a2[i1++];
    else
      while (i3<n && i1<n/2 && a2[i1]<=a2[i2])
	a3[i3++]=a2[i1++];
    if (i1==n/2)
      while (i3<n)
	a3[i3++]=a2[i2++];
    else
      while (i3<n && i2<n && a2[i2]<=a2[i1])
	a3[i3++]=a2[i2++];
  }
  return tNP-t0;
}

#define SAMPLES	8

U0 CompareSorts(I64 n)
{
  I64 i;
  F64 t1=0,t2=0;
  for (i=0;i<SAMPLES;i++) {
    //The times depend heavily on the random num arrays
    //so run multiple times, to get an average.
    Init(n);

    t1+=Sort1(n);
//    Output(a2,n);

    t2+=Sort2(n);
//    Output(a3,n);
  }
  CleanUp;
  "Array N:%08X  Timings Sampled:%d\n",n,SAMPLES;
  "Single Core Average Time:%9.5fs\n",t1/SAMPLES;
  "Dual Core Average Time  :%9.5fs\n",t2/SAMPLES;
  NewLine;
}

CompareSorts(0x4000);
CompareSorts(0x8000);
CompareSorts(0x10000);
