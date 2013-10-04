U8 *CmpPmt(Bool dbl_semicolon=FALSE)
{
  I64 i;
  U8 *result,*st;
  for (i=MinI64(3,Fs->answers_displayed-1);i>=0;i--) {
    if (Fs->answer_types[i]==IT_F64)
      "%8.6fs ansf%d=%15.7g\n",Fs->answer_times[i],i,Fs->answers[i];
    else
      "%8.6fs ans%d=0x%08X=%d\n",Fs->answer_times[i],i,Fs->answers[i],Fs->answers[i];
  }
  if (st=CurDir) {
    "%s",st;
    Free(st);
  }
  '>';
  if (IsInDbg&&IsRaw)
    RawDr;
  st=MGetS(GETSF_SHIFT_ESC_EXIT);
  if (dbl_semicolon) {
    i=StrLen(st);
    result=MAlloc(i+1+2);
    MemCpy(result,st,i+1);
    i--;
    while (i>=0 && Bt(white_space_bitmap,result[i]))
      i--;
    i++;
    if (i>0 && result[i-1]==';')
      result[i++]=';'; //The Lex goes one beyond
    result[i++]='\n';//#define goes to '\n'
    result[i]=0;
  } else
    result=StrNew(st);
  Free(st);
  return result;
}

CLex *LexNew(U8 *buf,I64 flags,U8 *filename=NULL)
{
  CLex *lx=MAlloc(sizeof(CLex));
  CLexFile *tempf=&lx->lfns[0];
  MemSet(lx,0,offset(CLex.end_of_zeroed_members)+sizeof(CLexFile));
  QueInit(lx);
  lx->flags=flags;
  lx->opts=1<<OPTf_WARN_UNUSED_VAR;
  lx->pass_trace_mask=0b1001111101;
  lx->htc.hash_mask=HTG_ALL-HTT_IMPORT_SYS_SYM-HTT_ASM_KEYWORD;
  lx->htc.define_hash_table=lx->htc.hash_table_lst=lx->htc.glbl_hash_table=lx->htc.local_hash_table=Fs->hash_table;
  if (flags&LF_KEEP_AT_SIGN)
    lx->alpha_numeric_bitmap=alpha_numeric_no_at_bitmap;
  else
    lx->alpha_numeric_bitmap=alpha_numeric_bitmap;
  lx->cur_lfn=tempf;
  QueInit(&lx->next_stream_blk);
  if (filename)
    tempf->name=FileNameAbs(filename);
  else
    tempf->name=StrNew(sys_temp_filename);
  if (flags & LF_PMT)
    buf=CAlloc(4);
  tempf->buf=tempf->buf_ptr=tempf->line_start=lx->cur_buf_ptr=buf;
  tempf->line_num=1;
  return lx;
}

U0 LexDel(CLex *lx)
{
  CLexFile *tempf=lx->cur_lfn;
  CDoc *doc;
  while (tempf!=&lx->lfns[0]) {
    if (doc=tempf->doc)
      DocDel(doc);
    Free(tempf->buf);
    Free(tempf->name);
    tempf--;
  }
  LinkedLstDel(lx->htc.next);
  Free(tempf->name);
  if (!(lx->flags & LF_DONT_FREE_BUF))
    Free(tempf->buf);
  Free(lx->ident);
  Free(lx->cur_index);
  Free(lx->dollar);
  Free(lx);
}

I64 LexSize(CLex *lx)
{
  CLexFile *tempf=lx->cur_lfn;
  CDoc *doc;
  I64 result=0;
  while (tempf!=&lx->lfns[0]) {
    if (doc=tempf->doc)
      result+=DocSize(doc);
    result+=MSize2(tempf->buf);
    result+=MSize2(tempf->name);
    tempf--;
  }
  result+=MSize2(tempf->name);
  if (!(lx->flags & LF_DONT_FREE_BUF))
    result+=MSize2(tempf->buf);
  result+=MSize2(lx->ident);
  result+=MSize2(lx);
  return result;
}

U0 SetSrcFileInfo(CLex *lx,CHashSrcSym *h)
{
  CLexFile *tempf=lx->cur_lfn;
  h->src_link=MPrint("FL:%s,%d",tempf->name,tempf->line_num);
  if (Bt(&lx->opts,OPTf_KEEP_PRIVATE))
    h->type|=HTF_PRIVATE;
  if (lx->cur_index)
    h->index=StrNew(lx->cur_index);
  else
    h->index=NULL;
}

U0 LexPutToken(CLex *lx)
{
  '"';
  if (lx->token==TK_IDENT || lx->token==TK_STR)
    "%s",lx->ident;
  else if (lx->token==TK_INTEGER)
    "INT:%X",lx->cur_i;
  else if (lx->token==TK_CHAR_CONST)
    "U8:%X",lx->cur_i;
  else if (lx->token==TK_F64)
    "FLOAT:%e",lx->cur_f;
  else if (lx->token=='\n')
    "<NEW_LINE>";
  else if (Bt(displayable_chars_bitmap,lx->token))
    '' lx->token;
  else {
    "T:%X",lx->token;
  }
  "\" ";
}

U32 lex_zeros=0;

U0 LexBackupLastChar(CLex *lx)
{
  CLexFile *tempf=lx->cur_lfn;
  tempf->buf_ptr=lx->cur_buf_ptr;
  if (lx->flags & LF_USE_LAST_U16) {
    tempf->last_U16=lx->last_U16;
    lx->flags&=~LF_USE_LAST_U16;
  } else
    tempf->last_U16=0;
}

Bool LexDollar(CLex *lx,CDoc *doc,CDocEntry *doc_e)
{
  U8 *st;
  if (lx->flags&LF_IN_QUOTES) {
    Free(lx->dollar);
    st=Doc2PlainText(doc,doc_e);
    lx->dollar=MPrint("$$%Q$$",st);
    lx->dollar_cnt=2;
    Free(st);
    return TRUE;
  } else
    return FALSE;
}

