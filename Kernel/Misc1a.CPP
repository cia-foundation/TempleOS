U0 PortNop()
{
  InU8(0x21); //This is an innoculous (reads IRQ Mask)
	      //access which should take
	      //fixed time because it's an ISA-bus standard.
	      //It takes 1.0uS-2.0uS
}

U16 EndianU16(U16 d)
{
  I64 result=0;
  result.u8[1]=d.u8[0];
  result.u8[0]=d.u8[1];
  return result;
}

U32 EndianU32(U32 d)
{
  I64 result=0;
  result.u8[3]=d.u8[0];
  result.u8[2]=d.u8[1];
  result.u8[1]=d.u8[2];
  result.u8[0]=d.u8[3];
  return result;
}

U64 EndianU64(U64 d)
{
  U64 result;
  result.u8[7]=d.u8[0];
  result.u8[6]=d.u8[1];
  result.u8[5]=d.u8[2];
  result.u8[4]=d.u8[3];
  result.u8[3]=d.u8[4];
  result.u8[2]=d.u8[5];
  result.u8[1]=d.u8[6];
  result.u8[0]=d.u8[7];
  return result;
}

I64 BCnt(I64 d)
{
  I64 result=0,i;
  for (i=0;i<8;i++)
    result+=set_bits_table[d.u8[i]];
  return result;
}

U0 IntSimpleTimer(CTask *)
{
  I64 i;
  if (mp_cnt>1)
    while (LBts(&sys_semas[SYS_SEMA_SYS_TIMER],0))
      PAUSE
  lock sys_jiffies++;
  sys_timer+=SYS_TIMER0_PERIOD+1;
  LBtr(&sys_semas[SYS_SEMA_SYS_TIMER],0);
  for (i=1;i<mp_cnt;i++)
    MPInt(I_TIMER,i);
}

I64 SysTimerRead()
{
  I64 i,result;
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
  result=sys_timer+SYS_TIMER0_PERIOD-i;
  LBtr(&sys_semas[SYS_SEMA_SYS_TIMER],0);
  POPFD
  return result;
}

I64 TaskTotalTime()
{
  return GetTSC-Fs->time_slice_start+Fs->total_time;
}

I64 TimeStampFreqCal()
{
  static I64 time_stamp_start=0,timer_start=0,HPET_start=0;
  I64 i;
  if (time_stamp_start) {
    PUSHFD
    CLI
    if (HPET_start) {
      time_stamp_freq=HPET_freq*(GetTSC-time_stamp_start);
      i=HPET-HPET_start;
    } else {
      time_stamp_freq=SYS_TIMER_FREQ*(GetTSC-time_stamp_start);
      i=SysTimerRead-timer_start;
    }
    if (!i)
      Dbg("Timer Cal Error");
    time_stamp_freq/=i;
    time_stamp_kHz_freq=time_stamp_freq/1000;
    POPFD
    time_stamp_freq_calibrated=TRUE;
  }
  PUSHFD
  CLI
  if (HPET_freq) {
    timer_start=0;
    HPET_start=HPET;
  } else {
    timer_start=SysTimerRead;
    HPET_start=0;
  }
  time_stamp_start=GetTSC;
  POPFD
  return time_stamp_freq;
}

F64 tS()
//Time since boot in seconds as a float
{ //This is slow, but accurate even if no HPET.
  if (HPET_freq)
    return ToF64(HPET-HPET_initial)/HPET_freq;
  else
    return SysTimerRead/ToF64(SYS_TIMER_FREQ);
}

Bool Blink(F64 Hz=2.5)
{
  if (!Hz) return 0;
  return ToI64(sys_jiffies*2*Hz/JIFFY_FREQ)&1;
}

U0 BusyWait(I64 ÊS) //Loosely timed
{
  I64 i;
  for (i=0;i<ÊS;i++)
    PortNop;
}

U0 SleepUntil(I64 wake_jiffy)
//This routine is not for power-saving
//it is to make a program pause without
//hogging the CCPU.
{
  Bool old_idle=LBts(&Fs->task_flags,TASKf_IDLE);
  Fs->wake_jiffy=wake_jiffy;
  Yield;
  LBEqu(&Fs->task_flags,TASKf_IDLE,old_idle);
}

