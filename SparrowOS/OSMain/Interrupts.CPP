asm {
USE64
SYS_MP_CRASH_HANDLER_ADDR:: //forward reference to work around compiler
	DU32	&MPCrash;

SYS_WAKE_HANDLER::
	PUSH	RAX
	MOV	EAX,U32 LAPIC_EOI
	MOV	U32 [RAX],0
	POP	RAX
	IRET

//I_TIMER
SYS_TIMER_HANDLER::
	CALL	SYS_SAVE_CONTEXT		//PREEMPTIVE
	CLD
	MOV	RBP,RSP
	MOV	RBX,U64 [RBP]
	MOV	U64 CTask.rip[RSI],RBX
	MOV	RAX,U64 16[RBP]
	MOV	U64 CTask.rflags[RSI],RAX
	MOV	RAX,U64 24[RBP]
	MOV	U64 CTask.rsp[RSI],RAX

	XOR	RAX,RAX
	MOV	RDI,U64 GS:CCPU.addr[RAX]

	LOCK
	INC	U64 CCPU.total_jiffies[RDI]

	BT	U64 CTask.task_flags[RSI],TASKf_IDLE
	JC	@@05
	CMP	RBX,U32 SYS_IDLE_START
	JB	@@10
	CMP	RBX,U32 SYS_IDLE_END
	JA	@@10
@@05:	LOCK
	INC	U64 CCPU.idle_pt_hits[RDI]

@@10:	MOV	RAX,U64 CCPU.timer_irq_chain[RDI]
	OR	RAX,RAX
	JZ	@@15
	PUSH	RSI
	CALL	RAX
	JMP	@@20
@@15:	ADD	RSP,8
@@20:	CLI
	MOV	RAX,U64 CCPU.num[RDI]
	OR	RAX,RAX
	JZ	@@25

	MOV	EAX,U32 LAPIC_EOI
	MOV	U32 [RAX],0
	JMP	@@30

@@25:	MOV	AL,0x20 		 //ACKNOWLEDGE INTERRUPT
	OUT	0x20,AL

@@30:	MOV	RAX,SYS_SEMAS
	BT	U32 SYS_SEMA_SINGLE_USER*SEMA_STRUCT_SIZE[RAX],0
	JC	@@35
	BT	U64 CTask.task_flags[RSI],TASKf_PREEMPT
	JC	I32 RESTORE_NEXT_RSI_TASK
@@35:	JMP	I32 RESTORE_RSI_NO_PREEMPT_TASK

// ************************************
SYS_IRQ_FAULT::
	PUSH	RBX
	MOV	RBX,U64 8[RSP]
	PUSH	RAX
	XOR	RAX,RAX
	MOV	FS:U32 CTask.fault_num[RAX],EBX
	POP	RAX
	POP	RBX
	ADD	RSP,8

	CALL	SYS_SAVE_CONTEXT

	XOR	RDX,RDX
	MOV	U64 CTask.fault_err_code[RSI],RDX
	MOV	EDX,U32 CTask.fault_num[RSI]
	BT	U64 [SYS_FAULT_ERR_CODE_BITMAP],RDX
	JNC	@@1
	POP	U64 CTask.fault_err_code[RSI]

@@1:	MOV	RBP,RSP
	MOV	RAX,U64 [RBP]
	MOV	U64 CTask.rip[RSI],RAX
	MOV	RAX,U64 16[RBP]
	MOV	U64 CTask.rflags[RSI],RAX
	MOV	RAX,U64 24[RBP]
	MOV	U64 CTask.rsp[RSI],RAX
	MOV	RSP,RAX
	MOV	RBP,CTask.rbp[RSI]
	MOV	RSI,CTask.rsi[RSI]
	CALL	&Fault2 //See $LK,"Fault2","FF:::/SparrowOS/OSMain/Dbg1a.CPP.Z,Fault2"$
	JMP	I32 RESTORE_FS_TASK_WITH_KEY_CHECK

SYS_FAULT_ERR_CODE_BITMAP::
	DU32	0x00027D00,0,0,0,0,0,0,0;
}

U8 *IntEntryGet(I64 irq,I64 cpu_num=-1)
{
  U8 *result;
  I64 *src;
  if (cpu_num<0) cpu_num=Gs->num;
  if (cpu_num>=mp_cnt&&cpu_num>Gs->num)
    throw(EXCEPT_MULTICORE,3);
  src=cpu_structs[cpu_num].idt(U8 *)+irq*16;
  result(I64).u16[0]=*src(U16 *);
  src(U8 *)+=6;
  result(I64).u16[1]=*src(U16 *)++;
  result(I64).u32[1]=*src(U32 *);
  return result;
}

