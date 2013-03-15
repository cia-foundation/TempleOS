asm {
	ALIGN	16,OC_NOP
USE16
//See $LK,"TempleOS MultiCore","FI:::/TempleOS/Doc/MultiCore.TXT"$.

//This code gets copied to $LK,"MP_VECT_ADDR","MN:MP_VECT_ADDR"$.
//See $LK,"MemCpy(MP_VECT_ADDR","FF:::/TempleOS/Kernel/MultiProc.CPP,MemCpy(mp:2"$.
SYS_MP_INIT_START::
	JMP	MP2_START2
	ALIGN	4,OC_NOP
MP2_SYS_TEMP_PTR:	DU32	0,0;

MP2_START2:
	CLI

	WBINVD

	MOV	AX,MP_VECT_ADDR/16
	MOV	DS,AX

//See $LK,"mp->sys_temp_ptr","FF:::/TempleOS/Kernel/MultiProc.CPP,mp->sys_temp_ptr:2"$
#assert MP2_SYS_TEMP_PTR-SYS_MP_INIT_START==CMPMain.sys_temp_ptr
	LGDT	U32 [CMPMain.sys_temp_ptr]

	MOV	EAX,SYS_START_CR0
	MOV_CR0_EAX

	DU8	0x66,0xEA;		 //JMP GDT_CS_SEL:MP_INIT_OS
	DU32	MP_INIT_OS;
	DU16	GDT_CS_SEL;

SYS_MP_INIT_END::
USE32
MP_INIT_OS:
	MOV	AX,GDT_ZERO_DS_SEL
	MOV	DS,AX
	MOV	ES,AX
	MOV	FS,AX
	MOV	GS,AX
	MOV	SS,AX

@@05:	LOCK
	BTS	U32 [SYS_MP_CNT_LOCK],0
	JC	@@05

	MOV	ESI,U32 [SYS_MP_CNT_INITIAL]
	LOCK
	INC	U32 [SYS_MP_CNT_INITIAL]
	LOCK
	BTR	U32 [SYS_MP_CNT_LOCK],0


	CMP	ESI,MP_MAX_PROCESSORS
	JAE	I32 _SYS_HLT

	IMUL2	ESI,sizeof(CCPU)
	ADD	ESI,U32 [SYS_CPU_STRUCTS]

	LEA	EAX,U32 CCPU.start_stk+sizeof(CCPU.start_stk)[ESI]
	MOV	ESP,EAX
	PUSH	U32 SYS_START_RFLAGS
	POPFD
	PUSH	U32 0
	CALL	INIT_EM64T
USE64
	FNINIT

	PUSH	RSI
	PUSH	RSI
	CALL	_SET_GS_BASE
	POP	RSI
@@10:	MOV	RBX,U64 CCPU.seth_task[RSI]
	OR	RBX,RBX
	JZ	@@10
	MOV	U64 CTask.gs[RBX],RSI
	MOV	RAX,RBX
	CALL	SET_FS_BASE

	JMP	I32 SYS_CONTEXT_RESTORE
}

U0 TSSBusy(I64 tr,Bool val=OFF)
{
  LBEqu((&sys_gdttab)(U8 *)+tr+4,9,val);
}

CTSS *TSSNew(I64 cpu_num)
{
  U32 *d,*d1;
  CTSS *tss=CAlloc(sizeof(CTSS));
  tss->io_map_offset=offset(CTSS.io_map);
  MemSet(tss->io_map,0xFF,0x10000/8);

  tss->st0=MAlloc(INTERRUPT_STK);
  tss->rsp0=tss->st0(U8 *)+MSize(tss->st0);
  tss->st1=MAlloc(INTERRUPT_STK);
  tss->rsp1=tss->st1(U8 *)+MSize(tss->st1);
  tss->st2=MAlloc(INTERRUPT_STK);
  tss->rsp2=tss->st2(U8 *)+MSize(tss->st2);

  tss->tr      =GDT_CPU_TR_SELS+cpu_num*16;
  tss->tr_ring3=GDT_CPU_TR_RING3_SELS+cpu_num*16;

  d=(&sys_gdttab)(U8 *)+tss->tr;
  d1=d(U8 *)+4;
  *d =0x0000FFFF;
  *d1=0x008F8900;
  d(U8 *)+=2;
  *d|=tss & 0x00FFFFFF;
  *d1++|=tss & 0xFF000000;
  *d1++=tss>>32;
  *d1=0;

  d=(&sys_gdttab)(U8 *)+tss->tr_ring3;
  d1=d(U8 *)+4;
  *d =0x0000FFFF;
  *d1=0x008FE900;
  d(U8 *)+=2;
  *d|=tss & 0x00FFFFFF;
  *d1++|=tss & 0xFF000000;
  *d1++=tss>>32;
  *d1=0;

  return tss;
}

