CLexFile *LexFilePush(CCmpCtrl *cc)
{//#include file push.
  CLexFile *result=CAlloc(sizeof(CLexFile));
  if (result->next=cc->lex_include_stk)
    result->depth=result->next->depth+1;
  else
    result->depth=-1; //Include depth starts with -1.
  return cc->lex_include_stk=result;
}

CLexFile *LexFilePop(CCmpCtrl *cc)
{//#include file pop.
  CLexFile *tempf;
  if (tempf=cc->lex_include_stk) {
    if ((cc->lex_include_stk=tempf->next) || !(cc->flags & CCF_DONT_FREE_BUF)) {
      if (tempf->flags & LFSF_DOC) {
	if (tempf->doc)
	  DocDel(tempf->doc);
      } else
	Free(tempf->buf);;
    }
    Free(tempf->full_name);
    Free(tempf);
  }
  return cc->lex_include_stk;
}

CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL)
{//MAlloc and Init CCmpCtrl.
//Frees buf in $LK,"CmpCtrlDel",A="MN:CmpCtrlDel"$ unless $LK,"CCF_DONT_FREE_BUF",A="MN:CCF_DONT_FREE_BUF"$ flag is set.
  //FileName is for error reporting.  If files are #included,
  //new names are used.  See $LK,"Psalmody CmpCtrlNew",A="FF:::/Apps/Psalmody/PsalmodyFile.CPP,CmpCtrlNew"$.
  CCmpCtrl *cc=CAlloc(sizeof(CCmpCtrl));
  CLexFile *tempf;
  QueInit(cc);
  cc->flags=flags;
  cc->opts=1<<OPTf_WARN_UNUSED_VAR;
  cc->htc.hash_mask=HTG_TYPE_MASK-HTT_IMPORT_SYS_SYM;
  cc->htc.define_hash_table=cc->htc.hash_table_lst=
	cc->htc.glbl_hash_table=cc->htc.local_hash_table=Fs->hash_table;
  if (flags&CCF_KEEP_AT_SIGN)
    cc->chars_bmp_alpha_numeric=chars_bmp_alpha_numeric_no_at;
  else
    cc->chars_bmp_alpha_numeric=chars_bmp_alpha_numeric;
  tempf=LexFilePush(cc);
  QueInit(&cc->next_stream_blk);
  if (filename)
    tempf->full_name=FileNameAbs(filename);
  else
    tempf->full_name=StrNew(blkdev.temp_filename);
  if (flags & CCF_PMT)
    buf=CAlloc(8);
  tempf->buf=tempf->buf_ptr=tempf->line_start=cc->cur_buf_ptr=buf;
  tempf->line_num=1;
  return cc;
}

U0 CmpCtrlDel(CCmpCtrl *cc)
{//Free CCmpCtrl.
  while (LexFilePop(cc));
  LinkedLstDel(cc->lex_prs_stk);
  LinkedLstDel(cc->htc.next);
  Free(cc->ps);
  Free(cc->cur_str);
  Free(cc->cur_help_index);
  Free(cc->dollar_buf);
  Free(cc);
}

I64 CmpCtrlSize(CCmpCtrl *cc)
{//Mem size of CCmpCtrl and its members.
  CLexFile *tempf=cc->lex_include_stk;
  I64 result=0;
  while (tempf) {
    if (tempf->next || !(cc->flags & CCF_DONT_FREE_BUF)) {
      if (tempf->flags & LFSF_DOC) {
	if (tempf->doc)
	  result+=DocSize(tempf->doc);
      } else
	result+=MSize2(tempf->buf);
    }
    result+=MSize2(tempf->full_name);
    result+=MSize2(tempf);
    tempf=tempf->next;
  }
  result+=MSize2(cc->cur_str);
  result+=MSize2(cc);
  return result;
}

U32 lex_zeros=0;

Bool LexDollar(CCmpCtrl *cc,CDoc *doc,CDocEntry *doc_e)
{
  U8 *st;
  if (cc->flags&CCF_IN_QUOTES) {
    Free(cc->dollar_buf);
    st=Doc2PlainText(doc,doc_e);
    cc->dollar_buf=MStrPrint("$$%Q$$",st);
    cc->dollar_cnt=2;
    Free(st);
    return TRUE;
  } else
    return FALSE;
}

