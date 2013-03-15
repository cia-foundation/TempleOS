/*
This is a JIT file, not AOT.

Asm labels can only be defined once
in a task.  Hit F5 to spawn a new task
and run it.
*/

//This is to demo glbl var access.
//Glbs defined elsewhere can accessed too, like sys_jiffies.
I64 glbl_hz=400,glbl_hz_step=100,glbl_hz_base=100;

asm {
//Opcodes are slightly different to make writing my x86_64 assembler easier.
//See $LK-A,"::/TempleOS/Compiler/OpCodes.TXT"$.
USE64	//Defaults to 64, but this is a good habit.


//See $LK,"::/TempleOS/Kernel/Str1a.CPP","FL:::/TempleOS/Kernel/Str1a.CPP,1"$ and $LK-A,"::/TempleOS/Kernel/Utils1a.CPP"$.
//These are asm syms and must be imported to work properly.
//When in doubt, import syms.
	IMPORT	PUT_STR,PUT_CHAR,PUT_HEX_U64;

JIFFIES_MSG:	DU8 "Jiffies:",0;

_BEEPS2::
//You can clobber RAX,RBX,RCX,RDX,R8,R9 in a pure asm subroutine.
	ENTER	0
	MOV	RCX,U64 SF_ARG1[RBP] //$LK,"SF_ARG1","FF:::/TempleOS/Kernel/Adam1a.HPP,SF_ARG1"$

	PUSH	U64 [&sys_jiffies]

@@05:	PUSH	RCX
//U0 Beep(I64 freq=500,Bool busy=FALSE)
	PUSH	FALSE	//Do not busy (spin) wait
	PUSH	U64 [&glbl_hz]	//evaluated at run time
	CALL	&Beep
	POP	RCX
	LOOP	@@05

	PUSH	RSI	//we must preserve this register.
	MOV	RSI,JIFFIES_MSG
	CALL	PUT_STR
	POP	RSI

	POP	RAX
	SUB	RAX,U64 [&sys_jiffies]
	NEG	RAX
	CALL	PUT_HEX_U64
	MOV	RAX,'\n'
	CALL	PUT_CHAR

	LEAVE
	RET1	8
}

//My convention is to put an underscore
//on C callable routines.
_extern _BEEPS2 U0 Beeps2(I64 cnt);

U0 AsmAndC2()
{
  I64 reg R15 i;

  i=PmtI64("$$FG,PURPLE$$\n\nNum of beeps 1-5 (%d):$$FG$$",3,1,5);
  Beeps2(i);

  asm {
LIST
//You can clobber RAX,RBX,RCX,RDX, but preserve the rest for code in a C+ routine.
USE64	//Defaults to 64, but this is a good habit.
	MOV	RCX,R15 //You can clobber RAX,RBX,RCX,RDX.
			    //You better preserve the rest.
@@05:	PUSH	RCX
//U0 Snd(F64 freq,I64 waveform=WF_SQUARE,F64 amp=1.0);
	MOV	RAX,1.0	//amplitude
	PUSH	RAX
	PUSH	WF_SQUARE

	MOV	RAX,RCX	//freq=loop*100.0+100.0
	IMUL2	RAX,glbl_hz_step	//evaluated at compile ttime
	ADD	RAX,U64 [&glbl_hz_base] //evaluated at run time
	PUSH	RAX
	FILD	U64 [RSP]	//Convert from int to floating
	FSTP	U64 [RSP]

	CALL	&Snd	//We can skip IMPORT for this symbol if JIT.

	MOV	RCX,0x20000000	//Simple delay loop
@@10:	LOOP	@@10

	POP	RCX
	LOOP	@@05
  }
  Snd(0);
}

AsmAndC2;