U0 MPCmdCtrlInit(CMPCmdCtrl *ctrl)
{
  MemSet(ctrl,0,sizeof(CMPCmdCtrl));
  QueInit(ctrl);
  ctrl->next_done=ctrl->last_done=&ctrl->next_done;
}

CTask *TaskIdleInit()
{
  CTask *task=Spawn(0,NULL,"Idle Task",Fs,,0);
  Bts(&task->task_flags,TASKf_IDLE);
  return task;
}

CCPU *CPUStructInit(I64 num,CCPU *c)
{
  MemSet(c,0,sizeof(CCPU));
  c->addr=c;
  c->num=num;
  c->idle_factor=0.01;
  MPCmdCtrlInit(&c->cmd_ctrl);
  if (num)
    c->idle_task=TaskIdleInit;
  else
    c->timer_irq_chain=&IntSimpleTimer;
  return c;
}

U0 MPInt(U8 num,I64 cpu_num=1)
{
  if (cpu_num>=mp_cnt) {
    if (!Bt(&sys_run_level,RLf_MP))
      return;
    else
      throw(EXCEPT_MULTICORE,0);
  }
  PushFD;
  Cli; //multitasking safe because each core has a local apic and IRQ's are off
  while (*LAPIC_ICR_LOW(U32 *)&0x1000)
    Pause;
  *LAPIC_ICR_HIGH(U32 *)=mp_apic_ids[cpu_num]<<24;
  *LAPIC_ICR_LOW(U32 *)=0x4000+num;
  PopFD;
}

U0 MPIntAll(U8 num)
{ //(All but self)
  PushFD;
  Cli; //multitasking safe because each core has a local apic and IRQ's are off
  while (*LAPIC_ICR_LOW(U32 *)&0x1000)
    Pause;
  *LAPIC_ICR_LOW(U32 *)=0xC4800+num;
  PopFD;
}

U0 MPNMInt()
{
  *LAPIC_ICR_LOW(U32 *)=0xC4400;
}

U0 MPHalt()
{
  mp_cnt=1;
  MPNMInt; //Hlt All other processors
  BusyWait(10000);
}

U0 MPWake(I64 target_cpu_mask)
{
  I64 i;
  target_cpu_mask&=1<<mp_cnt-1;
  Btr(&target_cpu_mask,Gs->num);
  PushFD;
  Cli;
  for (i=Bsf(target_cpu_mask);target_cpu_mask;i++)
    if (Btr(&target_cpu_mask,i)) {
      if (Bt(&cpu_structs[i].cpu_flags,CPUf_SETH_SLEEP))
	cpu_structs[i].seth_task->wake_jiffy=0;
      MPInt(I_WAKE,i);
      if (Bt(&cpu_structs[i].cpu_flags,CPUf_SETH_SLEEP)) //In case went back to sleep
	cpu_structs[i].seth_task->wake_jiffy=0;
    }
  PopFD;
}

U0 MPAPICInit()
{ //Called by adam during start-up
  //and other cores during initialization
  //after $LK,"MPStart","MN:MPStart"$().
  *LAPIC_SVR(U32 *)|=LAPICF_APIC_ENABLED;
  mp_apic_ids[Gs->num]=*LAPIC_APIC_ID(U32 *)>>24;
  *LAPIC_LDR(U32 *)=mp_apic_ids[Gs->num]<<24;
  *LAPIC_DFR(U32 *)=0xF0000000;
/*
  MemSet(LAPIC_IRR,0,0x20);
  MemSet(LAPIC_ISR,0,0x20);
  MemSet(LAPIC_TMR,0,0x20);
*/

  SetRAX(Gs->tss->tr);
  asm {	LTR	AX  }
  if (Gs->num) {
    IntInit1;
    SetFlags(SYS_NORMAL_RFLAGS);
  }
}

