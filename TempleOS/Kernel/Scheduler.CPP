/*
See $LK,"TempleOS MultiTasking","FI:::/TempleOS/Doc/MultiTasking.TXT"$.

Tasks are ordered in a queue based
on window front-to-back order with the
window mgr as the head, which
paints the wallpaper.  The scheduler
goes in a circle, checking if a task is
ready and swapping-it-in if so.
Swapping just involves storing and
restoring registers -- no disk I/O for
virtual memory and no addr map
changes.  Tasks can be switched in half
a microsecond.

The scheduler checks if a task is
waiting for a certain time or waiting
on a message and skips it if it's not
ready.  A task runs until it
voluntarily yields ctrl with a call
to $LK,"Yield","MN:Yield"$ or gets preempted by the
timer interrupt.  See $LK,"SYS_TIMER_HANDLER","MN:SYS_TIMER_HANDLER"$.
The timer freqency is $LK,"JIFFY_FREQ","MN:JIFFY_FREQ"$.  Tasks
can ask not to be preempted with
$LK,"Preempt","MN:Preempt"$(ON/OFF).  Tasks waiting on I/O
often loop, checking for a stat and
swapping-out.  This does not really
degrade performance, though it pegs the
CPU Load.

The scheduler checks for a few critical
key-strokes such as <CTRL-ALT-X>, which
kills a task; <CTRL-ALT-DEL>, which reboots;
<CTRL-ALT-T>, which changes focus and <CTRL-ALT-C>,
which breaks execution of a program.

Each core has it's own circular task
que.  For AP processors, they have a
"Seth" task which stays in a loop
waiting for jobs or requests to spawn
tasks.  See $LK,"MPSethServerTask","MN:MPSethServerTask"$().

All cores have an idle task.  If the
scheduler determined an IRQ opportunity
hasn't happened, the scheduler swaps in
the idle task(enabling IRQs).

$HL,1$*/

