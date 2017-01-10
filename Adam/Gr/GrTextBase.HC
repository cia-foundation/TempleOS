#help_index "TextBase Layer;Char/TextBase Layer"
#help_file "::/Doc/TextBase"

asm {
//************************************
_TEXT_CHAR::
//Bool TextChar(CTask *task,Bool allow_border=FALSE,I64 x,I64 y,I64 d);
//Must be called 30fps in Fs->draw_it() callback.
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RCX,U64 SF_ARG1[RBP]
	MOV	RBX,U64 CTask.scroll_x[RCX]
	SAR	RBX,3
	ADD	RBX,U64 SF_ARG3[RBP]
	MOV	RAX,U64 CTask.scroll_y[RCX]
	SAR	RAX,3
	ADD	RAX,U64 SF_ARG4[RBP]

	TEST	U8 SF_ARG2[RBP],0xFF
	JNZ	@@10

//Border not allowed
	TEST	RBX,RBX		//Check X
	JS	@@05
	ADD	RBX,U64 CTask.win_left[RCX]
	CMP	RBX,U64 CTask.win_right[RCX]
	JG	@@05
	TEST	RBX,RBX
	JS	@@05
	CMP	RBX,TEXT_COLS
	JGE	@@05
	TEST	RAX,RAX		//Check Y
	JS	@@05
	ADD	RAX,U64 CTask.win_top[RCX]
	CMP	RAX,U64 CTask.win_bottom[RCX]
	JG	@@05
	TEST	RAX,RAX
	JS	@@05
	CMP	RAX,TEXT_ROWS
	JGE	@@05
	JMP	@@15

@@05:	XOR	RAX,RAX		//return FALSE
	POP	RBP
	RET1	40

//Border allowed
@@10:	MOV	RDX,-1		//Check X
	CMP	RBX,RDX
	JL	@@05
	ADD	RBX,U64 CTask.win_left[RCX]
	MOV	RDX,U64 CTask.win_right[RCX]
	INC	RDX
	CMP	RBX,RDX
	JG	@@05
	TEST	RBX,RBX
	JS	@@05
	CMP	RBX,TEXT_COLS
	JGE	@@05
	MOV	RDX,-1		//Check Y
	CMP	RAX,RDX
	JL	@@05
	ADD	RAX,U64 CTask.win_top[RCX]
	MOV	RDX,U64 CTask.win_bottom[RCX]
	INC	RDX
	CMP	RAX,RDX
	JG	@@05
	TEST	RAX,RAX
	JS	@@05
	CMP	RAX,TEXT_ROWS
	JGE	@@05

@@15:	IMUL2	RAX,TEXT_COLS
	ADD	RBX,RAX
	SHL	RBX,2
	ADD	RBX,U64 [&gr.text_base]
	MOV	RAX,U64 SF_ARG5[RBP]
	MOV	U32 [RBX],EAX

	MOV	RAX,TRUE
	POP	RBP
	RET1	40
//************************************
_TEXT_LEN_STR::
//Bool TextLenStr(CTask *task,I64 x,I64 y,I64 len,I64 attr,U8 *s)
//Must be called 30fps in Fs->draw_it() callback.
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI
	MOV	RBX,U64 SF_ARG1[RBP]
	MOV	RSI,U64 SF_ARG6[RBP]
	MOV	RDI,U64 CTask.scroll_x[RBX]
	SAR	RDI,3
	ADD	RDI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG4[RBP]

	TEST	RDI,RDI
	JNS	@@05
	ADD	RCX,RDI
	SUB	RSI,RDI
	XOR	RDI,RDI
@@05:	ADD	RDI,U64 CTask.win_left[RBX]
	MOV	RDX,RCX
	ADD	RDX,RDI
	DEC	RDX
	CMP	RDX,U64 CTask.win_right[RBX]
	JLE	@@10
	MOV	RAX,RDX
	SUB	RAX,U64 CTask.win_right[RBX]
	SUB	RDX,RAX
	SUB	RCX,RAX
@@10:	TEST	RDI,RDI
	JNS	@@15
	ADD	RCX,RDI
	SUB	RSI,RDI
	XOR	RDI,RDI
@@15:	INC	RDX
	SUB	RDX,TEXT_COLS
	JLE	@@20
	SUB	RCX,RDX
@@20:	CMP	RCX,1
	JL	@@30

	MOV	RAX,U64 CTask.scroll_y[RBX]
	SAR	RAX,3
	ADD	RAX,U64 SF_ARG3[RBP]
	TEST	RAX,RAX
	JS	@@30
	ADD	RAX,U64 CTask.win_top[RBX]
	CMP	RAX,U64 CTask.win_bottom[RBX]
	JG	@@30
	TEST	RAX,RAX
	JS	@@30
	CMP	RAX,TEXT_ROWS
	JGE	@@30

	IMUL2	RAX,TEXT_COLS
	ADD	RDI,RAX
	SHL	RDI,2
	ADD	RDI,U64 [&gr.text_base]
	MOV	RAX,U64 SF_ARG5[RBP]
@@25:	LODSB
	STOSD
	DEC	RCX
	JNZ	@@25

	POP	RDI
	POP	RSI
	MOV	RAX,TRUE
	POP	RBP
	RET1	48

@@30:	POP	RDI
	POP	RSI
	XOR	RAX,RAX
	POP	RBP
	RET1	48
//************************************
_TEXT_LEN_ATTR_STR::
//Bool TextLenAttrStr(CTask *task,I64 x,I64 y,I64 len,U32 *_attr)
//Must be called 30fps in Fs->draw_it() callback.
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI
	MOV	RBX,U64 SF_ARG1[RBP]
	MOV	RSI,U64 SF_ARG5[RBP]
	MOV	RDI,U64 CTask.scroll_x[RBX]
	SAR	RDI,3
	ADD	RDI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG4[RBP]

	TEST	RDI,RDI
	JNS	@@05
	ADD	RCX,RDI
	SHL	RDI,2
	SUB	RSI,RDI
	XOR	RDI,RDI
@@05:	ADD	RDI,U64 CTask.win_left[RBX]
	MOV	RDX,RCX
	ADD	RDX,RDI
	DEC	RDX
	CMP	RDX,U64 CTask.win_right[RBX]
	JLE	@@10
	MOV	RAX,RDX
	SUB	RAX,U64 CTask.win_right[RBX]
	SUB	RDX,RAX
	SUB	RCX,RAX
@@10:	TEST	RDI,RDI
	JNS	@@15
	ADD	RCX,RDI
	SHL	RDI,2
	SUB	RSI,RDI
	XOR	RDI,RDI
@@15:	INC	RDX
	SUB	RDX,TEXT_COLS
	JLE	@@20
	SUB	RCX,RDX
@@20:	CMP	RCX,1
	JL	@@30

	MOV	RAX,U64 CTask.scroll_y[RBX]
	SAR	RAX,3
	ADD	RAX,U64 SF_ARG3[RBP]
	TEST	RAX,RAX
	JS	@@30
	ADD	RAX,U64 CTask.win_top[RBX]
	CMP	RAX,U64 CTask.win_bottom[RBX]
	JG	@@30
	TEST	RAX,RAX
	JS	@@30
	CMP	RAX,TEXT_ROWS
	JGE	@@30

	IMUL2	RAX,TEXT_COLS
	ADD	RDI,RAX
	SHL	RDI,2
	ADD	RDI,U64 [&gr.text_base]
@@25:	MOVSD
	DEC	RCX
	JNZ	@@25

	POP	RDI
	POP	RSI
	MOV	RAX,TRUE
	POP	RBP
	RET1	40

@@30:	POP	RDI
	POP	RSI
	XOR	RAX,RAX
	POP	RBP
	RET1	40
//************************************
_TEXT_LEN_ATTR:://Bool TextLenAttr(CTask *task,I64 x,I64 y,I64 len,I64 attr)
//Must be called 30fps in Fs->draw_it() callback.
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI
	MOV	RBX,U64 SF_ARG1[RBP]
	MOV	RDI,U64 CTask.scroll_x[RBX]
	SAR	RDI,3
	ADD	RDI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG4[RBP]

	TEST	RDI,RDI
	JNS	@@05
	ADD	RCX,RDI
	XOR	RDI,RDI
@@05:	ADD	RDI,U64 CTask.win_left[RBX]
	MOV	RDX,RCX
	ADD	RDX,RDI
	DEC	RDX
	CMP	RDX,U64 CTask.win_right[RBX]
	JLE	@@10
	MOV	RAX,RDX
	SUB	RAX,U64 CTask.win_right[RBX]
	SUB	RDX,RAX
	SUB	RCX,RAX
@@10:	TEST	RDI,RDI
	JNS	@@15
	ADD	RCX,RDI
	XOR	RDI,RDI
@@15:	INC	RDX
	SUB	RDX,TEXT_COLS
	JLE	@@20
	SUB	RCX,RDX
@@20:	CMP	RCX,1
	JL	@@35

	MOV	RAX,U64 CTask.scroll_y[RBX]
	SAR	RAX,3
	ADD	RAX,U64 SF_ARG3[RBP]
	TEST	RAX,RAX
	JS	@@35
	ADD	RAX,U64 CTask.win_top[RBX]
	CMP	RAX,U64 CTask.win_bottom[RBX]
	JG	@@35
	TEST	RAX,RAX
	JS	@@35
	CMP	RAX,TEXT_ROWS
	JGE	@@35

	IMUL2	RAX,TEXT_COLS
	ADD	RDI,RAX
	SHL	RDI,2
	ADD	RDI,U64 [&gr.text_base]
	MOV	RBX,U64 SF_ARG5[RBP]
	MOV	RSI,RDI
@@25:	LODSD
	TEST	AL,AL
	JNZ	@@30
	MOV	RAX,RBX
	STOSD
	DEC	RCX
	JNZ	@@25

@@30:	POP	RDI
	POP	RSI
	MOV	RAX,TRUE
	POP	RBP
	RET1	40

@@35:	POP	RDI
	POP	RSI
	XOR	RAX,RAX
	POP	RBP
	RET1	40
}
public _extern _TEXT_CHAR Bool TextChar(CTask *task,Bool allow_border=FALSE,
I64 x,I64 y,I64 d); //Plot char. 30fps in Fs->draw_it() callback.
public _extern _TEXT_LEN_STR Bool TextLenStr(CTask *task,I64 x,I64 y,I64 len,
I64 attr,U8 *s); //Plot str with len.  30fps in Fs->draw_it() callback.
public _extern _TEXT_LEN_ATTR_STR Bool TextLenAttrStr(CTask *task,I64 x,I64 y,
I64 len,U32 *_attr); //Plot attr str with len.	30fps in Fs->draw_it() callback.
public _extern _TEXT_LEN_ATTR Bool TextLenAttr(CTask *task,I64 x,I64 y,I64 len,
I64 attr); //Plot attrs with len.  30fps in Fs->draw_it() callback.

