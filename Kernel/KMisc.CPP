U0 PortNop()
{//Innoculous (reads IRQ Mask) which should take fixed time
//because it's an ISA-bus standard.  It takes 1.0uS-2.0uS.
  InU8(0x21);
}

U16 EndianU16(U16 d)
{//Swap big<-->little endian.
  I64 res=0;
  res.u8[1]=d.u8[0];
  res.u8[0]=d.u8[1];
  return res;
}

U32 EndianU32(U32 d)
{//Swap big<-->little endian.
  I64 res=0;
  res.u8[3]=d.u8[0];
  res.u8[2]=d.u8[1];
  res.u8[1]=d.u8[2];
  res.u8[0]=d.u8[3];
  return res;
}

I64 EndianI64(I64 d)
{//Swap big<-->little endian.
  U64 res;
  res.u8[7]=d.u8[0];
  res.u8[6]=d.u8[1];
  res.u8[5]=d.u8[2];
  res.u8[4]=d.u8[3];
  res.u8[3]=d.u8[4];
  res.u8[2]=d.u8[5];
  res.u8[1]=d.u8[6];
  res.u8[0]=d.u8[7];
  return res;
}

I64 BCnt(I64 d)
{//Count set bits in I64.
  I64 res=0,i;
  for (i=0;i<8;i++)
    res+=set_bits_table[d.u8[i]];
  return res;
}

U0 IntCore0TimerHandler(CTask *)
{//Called from $LK,"IntCore0TimerHandler",A="FF:::/Kernel/KInts.CPP,IntCore0TimerHandler"$
  I64 i;
  if (mp_cnt>1)
    while (LBts(&sys_semas[SYS_SEMA_SYS_TIMER],0))
      PAUSE
  lock cnts.jiffies++;
  cnts.timer+=SYS_TIMER0_PERIOD+1;
  LBtr(&sys_semas[SYS_SEMA_SYS_TIMER],0);
  for (i=1;i<mp_cnt;i++)
    MPInt(I_TIMER,i);
  OutU8(0x20,0x20); //Acknowledge PIC Interrupt
}

I64 SysTimerRead()
{//System timer count with overflow already handled.
  I64 i,res;
  PUSHFD
  CLI
  if (mp_cnt>1)
    while (LBts(&sys_semas[SYS_SEMA_SYS_TIMER],0))
      PAUSE
  OutU8(0x43,0); //Latch Timer0
  if ((i=InU8(0x40)+InU8(0x40)<<8)==SYS_TIMER0_PERIOD) {
    if (InU8(0x20) & 1)
      i=-1;
  }
  res=cnts.timer+SYS_TIMER0_PERIOD-i;
  LBtr(&sys_semas[SYS_SEMA_SYS_TIMER],0);
  POPFD
  return res;
}

I64 TimeStampFreqCal()
{
  static I64 time_stamp_start=0,timer_start=0,HPET_start=0;
  I64 i;
  if (time_stamp_start) {
    PUSHFD
    CLI
    if (HPET_start) {
      cnts.time_stamp_freq=cnts.HPET_freq*(GetTSC-time_stamp_start);
      i=HPET-HPET_start;
    } else {
      cnts.time_stamp_freq=SYS_TIMER_FREQ*(GetTSC-time_stamp_start);
      i=SysTimerRead-timer_start;
    }
    if (!i)
      AdamErr("Timer Cal Error");
    else {
      cnts.time_stamp_freq/=i;
      cnts.time_stamp_kHz_freq=cnts.time_stamp_freq/1000;
      cnts.time_stamp_calibrated=TRUE;
    }
    POPFD
  }
  PUSHFD
  CLI
  if (cnts.HPET_freq) {
    timer_start=0;
    HPET_start=HPET;
  } else {
    timer_start=SysTimerRead;
    HPET_start=0;
  }
  time_stamp_start=GetTSC;
  POPFD
  return cnts.time_stamp_freq;
}

F64 tS()
{//Time since boot in seconds as a float.
  if (cnts.HPET_freq)
    return ToF64(HPET-cnts.HPET_initial)/cnts.HPET_freq;
  else
    return SysTimerRead/ToF64(SYS_TIMER_FREQ);
}

Bool Blink(F64 Hz=2.5)
{//Return TRUE, then FALSE, then TRUE at given frequency.
  if (!Hz) return 0;
  return ToI64(cnts.jiffies*2*Hz/JIFFY_FREQ)&1;
}

