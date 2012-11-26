/*
"Fixed point" means you use integers
that are scaled by a value.  A common
example would be using number of pennies
instead of dollars with a float.

Fixed-point used to be much faster,
but modern processors do well with
floats.  It also depends on the compiler
and my compiler used to do floats poorly.

I often use 64-bit ints with upper 32-bits
as integer and lower 32-bits as fraction.

See $LK,"::/SparrowOS/Demo/SubIntAccess.CPP.Z","FI:::/SparrowOS/Demo/SubIntAccess.CPP.Z"$ for how
to access upper or lower 32-bits.

*/
#define SAMPLE_SIZE	10000000

I32 coordinates[65536];

asm {
	IMPORT coordinates;

_ASM_FIXED_POINT::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,coordinates
	MOV	RDI,ToI64(Sin(1.0)*0x100000000)
	XOR	RBX,RBX	//SUM
	MOV	RCX,SAMPLE_SIZE-1
@@05:	XOR	RDX,RDX
	MOV	DX,CX
	MOVSXD	RAX,U32 [RSI+RDX*4]
	IMUL	RDI
	SAR	RAX,32
	ADD	RBX,RAX
	DEC	RCX
	JGE	@@05
	MOV	RAX,RBX
	POP	RDI
	POP	RSI
	LEAVE
	RET

SINE_VAL:	DU64	Sin(1.0);
RET_VAL:	DU64	0;

_ASM_FLOAT::
	ENTER	0
	PUSH	RSI
	MOV	RSI,coordinates
	FLD	U64 [SINE_VAL]
	FLDZ
	MOV	RCX,SAMPLE_SIZE-1
@@05:	XOR	RDX,RDX
	MOV	DX,CX
	FILD	U32 [RSI+RDX*4]
	FMUL	ST0,ST2
	FADDP	ST1,ST0
	DEC	RCX
	JGE	@@05
	FISTP	U64 [RET_VAL]
	MOV	RAX,U64 [RET_VAL]
	FFREE	ST0
	FINCSTP
	POP	RSI
	LEAVE
	RET
}

_extern _ASM_FIXED_POINT I64 AsmFixedPt();
_extern _ASM_FLOAT I64 AsmFloat();

U0 Main()
{
  I64 start,end,overhead_time,test_time;
  F64 d1,fsum;
  I64 reg i,temp,reg d2,reg sum;
  Bool old_preempt=Preempt(OFF);

  CPURep;

  //Set-up some sample coordinates
  for (i=0;i<65536;i++)
    coordinates[i]=RandU32;

//Measure Loop Overhead
  start=GetTimeStamp;
  for (i=SAMPLE_SIZE-1;i>=0;i--) {
  }
  end=GetTimeStamp;
  overhead_time=end-start;
  "$$FG,RED$$Overhead Cycles       :%10.5f$$FG$$\n",
    ToF64(overhead_time)/SAMPLE_SIZE;

//Measure F64 arithmetic
// (Some of this is due to crappy
// compiler code.)
  d1=Sin(1.0);
  fsum=0;
  start=GetTimeStamp;
  for (i=SAMPLE_SIZE-1;i>=0;i--)
    fsum+=d1*coordinates[i&65535];
  end=GetTimeStamp;
  test_time=end-start;
  "Float Sum             :%X\n",ToI64(fsum);
  "$$FG,RED$$Float Cycles          :%10.5f$$FG$$\n",
    ToF64(test_time)/SAMPLE_SIZE;


//Measure fixed point arithmetic
  d2=Sin(1.0)*0x100000000;
  sum=0;
  start=GetTimeStamp;
  for (i=SAMPLE_SIZE-1;i>=0;i--) {
	  temp=d2*coordinates[i&65535];
    sum+=temp.i32[1];
  }
  end=GetTimeStamp;
  test_time=end-start;
  "Fixed Point Sum       :%X\n",sum;
  "$$FG,RED$$Fixed Point Cycles    :%10.5f$$FG$$\n",
    ToF64(test_time)/SAMPLE_SIZE;

//Measure fixed point arithmetic
  start=GetTimeStamp;
  sum=AsmFixedPt;
  end=GetTimeStamp;
  test_time=end-start;
  "Asm Fixed Point Sum   :%X\n",sum;
  "$$FG,RED$$Asm Fixed Point Cycles:%10.5f$$FG$$\n",
    ToF64(test_time)/SAMPLE_SIZE;

//Measure float arithmetic
  start=GetTimeStamp;
  sum=AsmFloat;
  end=GetTimeStamp;
  test_time=end-start;
  "Asm Float Sum         :%X\n",sum;
  "$$FG,RED$$Asm Float Cycles      :%10.5f$$FG$$\n",
    ToF64(test_time)/SAMPLE_SIZE;

  Preempt(old_preempt);
}

Main;

/*  Program Output$HL,0$$WW+H,1$$FD,1$

8 Cores 2.660GHz
$FG,4$Overhead Cycles       :   2.00814$FG$
Float Sum             :FFFFE1D361BEED68
$FG,4$Float Cycles          :  10.16076$FG$
Fixed Point Sum       :FFFFE1D361729914
$FG,4$Fixed Point Cycles    :   5.29392$FG$
Asm Fixed Point Sum   :FFFFE1D361729914
$FG,4$Asm Fixed Point Cycles:   4.20464$FG$
Asm Float Sum         :FFFFE1D361BEED56
$FG,4$Asm Float Cycles      :   3.04635$FG$

$HL,1$*/