public U0 TextPrint(CTask *task,I64 x,I64 y,I64 attr,U8 *fmt,...)
{//Plot chars. 30fps in Fs->draw_it() callback.
//You probably want $LK,"GrPrint",A="MN:GrPrint"$() or just $LK,"Print",A="MN:Print"$().
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  TextLenStr(task,x,y,StrLen(buf),attr<<8,buf);
  Free(buf);
}

public U0 TextBorder(CTask *task=NULL,
	I64 l,I64 r,I64 t,I64 b,I64 attr,Bool solid)
{//Plot border square. 30fps in Fs->draw_it() callback.
//Draws window borders or DolDoc text bttn borders.
  //Set task=sys_winmgr_task for no clipping.
  I64 i;
  if (!task) task=Fs;
  attr<<=8;
  TextChar(task,,l-1,t-1,text.border_chars[6+solid]+attr);
  TextChar(task,,r+1,t-1,text.border_chars[8+solid]+attr);
  TextChar(task,,l-1,b+1,text.border_chars[10+solid]+attr);
  TextChar(task,,r+1,b+1,text.border_chars[12+solid]+attr);
  for (i=l;i<=r;i++) {
    TextChar(task,,i,t-1,text.border_chars[2+solid]+attr);
    TextChar(task,,i,b+1,text.border_chars[2+solid]+attr);
  }
  for (i=t;i<=b;i++) {
    TextChar(task,,l-1,i,text.border_chars[4+solid]+attr);
    TextChar(task,,r+1,i,text.border_chars[4+solid]+attr);
  }
}

public U0 TextRect(I64 l,I64 r,I64 t,I64 b,I64 d)
{//Fill text rect. 30fps in Fs->draw_it() callback.
  I64 y,w;
  if (l>r || t>b) return;
  if (t<0) t=0;
  if (b>=TEXT_ROWS) b=TEXT_ROWS-1;
  if (l<0) l=0;
  if (r>=TEXT_COLS) r=TEXT_COLS-1;
  if (w=r-l+1)
    for (y=t;y<=b;y++)
      MemSetU32(gr.text_base(U8 *)+(y*TEXT_COLS+l)*sizeof(U32),d,w);
}

