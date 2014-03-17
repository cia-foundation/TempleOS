/*
This is a JIT file, not AOT.

Asm labels can only be defined once
in a task.  Hit F5 to spawn a new task
each time and run it.
*/

U0 AsmAndC3()
{
  I64 i;
  for (i=0;i<3;i++) {
	MOV	AL,0x41
	STC
	ADC	AL,0 //Round-about way to make the not special constant, 0x42.
	CALL	PUT_HEX_U8
	MOV	RCX,time_stamp_freq>>5 //JIT Cosnt.  Simple delay loop.
@@1:	LOOP	@@1
	MOV	RAX,' What?\n'
	CALL	PUT_CHARS
  }
}

AsmAndC3;
