CLexFile *LexFilePush(CCmpCtrl *cc)
{//#include file push.
  CLexFile *res=CAlloc(sizeof(CLexFile));
  if (res->next=cc->lex_include_stk)
    res->depth=res->next->depth+1;
  else
    res->depth=-1; //Include depth starts with -1.
  return cc->lex_include_stk=res;
}

CLexFile *LexFilePop(CCmpCtrl *cc)
{//#include file pop.
  CLexFile *tmpf;
  if (tmpf=cc->lex_include_stk) {
    if ((cc->lex_include_stk=tmpf->next) || !(cc->flags & CCF_DONT_FREE_BUF)) {
      if (tmpf->flags & LFSF_DOC) {
	if (tmpf->doc)
	  DocDel(tmpf->doc);
      } else
	Free(tmpf->buf);;
    }
    Free(tmpf->full_name);
    Free(tmpf);
  }
  return cc->lex_include_stk;
}

CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL)
{//MAlloc and Init CCmpCtrl.
//Frees buf in $LK,"CmpCtrlDel",A="MN:CmpCtrlDel"$ unless $LK,"CCF_DONT_FREE_BUF",A="MN:CCF_DONT_FREE_BUF"$ flag is set.
  //FileName is for error reporting.  If files are #included,
  //new names are used.  See $LK,"Psalmody CmpCtrlNew",A="FF:::/Apps/Psalmody/PsalmodyFile.HC,CmpCtrlNew"$.
  CCmpCtrl *cc=CAlloc(sizeof(CCmpCtrl));
  CLexFile *tmpf;
  QueInit(cc);
  cc->flags=flags;
  cc->opts=1<<OPTf_WARN_UNUSED_VAR|1<<OPTf_WARN_HEADER_MISMATCH;
  cc->htc.hash_mask=HTG_TYPE_MASK-HTT_IMPORT_SYS_SYM;
  cc->htc.define_hash_table=cc->htc.hash_table_lst=
	cc->htc.glbl_hash_table=cc->htc.local_hash_table=Fs->hash_table;
  if (flags&CCF_KEEP_AT_SIGN)
    cc->char_bmp_alpha_numeric=char_bmp_alpha_numeric_no_at;
  else
    cc->char_bmp_alpha_numeric=char_bmp_alpha_numeric;
  tmpf=LexFilePush(cc);
  QueInit(&cc->next_stream_blk);
  if (filename)
    tmpf->full_name=FileNameAbs(filename);
  else
    tmpf->full_name=StrNew(blkdev.tmp_filename);
  if (flags & CCF_PMT)
    buf=CAlloc(8);
  tmpf->buf=tmpf->buf_ptr=tmpf->line_start=cc->cur_buf_ptr=buf;
  tmpf->line_num=1;
  return cc;
}

U0 CmpCtrlDel(CCmpCtrl *cc)
{//Free CCmpCtrl.
  while (LexFilePop(cc));
  LinkedLstDel(cc->lex_prs_stk);
  LinkedLstDel(cc->htc.next);
  Free(cc->ps);
  Free(cc->cur_str);
  Free(cc->cur_help_idx);
  Free(cc->dollar_buf);
  Free(cc);
}

I64 CmpCtrlSize(CCmpCtrl *cc)
{//Mem size of CCmpCtrl and its members.
  CLexFile *tmpf=cc->lex_include_stk;
  I64 res=0;
  while (tmpf) {
    if (tmpf->next || !(cc->flags & CCF_DONT_FREE_BUF)) {
      if (tmpf->flags & LFSF_DOC) {
	if (tmpf->doc)
	  res+=DocSize(tmpf->doc);
      } else
	res+=MSize2(tmpf->buf);
    }
    res+=MSize2(tmpf->full_name);
    res+=MSize2(tmpf);
    tmpf=tmpf->next;
  }
  res+=MSize2(cc->cur_str);
  res+=MSize2(cc);
  return res;
}

U32 lex_zeros=0;

Bool LexDollar(CCmpCtrl *cc,CDoc *doc,CDocEntry *doc_e)
{
  U8 *st;
  if (cc->flags&CCF_IN_QUOTES) {
    Free(cc->dollar_buf);
    st=Doc2PlainText(doc,doc_e);
    cc->dollar_buf=MStrPrint("$$%$$Q$$",st);
    cc->dollar_cnt=2;
    Free(st);
    return TRUE;
  } else
    return FALSE;
}

