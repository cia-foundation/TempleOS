/*Asm labels can only be defined once
in a task.  <F5> will spawn a new task
each time, so you don't get redefine
error, like when repeatedly #including
it from the cmd line.

These are many useful kernel
routines $LK,"::/Kernel/StrA.HC",A="FF:::/Kernel/StrA.HC,PUT_CHARS"$
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
//See $LK,"::/Compiler/OpCodes.DD"$.

MY_WORLD_MSG:
//Define U8 does not put terminating zeros
//on strings.
	DU8	"World\n",0;

//My convention is to put an underscore
//on C callable asm routines.
_HELLO_WORLD::
	PUSH	RSI	//See $LK,"REGG_LOCAL_VARS",A="MN:REGG_LOCAL_VARS"$ & $LK,"REGG_LOCAL_NON_PTR_VARS",A="MN:REGG_LOCAL_NON_PTR_VARS"$
	MOV	RCX,10
@@05:	MOV	RAX,RCX
	CALL	PUT_HEX_U8
	MOV	RAX,CH_SPACE
	CALL	PUT_CHARS
	MOV	RAX,'Hello ' //Supports multi-byte char consts
	CALL	PUT_CHARS
//We broke it in two pieces to show different ways.
	MOV	RSI,MY_WORLD_MSG
	CALL	PUT_STR
	LOOP	@@05
	POP	RSI
	RET
};

Call(_HELLO_WORLD);
