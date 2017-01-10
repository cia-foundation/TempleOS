/*On each core, tasks are linked in a
circular doubly-linked list queue with
the Seth task as the head.  On Core0,
the queue order represents the front-to-back
window stack order with the window mgr
as the wallpaper.

The scheduler is round-robin.  It checks
if a task is ready and runs it or skips it.
Swapping tasks just involves storing and
restoring regs (no disk I/O for virtual
memory and no addr map changes).  It is
always fully identity-mapped on all cores.
Tasks can be switched in half a microsecond.

The scheduler checks if a task is
waiting for a certain time or waiting
on a message and skips if not ready.
A task runs until it voluntarily yields ctrl
with a call to $LK,"Yield",A="MN:Yield"$().  Tasks waiting on I/O
often loop, checking for a status and
$LK,"Yield",A="MN:Yield"$ing.  This does not really degrade
performance, but pegs the CPU Load.

The scheduler checks for a few keys:

<CTRL-ALT-x> kill a task.
<CTRL-ALT-DEL> reboots.
<CTRL-ALT-n> Next task.
<CTRL-ALT-c> breaks execution of a program.

Each core has its own circular task queue.
For AP processors, they have a "Seth" task
which stays in a loop waiting for jobs or
requests to spawn tasks.  See $LK,"CoreAPSethTask",A="MN:CoreAPSethTask"$().
$HL,1$*/

U0 TaskFocusNext()
{
  CTask *task,*_task=sys_focus_task;
  sys_focus_task=NULL;
  if (!_task) _task=adam_task;
  task=_task->next_task;
  do {
    if (!Bt(&task->win_inhibit,WIf_SELF_FOCUS)) {
      sys_focus_task=task;
      CallExtNum(EXT_WIN_TO_TOP,task,TRUE);
      return;
    }
    task=task->next_task;
  } while (task!=_task);
}

