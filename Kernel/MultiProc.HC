asm {
	ALIGN	16,OC_NOP
USE16
//See $LK,"TempleOS MultiCore",A="FI:::/Doc/MultiCore.DD"$.

//This code gets copied to $LK,"MP_VECT_ADDR",A="MN:MP_VECT_ADDR"$.
//See $LK,"MemCpy(MP_VECT_ADDR",A="FF:::/Kernel/MultiProc.HC,MemCpy(mp:2"$.
COREAP_16BIT_INIT::
	JMP	@@05

	ALIGN	4,OC_NOP
AP_GDT_PTR:	DU16	sizeof(CGDT)-1;
		DU64	0;

@@05:	CLI
	WBINVD
	MOV	AX,MP_VECT_ADDR/16
	MOV	DS,AX
	LGDT	U32 [CAP16BitInit.ap_gdt_ptr]  //See $LK,"mp->ap_gdt_ptr",A="FF:::/Kernel/MultiProc.HC,mp->ap_gdt_ptr:2"$

	MOV	EAX,SYS_START_CR0
	MOV_CR0_EAX
	DU8	0x66,0xEA;		 //JMP CGDT.cs32:AP_32BIT_INIT
	DU32	AP_32BIT_INIT;
	DU16	CGDT.cs32;
COREAP_16BIT_INIT_END::

USE32
AP_32BIT_INIT:
	MOV	AX,CGDT.ds
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

	CMP	ESI,MP_PROCESSORS_NUM
	JAE	I32 _SYS_HLT

	IMUL2	ESI,sizeof(CCPU)
	ADD	ESI,U32 [SYS_CPU_STRUCTS]

	LEA	ESP,U32 CCPU.start_stk+sizeof(CCPU.start_stk)[ESI]
	PUSH	U32 RFLAGG_START
	POPFD
	PUSH	U32 0	//Return from next call will be 64-bit
	CALL	SYS_ENTER_LONG_MODE
USE64
	FNINIT
	MOV	RAX,RSI
	CALL	SET_GS_BASE
@@10:	MOV	RAX,U64 CCPU.seth_task[RSI]
	TEST	RAX,RAX
	JZ	@@10
	MOV	U64 CTask.gs[RAX],RSI
	CALL	SET_FS_BASE

	JMP	I32 _TASK_CONTEXT_RESTORE
}

U0 TSSBusy(I64 tr,Bool val=OFF)
{//See $LK,"::/Demo/Lectures/Ring3.HC"$.
  LBEqu((&sys_gdt)(U8 *)+tr+4,9,val);
}

