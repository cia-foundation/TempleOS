asm {
INT_MP_CRASH_ADDR:: //Forward reference to work around compiler
	DU32	&IntMPCrash;

INT_WAKE::
	PUSH	RDX
	PUSH	RAX
	MOV	EAX,&dev
	MOV	EDX,U32 LAPIC_EOI
	MOV	RAX,U64 CDevGlbls.uncached_alias[RAX]
	MOV	U32 [RAX+RDX],0
	POP	RAX
	POP	RDX
	IRET

IRQ_TIMER::  //I_TIMER
	CALL	TASK_CONTEXT_SAVE
	CLD

	MOV	RAX,U64 [RSP]
	MOV	U64 CTask.rip[RSI],RAX
	MOV	RAX,U64 16[RSP]
	MOV	U64 CTask.rflags[RSI],RAX
	MOV	RAX,U64 24[RSP]
	MOV	U64 CTask.rsp[RSI],RAX

	XOR	RAX,RAX
	MOV	RDI,U64 GS:CCPU.addr[RAX]
	LOCK
	INC	U64 CCPU.total_jiffies[RDI]

	BT	U64 CTask.task_flags[RSI],TASKf_IDLE
	JNC	@@05
	LOCK
	INC	U64 CCPU.idle_pt_hits[RDI]

@@05:	MOV	RAX,U64 CCPU.profiler_timer_irq[RDI]
	TEST	RAX,RAX
	JZ	@@10
	PUSH	RSI
	CALL	RAX	//See $LK,"ProfTimerInt",A="MN:ProfTimerInt"$().
	JMP	@@15
@@10:	ADD	RSP,8
@@15:	CLI
	MOV	RAX,U64 CCPU.num[RDI]
	TEST	RAX,RAX
	JZ	@@20

	MOV	EAX,&dev
	MOV	EDX,U32 LAPIC_EOI
	MOV	RAX,U64 CDevGlbls.uncached_alias[RAX]
	MOV	U32 [RAX+RDX],0
	JMP	@@25

@@20:	CALL	&IntCore0TimerHndlr	//Only Core 0 calls this.
@@25:	XOR	RAX,RAX
	CMP	RSI,U64 GS:CCPU.idle_task[RAX]
	JE	I32 RESTORE_SETH_TASK_IF_READY
	JMP	I32 RESTORE_RSI_TASK
//************************************
INT_FAULT::
	PUSH	RBX
	PUSH	RAX
	MOV	BL,U8 16[RSP]	//We pushed fault_num $LK,"IntFaultHndlrsNew",A="MN:IntFaultHndlrsNew"$().
	XOR	RAX,RAX
	MOV	FS:U8 CTask.fault_num[RAX],BL
	POP	RAX
	POP	RBX
	ADD	RSP,8		//Pop fault_num

	CALL	TASK_CONTEXT_SAVE

	XOR	RDX,RDX
	MOV	U64 CTask.fault_err_code[RSI],RDX
	MOV	EDX,U32 CTask.fault_num[RSI]
	BT	U64 [INT_FAULT_ERR_CODE_BITMAP],RDX
	JNC	@@1
	POP	U64 CTask.fault_err_code[RSI]

@@1:	MOV	RAX,U64 [RSP]
	MOV	U64 CTask.rip[RSI],RAX
	MOV	RAX,U64 16[RSP]
	MOV	U64 CTask.rflags[RSI],RAX
	MOV	RSP,U64 24[RSP]
	MOV	U64 CTask.rsp[RSI],RSP
	MOV	RBP,CTask.rbp[RSI]
	PUSH	U64 CTask.fault_err_code[RSI]
	PUSH	U64 CTask.fault_num[RSI]
	MOV	RSI,CTask.rsi[RSI]
	CALL	&Fault2 	//See $LK,"Fault2",A="FF:::/Kernel/KDbg.HC,Fault2"$
	JMP	I32 RESTORE_FS_TASK

INT_FAULT_ERR_CODE_BITMAP::
	DU32	0x00027D00,0,0,0,0,0,0,0;
}

U8 *IntEntryGet(I64 irq)
{//Get interrupt vector.
  U8 *res;
  I64 *src;
  src=dev.idt(U8 *)+irq*16;
  res(I64).u16[0]=*src(U16 *);
  src(U8 *)+=6;
  res(I64).u16[1]=*src(U16 *)++;
  res(I64).u32[1]=*src(U32 *);
  return res;
}

