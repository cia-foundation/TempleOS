Bool OptEqu(I64 num,Bool val)
{//Set compiler option to value.
  return BEqu(&Fs->last_lex->opts,num,val);
}

Bool OptOn(I64 num)
{//Turn on compiler option.
  return OptEqu(num,ON);
}

Bool OptOff(I64 num)
{//Turn off compiler option.
  return OptEqu(num,OFF);
}

Bool GetOpt(I64 num)
{//Get state of compiler option.
  return Bt(&Fs->last_lex->opts,num);
}

Bool OTrace(Bool val)
{//Displays intermediate code after passes through optimization.
  return OptEqu(OPTf_OPT_TRACE,val);
}

I64 OTraceMask(I64 i=0b10001111101)
{//Ctrls which optimizer passes are displayed.
  I64 old=Fs->last_lex->otrace_mask;
  Fs->last_lex->otrace_mask=i;
  return old;
}

Bool CTrace(Bool val)
{//Displays assembly code output from compiler.
  return OptEqu(OPTf_COMPILE_TRACE,val);
}

Bool Echo(Bool val)
{//Displays text as it is being compiled.
  return OptEqu(OPTf_ECHO,val);
}

U0 StreamPrint(U8 *fmt,...)
{//Injects text into the compile stream. Used in #exe{} blocks.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv),*st;
  CCmpCtrl *cc=Fs->last_lex;
  CStreamBlk *tempe=cc->last_stream_blk;
  if (tempe!=&cc->next_stream_blk) {
    st=MStrPrint("%s%s",tempe->body,buf);
    Free(tempe->body);
    tempe->body=st;
  } else
    PrintErr("No exe{} blk\n");
  Free(buf);
}

U0 StreamDir()
{
  U8 *dirname;
  if (dirname=DirFile(Fs->last_lex->lex_include_stk->full_name)) {
    StreamPrint("\"%s\"",dirname);
    Free(dirname);
  }
}

CD2I32 *LexD2I32(CCmpCtrl *cc,CD2I32 *p)
{//Not HolyC. Sprite-like lex 2D point.
  if (cc->token!='(')
    LexExcept(cc,"expecting '(' at ");
  Lex(cc); //Skip (
  p->x=LexExpressionI64(cc);
  if (cc->token!=',')
    LexExcept(cc,"expecting ',' at ");
  Lex(cc); //Skip ,
  p->y=LexExpressionI64(cc);
  if (cc->token!=')')
    LexExcept(cc,"expecting ')' at ");
  Lex(cc); //Skip )
  return p;
}

CD3I32 *LexD3I32(CCmpCtrl *cc,CD3I32 *p)
{//Not HolyC. Sprite-like lex 3D point.
  if (cc->token!='(')
    LexExcept(cc,"expecting '(' at ");
  Lex(cc); //Skip (
  p->x=LexExpressionI64(cc);
  if (cc->token!=',')
    LexExcept(cc,"expecting ',' at ");
  Lex(cc); //Skip ,
  p->y=LexExpressionI64(cc);
  if (cc->token!=',')
    LexExcept(cc,"expecting ',' at ");
  Lex(cc); //Skip ,
  p->z=LexExpressionI64(cc);
  if (cc->token!=')')
    LexExcept(cc,"expecting ')' at ");
  Lex(cc); //Skip )
  return p;
}

U8 *CmdLinePmt()
{
  I64 i;
  U8 *result,*st;
  if (Fs->new_answer) {
    if (Fs->answer_type==RT_F64)
      "%8.6fs ansf=%15.7g\n",Fs->answer_time,Fs->answer;
    else
      "%8.6fs ans=0x%08X=%d\n",Fs->answer_time,Fs->answer,Fs->answer;
    Fs->new_answer=FALSE;
  }
  if (st=CurDir) {
    "%s",st;
    Free(st);
  }
  '>';
  if (IsDbg&&IsRaw)
    RawDr;
  st=GetStr(,,GSF_SHIFT_ESC_EXIT);

  i=StrLen(st);
  result=MAlloc(i+1+2);
  MemCpy(result,st,i+1);
  i--;
  while (i>=0 && Bt(chars_bmp_white_space,result[i]))
    i--;
  i++;
  if (i>0 && result[i-1]==';')
    result[i++]=';'; //The Lex goes one beyond
  result[i++]='\n';//#define goes to '\n'
  result[i]=0;

  Free(st);
  return result;
}
