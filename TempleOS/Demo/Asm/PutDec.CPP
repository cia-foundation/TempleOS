asm {
PUT_DEC_U64::
//RAX is number to print in decimal.
//Preserves all regs.
	ENTER	24		//24 char buffer on stack
	PUSH_C_REGS		//$LK,"Save regs","FF:::/TempleOS/Kernel/Adam1a.HPP,PUSH_C_REGS"$ which C code is free to clobber
	PUSH	RSI		//This reg is not saved.
	LEA	RSI,-1[RBP]	//Load addr of end of buffer.
				//We picked RSI because of PUT_STR
				//We'll move backward
	MOV	U8 [RSI],0	//Terminator

	OR	RAX,RAX		//Special case -- zero
	JNZ	@@05
	DEC	RSI
	MOV	U8 [RSI],'0'
	JMP	@@15

@@05:	MOV	RBX,10		//Divides by 10

@@10:	XOR	RDX,RDX
	DIV	RBX		//RAX=(RDX:RAX)/RBX with remainder in RDX
	ADD	RDX,'0'
	DEC	RSI
	MOV	U8 [RSI],DL
	OR	RAX,RAX
	JNZ	@@10

@@15:	CALL	&PUT_STR
	POP	RSI
	POP_C_REGS
	LEAVE
	RET

_DO_IT::			//The convention is C-callable routines have underscores.
	MOV	RAX,122333221
	CALL	PUT_DEC_U64
	PUSH	1
	CALL	&NewLine	//Print one carriage return
	RET
}

Call(_DO_IT);