I64 LexGetChar(CCmpCtrl *cc)
{//Get one char from stream. Allow put-back one.
  U8 *ptr,*src;
  CLexFile *tmpf;
  CDoc *doc;
  CDocEntry *doc_e;
  if (!Btr(&cc->flags,CCf_USE_LAST_U16)) {
lgc_start1:
    if (!(src=cc->cur_buf_ptr++)) {
      cc->cur_buf_ptr=NULL;
      goto lgc_here;
    }
    switch [cc->last_U16=*src++] {
      case 0:
lgc_here:
	tmpf=cc->lex_include_stk;
	if (tmpf->flags & LFSF_DOC) {
	  doc=tmpf->doc;
	  doc_e=tmpf->cur_entry;
	  doc_e=doc_e->next;
lgc_start2:
	  if (doc_e!=doc) {
	    tmpf->cur_entry=doc_e;
	    switch [doc_e->type_u8] {
	      case DOCT_TEXT:
		if (doc_e->de_flags & ~(DOCEF_TAG|DOCEF_DEFINE|DOCEF_TAG_CB|
		      DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT|
		      DOCEF_SKIP|DOCEF_FILTER_SKIP) &&
		      LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tmpf->line_num=doc_e->y+1;
		  tmpf->buf_ptr=cc->cur_buf_ptr=src;
		} else if (*(src=doc_e->tag))
		  tmpf->buf_ptr=cc->cur_buf_ptr=src;
		else {
		  doc_e=doc_e->next;
		  goto lgc_start2;
		}
		break;
	      case DOCT_NEW_LINE:
		tmpf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tmpf->line_start=doc_e->next;
		tmpf->line_num=doc_e->y+2;//+1 because NEW_LINE is on prev line
//+1 because doc y starts at zero
		cmp.compiled_lines++;
		cc->last_U16='\n';
		goto lgc_done;
	      case DOCT_TAB:
		tmpf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tmpf->line_num=doc_e->y+1;
		cc->last_U16='\t';
		goto lgc_done;
	      case DOCT_INS_BIN:
		tmpf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tmpf->line_num=doc_e->y+1;
		Free(cc->cur_str);
		cc->cur_str=NULL;
		cc->cur_str_len=0;
		if (doc_e->bin_data) {
		  ptr=MAlloc(doc_e->bin_data->size);
		  if (doc_e->bin_data->data)
		    MemCpy(ptr,doc_e->bin_data->data,doc_e->bin_data->size);
		  cc->cur_str=ptr;
		  cc->cur_str_len=doc_e->bin_data->size;
		}
		cc->last_U16=TK_INS_BIN;
		goto lgc_done;
	      case DOCT_INS_BIN_SIZE:
		tmpf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		if (doc_e->bin_data)
		  cc->cur_i64=doc_e->bin_data->size;
		else
		  cc->cur_i64=0;
		tmpf->line_num=doc_e->y+1;
		cc->last_U16=TK_INS_BIN_SIZE;
		goto lgc_done;
	      case DOCT_SHIFTED_Y:
		if (LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tmpf->line_num=doc_e->y+1;
		  tmpf->buf_ptr=cc->cur_buf_ptr=src;
		} else {
		  tmpf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		  tmpf->line_num=doc_e->y+1;
		  if (doc_e->attr<0)
		    cc->last_U16=TK_SUPERSCRIPT;
		  else if (doc_e->attr>0)
		    cc->last_U16=TK_SUBSCRIPT;
		  else
		    cc->last_U16=TK_NORMALSCRIPT;
		  goto lgc_done;
		}
		break;
	      case DOCT_MARKER:
	      case DOCT_CURSOR:
		doc_e=doc_e->next;
		goto lgc_start2;
	      case 0xFF: //nobound switch
	      default:
		if (LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tmpf->line_num=doc_e->y+1;
		  tmpf->buf_ptr=cc->cur_buf_ptr=src;
		} else {
		  doc_e=doc_e->next;
		  goto lgc_start2;
		}
	    }
	  }
	  if (doc_e!=doc)
	    goto lgc_start1;
	  tmpf->cur_entry=doc->head.last; //When take next, will still be end.
	}
	tmpf=cc->lex_include_stk;
	if (tmpf->next) {
	  tmpf=LexFilePop(cc);
	  cc->cur_buf_ptr=tmpf->buf_ptr;
	  cc->flags&=~CCF_USE_LAST_U16;
	  if (!(cc->last_U16=tmpf->last_U16))
	    goto lgc_start1;
	} else {
	  if (cc->flags & CCF_PMT) {
	    Free(tmpf->buf);
	    ptr=CmdLinePmt;
	    if (StrCmp(ptr,"\n") && !cc->pmt_line++ && !StrCmp(ptr,"?\n") &&
		cc->flags & CCF_QUESTION_HELP) {
	      Free(ptr);
	      ptr=StrNew("Help;;\n");
	    }
	    tmpf->buf=tmpf->buf_ptr=tmpf->line_start=cc->cur_buf_ptr=ptr;
	    goto lgc_start1;
	  } else {
	    if (src)
	      cc->cur_buf_ptr=src-1;
	    cc->last_U16=TK_EOF;
	  }
	}
	break;
      case CH_CURSOR:
	goto lgc_start1;
      case '\n':
	tmpf=cc->lex_include_stk;
	if (!(tmpf->flags & LFSF_DOC)) {
	  tmpf->line_num++;
	  cmp.compiled_lines++;
	  tmpf->line_start=src;
	}
	break;
      case 0xFF: //nobound switch
    }
lgc_done:
    if (cc->last_U16==CH_SHIFT_SPACE)
      cc->last_U16=CH_SPACE;
    if (cc->opts & OPTF_ECHO &&
	  cc->last_U16<256 && Bt(char_bmp_printable,cc->last_U16))
      '' cc->last_U16;
  }
  return cc->last_U16;
}

