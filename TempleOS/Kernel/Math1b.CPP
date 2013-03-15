F64 Limit(F64 p,F64 lo,F64 hi)
{
  if (p<lo)
    return lo;
  if (p>hi)
    return hi;
  return p;
}

F64 Min(F64 n1,F64 n2)
{
  if (n1<=n2)
    return n1;
  else
    return n2;
}

F64 Max(F64 n1,F64 n2)
{
  if (n1>=n2)
    return n1;
  else
    return n2;
}

F64 Pow10I64(I64 i)
{
  if (i>308)
    return ì;
  else if (i<-308)
    return 0.0;
  else
    return pow10_I64[i+309];
}

U64 FloorU64(U64 num,U64 to)
{
  return num-num%to;
}

U64 CeilU64(U64 num,U64 to)
{
  num+=to-1;
  return num-num%to;
}

I64 RoundI64(I64 num,I64 to)
{
  return num-num%to;
}

I64 FloorI64(I64 num,I64 to)
{
  if (num>=0)
    return num-num%to;
  else {
    num++;
    return num-num%to-to;
  }
}

I64 CeilI64(I64 num,I64 to)
{
  if (num>=0) {
    num+=to-1;
    return num-num%to;
  } else {
    num+=to-1;
    return num-num%to-to;
  }
}

//See $LK-A,"::/TempleOS/Doc/Acknowledgements.TXT"$ (6).
#ifdef SYS_DETERMINISTIC
$FG,6$U16 RandU16()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u16[0];
}

I16 RandI16()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i16[0];
}

U32 RandU32()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u32[0];
}

I32 RandI32()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i32[0];
}

U64 RandU64()
{
  U64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

I64 RandI64()
{
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

F64 Rand()
{ //0.0<=x<1.0
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result&=0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}
#else
$FG,5$U16 RandU16()
{
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  return sys_rnd_seed.u16[0];
}

I16 RandI16()
{
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  return sys_rnd_seed.i16[0];
}

U32 RandU32()
{
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  return sys_rnd_seed.u32[0];
}

I32 RandI32()
{
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  return sys_rnd_seed.i32[0];
}

U64 RandU64()
{
  U64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  return result;
}

I64 RandI64()
{
  I64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  return result;
}

F64 Rand()
{ //0.0<=x<1.0
  I64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTimeStamp>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  result&=0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}
$FD$#endif

U16 Rand2U16()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u16[0];
}

I16 Rand2I16()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i16[0];
}

U32 Rand2U32()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u32[0];
}

I32 Rand2I32()
{
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i32[0];
}

U64 Rand2U64()
{
  U64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

I64 Rand2I64()
{
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

F64 Rand2()
{ //0.0<=x<1.0
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result&=0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}

U0 Randomize(I64 seed=0)
{ //Randomizes seed.
  Fs->rnd_seed=seed;
}