U0 BusyWait(I64 æS)
{//Loosely timed.
  I64 i;
  if (cnts.HPET_freq) {
    i=HPET+cnts.HPET_freq*æS/1000000;
    while (HPET<i);
  } else
    for (i=0;i<æS;i++)
      PortNop;
}

U0 SleepUntil(I64 wake_jiffy)
{//Not for power-saving.  It is to make a program pause without hogging the CPU.
  Bool old_idle=LBts(&Fs->task_flags,TASKf_IDLE);
  Fs->wake_jiffy=wake_jiffy;
  Yield;
  LBEqu(&Fs->task_flags,TASKf_IDLE,old_idle);
}

U0 Sleep(I64 mS)
{//Not for power-saving.  It is to make a program pause without hogging the CPU.
  if (!mS)
    Yield;
  else
    SleepUntil(cnts.jiffies+mS*JIFFY_FREQ/1000);
}

U0 Snd(F64 freq)
{//Play freq.  Zero means off.
  I64 period;
  CSndData *d;
  if (!Bt(&sys_semas[SYS_SEMA_MUTE],0) &&
	!LBts(&sys_semas[SYS_SEMA_SND],0)) { //Mutex. Just throw-out if in use
    if (freq!=snd.freq) {
      snd.freq=freq;
      if (0<freq<=20000) {
	period=SYS_TIMER_FREQ/freq;
	OutU8(0x43,0xB6);
	OutU8(0x42,period);
	OutU8(0x42,period.u8[1]);
	OutU8(0x61,3|InU8(0x61));
      } else
	OutU8(0x61,InU8(0x61)&~3);
    }
    if (!IsDbg && snd.record) {
      d=ACAlloc(sizeof(CSndData));
      d->freq=freq;
      d->time=tS;
      QueIns(d,snd.record_root.last);
    }
    LBtr(&sys_semas[SYS_SEMA_SND],0);
  }
}

U0 Beep(I64 freq=500,Bool busy=FALSE)
{//Make beep at given freq.
  Snd(freq);
  if (busy)
    BusyWait(500000);
  else
    Sleep(500);
  Snd(0);
  if (busy)
    BusyWait(200000);
  else
    Sleep(200);
}

Bool Mute(Bool val)
{//Turn-off or on sound.
  Bool res;
  if (val) {
    PUSHFD
    CLI
    Snd(0);
    res=LBts(&sys_semas[SYS_SEMA_MUTE],0);
    POPFD
  } else
    res=LBtr(&sys_semas[SYS_SEMA_MUTE],0);
  return res;
}

Bool IsMute()
{//Return is-mute flag.
  return Bt(&sys_semas[SYS_SEMA_MUTE],0);
}

Bool Silent(Bool val=ON)
{//Turns-off StdOut console text output, not sound.
  return LBEqu(&Fs->display_flags,DISPLAYf_SILENT,val);
}

Bool IsSilent()
{//Is StdOut turned-off?
  return Bt(&Fs->display_flags,DISPLAYf_SILENT);
}

Bool SysDbg(Bool val)
{//Sets or resets sysdbg bit you can use while debugging.
  return LBEqu(&sys_semas[SYS_SEMA_DEBUG],0,val);
}

Bool IsSysDbg()
{//Is sysdbg bit set?
  return Bt(&sys_semas[SYS_SEMA_DEBUG],0);
}

Bool Raw(Bool val)
{//Switches to direct screen, non-windowed output.
  if (!val)
    VGAFlush;
  return !LBEqu(&Fs->display_flags,DISPLAYf_NOT_RAW,!val);
}

Bool IsRaw()
{//Are we in BLACK & WHITE raw screen mode?
  return !Bt(&Fs->display_flags,DISPLAYf_NOT_RAW);
}

Bool SingleUser(Bool val)
{//Sets or resets single-user mode bit.
  return LBEqu(&sys_semas[SYS_SEMA_SINGLE_USER],0,val);
}

Bool IsSingleUser()
{//Are we in single-user mode?
  return Bt(&sys_semas[SYS_SEMA_SINGLE_USER],0);
}

Bool InDbg(Bool val)
{//Set in-debugger flag.
  return LBEqu(&sys_semas[SYS_SEMA_IN_DEBUGGER],0,val);
}

Bool IsDbg()
{//Return in-debugger flag.
  return Bt(&sys_semas[SYS_SEMA_IN_DEBUGGER],0);
}

U0 ProgressBarsRst()
{
  MemSet(sys_progresses,0,sizeof(sys_progresses));
}
