/*$WW,1$
$TR-C,"Main Compiler"$
$ID,2$The mini compiler is like the main compiler, except the main compiler's lexical analyser removes comments and does preprocessing.  $LK,"Lex",A="MN:Lex"$(),$LK,"Echo",A="MN:Echo"$(ON).

The main compiler generates $LK,"Intermediate Code",A="FF:::/Compiler/CompilerA.HH,IC_END"$ at the parser stage.  See $LK,"PrsExpression",A="MN:PrsExpression"$(), $LK,"PrsStmt",A="MN:PrsStmt"$().

The main compiler optimizes   See $LK,"Intermediate Code Attributes",A="MN:intermediate_code_table"$, $LK,"Combining Consts",A="FF:::/Compiler/OptPass012.HC,case IC_MUL"$, $LK,"Choosing Reg Vars",A="FF:::/Compiler/OptPass3.HC,cmp.num_reg_vars"$.  Use $LK,"PassTrace",A="MN:PassTrace"$() to see the optimization stages.

The main compiler makes machine code in the back end.  See $LK,"IC Struct",A="MN:CIntermediateCode"$, $LK,"COCCompile",A="MN:COCCompile"$ and $LK,"OptPass789A",A="FF:::/Compiler/OptPass789A.HC,IC_MUL"$(), $LK,"BackEnd",A="FF:::/Compiler/BackA.HC,ICMul"$.  Set $LK,"Trace",A="MN:Trace"$(ON) to see the output of the backend.
$ID,-2$
$TR-C,"Mini Compiler"$
$ID,2$For this mini compiler, some things you should know about 64-bit asm:

* Putting a 0x48, known as the REX byte, in front of an inst makes it 64-bit size.

* "PUSH EAX", "POP EAX" and "XOR EAX,EAX" will behave as 64-bit even without REX because the stk is always 64 bit and because the XOR clears the upper 32-bits.

It is okay in TempleOS to change RAX, RBX, RCX, RDX, R8 and R9 without restoring them to their original values.
$ID,-2$$WW,0$*/

#define TK_EOF		0
#define TK_NUM		1
#define TK_OP		2
#define TK_LEFT		3
#define TK_RIGHT	4

#define OP_MUL		1
#define OP_DIV		2
#define OP_ADD		3
#define OP_SUB		4

I64 Lex(U8 **_src,I64 *num)
{//See $LK,"Lex",A="MN:Lex"$().
  U8 *src=*_src;
  I64 i;
  while (TRUE) {
    switch (*src) {
      case 0:
      case ';':
	*_src=src;
	return TK_EOF;
      case CH_SPACE:
      case '\r':
      case '\n':
	src++;
	break;
      case '0'...'9':
	i=0;
	do {
	  i=i*10+*src-'0';
	  src++;
	} while ('0'<=*src<='9');
	*num=i;
	*_src=src;
	return TK_NUM;
      case '*':
	*num=OP_MUL;
	*_src=src+1;
	return TK_OP;
      case '/':
	*num=OP_DIV;
	*_src=src+1;
	return TK_OP;
      case '+':
	*num=OP_ADD;
	*_src=src+1;
	return TK_OP;
      case '-':
	*num=OP_SUB;
	*_src=src+1;
	return TK_OP;
      case '(':
	*_src=src+1;
	return TK_LEFT;
      case ')':
	*_src=src+1;
	return TK_RIGHT;
      default:
	throw;
    }
  }
}

#define PREC_EOF	0
#define PREC_TERM	1
#define PREC_MUL	2
#define PREC_ADD	3
#define PREC_PAREN	4

extern I64 Parse(U8 **_src,U8 **_dst);

U0 PrsTerm(U8 **_src,U8 **_dst,I64 prec)
{//See $LK,"PrsExpression",A="MN:PrsExpression"$().
  I64 i;
  U8 *src2;
  U8 *dst2;
  if (Parse(_src,_dst)==PREC_TERM) {
    src2=*_src;
    dst2=*_dst;
    while (TRUE) {
//This is inefficient.  The main compiler doesn't back-up like this.
      i=Parse(&src2,&dst2);
      if (PREC_MUL<=i<prec) {
	*_src=src2;
	*_dst=dst2;
      } else
	break;
    }
  } else
    throw;
}