#assert !offset(CMPCmdCtrl.next_waiting)

Bool MPCmdCtrlDo(CMPCmdCtrl *ctrl)
{ //Called while locked
  CMPCmd *tempm=ctrl->next_waiting;
  while (tempm!=ctrl && !Bt(&tempm->target_cpu_mask,Gs->num))
    tempm=tempm->next;
  if (tempm!=ctrl) {
    QueRem(tempm);
    LBts(&tempm->flags,MPCf_DISPATCHED);
    tempm->handler_cpu=Gs->num;
    LBtr(&ctrl->flags,MPCCf_LOCKED);
    switch (tempm->cmd_code) {
      case MPCT_CALL:
	tempm->result=(*tempm->add)(tempm->data);
	Preempt(OFF);
	Sti;
	break;
      case MPCT_SPAWN_TASK:
	if (tempm->desc)
	  tempm->task=Spawn(tempm->add,tempm->data,tempm->desc);
	else
	  tempm->task=Spawn(tempm->add,tempm->data,"MP Job");
	break;
    }
    if (Bt(&tempm->flags,MPCf_FREE_ON_COMPLETE)) {
      Free(tempm->desc);
      Free(tempm);
    } else {
      while (LBts(&ctrl->flags,MPCCf_LOCKED))
	Yield;
      QueIns(tempm,ctrl->last_done);
      LBts(&tempm->flags,MPCf_DONE);
      LBtr(&ctrl->flags,MPCCf_LOCKED);
    }
    return TRUE;
  } else {
    LBtr(&ctrl->flags,MPCCf_LOCKED);
    return FALSE;
  }
}

U0 MPSethServerTask()
{
  CMPCmdCtrl *my_ctrl=&Gs->cmd_ctrl;
  Sti;
  while (TRUE) {
    TaskFinishOffDying;
    if (!LBts(&my_ctrl->flags,MPCCf_LOCKED) &&
	!MPCmdCtrlDo(my_ctrl) && //Jobs for this core are highest priority.
	!LBts(&mp_ctrl->flags,MPCCf_LOCKED))
	MPCmdCtrlDo(mp_ctrl);
    if (my_ctrl->next_waiting==my_ctrl &&
	mp_ctrl->next_waiting==mp_ctrl) {
      LBts(&Gs->cpu_flags,CPUf_SETH_SLEEP);
      Sleep(20); //We don't want this too long because of need for $LK,"TaskFinishOffDying","MN:TaskFinishOffDying"$().
      LBtr(&Gs->cpu_flags,CPUf_SETH_SLEEP);
    }
  }
}

CMPCmd *MPQueJob(I64 (*fp_add)(U8 *data),U8 *data=NULL,
       U8 *desc=NULL,
       I64 flags=1<<MPCf_FREE_ON_COMPLETE,
       Bool spawn=FALSE,I64 target_cpu_mask=ALL_MASK)
{
  I64 i=Bsf(target_cpu_mask);
  CMPCmdCtrl *ctrl;
  CMPCmd *tempm;
  if (mp_cnt<2 || i<0) throw(EXCEPT_MULTICORE,1);
  tempm=ACAlloc(sizeof(CMPCmd));
  if (desc)
    tempm->desc=AStrNew(desc);
  if (spawn)
    tempm->cmd_code=MPCT_SPAWN_TASK;
  else
    tempm->cmd_code=MPCT_CALL;
  tempm->add=fp_add;
  tempm->data=data;
  tempm->target_cpu_mask=target_cpu_mask;
  tempm->flags=flags;
  if (i==Bsr(target_cpu_mask))
    tempm->ctrl=ctrl=&cpu_structs[i].cmd_ctrl;
  else
    tempm->ctrl=ctrl=mp_ctrl;
  PushFD;
  Cli;
  while (LBts(&ctrl->flags,MPCCf_LOCKED))
    Yield;
  QueIns(tempm,ctrl->last_waiting);
  LBtr(&ctrl->flags,MPCCf_LOCKED);
  PopFD;
  MPWake(target_cpu_mask);
  return tempm;
}