U8 *IntEntrySet(I64 irq,U0 (*fp_new_handler)(),I64 cpu_num=-1,
    I64 type=IDTET_IRQ,I64 dpl=0)
{
//See $LK,"IDTET_IRQ","MN:IDTET_IRQ"$
//See $LK,"::/SparrowOS/Demo/Lectures/InterruptDemo.CPP.Z","FI:::/SparrowOS/Demo/Lectures/InterruptDemo.CPP.Z"$
//See $LK,"::/SparrowOS/Demo/MultiCore/Interrupts.CPP.Z","FI:::/SparrowOS/Demo/MultiCore/Interrupts.CPP.Z"$
  I64 fp=fp_new_handler;
  U8 *result,*dst;

  if (cpu_num<0) cpu_num=Gs->num;

  result=IntEntryGet(irq,cpu_num);
  dst=cpu_structs[cpu_num].idt(U8 *)+irq*16;

  PushFD;
  Cli;
  *dst(U16 *)++=fp.u16[0];
  *dst(U16 *)++=GDT_CS64_SEL;
  *dst(U16 *)++=0x8000+type<<8+dpl<<13;
  *dst(U16 *)++=fp.u16[1];
  *dst(U32 *)++=fp.u32[1];
  *dst(U32 *)++=0;
  PopFD;
  return result;
}

U0 IntsInit()
{
//Init 8259
  OutU8(0x20,0x11); //IW1
  OutU8(0xA0,0x11); //IW1
  OutU8(0x21,0x20); //IW2
  OutU8(0xA1,0x28); //IW2
  OutU8(0x21,0x04); //IW3
  OutU8(0xA1,0x02); //IW3
  OutU8(0x21,0x0D); //IW4
  OutU8(0xA1,0x09); //IW4
  OutU8(0x21,0xFA); //Mask all but IRQ0 (timer) and IRQ2 Cascade
  OutU8(0xA1,0xFF);
}

U0 IntEOI()
{
  *LAPIC_EOI(U32 *)=0;
}

interrupt U0 IntNop1()
{ //Make unplanned IRQs stop!
  //IRQs on cores other than zero are APIC, I'm pretty sure.
  //I think the PIC only generates core 0 IRQs.
  IntEOI;
}

interrupt U0 IntNop2()
{ //Make unplanned IRQs stop by all means!
  OutU8(0xA0,0x20);
  OutU8(0x20,0x20);
  IntEOI;
}

U8 *IntFaultHandlersNew()
{
  I64 i;
  U8 *result=MAlloc(256*10,Fs->code_heap),*dst=result;
  for (i=0;i<256;i++) {
    *dst++=0x68; //PUSH I32 xxxxxxxx
    *dst(I32 *)++=i;
    *dst++=0xE9; //JMP	I32 xxxxxxxx
    *dst(I32 *)=SYS_IRQ_FAULT-dst-4;
    dst+=4;
  }
  return result;
}

U0 IntInit1()
{ //Interrupt descriptor table part1
  I64 i,temp_ptr[2];
  U32 *ptr;
  Gs->idt=CAlloc(16*256);
/*
During boot, we don't know what hardware PIC IRQ's were set to.
When we unmask for the first time, after initializing the PIC, I'm
not sure of old pending IRQs will be issued.

After boot, we might get a APIC IRQ because we never initialize that.
Acknowledging unplanned IRQs seems safest to me, even
if we accidentally turn-off a valid IRQ and miss it.
We only support keyboard, mouse and timer, anyway.
*/
  for (i=0;i<256;i++)
    if (!Gs->num)
      IntEntrySet(i,&IntNop2);
    else
      IntEntrySet(i,&IntNop1);

  ptr=temp_ptr;
  *ptr(U16 *)++=256*16-1;
  *ptr(U32 *)++=Gs->idt(I64).u32[0];
  *ptr(U32 *)++=Gs->idt(I64).u32[1];
  *ptr(U16 *)++=0;
  SetRAX(temp_ptr);
  asm {	LIDT U64 [RAX] }
}

U0 IntInit2()
{ //Interrupt descriptor table part2
  I64 i;
  PushFD;
  Cli;
  for (i=0;i<0x20;i++)
    IntEntrySet(i,&sys_fault_handlers_code[10*i]);
/*
In theory, we use the PIC mask register to insure we don't get
anything but keyboard, mouse and timer IRQs.  In practice, I've
gotten IRQ 0x27, perhaps because I didn't initialize the APIC.
I go ahead and ACK PIC in $LK,"IntNop2","MN:IntNop2"$().
I have no idea why I got a IRQ 0x27.
*/
  IntEntrySet(I_NMI,_SYS_HLT);
  IntEntrySet(I_TIMER,SYS_TIMER_HANDLER);
  IntEntrySet(I_MP_CRASH,*SYS_MP_CRASH_HANDLER_ADDR(U32 *));
  IntEntrySet(I_WAKE,SYS_WAKE_HANDLER);
  PopFD;
}
