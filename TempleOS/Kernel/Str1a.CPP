U8 hex_digits[16]="0123456789ABCDEF";
U8 *Bin2Hex(U8 *dst,I64 n;I64 width)
{
  U8 *result=dst+width;
  dst=result;
  while (width--) {
    *--dst=hex_digits[n&15];
    n>>=4;
  }
  return result;
}

U0 PutHex(I64 n,I64 width)
{
  U8 buf[17];
  if (width>16) width=16;
  *Bin2Hex(buf,n,width)=0;
  "%s",buf;
}

asm {
USE64
// ************************************
// IN:	RAX=NUM TO PRINT
PUT_HEX_U64::
	PUSH_C_REGS
	PUSH	16
	PUSH	RAX
	CALL	&PutHex
	POP_C_REGS
	RET
PUT_HEX::
PUT_HEX_U32::
	PUSH_C_REGS
	PUSH	8
	PUSH	RAX
	CALL	&PutHex
	POP_C_REGS
	RET
PUT_HEX_U16::
	PUSH_C_REGS
	PUSH	4
	PUSH	RAX
	CALL	&PutHex
	POP_C_REGS
	RET
PUT_HEX_U8::
	PUSH_C_REGS
	PUSH	2
	PUSH	RAX
	CALL	&PutHex
	POP_C_REGS
	RET
// ************************************
PUT_CHAR::
// IN:	RAX=Char
	PUSH_C_REGS
	PUSH	RAX
	CALL	&PutChar
	POP_C_REGS
	RET
PUT_STR::
// IN:	RSI=String
	PUSH_C_REGS
	PUSH	RSI
	CALL	&PutS
	POP_C_REGS
	RET
// ************************************
_STRCAT::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG1[RBP]
@@05:	LODSB
	TEST	AL,AL
	JNZ	@@05
	MOV	RDI,RSI
	DEC	RDI
	MOV	RSI,U64 SF_ARG2[RBP]
	TEST	RSI,RSI
	JZ	@@15
@@10:	LODSB
	STOSB
	TEST	AL,AL
	JNZ	@@10
@@15:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_STRCPY::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RDI,U64 SF_ARG1[RBP]
	TEST	RDI,RDI
	JZ	@@15
	MOV	RSI,U64 SF_ARG2[RBP]
	TEST	RSI,RSI
	JNZ	@@05
	XOR	RAX,RAX
	JMP	@@10
@@05:	LODSB
@@10:	STOSB
	TEST	AL,AL
	JNZ	@@05
@@15:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_STRCMP::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG2[RBP]
	MOV	RDI,U64 SF_ARG1[RBP]
@@05:	LODSB
	TEST	AL,AL
	JZ	@@20
	SCASB
	JE	@@05
	JA	@@15
@@10:	MOV	RAX,1
	JMP	@@25
@@15:	MOV	RAX,-1
	JMP	@@25
@@20:	SCASB
	JNE	@@10
	XOR	RAX,RAX
@@25:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
TO_UPPER::
	CMP	AL,'a'
	JB	@@05
	CMP	AL,'z'
	JA	@@05
	ADD	AL,'A'-'a'
@@05:	RET
// ************************************
_STRICMP::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG2[RBP]
	MOV	RDI,U64 SF_ARG1[RBP]
@@05:	LODSB
	TEST	AL,AL
	JZ	@@30
	CMP	AL,'a'
	JB	@@10
	CMP	AL,'z'
	JA	@@10
	ADD	AL,'A'-'a'
@@10:	MOV	BL,U8 [RDI]
	INC	RDI
	CMP	BL,'a'
	JB	@@15
	CMP	BL,'z'
	JA	@@15
	ADD	BL,'A'-'a'
@@15:	CMP	AL,BL
	JE	@@05
	JA	@@25
@@20:	MOV	RAX,1
	JMP	@@35
@@25:	MOV	RAX,-1
	JMP	@@35
@@30:	MOV	BL,U8 [RDI]
	TEST	BL,BL
	JNE	@@20
	XOR	RAX,RAX
@@35:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_STRNCMP::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RCX,U64 SF_ARG3[RBP]
	MOV	RSI,U64 SF_ARG2[RBP]
	MOV	RDI,U64 SF_ARG1[RBP]
@@05:	TEST	RCX,RCX
	JZ	@@25
	DEC	RCX
	LODSB
	TEST	AL,AL
	JZ	@@20
	SCASB
	JE	@@05
	JA	@@15
@@10:	MOV	RAX,1
	JMP	@@30
@@15:	MOV	RAX,-1
	JMP	@@30
@@20:	MOV	BL,U8 [RDI]
	TEST	BL,BL
	JNE	@@10
@@25:	XOR	RAX,RAX
@@30:	POP	RDI
	POP	RSI
	LEAVE
	RET1	24
// ************************************
_STRNICMP::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RCX,U64 SF_ARG3[RBP]
	MOV	RSI,U64 SF_ARG2[RBP]
	MOV	RDI,U64 SF_ARG1[RBP]
@@05:	TEST	RCX,RCX
	JZ	@@35
	DEC	RCX
	LODSB
	TEST	AL,AL
	JZ	@@30
	CMP	AL,'a'
	JB	@@10
	CMP	AL,'z'
	JA	@@10
	ADD	AL,'A'-'a'
@@10:	MOV	BL,U8 [RDI]
	INC	RDI
	CMP	BL,'a'
	JB	@@15
	CMP	BL,'z'
	JA	@@15
	ADD	BL,'A'-'a'
@@15:	CMP	AL,BL
	JE	@@05
	JA	@@25
@@20:	MOV	RAX,1
	JMP	@@40
@@25:	MOV	RAX,-1
	JMP	@@40
@@30:	SCASB
	JNE	@@20
@@35:	XOR	RAX,RAX
@@40:	POP	RDI
	POP	RSI
	LEAVE
	RET1	24
// ************************************
_STRMATCH::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG2[RBP]
	TEST	RSI,RSI
	JZ	@@25
	MOV	RDI,U64 SF_ARG1[RBP]
	TEST	RDI,RDI
	JZ	@@25
	MOV	DL,U8 [RDI]
	TEST	DL,DL
	JZ	@@20
	JMP	@@10
@@05:	INC	RSI
@@10:	LODSB
	TEST	AL,AL
	JZ	@@25
	CMP	AL,DL
	JNE	@@10
	DEC	RSI
	MOV	RCX,1
@@15:	MOV	AL,U8 [RDI+RCX]
	TEST	AL,AL
	JZ	@@20
	CMP	AL,U8 [RSI+RCX]
	JNE	@@05
	INC	RCX
	JMP	@@15

	DEC	RSI
@@20:	MOV	RAX,RSI
	JMP	@@30
@@25:	XOR	RAX,RAX
@@30:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
// ************************************
_STRIMATCH::
	ENTER	0
	PUSH	RSI
	PUSH	RDI
	MOV	RSI,U64 SF_ARG2[RBP]
	TEST	RSI,RSI
	JZ	@@25
	MOV	RDI,U64 SF_ARG1[RBP]
	TEST	RDI,RDI
	JZ	@@25
	MOV	AL,U8 [RDI]
	CALL	TO_UPPER
	MOV	DL,AL
	TEST	DL,DL
	JZ	@@20
	JMP	@@10
@@05:	INC	RSI
@@10:	LODSB
	CALL	TO_UPPER
	TEST	AL,AL
	JZ	@@25
	CMP	AL,DL
	JNE	@@10
	DEC	RSI
	MOV	RCX,1
@@15:	MOV	AL,U8 [RDI+RCX]
	CALL	TO_UPPER
	TEST	AL,AL
	JZ	@@20
	MOV	BL,U8 [RSI+RCX]
	XCHG	AL,BL
	CALL	TO_UPPER
	CMP	AL,BL
	JNE	@@05
	INC	RCX
	JMP	@@15

	DEC	RSI
@@20:	MOV	RAX,RSI
	JMP	@@30
@@25:	XOR	RAX,RAX
@@30:	POP	RDI
	POP	RSI
	LEAVE
	RET1	16
}
_extern _STRCMP I64 StrCmp(U8 *st1,U8 *st2);
_extern _STRICMP I64 StrICmp(U8 *st1,U8 *st2);
_extern _STRNCMP I64 StrNCmp(U8 *st1,U8 *st2,I64 n);
_extern _STRNICMP I64 StrNICmp(U8 *st1,U8 *st2,I64 n);
_extern _STRMATCH U8 *StrMatch(U8 *pattern,U8 *st);
_extern _STRIMATCH U8 *StrIMatch(U8 *pattern,U8 *st);
_extern _STRCPY U0 StrCpy(U8 *dst,U8 *src);
_extern _STRCAT U0 StrCat(U8 *dst,U8 *src);