CMPCmd *MPJob(I64 (*fp_add)(U8 *data),U8 *data=NULL,
       I64 target_cpu=1,I64 flags=1<<MPCf_FREE_ON_COMPLETE)
//Set flags to zero if you wish to
//get the result.
{
  return MPQueJob(fp_add,data,NULL,flags,FALSE,1<<target_cpu);
}

I64 MPJobResult(CMPCmd *tempm)
{
  I64 result;
  CMPCmdCtrl *ctrl=tempm->ctrl;
  if (mp_cnt<2) throw(EXCEPT_MULTICORE,2);
  while (!Bt(&tempm->flags,MPCf_DONE))
    Yield;
  PushFD;
  Cli;
  while (LBts(&ctrl->flags,MPCCf_LOCKED))
    Yield;
  QueRem(tempm);
  LBtr(&ctrl->flags,MPCCf_LOCKED);
  PopFD;
  result=tempm->result;
  Free(tempm->desc);
  Free(tempm);
  return result;
}

CTask *MPSpawn(U0 (*fp_add)(U8 *data),U8 *data=NULL,U8 *name=NULL,I64 target_cpu=1)
{
  CTask *result;
  CMPCmd *tempm=MPQueJob(fp_add,data,name,0,TRUE,1<<target_cpu);
  CMPCmdCtrl *ctrl;
  while (!Bt(&tempm->flags,MPCf_DONE))
    Yield;
  result=tempm->task;
  ctrl=tempm->ctrl;
  PushFD;
  Cli;
  while (LBts(&ctrl->flags,MPCCf_LOCKED))
    Yield;
  QueRem(tempm);
  LBtr(&ctrl->flags,MPCCf_LOCKED);
  PopFD;
  Free(tempm->desc);
  Free(tempm);
  return result;
}

U0 MPCPUTaskInit()
{ //Called by multicore's seth task after $LK,"MPStart","MN:MPStart"$()
  //as the first thing a CPU does before waiting for jobs
  MPAPICInit;
  Fs->rip=&MPSethServerTask;
  Fs->time_slice_start=GetTimeStamp;
  TaskContextRestore;
}

U0 MPCmdQueDel(CMPCmd *root)
{
  CMPCmd *tempm=root->next,*tempm1;
  while (tempm!=root) {
    tempm1=tempm->next;
    QueRem(tempm);
    Free(tempm->desc);
    Free(tempm);
    tempm=tempm1;
  }
}

