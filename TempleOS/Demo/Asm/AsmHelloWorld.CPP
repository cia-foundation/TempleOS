/*
This is a JIT file, not AOT.

Asm labels can only be defined once
in a task.  Hit F5 to spawn a new task
and run it.

These are many useful kernel
routines $LK,"::/TempleOS/Kernel/Str1a.CPP","FF:::/TempleOS/Kernel/Str1a.CPP,PUT_CHAR"$
intended to be called from
asm.  Generally, they preserve
regs.

You can call any routine you
like, C or asm, if you import it.
Be aware that C routines do not
preserve RAX,RBX,RCX,RDX,R8,R9.
When calling from the shell or
from C, preserve all other regs.

*/

asm {
//Opcodes are slightly different to make writing my x86_64 assembler easier.
//See $LK-A,"::/TempleOS/Compiler/OpCodes.TXT"$.

USE64	//Defaults to 64, but this is a good habit.

	IMPORT	PUT_HEX_U8,PUT_CHAR;

MY_WORLD_MSG:
//Define U8 does not put terminating zeros
//on strings.
	DU8	"World\n",0;

//My convention is to put an underscore
//on C callable routines.
_HELLO_WORLD::
	PUSH	RSI	//See $LK,"CREG_VARS_MASK","MN:CREG_VARS_MASK"$ & $LK,"CREG_NON_PTR_MASK","MN:CREG_NON_PTR_MASK"$
	MOV	RCX,10
@@05:	MOV	RAX,RCX
	CALL	PUT_HEX_U8
	MOV	RAX,CH_SPACE
	CALL	PUT_CHAR
	MOV	RAX,'Hello ' //Supports multi-byte char constants
	CALL	PUT_CHAR
//We broke it in two pieces to show different ways.
	MOV	RSI,MY_WORLD_MSG
//Alternatively, if you use an '&', you don't need
//to import a SYS_SYM.  This only works for JIT asm.
	CALL	&PUT_STR
	LOOP	@@05
	POP	RSI
	RET
};

Call(_HELLO_WORLD);