U0 LexSkipEol(CCmpCtrl *cc)
{//$LK,"LexGetChar",A="MN:LexGetChar"$ to NULL until end-of-line.
  I64 ch;
  do ch=$WW,0$LexGetChar(cc);
  while (Bt(char_bmp_non_eol,ch));
}

U8 *LexFirstRem(CCmpCtrl *cc,U8 *marker,I64 _len=NULL)
{//$LK,"LexGetChar",A="MN:LexGetChar"$() chars making str until marker.
  U8 *res,*ptr;
  CQueVectU8 *tmpv=QueVectU8New;
  I64 i,len=0;
  while (TRUE) {
    i=LexGetChar(cc);
    if (!i||StrOcc(marker,i))
      break;
    QueVectU8Put(tmpv,len++,i);
  }
  if (i)
    Bts(&cc->flags,CCf_USE_LAST_U16);
  res=ptr=MAlloc(len+1);
  for (i=0;i<len;i++)
    *ptr++=QueVectU8Get(tmpv,i);
  *ptr=0;
  QueVectU8Del(tmpv);
  if (_len) *_len=len;
  return res;
}

U0 LexIncludeStr(CCmpCtrl *cc,U8 *abs_filename,U8 *src,Bool actual_file)
{
  LexBackupLastChar(cc);
  CLexFile *tmpf=LexFilePush(cc);
  if (actual_file)
    tmpf->full_name=StrNew(abs_filename);
  else
    tmpf->full_name=StrNew(blkdev.tmp_filename);
  tmpf->line_num=1;
  tmpf->buf=tmpf->buf_ptr=tmpf->line_start=cc->cur_buf_ptr=src;
}

CDoc *LexDocRead(U8 *abs_filename,I64 flags)
{
  CDoc *doc=DocNew(abs_filename);
  U8 *src;
  I64 size=0;
  doc->flags|=flags;
  src=FileRead(abs_filename,&size);
  if (!src || !size) {
    Free(src);
    src=CAlloc(1);
    size=0;
  }
  DocLoad(doc,src,size);
  Free(src);
  return doc;
}

I64 cmp_type_flags_src_code[(DOCT_TYPES_NUM+63)/64]={
  1<<DOCT_TEXT|1<<DOCT_TAB|1<<DOCT_INS_BIN|1<<DOCT_INS_BIN_SIZE};

U0 LexAttachDoc(CCmpCtrl *cc,CLexFile *tmpf=NULL,
	CDoc *doc=NULL,U8 *abs_filename=NULL,CDocEntry *doc_e=NULL,I64 col=0)
{//Start lexing doc. Give either doc or abs_filename.
  if (!doc)
    doc=LexDocRead(abs_filename,DOCF_DBL_DOLLARS);
  if (!tmpf) {
    LexBackupLastChar(cc);
    tmpf=LexFilePush(cc);
  }
  if (!doc_e)
    doc_e=doc->head.next;
  tmpf->full_name=StrNew(doc->filename.name);
  tmpf->doc=doc;
  while (doc_e!=doc) {
    if (Bt(cmp_type_flags_src_code,doc_e->type_u8))
      break;
    doc_e=doc_e->next;
    col=doc_e->min_col;
  }
  if (doc_e!=doc) {
    col=ClampI64(col,doc_e->min_col,doc_e->max_col);
    tmpf->line_start=doc_e;
    tmpf->buf=NULL;
    tmpf->line_num=doc_e->y+1;
    if (doc_e->type_u8==DOCT_TEXT) {
      tmpf->cur_entry=doc_e;
      tmpf->buf_ptr=doc_e->tag;
    } else {
      tmpf->cur_entry=doc_e->last; //TODO: might be problem at begin of file
      tmpf->buf_ptr=&lex_zeros;
    }
    tmpf->flags=LFSF_DOC;
  } else {//TODO: DocDel(doc)?
    col=0;
    tmpf->buf=tmpf->buf_ptr=tmpf->line_start=CAlloc(1);
    tmpf->line_num=1;
    tmpf->flags=0;
  }
  cc->cur_buf_ptr=tmpf->buf_ptr+col;
  tmpf->last_U16=0;
}