I64 LexGetChar(CCmpCtrl *cc)
{//Get one char from stream. Allow put-back one.
  U8 *ptr,*src;
  CLexFile *tempf;
  CDoc *doc;
  CDocEntry *doc_e;
  if (!Btr(&cc->flags,CCf_USE_LAST_U16)) {
lgc_start1:
    if (!(src=cc->cur_buf_ptr++)) {
      cc->cur_buf_ptr=NULL;
      goto lgc_here;
    }
    nobound_switch (cc->last_U16=*src++) {
      case 0:
lgc_here:
	tempf=cc->lex_include_stk;
	if (tempf->flags & LFSF_DOC) {
	  doc=tempf->doc;
	  doc_e=tempf->cur_entry;
	  doc_e=doc_e->next;
lgc_start2:
	  if (doc_e!=doc) {
	    tempf->cur_entry=doc_e;
	    nobound_switch (doc_e->type_u8) {
	      case DOCT_TEXT:
		if (doc_e->de_flags & ~(DOCEF_TAG|DOCEF_DEFINE|DOCEF_TAG_CB|
		      DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT|
		      DOCEF_SKIP|DOCEF_FILTER_SKIP) &&
		      LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf_ptr=cc->cur_buf_ptr=src;
		} else if (*(src=doc_e->tag))
		  tempf->buf_ptr=cc->cur_buf_ptr=src;
		else {
		  doc_e=doc_e->next;
		  goto lgc_start2;
		}
		break;
	      case DOCT_NEW_LINE:
		tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tempf->line_start=doc_e->next;
		tempf->line_num=doc_e->y+2;//+1 because NEW_LINE is on prev line
//+1 because doc y starts at zero
		cmp.compiled_lines++;
		cc->last_U16='\n';
		goto lgc_done;
	      case DOCT_TAB:
		tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+1;
		cc->last_U16='\t';
		goto lgc_done;
	      case DOCT_PAGE_BREAK:
		tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+2; //TODO: should be a page down
		cc->last_U16=CH_FORM_FEED;
		goto lgc_done;
	      case DOCT_INS_BIN:
		tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+1;
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
		tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		if (doc_e->bin_data)
		  cc->cur_i64=doc_e->bin_data->size;
		else
		  cc->cur_i64=0;
		tempf->line_num=doc_e->y+1;
		cc->last_U16=TK_INS_BIN_SIZE;
		goto lgc_done;
	      case DOCT_SHIFTED_Y:
		if (LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf_ptr=cc->cur_buf_ptr=src;
		} else {
		  tempf->buf_ptr=cc->cur_buf_ptr=&lex_zeros;
		  tempf->line_num=doc_e->y+1;
		  if (doc_e->attr<0)
		    cc->last_U16=TK_SUPERSCRIPT;
		  else if (doc_e->attr>0)
		    cc->last_U16=TK_SUBSCRIPT;
		  else
		    cc->last_U16=TK_NORMALSCRIPT;
		  goto lgc_done;
		}
		break;
	      case DOCT_CURSOR:
		doc_e=doc_e->next;
		goto lgc_start2;
	      case 0xFF: //nobound_switch
	      default:
		if (LexDollar(cc,doc,doc_e) && *(src=cc->dollar_buf)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf_ptr=cc->cur_buf_ptr=src;
		} else {
		  doc_e=doc_e->next;
		  goto lgc_start2;
		}
	    }
	  }
	  if (doc_e!=doc)
	    goto lgc_start1;
	  tempf->cur_entry=doc->root.last; //When take next, will still be end.
	}
	tempf=cc->lex_include_stk;
	if (tempf->next) {
	  tempf=LexFilePop(cc);
	  cc->cur_buf_ptr=tempf->buf_ptr;
	  cc->flags&=~CCF_USE_LAST_U16;
	  if (!(cc->last_U16=tempf->last_U16))
	    goto lgc_start1;
	} else {
	  if (cc->flags & CCF_PMT) {
	    Free(tempf->buf);
	    ptr=CmdLinePmt;
	    if (StrCmp(ptr,"\n") && !cc->pmt_line++ && !StrCmp(ptr,"?\n") &&
		cc->flags & CCF_QUESTION_HELP) {
	      Free(ptr);
	      ptr=StrNew("Help;;\n");
	    }
	    tempf->buf=tempf->buf_ptr=tempf->line_start=cc->cur_buf_ptr=ptr;
	    goto lgc_start1;
	  } else {
	    if (src)
	      cc->cur_buf_ptr=src-1;
	    cc->last_U16=TK_EOF;
	  }
	}
	break;
      case CH_CURSOR:
      case CH_FORM_FEED:
	goto lgc_start1;
      case '\n':
	tempf=cc->lex_include_stk;
	if (!(tempf->flags & LFSF_DOC)) {
	  tempf->line_num++;
	  cmp.compiled_lines++;
	  tempf->line_start=src;
	}
	break;
      case 0xFF: //nobound_switch
    }
lgc_done:
    if (cc->last_U16==CH_SHIFT_SPACE)
      cc->last_U16=CH_SPACE;
    if (cc->opts & OPTF_ECHO &&
	  cc->last_U16<256 && Bt(chars_bmp_printable,cc->last_U16))
      '' cc->last_U16;
  }
  return cc->last_U16;
}