U0 Sleep(I64 mS)
//This routine is not for power-saving
//it is to make a program pause without
//hogging the CCPU.
{
  if (!mS)
    Yield;
  else
    SleepUntil(sys_jiffies+mS*JIFFY_FREQ/1000);
}

U0 Snd(F64 freq)
{
  I64 p;
  CSndData *d;
  if (!Bt(&sys_semas[SYS_SEMA_MUTE],0) &&
	!LBts(&sys_semas[SYS_SEMA_SND],0)) {
    if (freq!=snd_freq) {
      snd_freq=freq;
      if (0<freq<=20000) {
	p=SYS_TIMER_FREQ/freq;
	OutU8(0x43,0xB6);
	OutU8(0x42,p);
	OutU8(0x42,p.u8[1]);
	OutU8(0x61,3|InU8(0x61));
      } else
	OutU8(0x61,InU8(0x61)&~3);
    }
    if (!IsDbg && snd_record) {
      d=ACAlloc(sizeof(CSndData));
      d->freq=freq;
      d->time_stamp=tS;
      QueIns(d,snd_record_root.last);
    }
    LBtr(&sys_semas[SYS_SEMA_SND],0);
  }
}

U0 Beep(I64 freq=500,Bool busy=FALSE)
{
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
{
  Bool result;
  if (val) {
    PUSHFD
    CLI
    Snd(0);
    result=LBts(&sys_semas[SYS_SEMA_MUTE],0);
    POPFD
  } else
    result=LBtr(&sys_semas[SYS_SEMA_MUTE],0);
  return result;
}

Bool IsMute() {
  return Bt(&sys_semas[SYS_SEMA_MUTE],0);
}

Bool Silent(Bool val) {
//This turns-off console output, not sound.
  return LBEqu(&Fs->display_flags,DISPLAYf_SILENT,val);
}

Bool IsSilent() {
  return Bt(&Fs->display_flags,DISPLAYf_SILENT);
}

Bool IsPreempt() {
  return Bt(&Fs->task_flags,TASKf_PREEMPT);
}

Bool SysDbg(Bool val) {
//Sets or rsts a bit which you can
//use while debugging.
  return LBEqu(&sys_semas[SYS_SEMA_DEBUG],0,val);
}

Bool IsSysDbg() {
  return Bt(&sys_semas[SYS_SEMA_DEBUG],0);
}

Bool Raw(Bool val) {
//Switches to direct screen output
//mode for when you don't have
//a working window.
  if (!val)
    VGAFlush;
  return !LBEqu(&Fs->task_flags,TASKf_NOT_RAW,!val);
}

Bool IsRaw() {
  return !Bt(&Fs->task_flags,TASKf_NOT_RAW);
}

Bool SingleUser(Bool val) {
  return LBEqu(&sys_semas[SYS_SEMA_SINGLE_USER],0,val);
}

Bool IsSingleUser() {
  return Bt(&sys_semas[SYS_SEMA_SINGLE_USER],0);
}

Bool InDbg(Bool val) {
  return LBEqu(&sys_semas[SYS_SEMA_IN_DEBUGGER],0,val);
}

Bool IsDbg() {
  return Bt(&sys_semas[SYS_SEMA_IN_DEBUGGER],0);
}

U0 ProgressBarsRst()
{
  MemSet(sys_progresses,0,sizeof(sys_progresses));
}

U0 CtrlAltDel(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_DEL);
}

U0 CtrlAltC(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_C);
}

U0 CtrlAltD(I64)
{
  if (!IsDbg) {
    if (Fs==Gs->idle_task)
      BptS(sys_winmgr_task->rip,sys_winmgr_task);
    else
      BptS(*ctrl_alt_let_ret_addr);
  }
}

U0 CtrlAltE(I64)
{
  SpawnUser;
}

U0 CtrlAltF(I64)
{
  VGAFlush;
}

U0 CtrlAltM(I64)
{
  Mute(!IsMute);
}

U0 CtrlAltT(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_TAB);
}

U0 CtrlAltX(I64)
{
  LBts(sys_ctrl_alt_flags,SYSf_CTRL_ALT_X);
}