I64 LexInStr(CCmpCtrl *cc,U8 *buf,I64 size,Bool *done)
{
  I64 i=0,j,k,ch;
  *done=TRUE;
  while (i<size-1) {
    ch=LexGetChar(cc);
    if (!ch || ch=='"') {
      buf[i++]=0;
      return i;
    } else if (ch=='\\') {
      switch (ch=LexGetChar(cc)) {
	case '0':
	  buf[i++]=0;
	  break;
	case '\'':
	  buf[i++]='\'';
	  break;
	case '\`':
	  buf[i++]='\`';
	  break;
	case '\\':
	  buf[i++]='\\';
	  break;
	case '"':
	  buf[i++]='"';
	  break;
	case 'd':
	  buf[i++]='$$';
	  break;
	case 'n':
	  buf[i++]='\n';
	  break;
	case 'r':
	  buf[i++]='\r';
	  break;
	case 't':
	  buf[i++]='\t';
	  break;
	case 'x':
	case 'X':
	  j=0;
	  for (k=0;k<2;k++) {
	    ch=ToUpper(LexGetChar(cc));
	    if (Bt(char_bmp_hex_numeric,ch)) {
	      if (ch<='9')
		j=j<<4+ch-'0';
	      else
		j=j<<4+ch-'A'+10;
	    } else {
	      cc->flags|=CCF_USE_LAST_U16;
	      break;
	    }
	  }
	  buf[i++]=j;
	  break;
	default:
	  cc->flags|=CCF_USE_LAST_U16;
	  buf[i++]='\\';
      }
    } else if (ch=='$$') {
      buf[i++]='$$';
      if (cc->dollar_cnt)
	cc->dollar_cnt--;
      else if (LexGetChar(cc)!='$$') {
	cc->dollar_cnt=1;
	cc->flags|=CCF_USE_LAST_U16;
      }
    } else
      buf[i++]=ch;
  }
  *done=FALSE;
  return i;
}