U0 LexSkipEol(CCmpCtrl *cc)
{//$LK,"LexGetChar",A="MN:LexGetChar"$ to NULL until end-of-line.
  I64 ch;
  do ch=$WW,0$LexGetChar(cc);
  while (Bt(chars_bmp_non_eol,ch));
}

U8 *LexFirstRem(CCmpCtrl *cc,U8 *marker,I64 _len=NULL)
{//$LK,"LexGetChar",A="MN:LexGetChar"$() chars making str until marker.
  U8 *result,*ptr;
  CQueVectU8 *tempv=QueVectU8New;
  I64 i,len=0;
  while (TRUE) {
    i=LexGetChar(cc);
    if (!i||StrOcc(marker,i))
      break;
    QueVectU8Put(tempv,len++,i);
  }
  if (i)
    Bts(&cc->flags,CCf_USE_LAST_U16);
  result=ptr=MAlloc(len+1);
  for (i=0;i<len;i++)
    *ptr++=QueVectU8Get(tempv,i);
  *ptr=0;
  QueVectU8Del(tempv);
  if (_len) *_len=len;
  return result;
}

U0 LexIncludeStr(CCmpCtrl *cc,U8 *abs_filename,U8 *src,Bool actual_file)
{
  LexBackupLastChar(cc);
  CLexFile *tempf=LexFilePush(cc);
  if (actual_file)
    tempf->full_name=StrNew(abs_filename);
  else
    tempf->full_name=StrNew(blkdev.temp_filename);
  tempf->line_num=1;
  tempf->buf=tempf->buf_ptr=tempf->line_start=cc->cur_buf_ptr=src;
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

I64 cmp_type_flags_src_code[(DOCT_NUM_TYPES+63)/64]={
  1<<DOCT_TEXT|1<<DOCT_TAB|1<<DOCT_INS_BIN|1<<DOCT_INS_BIN_SIZE};

U0 LexAttachDoc(CCmpCtrl *cc,CLexFile *tempf=NULL,
	CDoc *doc=NULL,U8 *abs_filename=NULL,CDocEntry *doc_e=NULL,I64 col=0)
{//Start lexing doc. Give either doc or abs_filename.
  if (!doc)
    doc=LexDocRead(abs_filename,DOCF_DBL_DOLLARS);
  if (!tempf) {
    LexBackupLastChar(cc);
    tempf=LexFilePush(cc);
  }
  if (!doc_e)
    doc_e=doc->root.next;
  tempf->full_name=StrNew(doc->filename.name);
  tempf->doc=doc;
  while (doc_e!=doc) {
    if (Bt(cmp_type_flags_src_code,doc_e->type_u8))
      break;
    doc_e=doc_e->next;
    col=doc_e->min_col;
  }
  if (doc_e!=doc) {
    col=ClampI64(col,doc_e->min_col,doc_e->max_col);
    tempf->line_start=doc_e;
    tempf->buf=NULL;
    tempf->line_num=doc_e->y+1;
    if (doc_e->type_u8==DOCT_TEXT) {
      tempf->cur_entry=doc_e;
      tempf->buf_ptr=doc_e->tag;
    } else {
      tempf->cur_entry=doc_e->last; //TODO: might be problem at begin of file
      tempf->buf_ptr=&lex_zeros;
    }
    tempf->flags=LFSF_DOC;
  } else {//TODO: DocDel(doc)?
    col=0;
    tempf->buf=tempf->buf_ptr=tempf->line_start=CAlloc(1);
    tempf->line_num=1;
    tempf->flags=0;
  }
  cc->cur_buf_ptr=tempf->buf_ptr+col;
  tempf->last_U16=0;
}

I64 LexInStr(CCmpCtrl *cc,U8 *buf,I64 size,Bool *done)
{
  I64 i=0,j,ch;
  *done=TRUE;
  while (i<size-1) {
    ch=LexGetChar(cc);
    if (!ch || ch=='"') {
      buf[i++]=0;
      return i;
    } else if (ch=='\\') {
      switch (ch=LexGetChar(cc)) {
	case 'n':
	  buf[i++]='\n';
	  break;
	case 'r':
	  buf[i++]='\r';
	  break;
	case '"':
	  buf[i++]='"';
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
	case 't':
	  buf[i++]='\t';
	  break;
	case '0':
	  buf[i++]=0;
	  break;
	case 'x':
	case 'X':
	  j=0;
	  while (TRUE) {
	    ch=ToUpper(LexGetChar(cc));
	    if (Bt(chars_bmp_hex_numeric,ch)) {
	      if (ch<='9')
		j=j<<4+ch-'0';
	      else
		j=j<<4+ch-'A'+10;
	    } else
	      break;
	  }
	  buf[i++]=j;
	  cc->flags|=CCF_USE_LAST_U16;
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
  I64 i,j,k,ch;
  CHash *temph;
  Bool str_done,neg_e;
  U8 *fbuf,*buf2,*buf3,buf[STR_LEN];
  cc->last_line_num=cc->lex_include_stk->line_num;
  while (TRUE) {
lex_cont:
    nobound_switch (ch=LexGetChar(cc)) {
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
	  else if (Bt(cc->chars_bmp_alpha_numeric,ch))
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
	temph=NULL;
	if (cc->htc.local_var_lst)
	  cc->local_var_entry=MemberFind(buf,cc->htc.local_var_lst);
	else
	  cc->local_var_entry=NULL;
	if (!cc->local_var_entry && cc->htc.hash_table_lst)
	  temph=HashFind(buf,cc->htc.hash_table_lst,cc->htc.hash_mask);
	if (temph)
	  j=temph->type;
	else
	  j=0;
	if (j & HTT_DEFINE_STR && !(cc->flags & CCF_NO_DEFINES)) {
	  LexIncludeStr(cc,
		temph->str,StrNew(temph(CHashDefineStr *)->data),FALSE);
	  cc->lex_include_stk->flags|=LFSF_DEFINE;
	} else {
	  cc->hash_entry=temph;
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
	    if (Bt(chars_bmp_hex_numeric,ch)) {
	      if (ch<='9')
		i=i<<4+ch-'0';
	      else
		i=i<<4+ch-'A'+10;
	    } else {
	      cc->cur_i64=i;
	      cc->flags|=CCF_USE_LAST_U16;
	      cc->token=TK_INTEGER;
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
	      cc->token=TK_INTEGER;
	      goto lex_end;
	    }
	  }
	}
	while (TRUE) {
	  if (Bt(chars_bmp_dec_numeric,ch))
	    i=i*10+ch-'0';
	  else {
	    if (ch=='.' || ch=='e' || ch=='E') break;
lex_is_int:
	    cc->cur_i64=i;
	    cc->flags|=CCF_USE_LAST_U16;
	    cc->token=TK_INTEGER;
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
	  if (Bt(chars_bmp_dec_numeric,ch)) {
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
	  if (Bt(chars_bmp_dec_numeric,ch))
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
	k=0;
	for (j=0;j<8;j++) {
	  if (!(ch=LexGetChar(cc)) || ch=='\'')
	    break;
	  if (ch=='\\') {
	    switch (ch=LexGetChar(cc)) {
	      case 'n':		k.u8[j]='\n';	break;
	      case 'r':		k.u8[j]='\r';	break;
	      case '"':		k.u8[j]='"';	break;
	      case '\'':	k.u8[j]='\'';	break;
	      case '\`':	k.u8[j]='\`';	break;
	      case '\\':	k.u8[j]='\\';	break;
	      case 't':		k.u8[j]='\t';	break;
	      case '0':		k.u8[j]=0;	break;
	      case 'x':
	      case 'X':
		i=0;
		while (TRUE) {
		  ch=ToUpper(LexGetChar(cc));
		  if (Bt(chars_bmp_hex_numeric,ch)) {
		    if (ch<='9')
		      i=i<<4+ch-'0';
		    else
		      i=i<<4+ch-'A'+10;
		  } else
		    break;
		}
		k.u8[j]=i;
		cc->flags|=CCF_USE_LAST_U16;
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
	if (cc->flags&CCF_KEEP_NUM_SIGN) {
	  cc->token=ch;
	  goto lex_end;
	}
	if (Lex(cc)!=TK_IDENT)	//skip '#'
	  goto lex_end;
	if (!(temph=cc->hash_entry))
	  goto lex_end;
	if (!(temph->type & HTT_KEYWORD))
	  goto lex_end;
	switch (i=temph(CHashGeneric *)->user_data0) {
	  case KW_INCLUDE:
	    if (Lex(cc)!=TK_STR)
	      goto lex_end;
	    fbuf=DftExt(cc->cur_str,"CPP.Z");
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
	      temph=CAlloc(sizeof(CHashDefineStr));
	      temph->str=cc->cur_str;
	      cc->cur_str=0;
	      temph->type=HTT_DEFINE_STR;
	      HashSrcFileSet(cc,temph);

	      do ch=LexGetChar(cc); //skip space between define name and start
	      while (Bt(chars_bmp_non_eol_white_space,ch));

	      i=j=0;
	      buf2=NULL;
	      if (ch) {
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
		  } else if (ch!='\n')
		    buf[j++]=ch;
		  else
		    break;
		  while (ch=LexGetChar(cc)) {
		    if (ch=='/') {
		      ch=LexGetChar(cc);
		      if (ch=='/') {
			do ch=LexGetChar(cc);
			while (Bt(chars_bmp_non_eol,ch));
			break;
		      } else {
			buf[j++]='/';
			cc->flags|=CCF_USE_LAST_U16;
		      }
		    } else if (ch=='\\')
		      break;
		    else if (Bt(chars_bmp_non_eol,ch))
		      buf[j++]=ch;
		    else
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
	      temph(CHashDefineStr *)->data=buf2;
	      temph(CHashDefineStr *)->cnt=-1;
	      HashAdd(temph,cc->htc.define_hash_table);
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
	    Free(cc->cur_help_index);
	    cc->cur_help_index=LexExtStr(cc,,FALSE);
	    break;
	  case KW_HELP_FILE:
	    if (Lex(cc)!=TK_STR)
	      goto lex_end;
	    temph=CAlloc(sizeof(CHashSrcSym));
	    fbuf=DftExt(cc->cur_str,"TXT.Z");
	    temph->str=FileNameAbs(fbuf);
	    Free(fbuf);
	    temph->type=HTT_HELP_FILE|HTF_PUBLIC;
	    HashSrcFileSet(cc,temph);
	    HashAdd(temph,cc->htc.glbl_hash_table);
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
	    cc->token=TK_DOT_DOT_DOT;
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
      case TK_NUM_TK:
      default:
	break;
    }
  }
lex_end:
  LexGetChar(cc); //Do this so WAS_NEW_LINE is right
  cc->flags|=CCF_USE_LAST_U16;
  return cc->token;
}
