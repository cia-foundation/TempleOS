//This is the software interrupt num
//we will use.  See $LK,"Software Int's",A="MN:I_WAKE"$ for
//picking a num.  Nums are subject to change.

#define I_F_UNARY_MINUS	I_USER

asm {

//This changes the sign of the floating
//point val in RAX
F_UNARY_MINUS_INT::
	PUSH	RAX
	FLD	U64 [RSP]
	FCHS
	FSTP	U64 [RSP]
	POP	RAX
	IRET

//This does the same thing, but not as
//an interrupt.
F_UNARY_MINUS_CALL::
	PUSH	RAX
	FLD	U64 [RSP]
	FCHS
	FSTP	U64 [RSP]
	POP	RAX
	RET

//This invokes the interrupt version
//with a C callable function.
_F_UM_INT::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RAX,SF_ARG1[RBP]
	INT	I_F_UNARY_MINUS
	POP	RBP
	RET1	8

//This invokes the call version
//with a C callable function.
_F_UM_CALL::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RAX,SF_ARG1[RBP]
	CALL	F_UNARY_MINUS_CALL
	POP	RBP
	RET1	8
}

_extern _F_UM_INT F64 UnaryMinusInt(F64 d);
_extern _F_UM_CALL F64 UnaryMinusCall(F64 d);

#define SAMPLE_SIZE	1000000
U0 TimeIns()
{
  I64 start,end;
  I64 i,old_irq;

  CPURep;
  old_irq=IntEntrySet(I_F_UNARY_MINUS,F_UNARY_MINUS_INT,IDTET_TRAP);

  //Measure interrupt time
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++)
    UnaryMinusInt(ã);
  end=GetTSC;
  "Interrupt Cycles: %10.5f\n",ToF64(end-start)/SAMPLE_SIZE;

  //Measure call time
  start=GetTSC;
  for (i=0;i<SAMPLE_SIZE;i++)
    UnaryMinusCall(ã);
  end=GetTSC;
  "Call      Cycles: %10.5f\n",ToF64(end-start)/SAMPLE_SIZE;

  IntEntrySet(I_F_UNARY_MINUS,old_irq,IDTET_IRQ);
}

TimeIns;

/*  Program Output$HL,0$$WW+H,1$$FD,1$
$PT$$FG$$BG$6 Cores 3.395GHz
Interrupt Cycles:  573.98543
Call	  Cycles:    9.74349
$HL,1$*/
