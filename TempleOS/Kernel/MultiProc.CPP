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
	TEST	RBX,RBX
	JZ	@@10
	MOV	U64 CTask.gs[RBX],RSI
	MOV	RAX,RBX
	CALL	SET_FS_BASE

	JMP	I32 SYS_CONTEXT_RESTORE
}

U0 TSSBusy(I64 tr,Bool val=OFF)
{ //See $LK-A,"::/TempleOS/Demo/Lectures/Ring3.CPP"$.
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

CTask *TaskIdleInit()
{
  CTask *task=Spawn(0,NULL,"Idle Task",,Fs,,0);
  Bts(&task->task_flags,TASKf_IDLE);
  return task;
}

CCPU *CPUStructInit(I64 num,CCPU *c)
{
  MemSet(c,0,sizeof(CCPU));
  c->addr=c;
  c->num=num;
  c->idle_factor=0.01;
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
      throw(EXCEPT_MULTICORE,1);
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

U0 MPWake(I64 target_cpu)
{
  if (0<=target_cpu<mp_cnt) {
    PushFD;
    Cli;
    if (Bt(&cpu_structs[target_cpu].cpu_flags,CPUf_SETH_SLEEP))
      cpu_structs[target_cpu].seth_task->wake_jiffy=0;
    if (!target_cpu)
      LBtr(&adam_task->task_flags,TASKf_AWAITING_MSG);
    MPInt(I_WAKE,target_cpu);
    if (!target_cpu)
      LBtr(&adam_task->task_flags,TASKf_AWAITING_MSG);
    if (Bt(&cpu_structs[target_cpu].cpu_flags,CPUf_SETH_SLEEP)) //In case went back to sleep
      cpu_structs[target_cpu].seth_task->wake_jiffy=0;
    PopFD;
  }
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
    SetRFlags(SYS_NORMAL_RFLAGS);
  }
}

#assert !offset(CSrvCtrl.next_waiting)

U0 MPSethServerTask()
{
  CSrvCtrl *my_ctrl=&Fs->srv_ctrl;
  CSrvCmd *tempc,*tempc1=&my_ctrl->next_waiting;
  Sti;
  while (TRUE) {
    TaskFinishOffDying;
    if (!LBts(&my_ctrl->flags,SVCRf_LOCKED)) {
      tempc=tempc1->next;
      //Jobs for this core are highest priority.
      if (tempc==tempc1 || !SrvCmdRun(tempc,GetRFlags)) {
	LBtr(&my_ctrl->flags,SVCRf_LOCKED) ;
	if (my_ctrl->next_waiting==my_ctrl) {
	  LBts(&Gs->cpu_flags,CPUf_SETH_SLEEP);
	  Sleep(20); //We don't want this too long because of need for $LK,"TaskFinishOffDying","MN:TaskFinishOffDying"$().
	  LBtr(&Gs->cpu_flags,CPUf_SETH_SLEEP);
	}
      }
    }
  }
}

CSrvCmd *JobQue(I64 (*fp_add)(U8 *data),U8 *data=NULL,
       I64 target_cpu=1,I64 flags=1<<SVCf_FREE_ON_COMPLETE,
       I64 cmd_code=SVCT_CALL,U8 *aux_str=NULL,I64 aux1=0,I64 aux2=0)
//This is to queue multicore jobs, handled by Seth tasks.
//Set flags to zero if you wish to get the result.
{
  CSrvCtrl *ctrl;
  CSrvCmd *tempc;
  if (!(0<=target_cpu<mp_cnt))
    throw(EXCEPT_MULTICORE,2);
  tempc=ACAlloc(sizeof(CSrvCmd));
  if (aux_str)
    tempc->aux_str=AStrNew(aux_str);
  tempc->cmd_code=cmd_code;
  tempc->add=fp_add;
  tempc->fun_arg=data;
  tempc->target_cpu=target_cpu;
  tempc->flags=flags;
  tempc->aux1=aux1;
  tempc->aux2=aux2;
  tempc->ctrl=ctrl=&cpu_structs[target_cpu].seth_task->srv_ctrl;
  PushFD;
  Cli;
  while (LBts(&ctrl->flags,SVCRf_LOCKED))
    Yield;
  QueIns(tempc,ctrl->last_waiting);
  LBtr(&ctrl->flags,SVCRf_LOCKED);
  PopFD;
  MPWake(target_cpu);
  return tempc;
}

CTask *SpawnQue(U0 (*fp_add)(U8 *data),U8 *data=NULL,
	    U8 *task_name=NULL,
	    I64 target_cpu,
	    CTask *parent=NULL, //NULL means adam
	    I64 stk_size=0,I64 flags=1<<SVCf_ADD_TO_QUE)
{
  CTask *result;
  CSrvCmd *tempc=JobQue(fp_add,data,target_cpu,flags,SVCT_SPAWN_TASK,task_name,parent,stk_size);
  CSrvCtrl *ctrl;

  while (!Bt(&tempc->flags,SVCf_DONE)) {
    LBts(&Fs->task_flags,TASKf_IDLE);
    Yield;
  }
  LBtr(&Fs->task_flags,TASKf_IDLE);

  result=tempc->spawned_task;
  ctrl=tempc->ctrl;
  PushFD;
  Cli;
  while (LBts(&ctrl->flags,SVCRf_LOCKED))
    Yield;
  QueRem(tempc);
  LBtr(&ctrl->flags,SVCRf_LOCKED);
  PopFD;
  SrvCmdDel(tempc);
  return result;
}

U0 MPCPUTaskInit()
{ //Called by multicore's seth task after $LK,"MPStart","MN:MPStart"$()
  //as the first thing a CPU does before waiting for jobs
  MPAPICInit;
  Fs->rip=&MPSethServerTask;
  Fs->time_slice_start=GetTSC;
  TaskContextRestore;
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
      SrvCmdQueDel(&c->seth_task->srv_ctrl.next_waiting);
      SrvCmdQueDel(&c->seth_task->srv_ctrl.next_done);
    }
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
    SPrint(buf,"Seth Task CPU#%d",i);
    task=Spawn(&MPCPUTaskInit,NULL,buf,,,SETH_STK,0);
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

  mp_crash=ACAlloc(sizeof(CMPCrash));

//Must be in code heap because init code uses 32 bit addr of cpu_struct
  c=cpu_structs=CAlloc(sizeof(CCPU)*MP_MAX_PROCESSORS,Fs->code_heap);
  MemCpy(c,&sys_temp_cpu0_struct,sizeof(CCPU));
  c->addr=cpu_structs;

  SetGs(c);
  adam_task->gs=c;
  adam_task->time_slice_start=GetTSC;
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
  sys_raw_flags|=RWF_SHOW_DOLLAR;
  "MP Crash CPU#%02X Task:%08X\n",mp_crash->cpu_num,mp_crash->task;
  Dbg(mp_crash->msg,mp_crash->msg_num);
}