CTSS *TSSNew(I64 cpu_num)
{
  U32 *d,*d1;
  CTSS *tss=CAlloc(sizeof(CTSS));
  tss->io_map_offset=offset(CTSS.io_map);
  MemSet(tss->io_map,0xFF,0x10000/8);

  tss->st0=MAlloc(MEM_INTERRUPT_STK);
  tss->rsp0=tss->st0(U8 *)+MSize(tss->st0);
  tss->st1=MAlloc(MEM_INTERRUPT_STK);
  tss->rsp1=tss->st1(U8 *)+MSize(tss->st1);
  tss->st2=MAlloc(MEM_INTERRUPT_STK);
  tss->rsp2=tss->st2(U8 *)+MSize(tss->st2);

  tss->tr      =offset(CGDT.tr)+cpu_num*16;
  tss->tr_ring3=offset(CGDT.tr_ring3)+cpu_num*16;

  d=(&sys_gdt)(U8 *)+tss->tr;
  d1=d(U8 *)+4;
  *d =0x0000FFFF;
  *d1=0x008F8900;
  d(U8 *)+=2;
  *d|=tss & 0x00FFFFFF;
  *d1++|=tss & 0xFF000000;
  *d1++=tss>>32;
  *d1=0;

  d=(&sys_gdt)(U8 *)+tss->tr_ring3;
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

CCPU *CPUStructInit(I64 num,CCPU *c,CTask *seth_task)
{//Seth is null when called by adam on CSysFixedArea.boot_cpu0
  MemSet(c,0,sizeof(CCPU));
  c->addr=c;
  c->num=num;
  c->idle_factor=0.01;
  QueInit(&c->next_dying);
  if (Bt(&sys_run_level,RLf_16MEG_ADAM_HEAP_CTRL)) {
    c->idle_task=Spawn(0,NULL,"Idle Task",,Fs,,0);
    LBts(&c->idle_task->task_flags,TASKf_IDLE);
    c->tss=TSSNew(num);
  }
  c->seth_task=seth_task;// It waits for this to be filled-in: $LK,"seth_task",A="FF:::/Kernel/MultiProc.HC,seth_task"$
  return c;
}

U0 MPInt(U8 num,I64 cpu_num=1)
{//Generate interrupt for specified core.
  if (cpu_num>=mp_cnt) {
    if (!Bt(&sys_run_level,RLf_MP))
      return;
    else
      throw('MultCore');
  }
  PUSHFD
  CLI //Multitasking safe because each core has a local apic and IRQ's are off
  while (*(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)&0x1000)
    PAUSE
  *(dev.uncached_alias+LAPIC_ICR_HIGH)(U32 *)=dev.mp_apic_ids[cpu_num]<<24;
  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0x4000+num;
  POPFD
}

U0 MPIntAll(U8 num)
{//Generate interrupt for all but own core.
  PUSHFD
  CLI //Multitasking safe because each core has a local apic and IRQ's are off
  while (*(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)&0x1000)
    PAUSE
  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0xC4800+num;
  POPFD
}

U0 MPNMInt()
{//Generate nonmaskable interrupt.
  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0xC4400;
}

U0 MPHalt()
{//Halt all other cores.
  mp_cnt=1;
  MPNMInt;
  Busy(10000);
}

U0 MPAPICInit()
{//Called by adam during start-up
//and other cores during initialization
  //after $LK,"Core0StartMP",A="MN:Core0StartMP"$().
  *(dev.uncached_alias+LAPIC_SVR)(U32 *)|=LAPICF_APIC_ENABLED;
  dev.mp_apic_ids[Gs->num]=*(dev.uncached_alias+LAPIC_APIC_ID)(U32 *)>>24;
  *(dev.uncached_alias+LAPIC_LDR)(U32 *)=dev.mp_apic_ids[Gs->num]<<24;
  *(dev.uncached_alias+LAPIC_DFR)(U32 *)=0xF0000000;

  //  MemSet(dev.uncached_alias+LAPIC_IRR,0,0x20);
  //  MemSet(dev.uncached_alias+LAPIC_ISR,0,0x20);
  //  MemSet(dev.uncached_alias+LAPIC_TMR,0,0x20);

  SetRAX(Gs->tss->tr);
  LTR	AX
  if (Gs->num) {
    IntInit1;
    SetRFlags(RFLAGG_NORMAL);
  }
}

#assert !offset(CJobCtrl.next_waiting)

U0 CoreAPSethTask()
{
  CJobCtrl *ctrl=&Fs->srv_ctrl;
  while (TRUE) {
    STI
    do {
      TaskKillDying;
      do PAUSE
      while (LBts(&ctrl->flags,JOBCf_LOCKED));
    } while (ctrl->next_waiting!=ctrl && JobRunOne(GetRFlags,ctrl));
    CLI
    LBts(&Fs->task_flags,TASKf_AWAITING_MSG);
    LBtr(&ctrl->flags,JOBCf_LOCKED);
    LBts(&Fs->task_flags,TASKf_IDLE);
    Yield;
    LBtr(&Fs->task_flags,TASKf_IDLE);
  }
}

CJob *JobQue(I64 (*fp_addr)(U8 *data),U8 *data=NULL,
       I64 target_cpu=1,I64 flags=1<<JOBf_FREE_ON_COMPLETE,
       I64 job_code=JOBT_CALL,U8 *aux_str=NULL,I64 aux1=0,I64 aux2=0)
{//Queue multicore jobs, handled by Seth tasks.
//Set flags to zero if you wish to get the res.
  //See $LK,"::/Demo/MultiCore/Lock.HC"$
  CJobCtrl *ctrl;
  CJob *tmpc;
  CTask *seth;
  if (!(0<=target_cpu<mp_cnt))
    throw('MultCore');
  tmpc=ACAlloc(sizeof(CJob));
  if (aux_str)
    tmpc->aux_str=AStrNew(aux_str);
  tmpc->job_code=job_code;
  tmpc->addr=fp_addr;
  tmpc->fun_arg=data;
  tmpc->flags=flags;
  tmpc->aux1=aux1;
  tmpc->aux2=aux2;
  seth=cpu_structs[target_cpu].seth_task;
  tmpc->ctrl=ctrl=&seth->srv_ctrl;
  PUSHFD
  CLI
  while (LBts(&ctrl->flags,JOBCf_LOCKED))
    Yield;
  if (ctrl->next_waiting==ctrl && LBtr(&seth->task_flags,TASKf_AWAITING_MSG))
    MPInt(I_WAKE,target_cpu);
  QueIns(tmpc,ctrl->last_waiting);
  LBtr(&ctrl->flags,JOBCf_LOCKED);
  POPFD
  return tmpc;
}

CTask *SpawnQue(U0 (*fp_addr)(U8 *data),U8 *data=NULL,U8 *task_name=NULL,
	I64 target_cpu, CTask *parent=NULL, //NULL means adam
	I64 stk_size=0,I64 flags=1<<JOBf_ADD_TO_QUE)
{
  CTask *res;
  CJob *tmpc=JobQue(fp_addr,data,target_cpu,
	flags,JOBT_SPAWN_TASK,task_name,parent,stk_size);
  CJobCtrl *ctrl;

  while (!Bt(&tmpc->flags,JOBf_DONE)) {
    LBts(&Fs->task_flags,TASKf_IDLE);
    Yield;
  }
  LBtr(&Fs->task_flags,TASKf_IDLE);

  res=tmpc->spawned_task;
  ctrl=tmpc->ctrl;
  PUSHFD
  CLI
  while (LBts(&ctrl->flags,JOBCf_LOCKED))
    Yield;
  QueRem(tmpc);
  LBtr(&ctrl->flags,JOBCf_LOCKED);
  POPFD
  JobDel(tmpc);
  return res;
}

U0 CoreAPSethInit()
{//Called by multicore's seth task after $LK,"Core0StartMP",A="MN:Core0StartMP"$()
//as the first thing a CPU does before waiting for jobs.
  MPAPICInit;
  Fs->rip=&CoreAPSethTask;
  TaskContextRestore;
}

U0 Core0StartMP()
{//Called by adam during $LK,"start-up",A="FF:::/Kernel/KMain.HC,Core0StartMP"$.
  CTask *task;
  U8 buf[STR_LEN];
  CAP16BitInit *mp=MP_VECT_ADDR;
  CCPU *c;
  I64 i,my_mp_cnt;
  CRAXRBCRCXRDX ee;

  CPUId(0x1,&ee);
  if (!Bt(&ee.rdx,9))
    return;

  PUSHFD
  CLI
  if (mp_cnt>1) {
    my_mp_cnt=mp_cnt;
    MPHalt; //sets mp_cnt to 1
    for (i=1;i<my_mp_cnt;i++) {
      c=&cpu_structs[i];
      JobQueDel(&c->seth_task->srv_ctrl.next_waiting);
      JobQueDel(&c->seth_task->srv_ctrl.next_done);
    }
  }
  MemSet(&cpu_structs[1],0,sizeof(CCPU)*(MP_PROCESSORS_NUM-1));

  //When you start-up other cores, they jump to an addr
  //specified by a byte vect number, $LK,"MPN_VECT",A="MN:MPN_VECT"$ which corresponds
  //to a location 4096*vect number, $LK,"MP_VECT_ADDR",A="MN:MP_VECT_ADDR"$$WW,0$.
  MemCpy(mp,COREAP_16BIT_INIT,COREAP_16BIT_INIT_END-COREAP_16BIT_INIT);
  MemCpy(&mp->ap_gdt_ptr,SYS_GDT_PTR,sizeof(CSysLimitBase));
  mp_cnt_initial=mp_cnt=1;
  mp_cnt_lock=0;

  *(dev.uncached_alias+LAPIC_LVT_ERR)(U32 *)=
	*(dev.uncached_alias+LAPIC_LVT_ERR)(U32 *)&0xFFFFFF00+MPN_VECT;
  WBINVD //Not sure why this is needed.	Might just need delay. $LK,"MemCpy",A="MN:MemCpy"$ above?

  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0xC4500; //assert init IPI
  Busy(10000);

  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0xC4600+MPN_VECT; //start-up
  Busy(200);
  *(dev.uncached_alias+LAPIC_ICR_LOW)(U32 *)=0xC4600+MPN_VECT;

  Busy(100000);
  for (i=0;i<10000;i++)
    LBts(&mp_cnt_lock,0); //Don't let more through
  my_mp_cnt=mp_cnt_initial;

  if (my_mp_cnt>MP_PROCESSORS_NUM)
    my_mp_cnt=MP_PROCESSORS_NUM;

  for (i=1;i<my_mp_cnt;i++) {
    StrPrint(buf,"Seth Task CPU%02X",i);
    task=Spawn(&CoreAPSethInit,NULL,buf,,,MEM_SETH_STK,0);
    task->rflags=RFLAGG_START;
//$LK,"CTask",A="MN:CTask"$ alloced off this core's seth_task's heap (Which is Adam)
    CPUStructInit(i,&cpu_structs[i],task);
    WBINVD //Not sure why this is needed.  Might just need delay.
  }

  //Make sure they're all up-and-running
  for (i=1;i<my_mp_cnt;i++)
    while (!Bt(&cpu_structs[i].seth_task->task_flags,TASKf_AWAITING_MSG))
      PAUSE;

  POPFD
  mp_cnt=my_mp_cnt; //Finalize cnt
}

U0 Core0Init()
{//Called by adam during start-up
  CRAXRBCRCXRDX ee;
  CPUId(0x1,&ee);

  mp_cnt_initial=mp_cnt=1;
  mp_cnt_lock=0;

  dbg.mp_crash=ACAlloc(sizeof(CMPCrash));

  //Must be in code heap because init code uses 32 bit addr of cpu_struct
  adam_task->gs=cpu_structs=
	CAlloc(sizeof(CCPU)*MP_PROCESSORS_NUM,Fs->code_heap);
  CPUStructInit(0,cpu_structs,adam_task);
  asm {//RAX has GS
    IMPORT SET_GS_BASE;
    CALL SET_GS_BASE
  }
  if (Bt(&ee.rdx,9)) {
//Unnecessary?
    //	  SetMSR(IA32_LAPIC_BASE,dev.uncached_alias+LAPIC_BASE+0x900);
    MPAPICInit;
  }
}

interrupt U0 IntMPCrash()
{//Entering the debugger from another core causes an interrupt on Core0
//Which calls this routine.
  *(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
  mp_cnt=1;
  Raw(ON);
  text.raw_flags|=RWF_SHOW_DOLLAR;
  "MP Crash CPU%02X Task:%08X\n"
  "RIP:%P\n",dbg.mp_crash->cpu_num,dbg.mp_crash->task,dbg.mp_crash->rip;
  Panic(dbg.mp_crash->msg,dbg.mp_crash->msg_num);
}