/*
These bitmaps go to 0-511 so that
$LK,"Lex","MN:Lex"$() can use them with $LK,"Token Codes","MN:TK_EOF"$.
*/
U32 alpha_bitmap[16]=
{0x0000000,0x00000000,0x87FFFFFF,0x07FFFFFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 alpha_numeric_bitmap[16]=
{0x0000000,0x03FF0000,0x87FFFFFF,0x07FFFFFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 alpha_numeric_no_at_bitmap[16]=
{0x0000000,0x03FF0000,0x87FFFFFE,0x07FFFFFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 word_bitmap[16]=
{0x0000000,0x03FF0080,0x87FFFFFE,0x07FFFFFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 filename_chars_bitmap[16]=
{0x0000000,0x03FF73FB,0xEFFFFFFF,0x2FFFFFFF,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 dec_numeric_bitmap[16]=
{0x0000000,0x03FF0000,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 hex_numeric_bitmap[16]=
{0x0000000,0x03FF0000,0x7E,0x7E,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 white_space_bitmap[16]=
{0x80003600,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 non_eol_white_space_bitmap[16]=
{0x80001200,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 zero_cr_nl_cursor_bitmap[16]=
{0x00002421,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 zero_tab_ff_cr_nl_cursor_bitmap[16]=
{0x00003621,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 zero_tab_ff_cr_nl_cursor_dollar_bitmap[16]=
{0x00003621,0x10,0,0,0,0,0,0,
0,0,0,0,0,0,0,0};
U32 getkey_chars_bitmap[16]=
{0x80002600,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 printable_chars_bitmap[16]=
{0x80003600,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 displayable_chars_bitmap[16]=
{0x80000000,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};
U32 safe_dollar_chars_bitmap[16]=
{0x80000000,0xFFFFFFEF,0xFFFFFFFF,0x7FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0,0,0,0,0,0,0,0};//same but no dollar sign
U32 non_eol_chars_bitmap[16]=
{0xFFFFDBFE,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

U8 *LstSub(I64 sub, U8 *lst)
{//Point to lst entry.
 //Not especially efficient but handy.  Use an array of U8*'s for efficiency.
  if (!lst) return NULL;
  while (*lst && sub>0) {
    while (*lst)  // Advance to end of cur entry.
      lst++;
    lst++;    // Skip trailing zero
    if (*lst=='@') // Check for '@' alias lst entry
      lst++;
    else
      sub--;
  }
  if (sub||!*lst)
    return NULL;
  else
    return lst;
}

I64 LstMatch(U8 *pattern, U8 *lst,I64 flags=0)
// RETURNS: -2 if Ambiguous, -1 if not found, else entry num
{ //Not especially efficient but handy.  Use hash tables for efficiency.
  I64 n,sub=0,result=-1;
  U8 *ptr;
  Bool exact_match=FALSE;
  if (!lst) return -1;
  n=StrLen(pattern);
  while (*lst) {
    if (*lst=='@') { //Check for '@' alias lst entry
      sub--;
      lst++;
    }
    ptr=pattern;
    if (flags & LMF_IGNORE_CASE)
      while (*ptr && ToUpper(*ptr)==ToUpper(*lst)) {
	ptr++;
	lst++;
      }
    else
      while (*ptr && *ptr==*lst) {
	ptr++;
	lst++;
      }
    if (!*ptr) {	  // Did we reach end of pattern
      if (!*lst)  // Did we reach end of lst entrycount
	return sub;  // Found Exact match
      else {
	if (result!=-1) {
	  if (!exact_match)
	    result=-2; // AMBIGUOUS RESPONSE unless later exact match
	} else {
	  if (!(flags & LMF_EXACT))
	    result=sub;
	}
      }
    }
    while (*lst)  // Advance to end of cur entry.
      lst++;
    lst++;    // Skip trailing zero
    sub++;
  }
  return result;
}

U0 UndefinedDefine(U8 *name)
{
  ST_ERR_ST "Undefined Define: '%s'.\n",name;
  throw(EXCEPT_UNDEF_DEFINE,name);
}


U8 *Define(U8 *name)
{
  CHashDefineStr *temph;
  if (temph=HashFind(name,Fs->hash_table,HTT_DEFINE_STR))
    return temph->data;
  else if (name)
    UndefinedDefine(name);
  else
    return NULL;
}

U8 *DefineSub(I64 sub,U8 *name)
{
  CHashDefineStr *temph;
  if (temph=HashFind(name,Fs->hash_table,HTT_DEFINE_STR)) {
    if (0<=sub<temph->cnt)
      return temph->sub_lst[sub];
    else
      return NULL;
  } else if (name)
    UndefinedDefine(name);
  else
    return NULL;
}

I64 DefineSubCnt(U8 *name)
{
  CHashDefineStr *temph;
  if (temph=HashFind(name,Fs->hash_table,HTT_DEFINE_STR))
    return temph->cnt;
  else if (name)
    UndefinedDefine(name);
  else
    return -1;
}

I64 DefineMatch(U8 *pattern,U8 *lst_name,I64 flags=0)
{
  return LstMatch(pattern,Define(lst_name),flags);
}

U0 PutDefine(U8 *name)
{
  "%s",Define(name);
}

U0 GetOutOfDollar()
{
  CDoc *doc;
  if (IsRaw) {
    if (sys_raw_flags&RWF_IN_DOLLAR)
      '$$';
  } else {
    if (fp_doc_put && (doc=(*fp_doc_put)(Fs)) && doc->flags&DOCF_IN_DOLLAR)
      '$$';
  }
}

U0 NewLine(I64 cnt=1)
{
  while (cnt-->0)
    '\n';
}

U0 Spaces(I64 cnt=1)
{
  while (cnt-->0)
    '' CH_SPACE;
}

U0 PutErr(U8 *st=NULL)
{
  GetOutOfDollar;
  PutDefine("ST_ERR");
  if (st) "%s\n",st;
}

U0 PutWarn(U8 *st=NULL)
{
  GetOutOfDollar;
  PutDefine("ST_WARN");
  if (st) "%s\n",st;
}

U0 PutDefineErr(U8 *st)
{
  PutErr;
  PutDefine(st);
}

U0 PutDefineWarn(U8 *st)
{
  PutWarn;
  PutDefine(st);
}

Bool YorN()
{
  I64 ch;
  "(y or n)? ";
  while (TRUE) {
    ch=ToUpper(GetChar(,FALSE));
    if (ch=='Y') {
      "$$PT$$YES$$FG$$\n";
      return TRUE;
    } else if (ch=='N') {
      "$$PT$$NO$$FG$$\n";
      return FALSE;
    }
  }
}

I64 PressAKey()
{
  PutDefine("ST_PRESS_A_KEY");
  return GetChar(,FALSE);
}

Bool AreYouSure()
{
  PutDefine("ST_ARE_YOU_SURE");
  return YorN;
}

CHashDefineStr *DefineLoad(U8 *name,U8 *st,Bool replace=FALSE)
{
  CHashDefineStr *temph=CAlloc(sizeof(CHashDefineStr));
  temph->type=HTT_DEFINE_STR;
  temph->str=StrNew(name);
  temph->data=StrNew(st);
  temph->cnt=-1;
  temph->caller=Caller;
  if (replace)
    HashReplace(temph,Fs->hash_table);
  else
    HashAdd(temph,Fs->hash_table);
  return temph;
}

CHashDefineStr *DefineLstLoad(U8 *name,U8 *st,Bool replace=FALSE)
{ //Not especially efficient but handy.
  I64 cnt=0;
  U8 *ptr,**lst;
  CHashDefineStr *temph=CAlloc(sizeof(CHashDefineStr));
  temph->type=HTT_DEFINE_STR;
  temph->str=StrNew(name);
  ptr=st;
  while (*ptr) {
    while (*ptr++);
    cnt++;
  }
  temph->data=MAlloc(ptr+1-st);
  MemCpy(temph->data,st,ptr+1-st);
  temph->cnt=cnt;
  temph->caller=Caller;

  lst=temph->sub_lst=MAlloc(cnt*sizeof(U8 *));
  ptr=st;
  while (*ptr) {
    if (*ptr!='@')
      *lst++=ptr;
    while (*ptr++);
  }

  if (replace)
    HashReplace(temph,Fs->hash_table);
  else
    HashAdd(temph,Fs->hash_table);
  return temph;
}

I64 StrOcc(U8 *src, I64 ch)
{
  I64 i=0;
  if (!src) return 0;
  while (*src)
    if (*src++==ch)
      i++;
  return i;
}

I64 Spaces2Tabs(U8 *dst,U8 *src)
{
  U8 *src2;
  I64 chged=0,space_cnt,space_cnt2,col=0;
  if (*src)
    while (TRUE) {
      src2=src;
      while (*src2==CH_SPACE)
	src2++;
      space_cnt=src2-src;
      while (col+space_cnt>=8) {
	space_cnt2=8-col;
	if (space_cnt2==1)
	  *dst++=CH_SPACE;
	else {
	  *dst++='\t';
	  chged+=space_cnt2-1;
	}
	space_cnt-=space_cnt2;
	col=0;
      }
      if (*src2=='\t') {
	if (space_cnt==1 && col==7)
	  *dst++=CH_SPACE;
	else
	  chged+=space_cnt;
	*dst++='\t';
	col=0;
      } else {
	while (space_cnt--) {
	  *dst++=CH_SPACE;
	  if (++col==8)
	    col=0;
	}
	if (*src2) {
	  *dst++=*src2;
	  if (++col==8)
	    col=0;
	} else
	  break;
      }
      src=++src2;
    }
  *dst=0;
  return chged;
}

U8 *Tabs2Spaces(U8 *src)
{
  I64 i,j,l=StrLen(src)<<1+2,col=0;
  U8 ch,*dst=MAlloc(l),*temp;
  while (ch=*src++) {
    if (ch=='\t') {
      j=(col+8) & ~7;
      for (i=col;i<j;i++) {
	dst[i]=CH_SPACE;
	if (i>=l-2) {
	  temp=MAlloc(l<<1);
	  MemCpy(temp,dst,i+1);
	  Free(dst);
	  l<<=1;
	  dst=temp;
	}
      }
      col=j;
    } else {
      dst[col]=ch;
      if (col>=l-2) {
	temp=MAlloc(l<<1);
	MemCpy(temp,dst,col+1);
	Free(dst);
	l<<=1;
	dst=temp;
      }
      col++;
    }
  }
  dst[col]=0;
  return dst;
}

U8 *ScaleIndent(U8 *src,F64 indent_scale_factor)
{
  I64 i,col=0;
  U8 ch,*dst,*dst2;
  while (ch=*src++) {
    if (ch=='\t')
      col=(col+8) & -0x8;
    else if (ch==CH_SPACE)
      col++;
    else
      break;
  }
  src--;
  col=Round(indent_scale_factor*col);
  dst=dst2=MAlloc(StrLen(src)+col/8+col&7+1);
  for (i=col/8;i>0;i--)
    *dst2++='\t';
  for (i=col&7;i>0;i--)
    *dst2++=CH_SPACE;
  StrCpy(dst2,src);
  return dst;
}

U8 *StrUtil(U8 *_src,I64 flags)
{//See $LK,"flags","MN:SUF_REM_SPACES"$
  U8 *src=_src,*dst=_src;
  I64 ch;

  if (flags & SUF_REM_LEADING)
    while (Bt(white_space_bitmap,*src))
      src++;
  while (ch=*src++) {
    if (Bt(white_space_bitmap,ch)) {
      if (!(flags & SUF_REM_SPACES)) {
	if (flags & SUF_SINGLE_SPACE) {
	  *dst++ = CH_SPACE;
	  while ((ch=*src++) && Bt(white_space_bitmap,ch));
	  src--;
	} else
	  *dst++ = ch;
      }
    } else {
      if (!(flags & SUF_REM_CTRL_CHARS) || ch>=CH_SHIFT_SPACE)
	*dst++=ch;
    }
  }
  *dst=0;

  if (flags & SUF_CAP_UNDERSCORES) {
    src=_src;
    dst=_src;
    while (*src) {
      *dst++=ToUpper(*src++);
      while (*src && *src!='_')
	*dst++=*src++;
      if (*src=='_')
	src++;
    }
    *dst=0;
  }

  if (flags & SUF_REM_TRAILING)
    while (dst!=_src && (!*dst || Bt(white_space_bitmap,*dst)))
      *dst-- =0;
  if (flags & SUF_TO_UPPER)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if ('a'<=ch<='z')
	*dst=ch-0x20;
    }
  if (flags & SUF_TO_LOWER)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if ('A'<=ch<='Z')
	*dst=ch+0x20;
    }
  if (flags & SUF_SAFE_DOLLAR)
    for (dst=_src;*dst;dst++) {
      ch=*dst;
      if (!Bt(safe_dollar_chars_bitmap,*dst))
	*dst='.';
    }
  if (flags & SUF_S2T)
    Spaces2Tabs(_src,_src);
  return _src;
}

U8 *MStrUtil(U8 *src,I64 flags,F64 indent_scale_factor=0)
//Return a $LK,"MAlloc","MN:MAlloc"$()ed result from StrUtil
{
  U8 *dst=StrNew(src),*dst2,*temp;
  StrUtil(dst,flags);
  if (flags & SUF_T2S) {
    temp=Tabs2Spaces(dst);
    Free(dst);
    dst=temp;
  }
  if (flags & SUF_SCALE_INDENT)
    dst2=ScaleIndent(dst,indent_scale_factor);
  else
    dst2=StrNew(dst); //Shorten
  Free(dst);
  return dst2;
}

U8 *StrFirstOcc(U8 *src,U8 *marker)
{
  U8 ch;
  while ((ch=*src++) && !StrOcc(marker,ch));
  if (ch)
    return src-1;
  else
    return NULL;
}

U8 *StrFirstRem(U8 *src,U8 *marker,U8 *dst=NULL)
{
  U8 *ptr=src,*result=dst,ch;
  if (dst) {
    while ((ch=*ptr++) && !StrOcc(marker,ch))
      *dst++=ch;
    *dst=0;
  } else
    while ((ch=*ptr++) && !StrOcc(marker,ch));
  if (ch)
    StrCpy(src,ptr);
  else
    *src=0;
  return result;
}

U8 *StrLastOcc(U8 *src,U8 *marker)
{
  U8 ch,*result=NULL;
  while (ch=*src++)
    if (StrOcc(marker,ch))
      result=src-1;
  return result;
}

U8 *StrLastRem(U8 *src,U8 *marker,U8 *dst=NULL)
{
  U8 *ptr;
  if (ptr=StrLastOcc(src,marker)) {
    if (dst)
      StrCpy(dst,ptr+1);
    *ptr=0;
  } else {
    if (dst)
      StrCpy(dst,src);
    *src=0;
  }
  return dst;
}

U8 *StrFind(U8 *pattern,U8 *src,I64 flags=0)
{
  Bool cont;
  U8 *saved_src=src;
  I64 plen=StrLen(pattern);
  do {
    cont=FALSE;
    if (flags & SFF_IGNORE_CASE)
      src=StrIMatch(pattern,src);
    else
      src=StrMatch(pattern,src);
    if (src && flags & SFF_WHOLE_LABELS_FRONT) {
      if (src!=saved_src) {
	if (Bt(alpha_numeric_bitmap,*(src-1))) {
	  src++;
	  if (*src)
	    cont=TRUE;
	  else
	    src=NULL;
	}
      }
    }
    if (src && flags & SFF_WHOLE_LABELS_BACK) {
      if (Bt(alpha_numeric_bitmap,*(src+plen))) {
	src++;
	if (*src)
	  cont=TRUE;
	else
	  src=NULL;
      }
    }
  } while (cont);
  return src;
}

Bool WildMatch(U8 *pattern,U8 *wild)
{
  I64 ch1,ch2;
  U8 *fall_back_src=NULL,*fall_back_wild=NULL;
  while (TRUE) {
    if (!(ch1=*pattern++)) {
      if (*wild && *wild!='*')
	return FALSE;
      else
	return TRUE;
    } else {
      if (!(ch2=*wild++))
	return FALSE;
      else {
	if (ch2=='*') {
	  fall_back_wild=wild-1;
	  fall_back_src=pattern;
	  if (!(ch2=*wild++))
	    return TRUE;
	  while (ch2!=ch1)
	    if (!(ch1=*pattern++))
	      return FALSE;
	} else
	  if (ch2!='?' && ch1!=ch2) {
	    if (fall_back_wild) {
	      wild=fall_back_wild;
	      pattern=fall_back_src;
	      fall_back_wild=NULL;
	      fall_back_src=NULL;
	    } else
	      return FALSE;
	  }
      }
    }
  }
}

U8 *StrAdd(U8 *base,U8 *addition)
//Concatinates a string onto another string.
//Returns new string.
//Dbls length when it grows.
{
  U8 *result;
  I64 blen=StrLen(base),alen=StrLen(addition),bmem=MSize(base);
  if (blen+alen+1>bmem) {
    result=MAlloc(bmem<<1+alen+4);
    MemCpy(result,base,blen);
    StrCpy(result+blen,addition);
    Free(base);
  } else {
    result=base;
    StrCpy(result+blen,addition);
  }
  return result;
}


I64 A2I(U8 *st,I64 radix=10,U8 **_end_ptr=NULL)
{ //ASCII to Int
  Bool neg=FALSE;
  U8 ch;
  I64 result=0,k=0;
  if (!(2<=radix<=36)) return 0;
  while (Bt(white_space_bitmap,*st))
    st++;
  if (*st=='-') {
    st++;
    neg=TRUE;
  } else if (*st=='+')
    st++;
  while (ch=ToUpper(*st++)) {
    if (radix>10) {
      if ('0'<=ch<='9')
	result=result*radix+ch-'0';
      else if ('A'<=ch<='A'+radix-11)
	result=result*radix+ch-'A'+10;
      else
	break;
    } else {
      if (k==1 && !result) {
	if (ch=='X') {
	  radix=16;
	  goto ai_skip;
	} else if (ch=='B') {
	  radix=2;
	  goto ai_skip;
	} else if (ch=='O') {
	  radix=8;
	  goto ai_skip;
	} else if (ch=='D') {
	  radix=10;
	  goto ai_skip;
	}
      }
      if ('0'<=ch<='0'+radix-1)
	result=result*radix+ch-'0';
      else
	break;
    }
ai_skip:
    k++;
  }
  if (_end_ptr) *_end_ptr=st-1;
  if (neg)
    return -result;
  else
    return result;
}

U0 ScanFlags(U8 *f,U8 *lst,U8 *st)
{
  I64 i;
  U8 *st2,*ptr;
  if (st) {
    st2=MAlloc(StrLen(st)+1);
    while (*st) {
      while (*st && *st!='+' && *st!='-')
	st++;
      if (*st=='+') {
	st++;
	if (*st) {
	  ptr=st2;
	  while (*st && *st!='+' && *st!='-' && *st!=CH_SPACE && *st!=CH_SHIFT_SPACE)
	    *ptr++=*st++;
	  *ptr=0;
	  i=LstMatch(st2,lst);
	  if (i>=0)
	    LBts(f,i);
	  else {
	    Free(st2);
	    throw(EXCEPT_FLAGS,1);
	  }
	}
      } else if (*st=='-') {
	st++;
	if (*st) {
	  ptr=st2;
	  while (*st && *st!='+' && *st!='-' && *st!=CH_SPACE && *st!=CH_SHIFT_SPACE)
	    *ptr++=*st++;
	  *ptr=0;
	  i=LstMatch(st2,lst);
	  if (i>=0)
	    LBtr(f,i);
	  else {
	    Free(st2);
	    throw(EXCEPT_FLAGS,1);
	  }
	}
      }
    }
    Free(st2);
  }
}

U8 *SPrintFlags(U8 *dst,U8 *lst,I64 flags)
{
  I64 i;
  *dst=0;
  for (i=0;i<=63;i++)
    if (Bt(&flags,i)) {
      StrCat(dst,"+");
      StrCat(dst,LstSub(i,lst));
    }
  return dst;
}

U0 SPutChar(U8 **_dst,U8 ch,U8 **_buf)
{
  I64 i;
  U8 *dst=*_dst,*buf;
  if (_buf) {
    buf=*_buf;
    i=dst-buf;
    if (i>=MSize(buf)) {
      buf=MAlloc(i<<1+1);
      MemCpy(buf,*_buf,i);
      dst=buf+i;
      *_buf=buf;
    }
  }
  *dst++=ch;
  *_dst=dst;
}

U0 OutStr(U8 *ptr,U8 **_buf,U8 **_dst,
     I64 len,Bool left_justify,Bool truncate_flag)
{
  I64 i,j;
  if (!ptr)
    i=0;
  else
    i=StrLen(ptr);
  if (truncate_flag && i>len)
    i=len;
  if (left_justify) {
    for (j=0;j<i;j++)
      SPutChar(_dst,*ptr++,_buf);
    for (j=0;j<len-i;j++)
      SPutChar(_dst,CH_SPACE,_buf);
  } else {
    for (j=0;j<len-i;j++)
      SPutChar(_dst,CH_SPACE,_buf);
    for (j=len-i;j<len;j++)
      SPutChar(_dst,*ptr++,_buf);
  }
}

U8 *MPrintQ(U8 *ptr,Bool dollar_flag)
{
  U8 **_buf,*buf,**_dst,*dst;
  I64 ch;
  buf=MAlloc(128);
  _buf=&buf;
  dst=buf;
  _dst=&dst;
  if (ptr)
    while (ch=*ptr++) {
      switch (ch) {
	case '$$':
	  if (dollar_flag) {
	    SPutChar(_dst,'\\',_buf);
	    SPutChar(_dst,'$$'>>6&7+'0',_buf);
	    SPutChar(_dst,'$$'>>3&7+'0',_buf);
	    SPutChar(_dst,'$$'&7+'0',_buf);
	  } else {
	    SPutChar(_dst,ch,_buf);
	    SPutChar(_dst,ch,_buf);
	  }
	  break;
	case '\n':
	  SPutChar(_dst,'\\',_buf);
	  SPutChar(_dst,'n',_buf);
	  break;
	case '\r':
	  SPutChar(_dst,'\\',_buf);
	  SPutChar(_dst,'r',_buf);
	  break;
	case '\t':
	  SPutChar(_dst,'\\',_buf);
	  SPutChar(_dst,'t',_buf);
	  break;
	case '"':
	case '\\':
	  SPutChar(_dst,'\\',_buf);
	  SPutChar(_dst,ch,_buf);
	  break;
	default:
	  if (CH_SHIFT_SPACE<=ch<=0x7F) {
	    SPutChar(_dst,ch,_buf);
	  } else {
	    SPutChar(_dst,'\\',_buf);
	    SPutChar(_dst,ch>>6&7+'0',_buf);
	    SPutChar(_dst,ch>>3&7+'0',_buf);
	    SPutChar(_dst,ch&7+'0',_buf);
	  }
      }
    }
  SPutChar(_dst,0,_buf);
  return buf;
}


U8 *MPrintq(U8 *ptr)
{
  U8 **_buf,*buf,**_dst,*dst;
  I64 i,ch,ch1;
  buf=MAlloc(128);
  _buf=&buf;
  dst=buf;
  _dst=&dst;
  if (ptr)
    while (ch=*ptr++) {
      ch1=*ptr;
      if (ch=='\\')
	switch (ch1) {
	  case 'n':
	    SPutChar(_dst,'\n',_buf);
	    ptr++;
	    break;
	  case 'r':
	    SPutChar(_dst,'\r',_buf);
	    ptr++;
	    break;
	  case '"':
	    SPutChar(_dst,'"',_buf);
	    ptr++;
	    break;
	  case '\'':
	    SPutChar(_dst,'\'',_buf);
	    ptr++;
	    break;
	  case '\`':
	    SPutChar(_dst,'\`',_buf);
	    ptr++;
	    break;
	  case '\\':
	    SPutChar(_dst,'\\',_buf);
	    ptr++;
	    break;
	  case 't':
	    SPutChar(_dst,'\t',_buf);
	    ptr++;
	    break;
	  case 'x':
	  case 'X':
	    i=0;
	    ptr++;
	    while (TRUE) {
	      ch1=ToUpper(*ptr++);
	      if (Bt(hex_numeric_bitmap,ch1)) {
		if (ch1<='9')
		  i=i<<4+ch1-'0';
		else
		  i=i<<4+ch1-'A'+10;
	      } else
		break;
	    }
	    ptr--;
	    SPutChar(_dst,i,_buf);
	    break;
	  case 'b':
	  case 'B':
	    i=0;
	    ptr++;
	    while (TRUE) {
	      ch1=*ptr++;
	      if ('0'<=ch1<='1')
		i=i<<1+ch1-'0';
	      else
		break;
	    }
	    ptr--;
	    SPutChar(_dst,i,_buf);
	    break;
	  case 'o':
	  case 'O':
	    i=0;
	    ptr++;
	    goto sq_octal;
	  case '0'...'7':
	    i=ch1-'0';
	    ptr++;
sq_octal:
	    while (TRUE) {
	      ch1=*ptr++;
	      if ('0'<=ch1<='7')
		i=i<<3+ch1-'0';
	      else
		break;
	    }
	    ptr--;
	    SPutChar(_dst,i,_buf);
	    break;
	  default:
	    SPutChar(_dst,ch,_buf);
	}
      else if (ch=='$$' && ch1=='$$') {
	SPutChar(_dst,ch,_buf);
	ptr++;
      } else {
	SPutChar(_dst,ch,_buf);
      }
    }
  SPutChar(_dst,0,_buf);
  return buf;
}

U8 *sprint_pos_powers_lst=
"kilo\0mega\0giga\0tera\0peta\0exa\0zetta\0yotta\0";
U8 *sprint_pos_powers_lets=
" kMGTPEZY";

U8 *sprint_neg_powers_lst=
"milli\0micro\0nano\0pico\0femto\0atto\0zepto\0yocto\0";
U8 *sprint_neg_powers_lets=
" mænpfazy";

#define TEMP_BUF_LEN	256
#define SLOP		8

U8 *SPrintJoin(U8 *dst,U8 *src,I64 argc,I64 *argv)
{ //$LK,"Print Codes","FA:::/TempleOS/Doc/HolyC.TXT,Print_Codes"$
/*
In float formatting, do not exceed 18-digits
before or after the decimal point
because the numbers before and after
the decimal point are stored
in 64-bits.  Use exponentiated forms
to avoid this.
*/
  Bool pad_zero,left_justify,truncate_flag,neg,neg_e,comma_flag,
      dollar_flag,sub_fmt_question,has_sub_fmt,k_flag,has_decimal;
  I64 i,j,l,ch,k,k0,n,n0,len,dec_len,dec_len0,sub_fmt,comma_cnt,comma_fmt_cnt,cur_arg=0;
  U64 m;
  F64 d,d0,d1;
  U8 *ptr,**_buf,*buf,**_dst,*temp1,temp_buf[TEMP_BUF_LEN];

  if (!src)
    throw(EXCEPT_SPRINT,0);
  if (dst) {
    _buf=NULL;
    buf=dst;
  } else {
    buf=MAlloc(128);
    _buf=&buf;
    dst=buf;
  }
  _dst=&dst;

  while (ch = *src++) {
    if (ch=='%') {
      if (*src=='-') {
	left_justify=TRUE;
	src++;
      } else
	left_justify=FALSE;
      if (*src=='0') {
	pad_zero=TRUE;
	src++;
      } else
	pad_zero=FALSE;
      len=0;
      while ('0'<=*src<='9')
	len=len*10+ *src++ -'0';
      if (*src=='*') {
	src++;
	if (cur_arg>=argc)
	  throw(EXCEPT_SPRINT,1);
	len=argv[cur_arg++];
      }
      dec_len=0;
      if (*src=='.') {
	src++;
	while ('0'<=*src<='9')
	  dec_len=dec_len*10+ *src++ -'0';
	if (*src=='*') {
	  src++;
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  dec_len=argv[cur_arg++];
	}
	has_decimal=TRUE;
      } else
	has_decimal=FALSE;

      dollar_flag=FALSE;
      comma_flag=FALSE;
      truncate_flag=FALSE;
      k_flag=FALSE;
      while (TRUE) {
	switch (*src) {
	  case '$$':
	    src++;
	    dollar_flag=TRUE;
	    break;
	  case ',':
	    src++;
	    comma_flag=TRUE;
	    break;
	  case 't':
	    src++;
	    truncate_flag=TRUE;
	    break;
	  case 'k':
	    src++;
	    k_flag=TRUE;
	    break;
	  case 'l': //harmless
	    src++;
	    break;
	  default:
	    goto sp_in_sub_fmt;
	}
      }

sp_in_sub_fmt:
      sub_fmt=0;
      if (*src!='h')
	has_sub_fmt=FALSE;
      else {
	src++;
	has_sub_fmt=TRUE;
	if (*src=='?') {
	  src++;
	  sub_fmt_question=TRUE;
	} else {
	  sub_fmt_question=FALSE;
	  if (*src=='*') {
	    src++;
	    if (cur_arg>=argc)
	      throw(EXCEPT_SPRINT,1);
	    sub_fmt=argv[cur_arg++];
	  } else {
	    if (*src=='-') {
	      src++;
	      neg=TRUE;
	    } else
	      neg=FALSE;
	    while ('0'<=*src<='9')
	      sub_fmt=sub_fmt*10+ *src++ -'0';
	    if (neg) sub_fmt=-sub_fmt;
	  }
	}
      }

      neg=FALSE;
      k=0;
      switch (*src++) {
	case 's':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=argv[cur_arg++];
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'S':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=Define(argv[cur_arg++]);
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'F':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=TextFileRead(argv[cur_arg++]);
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  Free(ptr);
	  break;
	case 'Q':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=MPrintQ(argv[cur_arg++],dollar_flag);
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  Free(ptr);
	  break;
	case 'q':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=MPrintq(argv[cur_arg++]);
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  Free(ptr);
	  break;
	case 'z':
	  if (cur_arg+1>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=LstSub(argv[cur_arg],argv[cur_arg+1]);
	  cur_arg=cur_arg+2;
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'Z':
	  if (cur_arg+1>=argc)
	    throw(EXCEPT_SPRINT,1);
	  ptr=DefineSub(argv[cur_arg],argv[cur_arg+1]);
	  cur_arg=cur_arg+2;
	  OutStr(ptr,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'c':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  temp_buf[0](I64)=argv[cur_arg++];
	  temp_buf[8]	    =0;
	  OutStr(temp_buf,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'C':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  temp_buf[0](I64)=argv[cur_arg++];
	  temp_buf[8]	    =0;
	  ptr=temp_buf;
	  while (*ptr) {
	    *ptr=ToUpper(*ptr);
	    ptr++;
	  }
	  OutStr(temp_buf,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'D':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  temp1=MPrintDate(argv[cur_arg++],sub_fmt);
	  OutStr(temp1,_buf,_dst,len,left_justify,truncate_flag);
	  Free(temp1);
	  break;
	case 'T':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  temp1=MPrintTime(argv[cur_arg++],sub_fmt);
	  OutStr(temp1,_buf,_dst,len,left_justify,truncate_flag);
	  Free(temp1);
	  break;
	case 'P':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  if (truncate_flag)
	    SPrintPSect(temp_buf,argv[cur_arg++],len);
	  else
	    SPrintPSect(temp_buf,argv[cur_arg++],0);
	  OutStr(temp_buf,_buf,_dst,len,left_justify,truncate_flag);
	  break;
	case 'd':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  if (m(I64)<0) {
	    neg=TRUE;
	    m=-m;
	  }
sp_out_dec:
	  if (has_sub_fmt) {
	    if (!len) len=12;
	    d=m;
	    goto sp_out_eng;
	  }
	  if (comma_flag) {
	    comma_fmt_cnt=comma_cnt=3;
	    do {
	      temp_buf[k++]=ModU64(&m,10)+'0';
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=3;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
sp_out_comma_num:
	    if (neg)
	      i=1;
	    else
	      i=0;
	    if (len<0)
	      len=0;
	    if (truncate_flag && k+i>len)
	      k=len-i;
	    if (k<0)
	      k=0;
	    if (pad_zero) {
	      if (neg)
		SPutChar(_dst,'-',_buf);
	      comma_cnt=(len-k-i+comma_fmt_cnt-comma_cnt+1)%(comma_fmt_cnt+1)+1;
	      for (;i<len-k;i++) {
		if (!--comma_cnt) {
		  SPutChar(_dst,',',_buf);
		  comma_cnt=comma_fmt_cnt;
		  if (++i>=len-k)
		    break;
		}
		SPutChar(_dst,'0',_buf);
	      }
	    } else {
	      for (;i<len-k;i++)
		SPutChar(_dst,CH_SPACE,_buf);
	      if (neg)
		SPutChar(_dst,'-',_buf);
	    }
	  } else {
	    do {
	      temp_buf[k++]=ModU64(&m,10)+'0';
	      if (!m) break;
	    } while (k<TEMP_BUF_LEN-SLOP);
sp_out_num:
	    if (neg)
	      i=1;
	    else
	      i=0;
	    if (len<0)
	      len=0;
	    if (truncate_flag && k+i>len)
	      k=len-i;
	    if (k<0)
	      k=0;
	    if (pad_zero) {
	      if (neg)
		SPutChar(_dst,'-',_buf);
	      for (;i<len-k;i++)
		SPutChar(_dst,'0',_buf);
	    } else {
	      for (;i<len-k;i++)
		SPutChar(_dst,CH_SPACE,_buf);
	      if (neg)
		SPutChar(_dst,'-',_buf);
	    }
	  }
	  for (i=k-1;i>=0;i--)
	    SPutChar(_dst,temp_buf[i],_buf);
	  break;
	case 'u':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  goto sp_out_dec;
	case 'f':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  d=argv[cur_arg++](F64);
	  if (d<0) {
	    neg=TRUE;
	    d=-d;
	  }

	  if (d==ì) {
sp_out_inf:
	    if (neg)
	      i=1;
	    else
	      i=0;
	    k=1;
	    if (len<0)
	      len=0;
	    if (truncate_flag && k+i>len)
	      k=len-i;
	    if (k<0)
	      k=0;
	    for (;i<len-k;i++)
	      SPutChar(_dst,CH_SPACE,_buf);
	    if (neg)
	      SPutChar(_dst,'-',_buf);
	    for (i=0;i<k;i++)
	      SPutChar(_dst,'ì',_buf);
	    break;
	  }

sp_out_f:
	  if (dec_len<0)
	    dec_len=0;
	  n=Log10(d);
	  if (i=dec_len) {
	    if (comma_flag)
	      i=i-i/4;
	    if (n+i>17) {
	      n+=i-17;
	      d*=Pow10I64(i-n);
	    } else {
	      n=0;
	      d*=Pow10I64(i);
	    }
	    i=dec_len;
	  } else if (n>17) {
	    n-=17;
	    d*=Pow10I64(-n);
	  } else
	    n=0;

	  m=Round(d);
	  if (comma_flag) {
	    comma_cnt=i&3;
	    while (i-- && k<TEMP_BUF_LEN-SLOP) {
	      if (i>2 && !comma_cnt--) {
		temp_buf[k++]=',';
		comma_cnt=2;
		if (!--i) break;
	      }
	      if (n) {
		n--;
		temp_buf[k++]='0';
	      } else
		temp_buf[k++]=ModU64(&m,10)+'0';
	      if (!i) break;
	    }
	  } else {
	    while (i-- && k<TEMP_BUF_LEN-SLOP) {
	      if (n) {
		n--;
		temp_buf[k++]='0';
	      } else
		temp_buf[k++]=ModU64(&m,10)+'0';
	    }
	  }
	  if (dec_len)
	    temp_buf[k++]='.';
	  if (comma_flag) {
	    comma_cnt=3;
	    do {
	      if (n) {
		n--;
		temp_buf[k++]='0';
	      } else
		temp_buf[k++]=ModU64(&m,10)+'0';
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=3;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
	  } else {
	    do {
	      if (n) {
		n--;
		temp_buf[k++]='0';
	      } else
		temp_buf[k++]=ModU64(&m,10)+'0';
	      if (!m) break;
	    } while (k<TEMP_BUF_LEN-SLOP);
	  }
	  goto sp_out_num;
	case 'e':
	  if (!len) len=12;
	  truncate_flag=TRUE;

	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  d=argv[cur_arg++](F64);
	  if (d<0) {
	    neg=TRUE;
	    d=-d;
	  }
	  if (d==ì) goto sp_out_inf;


	  if (d)
	    n=Floor(Log10(d));
	  else
	    n=0;
sp_out_e:
	  d/=Pow10I64(n);

	  k0=k;
	  for (l=0;l<2;l++) {
	    n0=n;
	    if (n<0) {
	      n=-n;
	      neg_e=TRUE;
	    } else
	      neg_e=FALSE;

	    i=3;
	    do temp_buf[k++]=ModU64(&n,10)+'0';
	    while (n && i--);
	    if (neg_e)
	      temp_buf[k++]='-';
	    temp_buf[k++]='e';
	    dec_len=len-k-neg-2;

	    if (d) {
	      d1=d+Pow10I64(-dec_len)/2;
	      if (d1<1.0) {
		d*=10;
		n=n0-1;
		k=k0;
	      } else if (d1>=10) {
		d/=10;
		n=n0+1;
		k=k0;
	      } else
		break;
	    } else
	      break;
	  }

	  goto sp_out_f;
	case 'g':
	  if (!len) len=12;
	  truncate_flag=TRUE;
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  d=argv[cur_arg++](F64);
	  if (d<0) {
	    neg=TRUE;
	    d=-d;
	  }
	  if (d==ì) goto sp_out_inf;
	  if (d)
	    n=Floor(Log10(d));
	  else
	    n=0;
	  if (n>=len-1-dec_len || n<-(dec_len-1))
	    goto sp_out_e;
	  else
	    goto sp_out_f;
	case 'n':
	  if (!len) len=12;
	  truncate_flag=TRUE;
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  d=argv[cur_arg++](F64);
	  k_flag=TRUE;
	  if (d<0) {
	    neg=TRUE;
	    d=-d;
	  }
sp_out_eng: //Engineering notation
	  if (d==ì) goto sp_out_inf;
	  if (k_flag) {
	    if (d)
	      n=FloorI64(Floor(Log10(d)),3);
	    else
	      n=0;
	    d/=Pow10I64(n);
	  } else {
	    if (d)
	      n=Floor(Ln(d)/Ln(1024));
	    else
	      n=0;

	    d/=1024`n;
	    n*=3;
	  }

	  k0=k;
	  dec_len0=dec_len;
	  for (l=0;l<2;l++) {
	    n0=n;
	    d0=d;

	    if (n<0) {
	      n=-n;
	      neg_e=TRUE;
	    } else
	      neg_e=FALSE;

	    if (has_sub_fmt && -24<=n<=24) {
	      if (sub_fmt_question) {
		if (neg_e)
		  i=-n/3;
		else
		  i=n/3;
		j=0;
	      } else {
		if (neg_e)
		  j=-n-sub_fmt;
		else
		  j=n-sub_fmt;
		if (k_flag)
		  d*=Pow10I64(j);
		else
		  d*=1024`(j/3.0);
		i=sub_fmt/3;
	      }
	      if (i<0)
		temp_buf[k++]=sprint_neg_powers_lets[-i];
	      else if (i>0)
		temp_buf[k++]=sprint_pos_powers_lets[i];
	      else if (len!=0)
		temp_buf[k++]=CH_SPACE;
	      if (!has_decimal) {
		dec_len=len-k-neg-2;
		if (j>0) {
		  if (comma_flag)
		    dec_len-=4*j/3;
		  else
		    dec_len-=j;
		}
		d1=d+Pow10I64(-dec_len+1)/2;
		if (d1>=10) {
		  dec_len--;
		  if (d1>=100)
		    dec_len--;
		}
	      }
	    } else {
	      i=3;
	      do temp_buf[k++]=ModU64(&n,10)+'0';
	      while (n && i--);
	      if (neg_e)
		temp_buf[k++]='-';
	      temp_buf[k++]='e';
	      if (!dec_len) {
		dec_len=len-k-neg-2;
		d1=d+Pow10I64(-dec_len+1)/2;
		if (d1>=10) {
		  dec_len--;
		  if (d1>=100)
		    dec_len--;
		}
	      }
	    }
	    if (comma_flag) {
	      if (len && dec_len>0 && !(dec_len&3))
		temp_buf[k++]=',';
	      dec_len-=dec_len/4;
	    }
	    if (d) {
	      d1=d+Pow10I64(-dec_len)/2;
	      if (d1<1.0) {
		d=d0*1000;
		n=n0-3;
		k=k0;
		dec_len=dec_len0;
	      } else if (d1>=1000) {
		d=d0/1000;
		n=n0+3;
		k=k0;
		dec_len=dec_len0;
	      } else
		break;
	    } else
	      break;
	  }
	  goto sp_out_f;
	case 'X':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  if (comma_flag) {
	    comma_fmt_cnt=comma_cnt=4;
	    do {
	      temp_buf[k]= m&15 +'0';
	      if (temp_buf[k]>'9') temp_buf[k]+='A'-0x3A;
	      k++;
	      m>>=4;
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=4;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_comma_num;
	  } else {
	    do {
	      temp_buf[k]= m&15 +'0';
	      if (temp_buf[k]>'9') temp_buf[k]+='A'-0x3A;
	      k++;
	      m>>=4;
	    } while (m && k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_num;
	  }
	case 'x':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  if (comma_flag) {
	    comma_fmt_cnt=comma_cnt=4;
	    do {
	      temp_buf[k]= m&15 +'0';
	      if (temp_buf[k]>'9') temp_buf[k]+='a'-0x3A;
	      k++;
	      m>>=4;
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=4;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_comma_num;
	  } else {
	    do {
	      temp_buf[k]= m&15 +'0';
	      if (temp_buf[k]>'9') temp_buf[k]+='a'-0x3A;
	      k++;
	      m>>=4;
	    } while (m && k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_num;
	  }
	case 'b':
	case 'B':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  if (comma_flag) {
	    comma_fmt_cnt=comma_cnt=4;
	    do {
	      temp_buf[k++]= m&1 +'0';
	      m>>=1;
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=4;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_comma_num;
	  } else {
	    do {
	      temp_buf[k++]= m&1 +'0';
	      m>>=1;
	    } while (m && k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_num;
	  }
	case 'o':
	case 'O':
	  if (cur_arg>=argc)
	    throw(EXCEPT_SPRINT,1);
	  m=argv[cur_arg++];
	  if (comma_flag) {
	    comma_fmt_cnt=comma_cnt=3;
	    do {
	      temp_buf[k++]= m&7 +'0';
	      m>>=3;
	      if (!m) break;
	      if (!--comma_cnt) {
		temp_buf[k++]=',';
		comma_cnt=3;
	      }
	    } while (k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_comma_num;
	  } else {
	    do {
	      temp_buf[k++]= m&7 +'0';
	      m>>=3;
	    } while (m && k<TEMP_BUF_LEN-SLOP);
	    goto sp_out_num;
	  }
	case '%':
	  SPutChar(_dst,'%',_buf);
	  break;
      }
    } else
      SPutChar(_dst,ch,_buf);
  }
  SPutChar(_dst,0,_buf);
  return buf;
}

U8 *SPrint(U8 *dst,U8 *fmt,...)
{ //See $LK,"SPrintJoin","MN:SPrintJoin"$()
  return SPrintJoin(dst,fmt,argc,argv);
}

/*
A2Date()
  This routine recognizes the following formats:
  1) "*+nnnn"  "*-nnnn"
  2) mm/dd
  3) mm/dd/yy

It also supports some funs
  SM() start of mon
  EM() end of mon
  SM() start of year
  EY() end of year
Full expressions are not implimented
but you can do stuff like SM(*-7)+3
and it will return the 3rd day after
the start of mon for seven days before
today.

*/
CDate A2Date(U8 *_src)
//ASCII date to CDate
{
  CDate result=0;
  CDateStruct ds,ds1;
  U8 *src=MStrUtil(_src,SUF_REM_SPACES|SUF_TO_UPPER),
     *v=StrNew(src),
     *p1=src;
  Bool start_mon=FALSE,end_mon=FALSE,
       start_year=FALSE,end_year=FALSE;

  MemSet(&ds,0,sizeof(CDateStruct));
  MemSet(&ds1,0,sizeof(CDateStruct));
  if (!StrNCmp(p1,"SM(",3)) {
    p1+=3;
    start_mon=TRUE;
  } else if (!StrNCmp(p1,"EM(",3)) {
    p1+=3;
    end_mon=TRUE;
  } else if (!StrNCmp(p1,"SY(",3)) {
    p1+=3;
    start_year=TRUE;
  } else if (!StrNCmp(p1,"EY(",3)) {
    p1+=3;
    end_year=TRUE;
  }
  if (*p1=='*') {
    p1++;
    if (*p1=='+' || *p1=='-')
      result.date=A2I(p1,10,&p1);
    result+=GetDateTime+local_time_offset;
  } else {
    StrFirstRem(p1,"/",v); //Put mon into v
    ds.mon=A2I(v);
    if (StrOcc(p1,'/')) {
      StrFirstRem(p1,"/",v); //Put day into v leaving year in p1
      ds.day_of_mon=A2I(v);
      ds.year=A2I(p1,10,&p1);
      if (ds.year<100) //if not 4 digit year
	ds.year+=2000;
    } else {
      ds.day_of_mon=A2I(p1,10,&p1);
      Date2DateStruct(&ds1,GetDateTime+local_time_offset);
      ds.year=ds1.year;
    }
    result=DateStruct2Date(&ds);
  }
  if (*p1==')') p1++;

  if (start_mon)
    result.date=FirstDayOfMon(result.date);
  else if (end_mon)
    result.date=LastDayOfMon(result.date);
  else if (start_year)
    result.date=FirstDayOfYear(result.date);
  else if (end_year)
    result.date=LastDayOfYear(result.date);

  if (*p1=='+' || *p1=='-')
    result.date+=A2I(p1);
  Free(src);
  Free(v);
  return result-local_time_offset;
}

F64 A2F64(U8 *src,U8 **_end_ptr=NULL)
//ASCII string to F64
/*
Does not allow more than 18-digits
before or after the decimal point
because the numbers before and after
the decimal point are stored
in 64-bits.  Use exponentiated forms
to avoid this.
*/
{
  U8 ch=*src++;
  I64 i,j,k;
  F64 d;
  Bool neg=FALSE,neg_e=FALSE;

  while (Bt(white_space_bitmap,ch))
    ch=*src++;
  if (ch=='-') {
    neg=TRUE;
    ch=*src++;
  }
  if (!StrNCmp(src-1,"inf",3)) {
    d=ì;
    src+=3;
    goto a2f_end;
  }
  if (*src=='ì') {
    d=ì;
    src++;
    goto a2f_end;
  }
  i=0;
  while (TRUE) {
    if (Bt(dec_numeric_bitmap,ch))
      i=i*10+ch-'0';
    else {
      if (ch=='.' || ch=='e' || ch=='E')
	break;
      d=i;
      goto a2f_end;
    }
    ch=*src++;
  }
  if (ch=='.')
    ch=*src++;
  k=0;
  while (TRUE) {
    if (Bt(dec_numeric_bitmap,ch)) {
      i=i*10+ch-'0';
      k++;
    } else {
      if (ch=='e' || ch=='E')
	break;
      d=i*Pow10I64(-k);
      goto a2f_end;
    }
    ch=*src++;
  }
  ch=*src++;
  if (ch=='-') {
    neg_e=TRUE;
    ch=*src++;
  }
  j=0;
  while (TRUE) {
    if (Bt(dec_numeric_bitmap,ch))
      j=j*10+ch-'0';
    else {
      if (neg_e)
	d=i*Pow10I64(-j-k);
      else
	d=i*Pow10I64(j-k);
      goto a2f_end;
    }
    ch=*src++;
  }
a2f_end:
  if (_end_ptr) *_end_ptr=src-1;
  if (neg)
    return -d;
  else
    return d;
}

U8 *ScanJoin(U8 *src,U8 *fmt,I64i argc,I64i *argv)
//Pass ptr to data,
//For %s pass ptr to ptr (be careful because addr
//of array is the same as array--create ptr to array and take addr
{
  U8 ch,*buf,*ptr,**pptr;
  Bool left_justify=FALSE;
  I64 cur_arg=0,i,len,*i_ptr,dec_len;
  F64 *d_ptr;
  if (!fmt)
    throw(EXCEPT_SCAN,0);
  while (ch = *fmt++) {
    if (ch=='%') {
      if (*fmt=='%') {
	src++;
	fmt++;
      } else {
	if (*fmt=='-') {
	  left_justify=TRUE;
	  fmt++;
	} else
	  left_justify=FALSE;
	len=0;
	while ('0'<=*fmt<='9')
	  len=len*10+ (*fmt++ -'0');
	if (*fmt=='*') {
	  fmt++;
	  if (cur_arg>=argc)
	    throw(EXCEPT_SCAN,1);
	  len=argv[cur_arg++];
	}
	ch=*fmt++;
	if (ch && !len) {
	  ptr=src;
	  while (*ptr && *ptr!=*fmt)
	    ptr++;
	  len=ptr-src;
	} else {
	  if (ch=='.') {
	    dec_len=0;
	    while ('0'<=*fmt<='9')
	      dec_len=dec_len*10+ (*fmt++-'0');
	    if (*fmt=='*') {
	      fmt++;
	      if (cur_arg>=argc)
		throw(EXCEPT_SCAN,1);
	      dec_len=argv[cur_arg++];
	    }
	    ch=*fmt++;
	  }
	}
	buf=MAlloc(len+1);
	for (i=0;i<len;i++)
	  buf[i]=*src++;
	buf[i]=0;
	switch (ch) {
	  case 's':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    pptr=argv[cur_arg++];
	    StrCpy(*pptr,buf);
	    break;
	  case 'c':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    ptr=argv[cur_arg++];
	    *ptr=*buf;
	    break;
	  case 'C':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    ptr=argv[cur_arg++];
	    *ptr=ToUpper(*buf);
	    break;
	  case 'z':
	    if (cur_arg+1>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=LstMatch(buf,argv[cur_arg++]);
	    break;
	  case 'Z':
	    if (cur_arg+1>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=DefineMatch(buf,argv[cur_arg++]);
	    break;
	  case 'd':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=A2I(buf);
	    break;
	  case 'X':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=A2I(buf,16);
	    break;
	  case 'b':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=A2I(buf,2);
	    break;
	  case 'e':
	  case 'f':
	  case 'g':
	  case 'n':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    d_ptr=argv[cur_arg++];
	    *d_ptr=A2F64(buf);
	    break;
	  case 'D':
	    if (cur_arg>=argc)
	      throw(EXCEPT_SCAN,1);
	    i_ptr=argv[cur_arg++];
	    *i_ptr=A2Date(buf);
	    break;
	}
	Free(buf);
      }
    } else
      src++;
  }
  return src;
}

U8 *Scan(U8 *src,U8 *fmt,...)
//Pass ptr to data,
//For %s pass ptr to ptr (be careful because addr
//of array is the same as array--create ptr to array and take addr
{
  return ScanJoin(src,fmt,argc,argv);
}


U0 Print(U8 *fmt,...)
{//$LK,"Print Extended Codes","FA:::/TempleOS/Doc/HolyC.TXT,Print_Codes"$
 //See $LK,"SPrintJoin","MN:SPrintJoin"$()
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  PutS(buf);
  Free(buf);
}

U8 *MPrint(U8 *fmt,...)
{ //See $LK,"SPrintJoin","MN:SPrintJoin"$()
  U8 *result,*buf=SPrintJoin(NULL,fmt,argc,argv);
  result=StrNew(buf);
  Free(buf);
  return result;
}

U0 DefinePrint(U8 *name,U8 *src,...)
{
  U8 *buf=SPrintJoin(NULL,src,argc,argv);
  DefineLoad(name,buf);
  Free(buf);
}

U0 PrintErr(U8 *fmt,...)
{
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  PutErr;
  "%s",buf;
  Free(buf);
}

U0 PrintWarn(U8 *fmt,...)
{
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  PutWarn;
  "%s",buf;
  Free(buf);
}

U0 Help()
{
  if (IsInDbg)
    DbgHelp;
  else
    PopUp("Type(\"::/TempleOS/Doc/HelpIndex.TXT\");DocHome;View;");
}
