F64 Clamp(F64 num,F64 lo,F64 hi)
{//Clamp to F64 [] range.
  if (num<lo)
    return lo;
  if (num>hi)
    return hi;
  return num;
}

F64 Min(F64 n1,F64 n2)
{//Min of two F64s.
  if (n1<=n2)
    return n1;
  else
    return n2;
}

F64 Max(F64 n1,F64 n2)
{//Max of two F64s.
  if (n1>=n2)
    return n1;
  else
    return n2;
}

F64 Pow10I64(I64 i)
{//F64 integer powers of ten.
  if (i>308)
    return ì;
  else if (i<-308)
    return 0.0;
  else
    return pow10_I64[i+309];
}

U64 FloorU64(U64 num,U64 to)
{//Integer multiples of num.
  return num-num%to;
}

U64 CeilU64(U64 num,U64 to)
{//Integer multiples of num.
  num+=to-1;
  return num-num%to;
}

I64 RoundI64(I64 num,I64 to)
{//Integer multiples of num.
  return num-num%to;
}

I64 FloorI64(I64 num,I64 to)
{//Integer multiples of num.
  if (num>=0)
    return num-num%to;
  else {
    num++;
    return num-num%to-to;
  }
}

I64 CeilI64(I64 num,I64 to)
{//Integer multiples of num.
  if (num>=0) {
    num+=to-1;
    return num-num%to;
  } else {
    num+=to-1;
    return num-num%to-to;
  }
}

//See $LK,"::/Doc/Acknowledgements.TXT"$.
I16 RandI16()
{//Random I16. Xor with TimsStamp.
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  return sys_rnd_seed.i16[0];
}

U16 RandU16()
{//Random U16. Xor with TimsStamp.
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  return sys_rnd_seed.u16[0];
}

I32 RandI32()
{//Random I32. Xor with TimsStamp.
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  return sys_rnd_seed.i32[0];
}

U32 RandU32()
{//Random U32. Xor with TimsStamp.
  sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  return sys_rnd_seed.u32[0];
}

I64 RandI64()
{//Random I64. Xor with TimsStamp.
  I64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  return result;
}

U64 RandU64()
{//Random U64. Xor with TimsStamp.
  U64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  return result;
}

F64 Rand()
{//Random F64. Xor with TimsStamp. 0.0<=x<1.0
  I64 result;
  result.u32[0]=sys_rnd_seed.i32[0]=(1664525*sys_rnd_seed.i32[0]+1013904223)^GetTSC>>4;
  result.u32[1]=sys_rnd_seed.i32[0]= 1664525*sys_rnd_seed.i32[0]+1013904223;
  result&=0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}

I16 Rand2I16()
{//Random I16. No time input.
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i16[0];
}

U16 Rand2U16()
{//Random U16. No time input.
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u16[0];
}

I32 Rand2I32()
{//Random I32. No time input.
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.i32[0];
}

U32 Rand2U32()
{//Random U32. No time input.
  Fs->rnd_seed.i32[0]=1664525*Fs->rnd_seed.i32[0]+1013904223;
  return Fs->rnd_seed.u32[0];
}

I64 Rand2I64()
{//Random I64. No time input.
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

U64 Rand2U64()
{//Random U64. No time input.
  U64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  return result;
}

F64 Rand2()
{//Random F64. No time input. 0.0<=x<1.0
  I64 result;
  result.u32[0]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result.u32[1]=Fs->rnd_seed.i32[0]= 1664525*Fs->rnd_seed.i32[0]+1013904223;
  result&=0x3FFFFFFFFFFFFFFF;
  return result/ToF64(0x4000000000000000);
}

U0 Randomize(I64 seed=0)
{//Randomizes $LK,"CTask",A="MN:CTask"$ seed.
  if (seed)
    Fs->rnd_seed=seed;
  else
    Fs->rnd_seed=GetTSC>>4;
}