I64 LexGetChar(CLex *lx)
{
  U8 *ptr,*src;
  CLexFile *tempf,*tempf1;
  Bool cont;
  CDoc *doc;
  CDocEntry *doc_e;
  if (lx->flags & LF_USE_LAST_U16) {
    lx->flags&=~LF_USE_LAST_U16;
    goto lgc_done;
  }
  while (TRUE) {
lex_getchar_start:
    if (!(src=lx->cur_buf_ptr++)) {
      lx->cur_buf_ptr--;
      goto lgc_here2;
    }
    nobound_switch (lx->last_U16=*src++) {
      case 0:
lgc_here2:
	tempf=lx->cur_lfn;
	if (tempf->flags & LFSF_IS_DOC) {
	  doc=tempf->doc;
	  doc_e=tempf->cur_entry;
	  doc_e=doc_e->next;
	  while (doc_e!=doc) {
	    cont=FALSE;
	    tempf->cur_entry=doc_e;
	    nobound_switch (doc_e->type_u8) {
	      case DOCT_TEXT:
		if (doc_e->flags1 & ~(DOCEF1_DEFINE|DOCEF1_TAG_CB|DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP) &&
		    LexDollar(lx,doc,doc_e) && *(src=lx->dollar)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=src;
		} else if (*(src=doc_e->tag))
		  tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=src;
		else {
		  cont=TRUE;
		  doc_e=doc_e->next;
		}
		break;
	      case DOCT_NEW_LINE:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		tempf->line_start=doc_e->next;
		tempf->line_num=doc_e->y+2;	//+1 because NEW_LINE is on previous line, +1 because doc y starts at zero
		compiled_lines++;
		if (tempf==lx->fun_lfn && tempf->line_num>lx->max_line)
		  lx->max_line=tempf->line_num;
		if (lx->opts & OPTF_ECHO)
		  NewLine;
		lx->last_U16='\n';
		goto lgc_done;
	      case DOCT_TAB:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+1;
		if (lx->opts & OPTF_ECHO)
		  '\t';
		lx->last_U16='\t';
		goto lgc_done;
	      case DOCT_PAGE_BREAK:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+2;	//TODO: should be a page down
		if (lx->opts & OPTF_ECHO)
		  '' CH_FORM_FEED;
		lx->last_U16=CH_FORM_FEED;
		goto lgc_done;
	      case DOCT_INS_BINARY:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		tempf->line_num=doc_e->y+1;
		Free(lx->ident);
		lx->ident=NULL;
		lx->ident_len=0;
		if (doc_e->bin_data) {
		  ptr=MAlloc(doc_e->bin_data->size);
		  if (doc_e->bin_data->data)
		    MemCpy(ptr,doc_e->bin_data->data,doc_e->bin_data->size);
		  lx->ident=ptr;
		  lx->ident_len=doc_e->bin_data->size;
		}
		lx->last_U16=TK_INS_BINARY;
		goto lgc_done;
	      case DOCT_INS_BINARY_TYPE:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		if (doc_e->bin_data)
		  lx->cur_i=doc_e->bin_data->type;
		else
		  lx->cur_i=0;
		tempf->line_num=doc_e->y+1;
		lx->last_U16=TK_INS_BINARY_TYPE;
		goto lgc_done;
	      case DOCT_INS_BINARY_SIZE:
		tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		if (doc_e->bin_data)
		  lx->cur_i=doc_e->bin_data->size;
		else
		  lx->cur_i=0;
		tempf->line_num=doc_e->y+1;
		lx->last_U16=TK_INS_BINARY_SIZE;
		goto lgc_done;
	      case DOCT_SHIFTED_Y:
		if (LexDollar(lx,doc,doc_e) && *(src=lx->dollar)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=src;
		} else {
		  tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=&lex_zeros;
		  tempf->line_num=doc_e->y+1;
		  if (doc_e->attr<0) {
		    lx->last_U16=TK_SUPERSCRIPT;
		    goto lgc_done;
		  } else if (doc_e->attr>0) {
		    lx->last_U16=TK_SUBSCRIPT;
		    goto lgc_done;
		  } else {
		    lx->last_U16=TK_NORMALSCRIPT;
		    goto lgc_done;
		  }
		}
		break;
	      case DOCT_CURSOR:
		cont=TRUE;
		doc_e=doc_e->next;
		break;
	      case 0xFF: //nobound_switch
	      default:
		if (LexDollar(lx,doc,doc_e) && *(src=lx->dollar)) {
		  tempf->line_num=doc_e->y+1;
		  tempf->buf=tempf->buf_ptr=lx->cur_buf_ptr=src;
		} else {
		  cont=TRUE;
		  doc_e=doc_e->next;
		}
	    }
	    if (!cont) break;
	  }
	  if (doc_e==doc) {
	    if (tempf!=&lx->lfns[0] || !(lx->flags&LF_DONT_FREE_BUF))
	      DocDel(doc);
	    tempf->doc=tempf->buf=NULL;
	    Free(tempf->name);
	    tempf->name=NULL;
	    goto lgc_here;
	  }
	} else {
lgc_here:
	  tempf=lx->cur_lfn;
	  if (tempf!=&lx->lfns[0]) {
	    Free(tempf->name);
	    tempf->name=NULL;
	    Free(tempf->buf);
	    tempf->buf=NULL;
	    tempf1=tempf;
	    lx->cur_lfn=--tempf;
	    lx->cur_buf_ptr=tempf->buf_ptr;
	    if (lx->last_U16=tempf->last_U16) {
	      lx->flags&=~LF_USE_LAST_U16;
	      goto lgc_done;
	    } else
	      lx->flags&=~LF_USE_LAST_U16;
	  } else {
	    if (lx->flags & LF_PMT) {
	      Free(tempf->buf);
	      lx->cur_buf_ptr=tempf->line_start=
		  tempf->buf_ptr=tempf->buf=CmpPmt(TRUE);
	    } else {
	      if (src)
		lx->cur_buf_ptr=src-1;
	      lx->last_U16=TK_EOF;
	      goto lgc_done;
	    }
	  }
	}
	break;
      case CH_CURSOR:
      case CH_FORM_FEED:
	goto lex_getchar_start;
      case '\n':
	tempf=lx->cur_lfn;
	if (tempf->flags & LFSF_IS_DOC)
	  goto lgc_done;
	else {
	  tempf->line_num++;
	  compiled_lines++;
	  tempf->line_start=src;
	  if (tempf==lx->fun_lfn && tempf->line_num>lx->max_line)
	    lx->max_line=tempf->line_num;
	}
      case 0xFF: //nobound_switch
      default:
	goto lgc_done;
    }
  }
lgc_done:
  if (lx->last_U16==CH_SHIFT_SPACE)
    lx->last_U16=CH_SPACE;
  if (lx->opts & OPTF_ECHO &&
    lx->last_U16<256 && Bt(printable_chars_bitmap,lx->last_U16))
    '' lx->last_U16;
  return lx->last_U16;
}