asm {
TASK_CONTEXT_SAVE::
//OUT:	RSI=FS
	PUSH	RSI
	PUSHFD
	XOR	RSI,RSI
	MOV	RSI,FS:CTask.addr[RSI]
	POP	U64 CTask.rflags[RSI]
	POP	U64 CTask.rsi[RSI]
	MOV	U64 CTask.rax[RSI],RAX

/*Divert the stk to the Task memory
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
@@10:	RET
//************************************
_TASK_CONTEXT_RESTORE::
	XOR	RAX,RAX
	INC	U64 GS:CCPU.swap_cnter[RAX]
	MOV	RSI,FS:CTask.addr[RAX]
	BT	U32 CTask.rflags[RSI],RFLAGf_INT
	JNC	@@05
	BTS	U32 GS:CCPU.cpu_flags[RAX],CPUf_RAN_A_TASK
@@05:	BT	U64 CTask.task_flags[RSI],TASKf_DISABLE_BPTS
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
	PUSH	CGDT.ds
	PUSH	U64 CTask.rsp[RSI]
	PUSH	U64 CTask.rflags[RSI]
	PUSH	CGDT.cs64
	PUSH	U64 CTask.rip[RSI]
	MOV	RSI,U64 CTask.rsi[RSI]
	IRET
//************************************
END_RSI_TASK:
	MOV	RAX,RSI
	CALL	SET_FS_BASE
_TASK_END_NOW::
	CALL	&TaskEnd
	MOV	RSI,RAX
	CALL	SET_FS_BASE
	JMP	I8 RESTORE_RSI_TASK

_YIELD::
	PUSHFD
	TEST	U8 [SYS_SEMAS+SEMA_SINGLE_USER*DFT_CACHE_LINE_WIDTH],1
	JZ	@@05
	POPFD		//If single user, don't change task.
	RET

@@05:	CLI
	CALL	TASK_CONTEXT_SAVE
	MOV	EBX,U32 _RET
	MOV	U64 CTask.rip[RSI],RBX
	POP	U64 CTask.rflags[RSI]
	MOV	U64 CTask.rsp[RSI],RSP
	MOV	RSI,U64 CTask.next_task[RSI]

RESTORE_RSI_TASK:
	TEST	U64 [SYS_CTRL_ALT_FLAGS],1<<CTRL_ALT_DEL|
1<<CTRL_ALT_TAB|1<<CTRL_ALT_X|1<<CTRL_ALT_C
	JNZ	HANDLE_SYSF_KEY_EVENT

RESTORE_RSI_TASK2:
@@20:	BT	U64 CTask.task_flags[RSI],TASKf_KILL_TASK
	JC	END_RSI_TASK
	TEST	U64 CTask.task_flags[RSI],
		1<<TASKf_AWAITING_MSG|1<<TASKf_SUSPENDED
	JNZ	@@25

	MOV	RAX,U64 [&cnts.jiffies]
	CMP	U64 CTask.wake_jiffy[RSI],RAX
	JG	@@25	//Jmp if not ready, yet.

	MOV	RAX,RSI
	CALL	SET_FS_BASE
	JMP	I32 _TASK_CONTEXT_RESTORE

@@25:	MOV	RSI,U64 CTask.next_task[RSI]
	XOR	RAX,RAX
	CMP	U64 GS:CCPU.seth_task[RAX],RSI
	JNE	@@20	//Jmp if not Seth
	BTR	U32 GS:CCPU.cpu_flags[RAX],CPUf_RAN_A_TASK
	JC	@@20	//Jmp if had chance for IRQ already
	MOV	RAX,U64 GS:CCPU.idle_task[RAX]
	MOV	RSP,U64 CTask.stk[RAX]
	ADD	RSP,MEM_DFT_STK+CTaskStk.stk_base	//Rst to top
	CALL	SET_FS_BASE
	STI	//Restore idle task so we can unmask IRQs.
	HLT
SYS_IDLE_PT::
	CLI

RESTORE_SETH_TASK_IF_READY:
	XOR	RAX,RAX
	MOV	RSI,GS:CCPU.seth_task[RAX]
	JMP	RESTORE_RSI_TASK

HANDLE_SYSF_KEY_EVENT:
	MOV	RAX,RSI
	CALL	SET_FS_BASE
	XOR	RBX,RBX
	MOV	RAX,GS:CCPU.num[RBX]
	TEST	RAX,RAX
	JNZ	I32 RESTORE_RSI_TASK2

	MOV	EAX,U32 SYS_CTRL_ALT_FLAGS
	LOCK
	BTR	U32 [RAX],CTRL_ALT_DEL
	JC	I32 &Reboot

	CMP	U64 GS:CCPU.idle_task[RBX],RSI
	JE	RESTORE_SETH_TASK_IF_READY

	LOCK
	BTR	U32 [RAX],CTRL_ALT_TAB
	JNC	@@05
	CALL	&TaskFocusNext
	JMP	I32 RESTORE_FS_TASK

@@05:	LOCK
	BTR	U32 [RAX],CTRL_ALT_X
	JC	END_FOCUS_USER
	LOCK
	BTR	U32 [RAX],CTRL_ALT_C
	JNC	I32 RESTORE_RSI_TASK

BREAK_FOCUS_USER:
	MOV	RSI,U64 [SYS_FOCUS_TASK]
	TEST	RSI,RSI
	JZ	RESTORE_SETH_TASK_IF_READY
	BT	U64 CTask.win_inhibit[RSI],WIf_SELF_FOCUS
	JC	I32 RESTORE_RSI_TASK
	LOCK
	BTR	U64 CTask.task_flags[RSI],TASKf_BREAK_LOCKED
	JNC	@@10
	LOCK
	BTS	U64 CTask.task_flags[RSI],TASKf_PENDING_BREAK
	JMP	I32 RESTORE_RSI_TASK

@@10:	MOV	RAX,&Break
	MOV	U64 CTask.rip[RSI],RAX
	BT	U64 CTask.task_flags[RSI],TASKf_BREAK_TO_SHIFT_ESC
	JC	I32 RESTORE_RSI_TASK

//Do these now, in case interrupt happens.
	MOV	U64 CTask.wake_jiffy[RSI],0
	PUSH	RSI
	CALL	&TaskRstAwaitingMsg
	JMP	I32 RESTORE_RSI_TASK

END_FOCUS_USER:
	MOV	RSI,U64 [SYS_FOCUS_TASK]
	CALL	&TaskFocusNext
	TEST	RSI,RSI
	JZ	I32 RESTORE_SETH_TASK_IF_READY
	MOV	RAX,RSI
	CALL	SET_FS_BASE
	BT	U64 CTask.win_inhibit[RSI],WIf_SELF_FOCUS
	JC	I32 RESTORE_RSI_TASK
	LOCK
	BTS	U64 CTask.task_flags[RSI],TASKf_KILL_TASK
	JMP	I32 END_RSI_TASK

RESTORE_FS_TASK:
	XOR	RSI,RSI
	MOV	RSI,FS:CTask.addr[RSI]
	JMP	I32 RESTORE_RSI_TASK
}

_extern _TASK_CONTEXT_RESTORE U0 TaskContextRestore(); //Restore a task context.
_extern _YIELD U0 Yield(); //Yield cpu to next task.
_extern _TASK_END_NOW U0 TaskEndNow(); //Terminate current task.

U0 TaskQueIns(CTask *task,CTask *pred=NULL)
{//Insert a task in the scheduler running task queue.
//You have no business with this, probably.
  CTask *last;
  PUSHFD
  CLI
  if (!pred) pred=Fs;
  last=pred->last_task;
  last->next_task=pred->last_task=task;
  task->last_task=last;
  task->next_task=pred;
  POPFD
}

U0 TaskQueRem(CTask *task)
{//Remove a task from the scheduler running task queue.
//Use $LK,"Suspend",A="MN:Suspend"$().
  CTask *next,*last;
  PUSHFD
  CLI
  next=task->next_task;
  last=task->last_task;
  last->next_task=next;
  next->last_task=last;
  POPFD
}

U0 TaskQueInsChild(CTask *task)
{
  CTask *last,*pred;
  PUSHFD
  CLI
  pred=task->parent_task->last_child_task;
  last=pred->last_sibling_task;
  last->next_sibling_task=pred->last_sibling_task=task;
  task->last_sibling_task=last;
  task->next_sibling_task=pred;
  POPFD
}
