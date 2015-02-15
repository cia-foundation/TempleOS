asm {
USE64
SYS_MP_CRASH_HANDLER_ADDR:: //Forward reference to work around compiler
	DU32	&MPCrash;

SYS_WAKE_HANDLER::
	PUSH	RAX
	MOV	EAX,U32 LAPIC_EOI
	MOV	U32 [RAX],0
	POP	RAX
	IRET

SYS_TIMER_HANDLER::  //I_TIMER
	CALL	TASK_CONTEXT_SAVE	//PREEMPTIVE
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

	MOV	EAX,U32 LAPIC_EOI
	MOV	U32 [RAX],0
	JMP	@@25

@@20:	CALL	&IntCore0TimerHandler	//Only Core 0 calls this.

@@25:	TEST	U8 [SYS_SEMAS+SYS_SEMA_SINGLE_USER*SEMA_STRUCT_SIZE],1
	JNZ	@@30
	BT	U64 CTask.task_flags[RSI],TASKf_PREEMPT
	JC	I32 RESTORE_NEXT_RSI_TASK
@@30:	JMP	I32 RESTORE_RSI_NO_PREEMPT_TASK
//************************************
SYS_IRQ_FAULT::
	PUSH	RBX
	PUSH	RAX
	MOV	BL,U8 16[RSP]	//We pushed fault_num $LK,"IntFaultHandlersNew",A="MN:IntFaultHandlersNew"$().
	XOR	RAX,RAX
	MOV	FS:U8 CTask.fault_num[RAX],BL
	POP	RAX
	POP	RBX
	ADD	RSP,8		//Pop fault_num

	CALL	TASK_CONTEXT_SAVE

	XOR	RDX,RDX
	MOV	U64 CTask.fault_err_code[RSI],RDX
	MOV	EDX,U32 CTask.fault_num[RSI]
	BT	U64 [SYS_FAULT_ERR_CODE_BITMAP],RDX
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
	CALL	&Fault2 	//See $LK,"Fault2",A="FF:::/Kernel/KDbg.CPP,Fault2"$
	JMP	I32 RESTORE_FS_TASK_WITH_KEY_CHECK

SYS_FAULT_ERR_CODE_BITMAP::
	DU32	0x00027D00,0,0,0,0,0,0,0;
}

U8 *IntEntryGet(I64 irq)
{//Get interrupt vector.
  U8 *result;
  I64 *src;
  src=dev.idt(U8 *)+irq*16;
  result(I64).u16[0]=*src(U16 *);
  src(U8 *)+=6;
  result(I64).u16[1]=*src(U16 *)++;
  result(I64).u32[1]=*src(U32 *);
  return result;
}

U8 *IntEntrySet(I64 irq,U0 (*fp_new_handler)(),I64 type=IDTET_IRQ,I64 dpl=0)
{//Set interrupt vector. See $LK,"IDTET_IRQ",A="MN:IDTET_IRQ"$.
//See $LK,"::/Demo/Lectures/InterruptDemo.CPP"$.
//See $LK,"::/Demo/MultiCore/Interrupts.CPP"$.
  I64 fp=fp_new_handler;
  U8 *result,*dst;
  PUSHFD
  CLI
  result=IntEntryGet(irq);
  dst=dev.idt(U8 *)+irq*16;
  *dst(U16 *)++=fp.u16[0];
  *dst(U16 *)++=offset(CGDT.cs64);
  *dst(U16 *)++=0x8000+type<<8+dpl<<13;
  *dst(U16 *)++=fp.u16[1];
  *dst(U32 *)++=fp.u32[1];
  *dst(U32 *)=0;
  POPFD
  return result;
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
  *LAPIC_EOI(U32 *)=0;
}

U8 *IntFaultHandlersNew()
{
  I64 i;
  U8 *result=MAlloc(256*7,Fs->code_heap),*dst=result;
  for (i=0;i<256;i++) {
    *dst++=0x6A; //PUSH I8 xx
    *dst(I8 *)++=i;
    *dst++=0xE9; //JMP	I32 xxxxxxxx
    *dst(I32 *)=SYS_IRQ_FAULT-dst-4;
    dst+=4;
  }
  return result;
}

U0 IntInit1()
{//Interrupt descriptor table part1.
  I64 i;
  CSysLimitBase temp_ptr;
  if (!Gs->num) {//Gs cur $LK,"CCPU",A="MN:CCPU"$ struct
    dev.idt=CAlloc(16*256);
    for (i=0;i<256;i++)
      IntEntrySet(i,&IntNop);
  }
  temp_ptr.limit=256*16-1;
  temp_ptr.base =dev.idt;
  SetRAX(&temp_ptr);
	LIDT U64 [RAX]
}

U0 IntInit2()
{//Interrupt descriptor table part2: Core 0 Only.
  I64 i;
  PUSHFD
  CLI
  for (i=0;i<0x20;i++)
    IntEntrySet(i,&dbg.fault_handlers_code[7*i]);
/*In theory, we use the PIC mask reg to insure we don't get
anything but keyboard, mouse and timer IRQs.  In practice, I've
gotten IRQ 0x27, perhaps because I didn't initialize the APIC.
I go ahead and ACK PIC in $LK,"IntNop",A="MN:IntNop"$().
I have no idea why I got a IRQ 0x27.
*/
  IntEntrySet(I_NMI,_SYS_HLT);
  IntEntrySet(I_TIMER,SYS_TIMER_HANDLER);
  IntEntrySet(I_MP_CRASH,*SYS_MP_CRASH_HANDLER_ADDR(U32 *));
  IntEntrySet(I_WAKE,SYS_WAKE_HANDLER);
  IntEntrySet(I_DBG,&dbg.fault_handlers_code[7*I_DBG]);
  POPFD
}