U0 LexSkipEol(CLex *lx)
{
  U8 ch;
  do ch=$WW,0$LexGetChar(lx);
  while (Bt(non_eol_chars_bitmap,ch));
}

U0 LexIncludeStr(CLex *lx,U8 *abs_filename,U8 *src,Bool actual_file)
{
  CLexFile *tempf;
  LexBackupLastChar(lx);
  tempf=++lx->cur_lfn;
  tempf->buf=src;
  tempf->buf_ptr=src;
  tempf->line_num=1;
  tempf->line_start=src;
  tempf->last_U16=0;
  if (actual_file) {
    tempf->name=StrNew(abs_filename);
    tempf->flags=LFSF_IS_INCLUDED;
  } else {
    tempf->name=StrNew(sys_temp_filename);
    tempf->flags=0;
  }
  lx->cur_buf_ptr=src;
}

I64 doc_stop_btypes[(DOCT_NUM_TYPES+63)/64]=
{
1<<DOCT_TEXT|
1<<DOCT_TAB|
1<<DOCT_INS_BINARY|
1<<DOCT_INS_BINARY_TYPE|
1<<DOCT_INS_BINARY_SIZE
};


CDoc *LexDocRead(U8 *abs_filename,I64 flags)
{
  CDoc *doc=DocNew;
  U8 *src;
  I64 size=0;
  doc->flags|=flags;
  StrCpy(doc->filename.name,abs_filename);
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

U0 LexAttachDoc(CLex *lx,CLexFile *tempf=NULL,
  CDoc *doc=NULL,U8 *abs_filename=NULL,CDocEntry *doc_e=NULL,I64 col=0)
{//Give either doc or abs_filename.
  if (!doc)
    doc=LexDocRead(abs_filename,DOCF_DBL_DOLLARS);
  if (!tempf) {
    LexBackupLastChar(lx);
    tempf=++lx->cur_lfn;
  }
  if (!doc_e)
    doc_e=doc->root.next;
  tempf->name=StrNew(doc->filename.name);
  tempf->line_start=&lex_zeros;
  tempf->doc=doc;
  while (doc_e!=doc) {
    if (Bt(doc_stop_btypes,doc_e->type_u8))
      break;
    doc_e=doc_e->next;
    col=doc_e->min_col;
  }
  if (doc_e!=doc) {
    col=LimitI64(col,doc_e->min_col,doc_e->max_col);
    tempf->line_start=doc_e;
    tempf->line_num=doc_e->y+1;
    if (doc_e->type_u8==DOCT_TEXT) {
      tempf->cur_entry=doc_e;
      tempf->buf=doc_e->tag;
      tempf->buf_ptr=doc_e->tag;
    } else {
      tempf->cur_entry=doc_e->last;  //TODO might be problem at begining of file
      tempf->buf=&lex_zeros;
      tempf->buf_ptr=&lex_zeros;
    }
    tempf->flags=LFSF_IS_INCLUDED|LFSF_IS_DOC;
  } else { //TODO: DocDel(doc)?
    col=0;
    tempf->flags=LFSF_IS_INCLUDED;
    tempf->buf=CAlloc(1);
    tempf->buf_ptr=&lex_zeros;
    tempf->line_num=1;
  }
  lx->cur_buf_ptr=tempf->buf_ptr+col;
  tempf->last_U16=0;
}

U0 LexPush(CLex *lx)
{
  CLexFile *tempf=lx->cur_lfn,
    *tempf1=&lx->stk[lx->stk_ptr++];
  if (lx->stk_ptr>=LFN_STK_CNT)
    LexExcept(lx,"ST_LEX_STK_OVERFLOW");
  LexBackupLastChar(lx);
  if (tempf->last_U16)
    lx->flags|=LF_USE_LAST_U16;
  MemCpy(tempf1,tempf,sizeof(CLexFile));
}

U0 LexPop(CLex *lx)
{
  CLexFile *tempf=lx->cur_lfn,
    *tempf1=&lx->stk[--lx->stk_ptr];
  lx->cur_buf_ptr=tempf1->buf_ptr;
  if (lx->last_U16=tempf1->last_U16)
    lx->flags|=LF_USE_LAST_U16;
  else
    lx->flags&=~LF_USE_LAST_U16;
  MemCpy(tempf,tempf1,sizeof(CLexFile));
}

I64 MemberMetaData(U8 *st,CMemberLst *tempm)
{
  CMemberLstMeta *meta=tempm->meta;
  while (meta) {
    if (!StrCmp(meta->str,st))
      return meta->user_data;
    meta=meta->next;
  }
  return 0;
}

CMemberLstMeta *MemberMetaFind(U8 *st,CMemberLst *tempm)
{
  CMemberLstMeta *meta=tempm->meta;
  while (meta) {
    if (!StrCmp(meta->str,st))
      return meta;
    meta=meta->next;
  }
  return NULL;
}

CMemberLst *MemberFind(U8 *st,CHashClass *tempc)
{
  I64 i;
  CMemberLst *tempm;
  do {
    tempm=tempc->member_lst;
    while (tempm) {
      if (!(i=StrCmp(tempm->str,st))) {
	tempm->use_cnt++;
	return tempm;
      }
      if (i<=0)
	tempm=tempm->left;
      else
	tempm=tempm->right;
    }
  } while (tempc=tempc->base_class);
  return NULL;
}

CMemberLst *MemberFind2(CHashClass *pattern,CHashClass *tempc)
{//just offset<0
  CMemberLst *tempm;
  tempm=tempc->member_lst2;
  while (tempm) {
    if (pattern==tempm->member_class2)
      return tempm;
    if (pattern<tempm->member_class2)
      tempm=tempm->left2;
    else
      tempm=tempm->right2;
  }
  return NULL;
}

U0 MemberAdd(CLex *lx,CMemberLst *tempm,CHashClass *tempc,I64 mode)
{
  U8 *st=tempm->str;
  CMemberLst **tempm1,*tempm2;
  CHashClass *tempc2;
  if (MemberFind(st,tempc) && StrCmp(st,"pad") && StrCmp(st,"reserved") && StrCmp(st,"_anon_"))
    LexExcept(lx,"ST_DUP_MEMBER");
  if (mode==PRS1B_LOCAL_VAR) {
    tempc2=tempm->member_class-tempm->member_class->ptr_cnt;
    if (Bt(&lx->opts,OPTf_WARN_DUP_TYPES) && MemberFind2(tempc2,tempc))
      LexPutWarn(lx,"ST_DUP_TYPE");
  } else
    tempc2=NULL;
  tempm->member_class2=tempc2;

  tempm1=&tempc->member_lst;
  while (tempm2=*tempm1) {
    if (StrCmp(tempm2->str,st)<=0)
      tempm1=&tempm2->left;
    else
      tempm1=&tempm2->right;
  }
  *tempm1=tempm;

  if (tempc2) {
    tempm1=&tempc->member_lst2;
    while (tempm2=*tempm1) {
      if (tempc2<tempm2->member_class2)
	tempm1=&tempm2->left2;
      else if (tempc2>tempm2->member_class2)
	tempm1=&tempm2->right2;
      else {
	tempm1=NULL;
	break;
      }
    }
    if (tempm1)
      *tempm1=tempm;
  }

  tempm->left=NULL;
  tempm->right=NULL;
  tempm->left2=NULL;
  tempm->right2=NULL;
  tempm2=tempc->last_in_member_lst;
  tempm2->next=tempc->last_in_member_lst=tempm;
}

U0 MemberLstDel(CHashClass *tempc)
{
  CMemberLst *tempm,*tempm1;
  CMemberLstMeta *temp_meta,*temp_meta1;
  tempm=tempc->member_lst;
  while (tempm) {
    tempm1=tempm->next;
    Free(tempm->str);
    LinkedLstDel(tempm->dim.next);
    if (tempm->flags & MLF_STR_DFT_AVAILABLE)
      Free(tempm->dft_val);
    if (tempm->flags & MLF_FUN)
      HashDel(tempm->fun_ptr-tempm->fun_ptr->ptr_cnt);
    temp_meta=tempm->meta;
    while (temp_meta) {
      temp_meta1=temp_meta->next;
      Free(temp_meta->str);
      if (temp_meta->flags&MLMF_IS_STR)
	Free(temp_meta->user_data);
      Free(temp_meta);
      temp_meta=temp_meta1;
    }
    Free(tempm);
    tempm=tempm1;
  }
  tempc->size=0;
  tempc->last_in_member_lst=&tempc->member_lst;
  tempc->member_lst=NULL;
  tempc->member_lst2=NULL;
  tempc->member_cnt=0;
  if (tempc->type&HTT_FUN)
    tempc(CHashFun *)->arg_cnt=0;
}

#assert !offset(CMemberLst.next)
#assert !offset(CMemberLstMeta.next)

CMemberLst *MemberLstCopy(CHashClass *tempc)
{
  CMemberLst *result=NULL,*tempm,*tempm1=&result;
  CMemberLstMeta *temp_meta,*temp_meta1;
  tempm=tempc->member_lst;
  while (tempm) {
    tempm1=tempm1->next=MAllocIdentical(tempm);
    tempm1->str=MAllocIdentical(tempm->str);
    tempm1->dim.next=LinkedLstCopy(tempm->dim.next);
    if (tempm->flags & MLF_STR_DFT_AVAILABLE)
      tempm1->dft_val=MAllocIdentical(tempm->dft_val);
    if (tempm->flags & MLF_FUN)
      tempm1->fun_ptr=HashEntryImportCopy(tempm->fun_ptr-tempm->fun_ptr->ptr_cnt)(CHashFun *)+tempm->fun_ptr->ptr_cnt;

    temp_meta=tempm->meta;
    temp_meta1=&tempm1->meta;
    while (temp_meta) {
      temp_meta1=temp_meta1->next=MAllocIdentical(temp_meta);
      temp_meta1->str=MAllocIdentical(temp_meta->str);
      if (temp_meta->flags&MLMF_IS_STR)
	temp_meta1->user_data=MAllocIdentical(temp_meta->user_data);
      temp_meta=temp_meta->next;
    }
    tempm=tempm->next;
  }
  return result;
}

I64 MemberLstSize(CHashClass *tempc)
{
  CMemberLst *tempm;
  CMemberLstMeta *temp_meta;
  I64 result=0;
  tempm=tempc->member_lst;
  while (tempm) {
    result+=MSize2(tempm->str);
    result+=LinkedLstSize(tempm->dim.next);
    if (tempm->flags & MLF_STR_DFT_AVAILABLE)
      result+=MSize2(tempm->dft_val);
    if (tempm->flags & MLF_FUN)
      result+=HashEntrySize2(tempm->fun_ptr-tempm->fun_ptr->ptr_cnt);
    temp_meta=tempm->meta;
    while (temp_meta) {
      result+=MSize2(temp_meta->str);
      if (temp_meta->flags&MLMF_IS_STR)
	result+=MSize2(temp_meta->user_data);
      result+=MSize2(temp_meta);
      temp_meta=temp_meta->next;
    }
    result+=MSize2(tempm);
    tempm=tempm->next;
  }
  return result;
}

I64 LexInStr(CLex *lx,
  U8 *buf,I64 size,Bool *done)
{
  I64 i=0,j,ch;
  *done=TRUE;
  while (i<size-1) {
    ch=LexGetChar(lx);
    if (!ch || ch=='"') {
      buf[i++]=0;
      return i;
    } else if (ch=='\\') {
      switch (ch=LexGetChar(lx)) {
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
	case 'x':
	case 'X':
	  j=0;
	  while (TRUE) {
	    ch=ToUpper(LexGetChar(lx));
	    if (Bt(hex_numeric_bitmap,ch)) {
	      if (ch<='9')
		j=j<<4+ch-'0';
	      else
		j=j<<4+ch-'A'+10;
	    } else
	      break;
	  }
	  buf[i++]=j;
	  lx->flags|=LF_USE_LAST_U16;
	  break;
	case 'b':
	case 'B':
	  j=0;
	  while (TRUE) {
	    ch=LexGetChar(lx);
	    if ('0'<=ch<='1')
	      j=j<<1+ch-'0';
	    else
	      break;
	  }
	  buf[i++]=j;
	  lx->flags|=LF_USE_LAST_U16;
	  break;
	case 'o':
	case 'O':
	  j=0;
	  goto lis_octal;
	case '0'...'7':
	  j=ch-'0';
lis_octal:
	  while (TRUE) {
	    ch=LexGetChar(lx);
	    if ('0'<=ch<='7')
	      j=j<<3+ch-'0';
	    else
	      break;
	  }
	  buf[i++]=j;
	  lx->flags|=LF_USE_LAST_U16;
	  break;
	default:
	  lx->flags|=LF_USE_LAST_U16;
	  buf[i++]='\\';
      }
    } else if (ch=='$$') {
      buf[i++]='$$';
      if (lx->dollar_cnt)
	lx->dollar_cnt--;
      else if (LexGetChar(lx)!='$$') {
	lx->dollar_cnt=1;
	lx->flags|=LF_USE_LAST_U16;
      }
    } else
      buf[i++]=ch;
  }
  *done=FALSE;
  return i;
}

I64 Lex(CLex *lx)
{
  I64 i,j,k,ch;
  CHash *temph;
  Bool str_done,neg_e;
  U8 *fbuf,*buf2,*buf3,buf[LEX_MAX_IDENT];
  lx->last_line_num=lx->cur_lfn->line_num;
  while (TRUE) {
lex_cont:
    nobound_switch (ch=LexGetChar(lx)) {
      case 0:
	return lx->token=TK_EOF;
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
	if (lx->flags&LF_KEEP_AT_SIGN) {
	  lx->token=ch;
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
	  if (i>=LEX_MAX_IDENT-1) //1 for terminator
	    break;
	  else if (!(ch=LexGetChar(lx)))
	    break;
	  else if (Bt(lx->alpha_numeric_bitmap,ch))
	    buf[i++]=ch;
	  else if (ch==TK_SUPERSCRIPT)
	    buf[i++]='>';
	  else if (ch==TK_SUBSCRIPT)
	    buf[i++]='<';
	  else if (ch==TK_NORMALSCRIPT)
	    buf[i++]='=';
	  else {
	    lx->flags|=LF_USE_LAST_U16;
	    break;
	  }
	}
	buf[i++]=0;
	temph=NULL;
	if (lx->htc.local_var_lst)
	  lx->local_var_entry=MemberFind(buf,lx->htc.local_var_lst);
	else
	  lx->local_var_entry=NULL;
	if (!lx->local_var_entry && lx->htc.hash_table_lst)
	  temph=HashFind(buf,
	      lx->htc.hash_table_lst,lx->htc.hash_mask);
	if (temph)
	  j=temph->type;
	else
	  j=0;
	if (j & HTT_DEFINE_STR && !(lx->flags & LF_NO_DEFINES)) {
	  LexIncludeStr(lx,
	    temph->str,StrNew(temph(CHashDefineStr *)->data),FALSE);
	  lx->cur_lfn->flags|=LFSF_IS_DEFINE;
	} else {
	  lx->hash_entry=temph;
	  Free(lx->ident);
	  lx->ident=StrNew(buf);
	  lx->ident_len=i;
	  lx->token=TK_IDENT;
	  goto lex_end;
	}
	break;
      case '0'...'9':
	i=ch-'0';
	ch=ToUpper(LexGetChar(lx));
	if (ch=='X') {
	  while (TRUE) {
	    ch=ToUpper(LexGetChar(lx));
	    if (Bt(hex_numeric_bitmap,ch)) {
	      if (ch<='9')
		i=i<<4+ch-'0';
	      else
		i=i<<4+ch-'A'+10;
	    } else {
	      lx->cur_i=i;
	      lx->flags|=LF_USE_LAST_U16;
	      lx->token=TK_INTEGER;
	      goto lex_end;
	    }
	  }
	} else if (ch=='B') {
	  while (TRUE) {
	    ch=LexGetChar(lx);
	    if (ch=='0')
	      i=i<<1;
	    else if (ch=='1')
	      i=i<<1+1;
	    else {
	      lx->cur_i=i;
	      lx->flags|=LF_USE_LAST_U16;
	      lx->token=TK_INTEGER;
	      goto lex_end;
	    }
	  }
	} else if (ch=='O') {
	  while (TRUE) {
	    ch=LexGetChar(lx);
	    if ('0'<=ch<='7')
	      i=i<<3+ch-'0';
	    else {
	      lx->cur_i=i;
	      lx->flags|=LF_USE_LAST_U16;
	      lx->token=TK_INTEGER;
	      goto lex_end;
	    }
	  }
	}
	while (TRUE) {
	  if (Bt(dec_numeric_bitmap,ch))
	    i=i*10+ch-'0';
	  else {
	    if (ch=='.' || ch=='e' || ch=='E') break;
lex_is_int:
	    lx->cur_i=i;
	    lx->flags|=LF_USE_LAST_U16;
	    lx->token=TK_INTEGER;
	    goto lex_end;
	  }
	  ch=LexGetChar(lx);
	}
	if (ch=='.') {
	  ch=LexGetChar(lx);
	  if (ch=='.') {
	    lx->flags|=LF_LAST_WAS_DOT;
	    goto lex_is_int;
	  }
	}
lex_float_start:
	k=0;
	while (TRUE) {
	  if (Bt(dec_numeric_bitmap,ch)) {
	    i=i*10+ch-'0';
	    k++;
	  } else {
	    if (ch=='e' || ch=='E')
	      break;
	    lx->cur_f=i*Pow10I64(-k);
	    lx->flags|=LF_USE_LAST_U16;
	    lx->token=TK_F64;
	    goto lex_end;
	  }
	  ch=LexGetChar(lx);
	}
	ch=LexGetChar(lx);
	neg_e=FALSE;
	if (ch=='-') {
	  neg_e=TRUE;
	  ch=LexGetChar(lx);
	}
	j=0;
	while (TRUE) {
	  if (Bt(dec_numeric_bitmap,ch))
	    j=j*10+ch-'0';
	  else {
	    if (neg_e)
	      lx->cur_f=i*Pow10I64(-j-k);
	    else
	      lx->cur_f=i*Pow10I64(j-k);
	    lx->flags|=LF_USE_LAST_U16;
	    lx->token=TK_F64;
	    goto lex_end;
	  }
	  ch=LexGetChar(lx);
	}
	break;
      case '"':
	lx->flags|=LF_IN_QUOTES;
	buf2=NULL;
	i=0;
	do {
	  j=LexInStr(lx,buf,LEX_MAX_IDENT,&str_done);
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
	Free(lx->ident);
	lx->ident=MAlloc(i);
	MemCpy(lx->ident,buf2,i);
	Free(buf2);
	lx->ident_len=i;
	lx->flags&=~LF_IN_QUOTES;
	lx->token=TK_STR;
	goto lex_end;
      case '\'':
	if (!(ch=LexGetChar(lx)))
	  k='\'';
	else {
	  k=0;
	  if (ch!='\'')
	    for (j=0;j<8;j++) {
	      if (ch=='\\') {
		switch (ch=LexGetChar(lx)) {
		  case 'n':
		    k.u8[j]='\n';
		    break;
		  case 'r':
		    k.u8[j]='\r';
		    break;
		  case '"':
		    k.u8[j]='"';
		    break;
		  case '\'':
		    k.u8[j]='\'';
		    break;
		  case '\`':
		    k.u8[j]='\`';
		    break;
		  case '\\':
		    k.u8[j]='\\';
		    break;
		  case 't':
		    k.u8[j]='\t';
		    break;
		  case 'x':
		  case 'X':
		    i=0;
		    while (TRUE) {
		      ch=ToUpper(LexGetChar(lx));
		      if (Bt(hex_numeric_bitmap,ch)) {
			if (ch<='9')
			  i=i<<4+ch-'0';
			else
			  i=i<<4+ch-'A'+10;
		      } else
			break;
		    }
		    k.u8[j]=i;
		    lx->flags|=LF_USE_LAST_U16;
		    break;
		  case 'b':
		  case 'B':
		    i=0;
		    while (TRUE) {
		      ch=LexGetChar(lx);
		      if ('0'<=ch<='1')
			i=i<<1+ch-'0';
		      else
			break;
		    }
		    k.u8[j]=i;
		    lx->flags|=LF_USE_LAST_U16;
		    break;
		  case 'o':
		  case 'O':
		    i=0;
		    goto lex_octal;
		  case '0'...'7':
		    i=ch-'0';
lex_octal:
		    while (TRUE) {
		      ch=LexGetChar(lx);
		      if ('0'<=ch<='7')
			i=i<<3+ch-'0';
		      else
			break;
		    }
		    k.u8[j]=i;
		    lx->flags|=LF_USE_LAST_U16;
		    break;
		  default:
		    lx->flags|=LF_USE_LAST_U16;
		    k.u8[j]='\\';
		}
	      } else if (ch=='$$') {
		ch=LexGetChar(lx);
		k.u8[j]='$$';
		if (ch!='$$')
		  lx->flags|=LF_USE_LAST_U16;
	      } else
		k.u8[j]=ch;
	      if (!(ch=LexGetChar(lx)) || ch=='\'')
		break;
	    }
	}
	lx->cur_i=k;
	lx->token=TK_CHAR_CONST;
	goto lex_end;
      case '#':
	if (lx->flags&LF_KEEP_NUM_SIGN) {
	  lx->token=ch;
	  goto lex_end;
	}
	if (Lex(lx)!=TK_IDENT)	//skip '#'
	  goto lex_end;
	if (!(temph=lx->hash_entry))
	  goto lex_end;
	if (!(temph->type & HTT_KEYWORD))
	  goto lex_end;
	switch (i=temph(CHashGeneric *)->user_data0) {
	  case KW_INCLUDE:
	    if (Lex(lx)!=TK_STR)
	      goto lex_end;
	    fbuf=DftExt(lx->ident,"CPP.Z");
	    buf2=FileNameAbs(fbuf);
	    Free(fbuf);
	    if (Bt(&sys_run_level,RLf_DOC))
	      LexAttachDoc(lx,,,buf2);
	    else
	      LexIncludeStr(lx,buf2,TextFileRead(buf2),TRUE);
	    Free(buf2);
	    break;
	  case KW_DEFINE:
	    lx->flags|=LF_NO_DEFINES;
	    if (Lex(lx)==TK_IDENT) {
	      temph=CAlloc(sizeof(CHashDefineStr));
	      temph->str=lx->ident;
	      lx->ident=0;
	      temph->type=HTT_DEFINE_STR;
	      SetSrcFileInfo(lx,temph);

	      do ch=LexGetChar(lx); //skip space between define name and start
	      while (Bt(non_eol_white_space_bitmap,ch));

	      i=j=0;
	      buf2=NULL;
	      if (ch) {
		do {
		  if (ch=='\\') {
		    if (ch=LexGetChar(lx)) {
		      if (ch!='\r' && ch!='\n') {
			buf[j++]='\\';
			buf[j++]=ch;
		      } else if (ch=='\r' && LexGetChar(lx)!='\n')
			lx->flags|=LF_USE_LAST_U16;
		    } else {
		      buf[j++]='\\';
		      break;
		    }
		  } else if (ch!='\n')
		    buf[j++]=ch;
		  else
		    break;
		  while (ch=LexGetChar(lx)) {
		    if (ch=='/') {
		      ch=LexGetChar(lx);
		      if (ch=='/') {
			do ch=LexGetChar(lx);
			while (Bt(non_eol_chars_bitmap,ch));
			break;
		      } else {
			buf[j++]='/';
			lx->flags|=LF_USE_LAST_U16;
		      }
		    } else if (ch=='\\')
		      break;
		    else if (Bt(non_eol_chars_bitmap,ch))
		      buf[j++]=ch;
		    else
		      break;
		    if (j>=LEX_MAX_IDENT-4) { //spare spot for '\\' next loop
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
	      if (IsReplaceSyms)
		HashReplace(temph,lx->htc.define_hash_table);
	      else
		HashAdd(temph,lx->htc.define_hash_table);
	    }
	    lx->flags&=~LF_NO_DEFINES;
	    break;
	  case KW_ELSE:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_ELSE;
	      goto lex_end;
	    }
lex_else:
	    j=1;
	    do {
	      if (ch=LexGetChar(lx)) {
		if (ch=='#') {
		  if (!Lex(lx))
		    goto lex_end;
		  i=PrsKeyWord(lx);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		}
	      } else {
		lx->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IF:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_IF;
	      goto lex_end;
	    }
lex_if:
	    lx->flags|=LF_IN_IF;
	    if (!Lex(lx)) {
	      lx->flags&=~LF_IN_IF;
	      goto lex_end;
	    }
	    if (LexExpression(lx)) {
	      lx->flags&=~LF_IN_IF;
	      switch (lx->token) {
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
	      lx->flags&=~LF_IN_IF;
	      if (lx->token!=TK_ENDIF && lx->token!=TK_ELSE) {
		if (lx->token==TK_IF || lx->token==TK_IFDEF || lx->token==TK_IFNDEF|| lx->token==TK_IFAOT || lx->token==TK_IFJIT)
		  j=2;
		else
		  j=1;
		do {
		  if (ch=LexGetChar(lx)) {
		    if (ch=='#') {
		      if (!Lex(lx))
			goto lex_end;
		      i=PrsKeyWord(lx);
		      if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
			j++;
		      else if (i==KW_ENDIF)
			j--;
		      else if (i==KW_ELSE && j==1)
			break;
		    }
		  } else {
		    lx->token=TK_EOF;
		    goto lex_end;
		  }
		} while (j);
	      }
	    }
	    break;
	  case KW_IFDEF:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_IFDEF;
	      goto lex_end;
	    }
lex_ifdef:
	    lx->flags|=LF_NO_DEFINES;
	    if (!Lex(lx)) {
	      lx->flags&=~LF_NO_DEFINES;
	      goto lex_end;
	    }
	    lx->flags&=~LF_NO_DEFINES;
	    if (lx->token!=TK_IDENT)
	      goto lex_end;
	    if (lx->hash_entry)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(lx)) {
		if (ch=='#') {
		  if (!Lex(lx))
		    goto lex_end;
		  i=PrsKeyWord(lx);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		lx->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFNDEF:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_IFNDEF;
	      goto lex_end;
	    }
lex_ifndef:
	    lx->flags|=LF_NO_DEFINES;
	    if (!Lex(lx)) {
	      lx->flags&=~LF_NO_DEFINES;
	      goto lex_end;
	    }
	    lx->flags&=~LF_NO_DEFINES;
	    if (lx->token!=TK_IDENT)
	      goto lex_end;
	    if (!lx->hash_entry)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(lx)) {
		if (ch=='#') {
		  if (!Lex(lx))
		    goto lex_end;
		 i=PrsKeyWord(lx);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		lx->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFAOT:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_IFAOT;
	      goto lex_end;
	    }
lex_ifaot:
	    if (lx->flags & LF_AOT_COMPILE)
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(lx)) {
		if (ch=='#') {
		  if (!Lex(lx))
		    goto lex_end;
		  i=PrsKeyWord(lx);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		lx->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_IFJIT:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_IFAOT;
	      goto lex_end;
	    }
lex_ifjit:
	    if (!(lx->flags & LF_AOT_COMPILE))
	      goto lex_cont;
	    j=1;
	    do {
	      if (ch=LexGetChar(lx)) {
		if (ch=='#') {
		  if (!Lex(lx))
		    goto lex_end;
		  i=PrsKeyWord(lx);
		  if (i==KW_IF || i==KW_IFDEF || i==KW_IFNDEF || i==KW_IFAOT || i==KW_IFJIT)
		    j++;
		  else if (i==KW_ENDIF)
		    j--;
		  else if (i==KW_ELSE && j==1)
		    break;
		}
	      } else {
		lx->token=TK_EOF;
		goto lex_end;
	      }
	    } while (j);
	    break;
	  case KW_ENDIF:
	    if (lx->flags & LF_IN_IF) {
	      lx->token=TK_ENDIF;
	      goto lex_end;
	    }
	    break;
	  case KW_ASSERT:
	    if (!Lex(lx))
	      goto lex_end;
	    if (!LexExpression(lx))
	      LexPutWarn(lx,"ST_ASSERT_FAILED");
	    goto lex_end;
	  case KW_EXE:
	    if (!Lex(lx))
	      goto lex_end;
	    (*fp_prs_stream_blk)(lx);
	    goto lex_end;
	  case KW_HELP_INDEX:
	    if (Lex(lx)!=TK_STR)
	      goto lex_end;
	    Free(lx->cur_index);
	    lx->cur_index=lx->ident;
	    lx->ident=NULL;
	    break;
	  case KW_HELP_FILE:
	    if (Lex(lx)!=TK_STR)
	      goto lex_end;
	    temph=CAlloc(sizeof(CHashSrcSym));
	    fbuf=DftExt(lx->ident,"TXT.Z");
	    temph->str=FileNameAbs(fbuf);
	    Free(fbuf);
	    temph->type=HTT_HELP_FILE|HTF_PUBLIC;
	    SetSrcFileInfo(lx,temph);
	    if (IsReplaceSyms)
	      HashReplace(temph,lx->htc.glbl_hash_table);
	    else
	      HashAdd(temph,lx->htc.glbl_hash_table);
	    break;
	}
	break;
      case '\n':
	if (!(lx->flags&LF_KEEP_NEW_LINES))
	  break; //else fall through
      case TK_INS_BINARY:
      case TK_INS_BINARY_TYPE:
      case TK_INS_BINARY_SIZE:
	lx->token=ch;
	goto lex_end;
      case '.':
	if (lx->flags&LF_KEEP_DOT) {
	  lx->token=ch;
	  goto lex_end;
	}
	if (lx->flags&LF_LAST_WAS_DOT) {
	  lx->flags&=~LF_LAST_WAS_DOT;
	  goto lex_dot_dot;
	}
	ch=LexGetChar(lx);
	if ('0'<=ch<='9') {
	  i=0;
	  goto lex_float_start;
	} else if (ch=='.') {
lex_dot_dot:
	  lx->token=TK_DOT_DOT;
	  if (LexGetChar(lx)=='.')
	    lx->token=TK_DOT_DOT_DOT;
	  else
	    lx->flags|=LF_USE_LAST_U16;
	  goto lex_end;
	}
	lx->flags|=LF_USE_LAST_U16;
	lx->token='.';
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
	if (!(i=dual_U16_tokens1[ch])) {
	  if (ch=='$$') {
	    ch=LexGetChar(lx);
	    if (ch=='$$') {
	      lx->token='$$';
	      goto lex_end;
	    } else if (ch) {
	      do ch=LexGetChar(lx);
	      while (ch && ch!='$$');
	      if (!ch) {
		lx->token=TK_EOF;
		goto lex_end;
	      } else
		goto lex_cont;
	    } else {
	      lx->flags|=LF_USE_LAST_U16;
	      lx->token='$$';
	      goto lex_end;
	    }
	  } else {
	    lx->token=ch;
	    goto lex_end;
	  }
	} else {
	  j=LexGetChar(lx);
	  if (i.u16[0]==j) {
	    i>>=16;
	    if (!i) { // "/*"
	      j=1;
	      do {
		if (!(ch=LexGetChar(lx)))
		  return lx->token=TK_EOF;
lex_check_comment:
		if (ch=='*') {
		  if (!(ch=LexGetChar(lx)))
		    return lx->token=TK_EOF;
		  if (ch=='/')
		    j--;
		  else
		    goto lex_check_comment;
		} else if (ch=='/') {
		  if (!(ch=LexGetChar(lx)))
		    return lx->token=TK_EOF;
		  if (ch=='*')
		    j++;
		  else
		    goto lex_check_comment;
		}
	      } while (j);
	      goto lex_cont;
	    } else {
	      lx->token=i;
	      goto lex_end;
	    }
	  }
	  if (i=dual_U16_tokens2[ch]) {
	    if (i.u16[0]==j) {
	      i>>=16;
	      if (!i) {  // "//"
		LexSkipEol(lx);
		if (lx->flags&LF_KEEP_NEW_LINES) {
		  lx->token='\n';
		  goto lex_end;
		} else
		  goto lex_cont;
	      } else {
		if (i==TK_SHL || i==TK_SHR) {
		  j=LexGetChar(lx);
		  if (j=='=') {
		    if (i==TK_SHL)
		      i=TK_SHL_EQUAL;
		    else
		      i=TK_SHR_EQUAL;
		  } else
		    lx->flags|=LF_USE_LAST_U16;
		}
		lx->token=i;
		goto lex_end;
	      }
	    }
	    if (i=dual_U16_tokens3[ch]) {
	      if (i.u16[0]==j) {
		lx->token=i.u16[1];
		goto lex_end;
	      }
	    }
	  }
	  lx->flags|=LF_USE_LAST_U16;
	  lx->token=ch;
	  goto lex_end;
      }
      case TK_NUM_TK:
      default:
	break;
    }
  }
lex_end:
  LexGetChar(lx); //Do this so WAS_NEW_LINE is right
  lx->flags|=LF_USE_LAST_U16;
  if (Bt(&lx->opts,OPTf_LEX_TRACE))
    LexPutToken(lx);
  return lx->token;
}

U8 *LexExtStr(CLex *lx,I64 *_size=NULL)
{ //_size includes terminator
  I64 len=lx->ident_len,len1,len2;
  U8 *st=lx->ident,*st1,*st2;
  lx->ident=NULL;
  while (lx->token==TK_STR) {
    st1=st;
    len1=len;
    if (Lex(lx)==TK_STR) {
      len2=lx->ident_len;
      st2=lx->ident;
      lx->ident=NULL;
      len=len1+len2-1;
      st=MAlloc(len);
      if (len1>1)
	MemCpy(st,st1,len1-1);
      MemCpy(st+len1-1,st2,len2);
      Free(st1);
      Free(st2);
    }
  }
  if (_size) *_size=len;
  return st;
}

U8 *LexPutLine(CLex *lx,U8 *start)
{
  U8 *ptr,ch;
  if (!start) return NULL;
  if (lx->cur_lfn->flags&LFSF_IS_DOC)
    return DocPutLine(lx->cur_lfn->doc,start);
  else {
    ptr=start;
    while (ch=*ptr++) {
      if (ch=='\n') {
	if (*ptr=='\r')
	  ptr++;
	break;
      } else if (ch=='\r') {
	if (*ptr=='\n')
	  ptr++;
	break;
      }
      '' ch;
    }
    if (!ch) ptr--;
    NewLine;
    return ptr;
  }
  return NULL;
}

U0 LexPutPos(CLex *lx)
{
  LexPutToken(lx);
  SetFix(lx->cur_lfn->name,lx->cur_lfn->line_num);
  if (IsRaw)
    "%s,%d ",lx->cur_lfn->name,lx->cur_lfn->line_num;
  else {
    PutFileLink(lx->cur_lfn->name,lx->cur_lfn->line_num);
    AdamErr("%s,%d\n",lx->cur_lfn->name,lx->cur_lfn->line_num);
    '' CH_SPACE;
  }
  LexPutLine(lx,lx->cur_lfn->line_start);
}

U0 LexPutWarn(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL)
{
  if (name) PrintWarn("%S",name);
  if (fun_class) {
    "%S'%s'.\n","ST_IN_FUN",fun_class->str;
    if (IsRaw)
      "%s\n",fun_class->src_link;
    else {
      "$$LK-A,\"%s\"$$\n",fun_class->src_link;
      AdamErr("%s\n",fun_class->src_link);
    }
  } else
    LexPutPos(lx);
  lx->warning_cnt++;
}

U0 LexExcept(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL)
{
  nounusedwarn fun_class;
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER)) {
    Preempt(OFF);
    Raw(ON);
  }
  if (name) PrintErr("%S",name);
  if (!IsRaw)
    AdamErr("Task:%08X %S\n",Fs,name);
  LexPutPos(lx);
  lx->error_cnt++;
  FlushMsgs;
  if (!Bt(&sys_run_level,RLf_ADAM_SERVER))
    Dbg("Type \"Fix;\"");
  throw(EXCEPT_COMPILER,0);
}
