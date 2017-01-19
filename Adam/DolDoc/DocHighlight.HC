#help_index "DolDoc/Misc"

CColorROPU32 highlight_hash_type_colors[HTt_TYPES_NUM]=
	DOC_COLOR_EXPORT_SYS_SYM,0,DOC_COLOR_DEFINE_STR,DOC_COLOR_GLBL_VAR,
	DOC_COLOR_CLASS,DOC_COLOR_KEYWORD,DOC_COLOR_FUN,0,0,DOC_COLOR_KEYWORD,
	DOC_COLOR_KEYWORD,DOC_COLOR_KEYWORD,DOC_COLOR_REG,0,0,0,0};

U32 *DocHighlight(CDocEntry *doc_e,U8 *src,I64 len,I64 _tmp_u32_attr)
{//Be aware of $LK,"::/Demo/ToHtmlToTXTDemo/ToHtml.HC"$.
  U32 *res=MAlloc((len+1)*sizeof(U32)),*dst=res;
  U8 *ptr;
  CDocSettings *s=&doc_e->settings;
  I64 ch,ch1,last_ch,tmp_u32_attr,mask_tmp_u32_attr=_tmp_u32_attr&0xFFFFF000,
	comment_depth,brace_depth=s->brace_depth,paren_depth=s->paren_depth;
  CHash *tmph;
  switch [s->state] {
    case DOCSS_NORMAL:
hl_normal:
      while (len) {
	while (len && !Bt(char_bmp_alpha_numeric,*src)) {
	  tmp_u32_attr=_tmp_u32_attr;
	  ch1=*src++;
	  switch (ch1) {
	    case '/':
	      if (len>=2) {
		if (*src=='/') {
		  tmp_u32_attr=DOC_COLOR_COMMENT<<8|mask_tmp_u32_attr;
		  *dst++=ch1+tmp_u32_attr;
		  *dst++=*src++ +tmp_u32_attr;
		  len-=2;
		  goto hl_cpp_comment;
		} else if (*src=='*') {
		  tmp_u32_attr=DOC_COLOR_COMMENT<<8|mask_tmp_u32_attr;
		  *dst++=ch1+tmp_u32_attr;
		  *dst++=*src++ +tmp_u32_attr;
		  len-=2;
		  comment_depth=1;
		  goto hl_comment;
		}
	      }
	      break;
	    case '\'':
	      tmp_u32_attr=DOC_COLOR_CHAR_CONST<<8|mask_tmp_u32_attr;
	      *dst++=ch1+tmp_u32_attr;
	      len--;
	      goto hl_single_quote;
	    case '\"':
	      tmp_u32_attr=DOC_COLOR_STR<<8|mask_tmp_u32_attr;
	      *dst++=ch1+tmp_u32_attr;
	      len--;
	      goto hl_dbl_quote;
	    case '(':
	      if (paren_depth++&1)
		tmp_u32_attr=DOC_COLOR_ALT_TEXT<<8|mask_tmp_u32_attr;
	      break;
	    case ')':
	      if (--paren_depth&1)
		tmp_u32_attr=DOC_COLOR_ALT_TEXT<<8|mask_tmp_u32_attr;
	      break;
	    case '{':
	      if (brace_depth++&1)
		tmp_u32_attr=DOC_COLOR_ALT_TEXT<<8|mask_tmp_u32_attr;
	      break;
	    case '}':
	      if (--brace_depth&1)
		tmp_u32_attr=DOC_COLOR_ALT_TEXT<<8|mask_tmp_u32_attr;
	      break;
	  }
	  *dst++=ch1+tmp_u32_attr;
	  if (!--len) goto hl_normal_done;
	}
	ptr=src;
	while (len && Bt(char_bmp_alpha_numeric,*src)) {
	  src++;
	  len--;
	}
	ch=*src;
	*src=0;
	if (tmph=HashFind(ptr,adam_task->hash_table,
	      HTT_EXPORT_SYS_SYM|HTT_DEFINE_STR|HTT_GLBL_VAR|HTT_CLASS|
	      HTT_INTERNAL_TYPE|HTT_FUN|HTT_KEYWORD|HTT_ASM_KEYWORD|
	      HTT_OPCODE|HTT_REG))
	  tmp_u32_attr=highlight_hash_type_colors[HashTypeNum(tmph)]<<8|
		mask_tmp_u32_attr;
	else
	  tmp_u32_attr=_tmp_u32_attr;
	while (ch1=*ptr++)
	  *dst++=ch1+tmp_u32_attr;
	*src=ch;
      }
hl_normal_done:
      s->state=DOCSS_NORMAL;
      s->comment_depth=0;
      break;
    case DOCSS_SINGLE_QUOTE:
      tmp_u32_attr=DOC_COLOR_CHAR_CONST<<8|mask_tmp_u32_attr;
hl_single_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+tmp_u32_attr;
	if (last_ch!='\\' && ch1=='\'')
	  goto hl_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      s->state=DOCSS_SINGLE_QUOTE;
      s->comment_depth=0;
      break;
    case DOCSS_DBL_QUOTE:
      tmp_u32_attr=DOC_COLOR_CHAR_CONST<<8|mask_tmp_u32_attr;
hl_dbl_quote:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+tmp_u32_attr;
	if (last_ch!='\\' && ch1=='\"')
	  goto hl_normal;
	if (last_ch=='\\' && ch1=='\\')
	  last_ch=0;
	else
	  last_ch=ch1;
      }
      s->state=DOCSS_DBL_QUOTE;
      s->comment_depth=0;
      break;
    case DOCSS_COMMENT:
      tmp_u32_attr=DOC_COLOR_COMMENT<<8|mask_tmp_u32_attr;
      comment_depth=s->comment_depth;
hl_comment:
      last_ch=0;
      while (len--) {
	ch1=*src++;
	*dst++=ch1+tmp_u32_attr;
	if (last_ch=='*' && ch1=='/') {
	  if (!--comment_depth)
	    goto hl_normal;
	} else if (last_ch=='/' && ch1=='*')
	  comment_depth++;
	last_ch=ch1;
      }
      s->state=DOCSS_COMMENT;
      s->comment_depth=comment_depth;
      break;
    case DOCSS_CPP_Z_COMMENT:
      tmp_u32_attr=DOC_COLOR_COMMENT<<8|mask_tmp_u32_attr;
hl_cpp_comment:
      while (len--)
	*dst++=*src++ +tmp_u32_attr;
      s->state=DOCSS_CPP_Z_COMMENT;
      s->comment_depth=0;
      break;
  }
  s->paren_depth=paren_depth;
  s->brace_depth=brace_depth;
  *dst=0;
  return res;
}