I64 Lex(CCmpCtrl *cc)
{//Fetch next token.
  I64 i,j,k,l,ch;
  CHash *tmph;
  Bool str_done,in_str,neg_e;
  U8 *fbuf,*buf2,*buf3,buf[STR_LEN];
  cc->last_line_num=cc->lex_include_stk->line_num;
  while (TRUE) {
lex_cont:
    switch [ch=LexGetChar(cc)] {
      case 0:
	return cc->token=TK_EOF;
      case TK_SUPERSCRIPT:
	ch='>';
	goto lex_ident;
      case TK_SUBSCRIPT:
	ch='<';
	goto lex_ident;
      case TK_NORMALSCRIPT:
	ch='=';
	goto lex_ident;
      case '@':
	if (cc->flags&CCF_KEEP_AT_SIGN) {
	  cc->token=ch;
	  goto lex_end;
	}
      case 'A'...'Z':
      case 'a'...'z':
      case '_':
      case 128...255:
lex_ident:
	i=0;
	buf[i++]=ch;
	while (TRUE) {
	  if (i>=STR_LEN)
	    LexExcept(cc,"Ident limited to STR_LEN chars at ");
	  else if (!(ch=LexGetChar(cc)))
	    break;
	  else if (Bt(cc->char_bmp_alpha_numeric,ch))
	    buf[i++]=ch;
	  else if (ch==TK_SUPERSCRIPT)
	    buf[i++]='>';
	  else if (ch==TK_SUBSCRIPT)
	    buf[i++]='<';
	  else if (ch==TK_NORMALSCRIPT)
	    buf[i++]='=';
	  else {
	    cc->flags|=CCF_USE_LAST_U16;
	    break;
	  }
	}
	buf[i++]=0;
	tmph=NULL;
	if (cc->htc.local_var_lst)
	  cc->local_var_entry=MemberFind(buf,cc->htc.local_var_lst);
	else
	  cc->local_var_entry=NULL;
	if (!cc->local_var_entry && cc->htc.hash_table_lst)
	  tmph=HashFind(buf,cc->htc.hash_table_lst,cc->htc.hash_mask);
	if (tmph)
	  j=tmph->type;
	else
	  j=0;
	if (j & HTT_DEFINE_STR && !(cc->flags & CCF_NO_DEFINES)) {
	  LexIncludeStr(cc,
		tmph->str,StrNew(tmph(CHashDefineStr *)->data),FALSE);
	  cc->lex_include_stk->flags|=LFSF_DEFINE;
	} else {
	  cc->hash_entry=tmph;
	  Free(cc->cur_str);
	  cc->cur_str=StrNew(buf);
	  cc->cur_str_len=i;
	  cc->token=TK_IDENT;
	  goto lex_end;
	}
	break;
      case '0'...'9':
	i=ch-'0';
	ch=ToUpper(LexGetChar(cc));
	if (ch=='X') {
	  while (TRUE) {
	    ch=ToUpper(LexGetChar(cc));
	    if (Bt(char_bmp_hex_numeric,ch)) {
	      if (ch<='9')
		i=i<<4+ch-'0';
	      else
		i=i<<4+ch-'A'+10;
	    } else {
	      cc->cur_i64=i;
	      cc->flags|=CCF_USE_LAST_U16;
	      cc->token=TK_I64;
	      goto lex_end;
	    }
	  }
	} else if (ch=='B') {
	  while (TRUE) {
	    ch=LexGetChar(cc);
	    if (ch=='0')
	      i=i<<1;
	    else if (ch=='1')
	      i=i<<1+1;
	    else {
	      cc->cur_i64=i;
	      cc->flags|=CCF_USE_LAST_U16;
	      cc->token=TK_I64;
	      goto lex_end;
	    }
	  }
	}
	while (TRUE) {
	  if (Bt(char_bmp_dec_numeric,ch))
	    i=i*10+ch-'0';
	  else {
	    if (ch=='.' || ch=='e' || ch=='E') break;
lex_is_int:
	    cc->cur_i64=i;
	    cc->flags|=CCF_USE_LAST_U16;
	    cc->token=TK_I64;
	    goto lex_end;
	  }
	  ch=LexGetChar(cc);
	}
	if (ch=='.') {
	  ch=LexGetChar(cc);
	  if (ch=='.') {
	    cc->flags|=CCF_LAST_WAS_DOT;
	    goto lex_is_int;
	  }
	}
lex_float_start:
	k=0;
	while (TRUE) {
	  if (Bt(char_bmp_dec_numeric,ch)) {
	    i=i*10+ch-'0';
	    k++;
	  } else {
	    if (ch=='e' || ch=='E')
	      break;
	    cc->cur_f64=i*Pow10I64(-k);
	    cc->flags|=CCF_USE_LAST_U16;
	    cc->token=TK_F64;
	    goto lex_end;
	  }
	  ch=LexGetChar(cc);
	}
	ch=LexGetChar(cc);
	neg_e=FALSE;
	if (ch=='-') {
	  neg_e=TRUE;
	  ch=LexGetChar(cc);
	}
	j=0;
	while (TRUE) {
	  if (Bt(char_bmp_dec_numeric,ch))
	    j=j*10+ch-'0';
	  else {
	    if (neg_e)
	      cc->cur_f64=i*Pow10I64(-j-k);
	    else
	      cc->cur_f64=i*Pow10I64(j-k);
	    cc->flags|=CCF_USE_LAST_U16;
	    cc->token=TK_F64;
	    goto lex_end;
	  }
	  ch=LexGetChar(cc);
	}
	break;
      case '"':
	cc->flags|=CCF_IN_QUOTES;
	buf2=NULL;
	i=0;
	do {
	  j=LexInStr(cc,buf,STR_LEN,&str_done);
	  buf3=MAlloc(i+j);
	  if (buf2) {
	    MemCpy(buf3,buf2,i);
	    Free(buf2);
	    buf2=buf3;
	    MemCpy(buf2+i,buf,j);
	  } else {
	    buf2=buf3;
	    MemCpy(buf2,buf,j);
	  }
	  i+=j;
	} while (!str_done);
	Free(cc->cur_str);
	cc->cur_str=MAlloc(i);
	MemCpy(cc->cur_str,buf2,i);
	Free(buf2);
	cc->cur_str_len=i;
	cc->flags&=~CCF_IN_QUOTES;
	cc->token=TK_STR;
	goto lex_end;
      case '\'':
	if (cc->flags&CCF_NO_CHAR_CONST)
	  break;
	k=0;
	for (j=0;j<8;j++) {
	  if (!(ch=LexGetChar(cc)) || ch=='\'')
	    break;
	  if (ch=='\\') {
	    switch (ch=LexGetChar(cc)) {
	      case '0':		k.u8[j]=0;	break;
	      case '\'':	k.u8[j]='\'';	break;
	      case '\`':	k.u8[j]='\`';	break;
	      case '"':		k.u8[j]='"';	break;
	      case '\\':	k.u8[j]='\\';	break;
	      case 'd':		k.u8[j]='$$';	break;
	      case 'n':		k.u8[j]='\n';	break;
	      case 'r':		k.u8[j]='\r';	break;
	      case 't':		k.u8[j]='\t';	break;
	      case 'x':
	      case 'X':
		i=0;
		for (l=0;l<2;l++) {
		  ch=ToUpper(LexGetChar(cc));
		  if (Bt(char_bmp_hex_numeric,ch)) {
		    if (ch<='9')
		      i=i<<4+ch-'0';
		    else
		      i=i<<4+ch-'A'+10;
		  } else {
		    cc->flags|=CCF_USE_LAST_U16;
		    break;
		  }
		}
		k.u8[j]=i;
		break;
	      default:
		k.u8[j]='\\';
		cc->flags|=CCF_USE_LAST_U16;
	    }
	  } else if (ch=='$$') {
	    ch=LexGetChar(cc);
	    k.u8[j]='$$';
	    if (ch!='$$')
	      cc->flags|=CCF_USE_LAST_U16;
	  } else
	    k.u8[j]=ch;
	}
	if (ch!='\'' && (ch=LexGetChar(cc)) && ch!='\'')
	  LexExcept(cc,"Char const limited to 8 chars at ");
	cc->cur_i64=k;
	cc->token=TK_CHAR_CONST;
	goto lex_end;
      case '#':
	if (cc->flags&CCF_KEEP_SIGN_NUM) {
	  cc->token=ch;
	  goto lex_end;
	}
	if (Lex(cc)!=TK_IDENT)	//skip '#'
	  goto lex_end;
	if (!(tmph=cc->hash_entry))
	  goto lex_end;
	if (!(tmph->type & HTT_KEYWORD))
	  goto lex_end;
	switch (i=tmph(CHashGeneric *)->user_data0) {
	  case KW_INCLUDE:
	    if (Lex(cc)!=TK_STR)
	      goto lex_end;
	    fbuf=ExtDft(cc->cur_str,"HC.Z");
	    buf2=FileNameAbs(fbuf);
	    Free(fbuf);
	    if (Bt(&sys_run_level,RLf_DOC))
	      LexAttachDoc(cc,,,buf2);
	    else
	      LexIncludeStr(cc,buf2,FileRead(buf2),TRUE);
	    Free(buf2);
	    break;
	  case KW_DEFINE:
	    cc->flags|=CCF_NO_DEFINES;
	    if (Lex(cc)==TK_IDENT) {
	      tmph=CAlloc(sizeof(CHashDefineStr));
	      tmph->str=cc->cur_str;
	      cc->cur_str=0;
	      tmph->type=HTT_DEFINE_STR;
	      HashSrcFileSet(cc,tmph);

	      do ch=LexGetChar(cc); //skip space between define name and start
	      while (Bt(char_bmp_non_eol_white_space,ch));

	      i=j=0;
	      buf2=NULL;
	      if (ch) {
		in_str=FALSE;
		do {
		  if (ch=='\\') {
		    if (ch=LexGetChar(cc)) {
		      if (ch!='\r' && ch!='\n') {
			buf[j++]='\\';
			buf[j++]=ch;
		      } else if (ch=='\r' && LexGetChar(cc)!='\n')
			cc->flags|=CCF_USE_LAST_U16;
		    } else {
		      buf[j++]='\\';
		      break;
		    }
		  } else if (ch!='\n') {
		    if (ch=='\"')
		      in_str=!in_str;
		    buf[j++]=ch;
		  } else
		    break;
		  while (ch=LexGetChar(cc)) {
		    if (ch=='/') {
		      ch=LexGetChar(cc);
		      if (ch=='/' && !in_str) {
			do ch=LexGetChar(cc);
			while (Bt(char_bmp_non_eol,ch));
			break;
		      } else {
			buf[j++]='/';
			cc->flags|=CCF_USE_LAST_U16;
		      }
		    } else if (ch=='\\') {
		      if (ch=LexGetChar(cc)) {
			if (ch=='\"') {
			  buf[j++]='\\';
			  buf[j++]=ch;
			} else {
			  cc->flags|=CCF_USE_LAST_U16;
			  ch='\\';
			  break;
			}
		      }
		    } else if (Bt(char_bmp_non_eol,ch)) {
		      if (ch=='\"')
			in_str=!in_str;
		      buf[j++]=ch;
		    } else
		      break;
		    if (j>=STR_LEN-4) {//Spot for ['\'][ch],[ch],[0]
		      buf[j++]=0;
		      buf3=MAlloc(i+j);
		      if (buf2) {
			MemCpy(buf3,buf2,i);
			Free(buf2);
			buf2=buf3;
			MemCpy(buf2+i,buf,j);
		      } else {
			buf2=buf3;
			MemCpy(buf2,buf,j);
		      }
		      i+=j-1;
		      j=0;
		    }
		  }
		} while (ch=='\\');
	      }
	      buf[j++]=0;
	      buf3=MAlloc(i+j);
	      if (buf2) {
		MemCpy(buf3,buf2,i);
		Free(buf2);
		buf2=buf3;
		MemCpy(buf2+i,buf,j);
	      } else {
		buf2=buf3;
		MemCpy(buf2,buf,j);
	      }
	      tmph(CHashDefineStr *)->data=buf2;
	      tmph(CHashDefineStr *)->cnt=-1;
	      HashAdd(tmph,cc->htc.define_hash_table);
	    }
	    cc->flags&=~CCF_NO_DEFINES;
	    break;
	  case KW_ELSE:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_ELSE;
	      goto lex_end;
	    }
lex_else:
	    j=1;
	    do {
	      if (ch=LexGetChar(cc)) {
		if (ch=='#') {
		  if (!Lex(cc))
		    goto lex_end;
		  i=PrsKeyWord(cc);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		}
	      } else {
		cc->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;

	  case KW_IF:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_IF;
	      goto lex_end;
	    }
lex_if:
	    cc->flags|=CCF_IN_IF;
	    if (!Lex(cc)) {
	      cc->flags&=~CCF_IN_IF;
	      goto lex_end;
	    }
	    if (LexExpression(cc)) {
	      cc->flags&=~CCF_IN_IF;
	      switch (cc->token) {
		case TK_IF:	goto lex_if;
		case TK_IFDEF:	goto lex_ifdef;
		case TK_IFNDEF:	goto lex_ifndef;
		case TK_IFAOT:	goto lex_ifaot;
		case TK_IFJIT:	goto lex_ifjit;
		case TK_ELSE:	goto lex_else;
		case TK_ENDIF:	goto lex_cont;
		default:	goto lex_end;
	      }
	    } else {
	      cc->flags&=~CCF_IN_IF;
	      if (cc->token!=TK_ENDIF && cc->token!=TK_ELSE) {
		if (cc->token==TK_IF || cc->token==TK_IFDEF ||
		      cc->token==TK_IFNDEF || cc->token==TK_IFAOT ||
		      cc->token==TK_IFJIT)
		  j=2;
		else
		  j=1;
		do {
		  if (ch=LexGetChar(cc)) {
		    if (ch=='#') {
		      if (!Lex(cc))
			goto lex_end;
		      i=PrsKeyWord(cc);
		      if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			    i==KW_IFAOT || i==KW_IFJIT)
			j++;
		      else if (i==KW_ENDIF)
			j--;
		      else if (i==KW_ELSE && j==1)
			break;
		    }
		  } else {
		    cc->token=TK_EOF;
		    goto lex_end;
		  }
		} while (j);
	      }
	    }
	    break;
	  case KW_IFDEF:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_IFDEF;
	      goto lex_end;
	    }
lex_ifdef:
	    cc->flags|=CCF_NO_DEFINES;
	    if (!Lex(cc)) {
	      cc->flags&=~CCF_NO_DEFINES;
	      goto lex_end;
	    }
	    cc->flags&=~CCF_NO_DEFINES;
	    if (cc->token!=TK_IDENT)
	      goto lex_end;
	    if (cc->hash_entry)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(cc)) {
		if (ch=='#') {
		  if (!Lex(cc))
		    goto lex_end;
		  i=PrsKeyWord(cc);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		cc->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFNDEF:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_IFNDEF;
	      goto lex_end;
	    }
lex_ifndef:
	    cc->flags|=CCF_NO_DEFINES;
	    if (!Lex(cc)) {
	      cc->flags&=~CCF_NO_DEFINES;
	      goto lex_end;
	    }
	    cc->flags&=~CCF_NO_DEFINES;
	    if (cc->token!=TK_IDENT)
	      goto lex_end;
	    if (!cc->hash_entry)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(cc)) {
		if (ch=='#') {
		  if (!Lex(cc))
		    goto lex_end;
		  i=PrsKeyWord(cc);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		cc->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFAOT:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_IFAOT;
	      goto lex_end;
	    }
lex_ifaot:
	    if (cc->flags & CCF_AOT_COMPILE)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(cc)) {
		if (ch=='#') {
		  if (!Lex(cc))
		    goto lex_end;
		  i=PrsKeyWord(cc);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		cc->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFJIT:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_IFAOT;
	      goto lex_end;
	    }
lex_ifjit:
	    if (!(cc->flags & CCF_AOT_COMPILE))
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(cc)) {
		if (ch=='#') {
		  if (!Lex(cc))
		    goto lex_end;
		  i=PrsKeyWord(cc);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF ||
			i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		cc->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_ENDIF:
	    if (cc->flags & CCF_IN_IF) {
	      cc->token=TK_ENDIF;
	      goto lex_end;
	    }
	    break;
	  case KW_ASSERT:
	    if (!Lex(cc))
	      goto lex_end;
	    if (!LexExpression(cc))
	      LexWarn(cc,"Assert Failed ");
	    goto lex_end;
	  case KW_EXE:
	    if (!Lex(cc))
	      goto lex_end;
	    PrsStreamBlk(cc);
	    goto lex_end;
	  case KW_HELP_INDEX:
	    if (Lex(cc)!=TK_STR)
	      goto lex_end;
	    Free(cc->cur_help_idx);
	    cc->cur_help_idx=LexExtStr(cc,,FALSE);
	    break;
	  case KW_HELP_FILE:
	    if (Lex(cc)!=TK_STR)
	      goto lex_end;
	    tmph=CAlloc(sizeof(CHashSrcSym));
	    fbuf=ExtDft(cc->cur_str,"DD.Z");
	    tmph->str=FileNameAbs(fbuf);
	    Free(fbuf);
	    tmph->type=HTT_HELP_FILE|HTF_PUBLIC;
	    HashSrcFileSet(cc,tmph);
	    HashAdd(tmph,cc->htc.glbl_hash_table);
	    break;
	}
	break;
      case '\n':
	if (!(cc->flags&CCF_KEEP_NEW_LINES))
	  break; //else fall through
      case TK_INS_BIN:
      case TK_INS_BIN_SIZE:
	cc->token=ch;
	goto lex_end;
      case '.':
	if (cc->flags&CCF_KEEP_DOT) {
	  cc->token=ch;
	  goto lex_end;
	}
	if (cc->flags&CCF_LAST_WAS_DOT) {
	  cc->flags&=~CCF_LAST_WAS_DOT;
	  goto lex_dot_dot;
	}
	ch=LexGetChar(cc);
	if ('0'<=ch<='9') {
	  i=0;
	  goto lex_float_start;
	} else if (ch=='.') {
lex_dot_dot:
	  cc->token=TK_DOT_DOT;
	  if (LexGetChar(cc)=='.')
	    cc->token=TK_ELLIPSIS;
	  else
	    cc->flags|=CCF_USE_LAST_U16;
	  goto lex_end;
	}
	cc->flags|=CCF_USE_LAST_U16;
	cc->token='.';
	goto lex_end;
      case '!':
      case '$$'...'&':
      case '('...'-':
      case '/':
      case ':'...'?':
      case '[':
      case ']'...'^':
      case '{'...'~':
      case '`':
	if (!(i=cmp.dual_U16_tokens1[ch])) {
	  if (ch=='$$') {
	    ch=LexGetChar(cc);
	    if (ch=='$$') {
	      cc->token='$$';
	      goto lex_end;
	    } else if (ch) {
	      do ch=LexGetChar(cc);
	      while (ch && ch!='$$');
	      if (!ch) {
		cc->token=TK_EOF;
		goto lex_end;
	      } else
		goto lex_cont;
	    } else {
	      cc->flags|=CCF_USE_LAST_U16;
	      cc->token='$$';
	      goto lex_end;
	    }
	  } else {
	    cc->token=ch;
	    goto lex_end;
	  }
	} else {
	  j=LexGetChar(cc);
	  if (i.u16[0]==j) {
	    i>>=16;
	    if (!i) {// "/*"
	      j=1;
	      do {
		if (!(ch=LexGetChar(cc)))
		  return cc->token=TK_EOF;
lex_check_comment:
		if (ch=='*') {
		  if (!(ch=LexGetChar(cc)))
		    return cc->token=TK_EOF;
		  if (ch=='/')
		    j--;
		  else
		    goto lex_check_comment;
		} else if (ch=='/') {
		  if (!(ch=LexGetChar(cc)))
		    return cc->token=TK_EOF;
		  if (ch=='*')
		    j++;
		  else
		    goto lex_check_comment;
		}
	      } while (j);
	      goto lex_cont;
	    } else {
	      cc->token=i;
	      goto lex_end;
	    }
	  }
	  if (i=cmp.dual_U16_tokens2[ch]) {
	    if (i.u16[0]==j) {
	      i>>=16;
	      if (!i) {// "//"
		LexSkipEol(cc);
		if (cc->flags&CCF_KEEP_NEW_LINES) {
		  cc->token='\n';
		  goto lex_end;
		} else
		  goto lex_cont;
	      } else {
		if (i==TK_SHL || i==TK_SHR) {
		  j=LexGetChar(cc);
		  if (j=='=') {
		    if (i==TK_SHL)
		      i=TK_SHL_EQU;
		    else
		      i=TK_SHR_EQU;
		  } else
		    cc->flags|=CCF_USE_LAST_U16;
		}
		cc->token=i;
		goto lex_end;
	      }
	    }
	    if (i=cmp.dual_U16_tokens3[ch]) {
	      if (i.u16[0]==j) {
		cc->token=i.u16[1];
		goto lex_end;
	      }
	    }
	  }
	  cc->flags|=CCF_USE_LAST_U16;
	  cc->token=ch;
	  goto lex_end;
	}
      case TK_TKS_NUM:
	break;
    }
  }
lex_end:
  LexGetChar(cc); //Do this so WAS_NEW_LINE is right
  cc->flags|=CCF_USE_LAST_U16;
  return cc->token;
}
