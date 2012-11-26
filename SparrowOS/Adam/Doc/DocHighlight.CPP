CHashTable *doc_hash;

U0 DocHighlightInit()
{
  doc_hash=MAllocIdentical(asm_hash);
  doc_hash->next=NULL;
}

DocHighlightInit;

U32 *DocHighlight(CDoc *doc,Bool full_refresh,CDocEntry *doc_e,CDocSettings **_s,
  U8 *src,I64 len,I64 _temp_u32_attr)
{ //Be aware of $LK,"::/SparrowOS/Adam/ToHtml.CPP.Z","FI:::/SparrowOS/Adam/ToHtml.CPP.Z"$.
  U32 *result=MAlloc((len+1)*sizeof(U32)),*dst=result;
  U8 *ptr,ch,ch1,last_ch;
  CDocSettings *s=*_s;
  I64 temp_u32_attr,mask_temp_u32_attr=_temp_u32_attr&0xFFFFF000,
     comment_depth,brace_depth=s->brace_depth,paren_depth=s->paren_depth;
  CHash *temph;
  nobound_switch (s->state) {
    case DOCSS_NORMAL:
hl_normal:
      while (len) {
	while (len && !Bt(alpha_numeric_bitmap,*src)) {
	  temp_u32_attr=_temp_u32_attr;
	  ch1=*src++;
	  switch (ch1) {
	    case '/':
	      if (len>=2) {
		if (*src=='/') {
		  temp_u32_attr=(s->comment_attr&15)<<8|mask_temp_u32_attr;
		  *dst++=ch1+temp_u32_attr;
		  *dst++=*src++ +temp_u32_attr;
		  len-=2;
		  goto hl_cpp_comment;
		} else if (*src=='*') {
		  temp_u32_attr=(s->comment_attr&15)<<8|mask_temp_u32_attr;
		  *dst++=ch1+temp_u32_attr;
		  *dst++=*src++ +temp_u32_attr;
		  len-=2;
		  comment_depth=1;
		  goto hl_comment;
		}
	      }
	      break;
	    case '\'':
	      temp_u32_attr=(s->quote_attr&15)<<8|mask_temp_u32_attr;
	      *dst++=ch1+temp_u32_attr;
	      len--;
	      goto hl_single_quote;
	    case '\"':
	      temp_u32_attr=(s->quote_attr&15)<<8|mask_temp_u32_attr;
	      *dst++=ch1+temp_u32_attr;
	      len--;
	      goto hl_dbl_quote;
	    case '(':
	      if (paren_depth++&1)
		temp_u32_attr=(s->alt_text_attr&15)<<8|mask_temp_u32_attr;
	      break;
	    case ')':
	      if (--paren_depth&1)
		temp_u32_attr=(s->alt_text_attr&15)<<8|mask_temp_u32_attr;
	      break;
	    case '{':
	      if (brace_depth++&1)
		temp_u32_attr=(s->alt_text_attr&15)<<8|mask_temp_u32_attr;
	      break;
	    case '}':
	      if (--brace_depth&1)
		temp_u32_attr=(s->alt_text_attr&15)<<8|mask_temp_u32_attr;
	      break;
	  }
	  *dst++=ch1+temp_u32_attr;
	  if (!--len) goto hl_normal_done;
	}
	ptr=src;
	while (len && Bt(alpha_numeric_bitmap,*src)) {
	  src++;
	  len--;
	}
	ch=*src;
	*src=0;
	if (temph=HashFind(ptr,doc_hash,HTT_KEYWORD|HTT_ASM_KEYWORD|HTT_OPCODE|HTT_REG)) {
	  if (temph->type&(HTT_KEYWORD|HTT_ASM_KEYWORD|HTT_OPCODE))
	    temp_u32_attr=(s->keyword_attr&15) <<8|mask_temp_u32_attr;
	  else
	    temp_u32_attr=(s->keyword2_attr&15)<<8|mask_temp_u32_attr;
	} else
	  temp_u32_attr=_temp_u32_attr;
	while (ch1=*ptr++)
	  *dst++=ch1+temp_u32_attr;
	*src=ch;
      }
hl_normal_done:
      if (s->state!=DOCSS_NORMAL||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_NORMAL;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_SINGLE_QUOTE:
      temp_u32_attr=(s->quote_attr&15)<<8|mask_temp_u32_attr;
hl_single_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch!='\\' && ch1=='\'')
	  goto hl_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      if (s->state!=DOCSS_SINGLE_QUOTE||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_SINGLE_QUOTE;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_DBL_QUOTE:
      temp_u32_attr=(s->quote_attr&15)<<8|mask_temp_u32_attr;
hl_dbl_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch!='\\' && ch1=='\"')
	  goto hl_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      if (s->state!=DOCSS_DBL_QUOTE||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_DBL_QUOTE;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_COMMENT:
      temp_u32_attr=(s->comment_attr&15)<<8|mask_temp_u32_attr;
      comment_depth=s->comment_depth;
hl_comment:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch=='*' && ch1=='/') {
	  if (!--comment_depth)
	    goto hl_normal;
	} else if (last_ch=='/' && ch1=='*')
	  comment_depth++;
	last_ch=ch1;
      }
      if (s->state!=DOCSS_COMMENT||s->comment_depth!=comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_COMMENT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=comment_depth;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_CPP_Z_COMMENT:
      temp_u32_attr=(s->comment_attr&15)<<8|mask_temp_u32_attr;
hl_cpp_comment:
      while (len--)
	*dst++=*src++ +temp_u32_attr;
      if (s->state!=DOCSS_CPP_Z_COMMENT||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_CPP_Z_COMMENT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_NORMAL+DOCSSF_BPT:
      temp_u32_attr=_temp_u32_attr & 0xFFFFF000 | RED<<8;
hl_bpt_normal:
      while (len) {
	while (len && !Bt(alpha_numeric_bitmap,*src)) {
	  ch1=*src++;
	  switch (ch1) {
	    case '/':
	      if (len>=2) {
		if (*src=='/') {
		  *dst++=ch1+temp_u32_attr;
		  *dst++=*src++ +temp_u32_attr;
		  len-=2;
		  goto hl_bpt_cpp_comment;
		} else if (*src=='*') {
		  *dst++=ch1+temp_u32_attr;
		  *dst++=*src++ +temp_u32_attr;
		  len-=2;
		  comment_depth=1;
		  goto hl_bpt_comment;
		}
	      }
	      break;
	    case '\'':
	      *dst++=ch1+temp_u32_attr;
	      len--;
	      goto hl_bpt_single_quote;
	    case '\"':
	      *dst++=ch1+temp_u32_attr;
	      len--;
	      goto hl_bpt_dbl_quote;
	    case '(':
	      paren_depth++;
	      break;
	    case ')':
	      paren_depth--;
	      break;
	    case '{':
	      brace_depth++;
	      break;
	    case '}':
	      brace_depth--;
	      break;
	  }
	  *dst++=ch1+temp_u32_attr;
	  if (!--len) goto hl_bpt_normal_done;
	}
	ptr=src;
	while (len && Bt(alpha_numeric_bitmap,*src)) {
	  src++;
	  len--;
	}
	ch=*src;
	*src=0;
	while (ch1=*ptr++)
	  *dst++=ch1+temp_u32_attr;
	*src=ch;
      }
hl_bpt_normal_done:
      if (s->state!=DOCSS_NORMAL+DOCSSF_BPT||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_NORMAL+DOCSSF_BPT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_SINGLE_QUOTE+DOCSSF_BPT:
      temp_u32_attr=_temp_u32_attr & 0xFFFFF000 | RED<<8;
hl_bpt_single_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch!='\\' && ch1=='\'')
	  goto hl_bpt_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      if (s->state!=DOCSS_SINGLE_QUOTE+DOCSSF_BPT||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_SINGLE_QUOTE+DOCSSF_BPT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_DBL_QUOTE+DOCSSF_BPT:
      temp_u32_attr=_temp_u32_attr & 0xFFFFF000 | RED<<8;
hl_bpt_dbl_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch!='\\' && ch1=='\"')
	  goto hl_bpt_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      if (s->state!=DOCSS_DBL_QUOTE+DOCSSF_BPT||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_DBL_QUOTE+DOCSSF_BPT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_COMMENT+DOCSSF_BPT:
      temp_u32_attr=_temp_u32_attr & 0xFFFFF000 | RED<<8;
      comment_depth=s->comment_depth;
hl_bpt_comment:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+temp_u32_attr;
	if (last_ch=='*' && ch1=='/') {
	  if (!--comment_depth)
	    goto hl_bpt_normal;
	} else if (last_ch=='/' && ch1=='*')
	  comment_depth++;
	last_ch=ch1;
      }
      if (s->state!=DOCSS_COMMENT+DOCSSF_BPT||s->comment_depth!=comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_COMMENT+DOCSSF_BPT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=comment_depth;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
    case DOCSS_CPP_Z_COMMENT+DOCSSF_BPT:
      temp_u32_attr=_temp_u32_attr & 0xFFFFF000 | RED<<8;
hl_bpt_cpp_comment:
      while (len--)
	*dst++=*src++ +temp_u32_attr;
      if (s->state!=DOCSS_CPP_Z_COMMENT+DOCSSF_BPT||s->comment_depth||
	  s->paren_depth!=paren_depth||s->brace_depth!=brace_depth) {
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->state=DOCSS_CPP_Z_COMMENT+DOCSSF_BPT;
	  s->paren_depth=paren_depth;
	  s->brace_depth=brace_depth;
	  s->comment_depth=0;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
      }
      break;
  }
  *dst=0;
  *_s=s;
  return result;
}

