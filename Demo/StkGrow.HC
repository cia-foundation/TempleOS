I64 Recurse(I64 n)
{
  if (n)
    return 1+CallStkGrow(0x800,0x1000000,&Recurse,n-1);
  else
    return 0;
}

I64 Recurse2(I64 n)
{
  if (n)
    return 1+Recurse2(n-1);
  else
    return 0;
}

I64 Recurse3(I64 n,I64 lo,I64 hi)
{
  if (lo<=n<hi)
    return 1+Recurse3(n-1,lo,hi);
  else if (n)
    return Recurse3(n-1,lo,hi);
  else
    return 0;
}

#define DEPTH	(2*1024*1024)

U0 StkGrowDemo()
{
  F64 t0;

  t0=tS;
  "%X:%X\n",DEPTH,Recurse(DEPTH);
  "Time:%7.5fs\n",tS-t0;

  //If you know the max stack ahead of time...
  //Recurse2's stack is 16 because you have 1 arg,
  //a return addr and no local variables.
  t0=tS;
  "%X:%X\n",DEPTH,CallStkGrow(DEPTH*16+0x800,DEPTH*16+0x800,&Recurse2,DEPTH);
  "Time:%7.5fs\n",tS-t0;

  //$LK,"CallStkGrow",A="MN:CallStkGrow"$() works with multiple args.
  t0=tS;
  "%X:%X\n",DEPTH,CallStkGrow(DEPTH*32+0x800,DEPTH*32+0x800,
	&Recurse3,DEPTH,1000,2000);
  "Time:%7.5fs\n",tS-t0;
}

StkGrowDemo;

//Be careful because you can fragment memory.