U8 *IntEntrySet(I64 irq,U0 (*fp_new_hndlr)(),I64 type=IDTET_IRQ,I64 dpl=0)
{//Set interrupt vector. See $LK,"IDTET_IRQ",A="MN:IDTET_IRQ"$.
//See $LK,"::/Demo/Lectures/InterruptDemo.HC"$.
  //See $LK,"::/Demo/MultiCore/Interrupts.HC"$.
  I64 fp=fp_new_hndlr;
  U8 *res,*dst;
  PUSHFD
  CLI
  res=IntEntryGet(irq);
  dst=dev.idt(U8 *)+irq*16;
  *dst(U16 *)++=fp.u16[0];
  *dst(U16 *)++=offset(CGDT.cs64);
  *dst(U16 *)++=0x8000+type<<8+dpl<<13;
  *dst(U16 *)++=fp.u16[1];
  *dst(U32 *)++=fp.u32[1];
  *dst(U32 *)=0;
  POPFD
  return res;
}

U0 IntsInit()
{//Init 8259
  OutU8(0x20,0x11); //IW1
  OutU8(0xA0,0x11); //IW1
  OutU8(0x21,0x20); //IW2
  OutU8(0xA1,0x28); //IW2
  OutU8(0x21,0x04); //IW3
  OutU8(0xA1,0x02); //IW3
  OutU8(0x21,0x0D); //IW4
  OutU8(0xA1,0x09); //IW4
  OutU8(0x21,0xFA); //Mask all but IRQ0 (timer) and IRQ2 Cascade.
  OutU8(0xA1,0xFF);
}

interrupt U0 IntNop()
{//Make unplanned IRQs stop by all means!
  OutU8(0xA0,0x20);
  OutU8(0x20,0x20);
  *(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
}

interrupt U0 IntDivZero()
{
  if (Gs->num) {
    mp_cnt=1;
    dbg.mp_crash->cpu_num=Gs->num;
    dbg.mp_crash->task=Fs;
    MOV	RAX,U64 8[RBP] //Get RIP off of stk.
    dbg.mp_crash->rip=GetRAX;
    dbg.mp_crash->msg="Div Zero";
    dbg.mp_crash->msg_num=0;
    MPInt(I_MP_CRASH,0);
    SysHlt;
  }
  throw('DivZero');
}

U8 *IntFaultHndlrsNew()
{
  I64 i;
  U8 *res=MAlloc(256*7,Fs->code_heap),*dst=res;
  for (i=0;i<256;i++) {
    *dst++=0x6A; //PUSH I8 xx
    *dst(I8 *)++=i;
    *dst++=0xE9; //JMP	I32 xxxxxxxx
    *dst(I32 *)=INT_FAULT-dst-4;
    dst+=4;
  }
  return res;
}

U0 IntInit1()
{//Interrupt descriptor table part1.
  I64 i;
  CSysLimitBase tmp_ptr;
  if (!Gs->num) {//Gs cur $LK,"CCPU",A="MN:CCPU"$ struct
    dev.idt=CAlloc(16*256);
    for (i=0;i<256;i++)
      IntEntrySet(i,&IntNop);
  }
  tmp_ptr.limit=256*16-1;
  tmp_ptr.base =dev.idt;
  SetRAX(&tmp_ptr);
  LIDT U64 [RAX]
}

U0 IntInit2()
{//Interrupt descriptor table part2: Core 0 Only.
  I64 i;
  PUSHFD
  CLI
  IntEntrySet(I_DIV_ZERO,&IntDivZero);
  for (i=1;i<0x20;i++)
    IntEntrySet(i,&dbg.int_fault_code[7*i]);
/*In theory, we use the PIC mask reg to insure we don't get
anything but keyboard, mouse and timer IRQs.  In practice, I've
gotten IRQ 0x27, perhaps because I didn't initialize the APIC.
I go ahead and ACK PIC in $LK,"IntNop",A="MN:IntNop"$().
I have no idea why I got a IRQ 0x27.
*/
  IntEntrySet(I_NMI,_SYS_HLT);
  IntEntrySet(I_TIMER,IRQ_TIMER);
  IntEntrySet(I_MP_CRASH,*INT_MP_CRASH_ADDR(U32 *));
  IntEntrySet(I_WAKE,INT_WAKE);
  IntEntrySet(I_DBG,&dbg.int_fault_code[7*I_DBG]);
  POPFD
}