U0 MPStart()
{ //Called by adam during start-up
  //and can be called again to rst multicores.
  //Doesn't always work when rstting.  Kill all tasks, first.
  //Must be called from core#0.
  CTask *task;
  U8 buf[128];
  CMPMain *mp=MP_VECT_ADDR;
  CCPU *c;
  I64 i,my_mp_cnt;
  CRAXRBCRCXRDX ee;

  CPUId(0x1,&ee);
  if (!Bt(&ee.rdx,9))
    return;

  PushFD;
  Cli;
  if (mp_cnt>1) {
    my_mp_cnt=mp_cnt;
    MPHalt; //sets mp_cnt to 1
    for (i=1;i<my_mp_cnt;i++) {
      c=&cpu_structs[i];
      MPCmdQueDel(&c->cmd_ctrl.next_waiting);
      MPCmdQueDel(&c->cmd_ctrl.next_done);
    }
    MPCmdQueDel(&mp_ctrl->next_waiting);
    MPCmdQueDel(&mp_ctrl->next_done);
  }
  MemSet(&cpu_structs[1],0,sizeof(CCPU)*(MP_MAX_PROCESSORS-1));

//When you start-up other cores, they jump to an addr
//specified by a byte vect number, $LK,"MPN_VECT","MN:MPN_VECT"$ which corresponds
//to a location 4096*vect number, $LK,"MP_VECT_ADDR","MN:MP_VECT_ADDR"$$WW,0$.
  MemCpy(mp,SYS_MP_INIT_START,SYS_MP_INIT_END-SYS_MP_INIT_START);

  mp->sys_temp_ptr=GDT_SIZE-1+(&sys_gdttab)(U8 *)<<16;
  mp_cnt_initial=mp_cnt=1;
  mp_cnt_lock=0;

  *LAPIC_LVT_ERR(U32 *)=*LAPIC_LVT_ERR(U32 *)&0xFFFFFF00+MPN_VECT;
  asm {WBINVD}; //Not sure why this is needed.	Might just need delay. $LK,"MemCpy","MN:MemCpy"$ above?

  *LAPIC_ICR_LOW(U32 *)=0xC4500; //assert init IPI
  BusyWait(10000);

  *LAPIC_ICR_LOW(U32 *)=0xC4600+MPN_VECT; //start-up
  BusyWait(200);
  *LAPIC_ICR_LOW(U32 *)=0xC4600+MPN_VECT;

  BusyWait(100000);
  for (i=0;i<10000;i++)
    LBts(&mp_cnt_lock,0); //Don't let more through
  my_mp_cnt=mp_cnt_initial;

  if (my_mp_cnt>MP_MAX_PROCESSORS)
    my_mp_cnt=MP_MAX_PROCESSORS;

  for (i=1;i<my_mp_cnt;i++) {
    c =&cpu_structs[i];
    //SPrintF not available yet.
    StrCpy(buf,"Seth Task CPU#0");
    buf[14]+=i;
    task=Spawn(&MPCPUTaskInit,NULL,buf,NULL,SETH_STK,0);
    task->rflags=SYS_START_RFLAGS;
    //$LK,"CTask","MN:CTask"$ allocated off this core's seth_task's heap (Which is Adam)
    task->task_in_que_signature=TASK_IN_QUE_SIGNATURE_VAL;
    CPUStructInit(i,c);
    c->tss=TSSNew(i);
    c->seth_task=task;
    asm {WBINVD}; //Not sure why this is needed.  Might just need delay.
  }

  //Make sure they're all up-and-running
  for (i=1;i<my_mp_cnt;i++) {
    c =&cpu_structs[i];
    while (!Bt(&c->cpu_flags,CPUf_SETH_SLEEP))
      Pause;
  }

  PopFD;
  mp_cnt=my_mp_cnt; //Finalize cnt
}

U0 MPInit()
{ //Called by adam during start-up
  CRAXRBCRCXRDX ee;
  CCPU *c;
  CPUId(0x1,&ee);

  mp_cnt_initial=mp_cnt=1;
  mp_cnt_lock=0;

  mp_ctrl=AMAlloc(sizeof(CMPCmdCtrl));
  MPCmdCtrlInit(mp_ctrl);

  mp_crash=ACAlloc(sizeof(CMPCrash));

//Must be in code heap because init code uses 32 bit addr of cpu_struct
  c=cpu_structs=CAlloc(sizeof(CCPU)*MP_MAX_PROCESSORS,Fs->code_heap);
  MemCpy(c,&sys_temp_cpu0_struct,sizeof(CCPU));
  c->addr=cpu_structs;

  SetGs(c);
  adam_task->gs=c;
  adam_task->time_slice_start=GetTimeStamp;
  c->seth_task=adam_task;
  c->tss=TSSNew(0);
  c->idle_task=TaskIdleInit;
  if (Bt(&ee.rdx,9)) {
//Unnecessary?
//    SetMSR(IA32_LAPIC_BASE,LAPIC_BASE+0x900);
    MPAPICInit;
  }
}

U0 MPCrash()
{ //Entering the debugger from another core causes an interrupt on core#0
  //Which calls this routine.
  IntEOI;
  mp_cnt=1;
  Raw(ON);
  sys_raw_dc_flags|=RDCF_SHOW_DOLLAR;
  "MP Crash CPU#%02X Task:%08X\n",mp_crash->cpu_num,mp_crash->task;
  Dbg(mp_crash->msg,mp_crash->msg_num);
}