I64 Parse(U8 **_src,U8 **_dst)
{//See $LK,"PrsExpression",A="MN:PrsExpression"$().
//See $LK,"Opcode Fmts",A="FF:::/Compiler/OpCodes.DD,IDIV"$ for details on asm insts.
  I64 i;
  U8 *dst=*_dst;
  switch (Lex(_src,&i)) {
    case TK_EOF:
      *dst++=0x58; //POP RAX
      *dst++=0xC3; //RET
      *_dst=dst;
      return PREC_EOF;
    case TK_NUM:
      *dst++=0x48; //REX
      *dst++=0xB8; //MOV RAX,immediate num
      *dst(I64 *)++=i;

      *dst++=0x50; //PUSH RAX
      *_dst=dst;
      return PREC_TERM;
    case TK_LEFT:
      PrsTerm(_src,_dst,PREC_PAREN);
      if (Parse(_src,_dst)!=PREC_PAREN)
	throw;
      return PREC_TERM;
    case TK_RIGHT:
      return PREC_PAREN;
    case TK_OP:
      switch (i) {
	case OP_MUL:
	  PrsTerm(_src,&dst,PREC_MUL);
	  *dst++=0x5A; //POP RDX
	  *dst++=0x58; //POP RAX

	  *dst++=0x48; //REX
	  *dst++=0x0F;
	  *dst++=0xAF; //IMUL RAX,RDX
	  *dst++=0xC2;

	  *dst++=0x50; //PUSH RAX
	  *_dst=dst;
	  return PREC_MUL;
	case OP_DIV:
	  PrsTerm(_src,&dst,PREC_MUL);
	  *dst++=0x5B; //POP RBX
	  *dst++=0x58; //POP RAX

	  *dst++=0x33; //XOR RDX,RDX
	  *dst++=0xD2;

	  *dst++=0x48; //REX
	  *dst++=0xF7; //IDIV RBX
	  *dst++=0xFB;

	  *dst++=0x50; //PUSH RAX
	  *_dst=dst;
	  return PREC_MUL;
	case OP_ADD:
	  PrsTerm(_src,&dst,PREC_ADD);
	  *dst++=0x5A; //POP RDX
	  *dst++=0x58; //POP RAX

	  *dst++=0x48; //REX
	  *dst++=0x03; //ADD RAX,RDX
	  *dst++=0xC2;

	  *dst++=0x50; //PUSH RAX
	  *_dst=dst;
	  return PREC_ADD;
	case OP_SUB:
	  PrsTerm(_src,&dst,PREC_ADD);
	  *dst++=0x5A; //POP RDX
	  *dst++=0x58; //POP RAX

	  *dst++=0x48; //REX
	  *dst++=0x2B; //SUB RAX,RDX
	  *dst++=0xC2;

	  *dst++=0x50; //PUSH RAX
	  *_dst=dst;
	  return PREC_ADD;
      }
  }
}

U0 Main()
{
  U8 *src,*src2,*code,*dst;

  //Fixed size, no buffer overrun check.
  //You can make it fancier if you like.
  code=MAlloc(512,Fs->code_heap);

  while (TRUE) {
    "This will compile an expression\n"
	  "consisting of ints, parentheses\n"
	  "and the operators +,-,* and /.\n";
    src=GetStr;
    if (*src) {
      src2=src;
      dst=code;
      try {
	PrsTerm(&src2,&dst,PREC_PAREN);
	if (Parse(&src2,&dst)!=PREC_EOF)
	  throw;
	"$$RED$$This code is not efficient, but the compiler is simple.$$FG$$\n";
	Un(code,dst-code); //Unassemble the code we created.
//$LK,"Call",A="MN:Call"$() is a function.  See $LK,"_CALL",A="FF:::/Kernel/KUtils.HC,_CALL"$::
	//See also $LK,"CallInd",A="MN:CallInd"$().  See $LK,"_CALL_IND",A="FF:::/Kernel/KUtils.HC,_CALL_IND"$::
	"$$LTBLUE$$Answer:%d$$FG$$\n",Call(code);
      } catch {
	"$$RED$$Error$$FG$$\n";
	PutExcept;
      }
      Free(src);
    } else {
      Free(src);
      break;
    }
  }
  Free(code);
}

Main;

