/*
This is a JIT file, not AOT.

Asm labels can only be defined once
in a task.  Hit F5 to spawn a new task
each time and run it.
*/

asm {
//Opcodes are slightly different to make writing my x86_64 assembler easier.
//See $LK,"::/Compiler/OpCodes.TXT"$.
USE64	//Defaults to 64, but this is a good habit.
	IMPORT	Beep;

_BEEPS::
//You can always clobber RAX,RBX,RCX,RDX,R8,R9
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RCX,U64 SF_ARG1[RBP]  //$LK,"SF_ARG1",A="FF:::/Kernel/Adam1a.HPP,SF_ARG1"$

@@05:	PUSH	RCX
//U0 Beep(I64 freq=500,Bool busy=FALSE)
	PUSH	FALSE	//Do not busy (spin) wait
	PUSH	500	//Hz
	CALL	Beep
	POP	RCX
	LOOP	@@05

	POP	RBP
//Return vals are in RAX
	RET1	8
}


//_extern binds a asm sym to a function.
//My convention is to put an underscore
//on C callable routines.
_extern _BEEPS U0 Beeps(I64 cnt);

I64 AsmAndC1()
{
  I64 noreg i;	//Normally this would be stored in a reg
		//Check with $LK,"Uf",A="MN:Uf"$("AsmAndC1").

  i=PmtI64("Num of beeps 1-5 (%d):",3,1,5);
  Beeps(i);

  asm {
//You can clobber RAX,RBX,RCX,RDX
USE64	//Defaults to 64, but this is a good habit.
	IMPORT	Snd; //Import an not use & or don't import and use &Snd.
	MOV	RCX,&i[RBP] //You can clobber RAX,RBX,RCX,RDX.
			    //You better preserve the rest.

@@05:	PUSH	RCX
//U0 Snd(F64 freq);

	MOV	RAX,RCX	//freq=loop*100.0+100.0
	IMUL2	RAX,100 //TempleOS uses nonstandard opcodes
			//to avoid multiple form of the same one.
			//See $LK,"::/Compiler/OpCodes.TXT"$.
	ADD	RAX,100
	PUSH	RAX
	FILD	U64 [RSP]	//Convert from int to floating
	FSTP	U64 [RSP]

	CALL	Snd

	MOV	RCX,time_stamp_freq>>3	//JIT Const.  Simple delay loop
@@10:	LOOP	@@10

	POP	RCX
	LOOP	@@05
  }
  Snd(0);
  return i;
}

"Beeps:%d\n",AsmAndC1;