asm {
USE64
// ************************************
SYS_CONTEXT_SAVE::
//OUT:	RSI=FS
	PUSH	RSI
	PUSHFD
	XOR	RSI,RSI
	MOV	RSI,FS:CTask.addr[RSI]
	POP	U64 CTask.rflags[RSI]
	POP	U64 CTask.rsi[RSI]
	MOV	U64 CTask.rax[RSI],RAX

/*
We divert the stk to the Task memory
and push onto it and divert it back.
It's a little faster.
*/
	MOV	RAX,RSP
	LEA	RSP,U64 CTask.r15+8[RSI]
	PUSH	R15
	PUSH	R14
	PUSH	R13
	PUSH	R12
	PUSH	R11
	PUSH	R10
	PUSH	R9
	PUSH	R8
	PUSH	RDI
	PUSH	RBP
	PUSH	RBX
	PUSH	RDX
	PUSH	RCX
	MOV	RSP,RAX

	RDTSC
	SHL	RDX,32
	ADD	RAX,RDX
	SUB	RAX,U64 CTask.time_slice_start[RSI]
	ADD	U64 CTask.total_time[RSI],RAX

	MOV	RAX,U64 CTask.fpu_mmx[RSI]
	FXSAVE	U64 [RAX]

	MOV	RDX,U64 CTask.bpt_lst[RSI]
@@05:	TEST	RDX,RDX
	JZ	@@10
	MOV	RDI,U64 CBpt.addr[RDX]
	MOV	AL,U8 CBpt.val[RDX]
	MOV	U8 [RDI],AL
	MOV	RDX,U64 CBpt.next[RDX]
	JMP	@@05
@@10:	LOCK
	BTR	U64 CTask.task_flags[RSI],TASKf_CURRENTLY_RUNNING
	RET
// ************************************
SYS_CONTEXT_RESTORE::
	XOR	RAX,RAX
	INC	U64 GS:CCPU.swap_cnter[RAX]
	MOV	RSI,FS:CTask.addr[RAX]
	BT	U32 CTask.rflags[RSI],RFLAGf_INT
	JNC	@@05
	LOCK	//We will be enabling IRQ's
	BTS	U32 GS:CCPU.cpu_flags[RAX],CPUf_HAD_IRQ_OPPORTUNITY
@@05:	LOCK
	BTS	U64 CTask.task_flags[RSI],TASKf_CURRENTLY_RUNNING
	BT	U64 CTask.task_flags[RSI],TASKf_DISABLE_BPTS
	JC	@@15
	MOV	RDX,U64 CTask.bpt_lst[RSI]
@@10:	TEST	RDX,RDX
	JZ	@@15
	MOV	RDI,U64 CBpt.addr[RDX]
	MOV	U8 [RDI],OC_BPT
	MOV	RDX,U64 CBpt.next[RDX]
	JMP	@@10

@@15:	INC	U64 CTask.swap_cnter[RSI]

	MOV	RAX,U64 CTask.fpu_mmx[RSI]
	FXRSTOR	U64 [RAX]

	RDTSC
	SHL	RDX,32
	ADD	RAX,RDX
	MOV	U64 CTask.time_slice_start[RSI],RAX

	MOV	RAX,RSP
	LEA	RSP,U64 CTask.rcx[RSI]
	POP	RCX
	POP	RDX
	POP	RBX
	POP	RBP
	POP	RDI
	POP	R8
	POP	R9
	POP	R10
	POP	R11
	POP	R12
	POP	R13
	POP	R14
	POP	R15
	MOV	RSP,RAX

	MOV	RAX,U64 CTask.rax[RSI]
	PUSH	GDT_ZERO_DS_SEL
	PUSH	U64 CTask.rsp[RSI]
	PUSH	U64 CTask.rflags[RSI]
	PUSH	GDT_CS64_SEL
	PUSH	U64 CTask.rip[RSI]
	MOV	RSI,U64 CTask.rsi[RSI]
	IRET
// ************************************
END_RSI_TASK:
	MOV	RAX,RSI
	CALL	SET_FS_BASE
SYS_TASK_END_NOW::
	CALL	&TaskEnd
	MOV	RSI,RAX
	CALL	SET_FS_BASE
	JMP	I32 RESTORE_RSI_TASK_WITH_KEY_CHECK

RESTORE_RSI_NO_PREEMPT_TASK:
	BT	U32 [SYS_CTRL_ALT_FLAGS],SYSf_CTRL_ALT_DEL
	JC	I32 HANDLE_SWAP_KEY_EVENT
	XOR	RAX,RAX
	CMP	U64 GS:CCPU.idle_task[RAX],RSI	//IDLE TASK HAS PREEMPT-OFF
	JNE	RESTORE_RSI_TASK_NO_KEY_CHECK
	JMP	I32 SWAP_IN_SETH_TASK

SYS_RET::
	RET

SYS_YIELD::
	PUSHFD
	PUSH	RAX
	MOV	RAX,SYS_SEMAS
	BT	U32 SYS_SEMA_SINGLE_USER*SEMA_STRUCT_SIZE[RAX],0
	JC	@@05	//If single user, don't change task.
	POP	RAX

	CLI
	PUSH	RSI
	XOR	RSI,RSI
	MOV	RSI,FS:CTask.addr[RSI]
	CMP	U64 CTask.next_task[RSI],RSI
	JNE	@@15	//Jmp if not just one task on core.

	PUSH	RAX
	MOV	RAX,U64 [SYS_JIFFIES]
	CMP	U64 CTask.wake_jiffy[RSI],RAX
	JG	@@10	//If needs to sleep, jmp and swap-out.

	POP	RAX
	POP	RSI
	POPFD
	RET

@@05:	POP	RAX
	POPFD
	RET

@@10:	POP	RAX
@@15:	POP	RSI
	CALL	SYS_CONTEXT_SAVE

	MOV	EBX,U32 SYS_RET
	MOV	U64 CTask.rip[RSI],RBX
	POP	U64 CTask.rflags[RSI]
	MOV	U64 CTask.rsp[RSI],RSP

RESTORE_NEXT_RSI_TASK:
	MOV	RSI,U64 CTask.next_task[RSI]

RESTORE_RSI_TASK_WITH_KEY_CHECK:
	TEST	U64 [SYS_CTRL_ALT_FLAGS],1<<SYSf_CTRL_ALT_DEL|
1<<SYSf_CTRL_ALT_TAB|1<<SYSf_CTRL_ALT_X|1<<SYSf_CTRL_ALT_C
	JNZ	HANDLE_SWAP_KEY_EVENT

RESTORE_RSI_TASK_NO_KEY_CHECK:
@@20:	BT	U64 CTask.task_flags[RSI],TASKf_KILL_TASK
	JC	I32 END_RSI_TASK
	TEST	U64 CTask.task_flags[RSI],1<<TASKf_AWAITING_MSG|1<<TASKf_SUSPENDED
	JNZ	@@25

	MOV	RAX,U64 [SYS_JIFFIES]
	CMP	U64 CTask.wake_jiffy[RSI],RAX
	JG	@@25	//Jmp if not ready, yet.

	CMP	U32 CTask.task_in_que_signature[RSI],TASK_IN_QUE_SIGNATURE_VAL
	JNE	SWAP_IN_SETH_TASK
	MOV	RAX,RSI
	CALL	SET_FS_BASE
	JMP	I32 SYS_CONTEXT_RESTORE

@@25:	MOV	RSI,U64 CTask.next_task[RSI]
	XOR	RAX,RAX
	CMP	U64 GS:CCPU.seth_task[RAX],RSI
	JNE	@@20	//Jmp if not Seth
	LOCK
	BTR	U32 GS:CCPU.cpu_flags[RAX],CPUf_HAD_IRQ_OPPORTUNITY
	JC	@@20	//Jmp if had chance for IRQ already
	MOV	RAX,U64 GS:CCPU.idle_task[RAX]
	MOV	RSP,U64 CTask.stk[RAX]
	ADD	RSP,DFT_STK+CTaskStk.stk_base	//Rst to top
	CALL	SET_FS_BASE
	STI	//Swap-in idle task so we can unmask IRQs.
	HLT
SYS_IDLE_PT::
	CLI

SWAP_IN_SETH_TASK:
	XOR	RAX,RAX
	MOV	RSI,GS:CCPU.seth_task[RAX]
	JMP	I32 RESTORE_RSI_TASK_WITH_KEY_CHECK

HANDLE_SWAP_KEY_EVENT:
	MOV	RAX,RSI
	CALL	SET_FS_BASE
	XOR	RAX,RAX
	MOV	RAX,GS:CCPU.num[RAX]
	TEST	RAX,RAX
	JNZ	I32 RESTORE_RSI_TASK_NO_KEY_CHECK

	MOV	EAX,U32 SYS_CTRL_ALT_FLAGS
	LOCK
	BTR	U32 [RAX],SYSf_CTRL_ALT_DEL
	JC	I32 &Reboot

	LOCK
	BTR	U32 [RAX],SYSf_CTRL_ALT_TAB
	JNC	@@05
	PUSH	RSI
	MOV	RSI,U64 [SYS_FOCUS_TASK]
	MOV	U64 [SYS_FOCUS_TASK],0
	CALL	FOCUS_RSI_NEXT_USER
	POP	RSI
	JMP	I32 RESTORE_FS_TASK_WITH_KEY_CHECK

@@05:	LOCK
	BTR	U32 [RAX],SYSf_CTRL_ALT_X
	JC	END_FOCUS_USER
	LOCK
	BTR	U32 [RAX],SYSf_CTRL_ALT_C
	JNC	I32 RESTORE_RSI_TASK_WITH_KEY_CHECK

BREAK_FOCUS_USER:
	MOV	RSI,U64 [SYS_FOCUS_TASK]
	TEST	RSI,RSI
	JZ	SWAP_IN_SETH_TASK
	BT	U64 CTask.task_flags[RSI],TASKf_USER
	JNC	I32 RESTORE_RSI_TASK_NO_KEY_CHECK
	LOCK
	BTR	U64 CTask.task_flags[RSI],TASKf_BREAK_LOCKED
	JNC	@@10
	LOCK
	BTS	U64 CTask.task_flags[RSI],TASKf_PENDING_BREAK
	JMP	I32 RESTORE_RSI_TASK_NO_KEY_CHECK

@@10:	MOV	RAX,&SysThrowBreak
	MOV	U64 CTask.rip[RSI],RAX
	BT	U64 CTask.task_flags[RSI],TASKf_BREAK_TO_SHIFT_ESC
	JC	@@15

//We do these two now, in case interrupt happens
	MOV	U64 CTask.wake_jiffy[RSI],0
	PUSH	RSI
	CALL	&TaskRstAwaitingMsg

@@15:	JMP	I32 RESTORE_RSI_TASK_NO_KEY_CHECK

END_FOCUS_USER:
	MOV	RSI,U64 [SYS_FOCUS_TASK]
	MOV	U64 [SYS_FOCUS_TASK],0
	PUSH	RSI
	CALL	FOCUS_RSI_NEXT_USER
	POP	RSI
	TEST	RSI,RSI
	JZ	I32 SWAP_IN_SETH_TASK
	MOV	RAX,RSI
	CALL	SET_FS_BASE
	BT	U64 CTask.task_flags[RSI],TASKf_USER
	JNC	I32 RESTORE_RSI_TASK_WITH_KEY_CHECK
	LOCK
	BTS	U64 CTask.task_flags[RSI],TASKf_KILL_TASK
	JMP	I32 END_RSI_TASK

RESTORE_FS_TASK_WITH_KEY_CHECK:
	XOR	RSI,RSI
	MOV	RSI,FS:CTask.addr[RSI]
	JMP	I32 RESTORE_RSI_TASK_WITH_KEY_CHECK

FOCUS_RSI_NEXT_USER:
	TEST	RSI,RSI
	JNZ	@@20
	MOV	ESI,U32 SYS_ADAM_TASK
@@20:	MOV	RDI,RSI
	MOV	RSI,U64 CTask.next_task[RSI]
@@25:	BT	U64 CTask.task_flags[ESI],TASKf_USER
	JNC	@@30
	CMP	U64 [SYS_FOCUS_TASK],RSI
	JE	@@30
	MOV	U64 [SYS_FOCUS_TASK],RSI
	MOV	RAX,U64 [SYS_EXTERN_TABLE]
	MOV	RAX,U64 EXT_WIN_TO_TOP*8[RAX]
	TEST	RAX,RAX
	JZ	@@35
	PUSH	TRUE
	PUSH	RSI
	CALL	RAX
	JMP	@@35
@@30:	MOV	RSI,U64 CTask.next_task[RSI]
	CMP	RSI,RDI
	JNE	@@25
	MOV	U64 [SYS_FOCUS_TASK],0
@@35:	RET

}

_extern SYS_CONTEXT_RESTORE U0 TaskContextRestore();
_extern SYS_YIELD U0 Yield();
_extern SYS_TASK_END_NOW U0 TaskEndNow();

U0 TaskQueIns(CTask *task,CTask *pred=NULL)
{
  CTask *last;
  PushFD;
  Cli;
  if (!pred) pred=Fs;
  last=pred->last_task;
  last->next_task=pred->last_task=task;
  task->last_task=last;
  task->next_task=pred;
  task->task_in_que_signature=TASK_IN_QUE_SIGNATURE_VAL;
  PopFD;
}

U0 TaskQueRem(CTask *task)
{
  CTask *next,*last;
  PushFD;
  Cli;
  task->task_in_que_signature=0;
  next=task->next_task;
  last=task->last_task;
  last->next_task=next;
  next->last_task=last;
  PopFD;
}

U0 TaskQueInsChild(CTask *task)
{
  CTask *last,*pred;
  PushFD;
  Cli;
  pred=task->parent_task->last_child_task;
  last=pred->last_sibling_task;
  last->next_sibling_task=pred->last_sibling_task=task;
  task->last_sibling_task=last;
  task->next_sibling_task=pred;
  PopFD;
}
