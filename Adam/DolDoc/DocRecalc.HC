#help_index "DolDoc"

I64 DocWordWrapDel(CDoc *doc,CDocEntry *doc_e,
	Bool full_refresh,Bool same_win,I64 left_margin,I64 right_margin,
	CDocEntry **_best_doc_e,I64 *_best_col)
{
  CDocEntry *doc_e2;
  U8 *ptr;
  I64 j,k;
  if (doc_e->de_flags&DOCEF_TAG && doc_e->tag)
    k=StrLen(doc_e->tag);
  else
    k=0;
  if (full_refresh)
    while (TRUE) {
      doc_e2=doc_e->next;
      if (doc_e2->type_u8==DOCT_SOFT_NEW_LINE && !same_win) {
	if (doc->cur_entry==doc_e2) {
	  doc->cur_entry=doc_e2->next;
	  doc->cur_col=doc->cur_entry->min_col;
	}
	if (*_best_doc_e==doc_e2) {
	  *_best_doc_e=doc_e2->next;
	  *_best_col=0;
	}
	DocEntryDel(doc,doc_e2);
      } else if (IsEditableText(doc_e) &&
	    doc_e->de_flags==doc_e2->de_flags && doc_e->type==doc_e2->type) {
	j=StrLen(doc_e2->tag);
	ptr=MAlloc(k+j+1,doc->mem_task);
	MemCpy(ptr,doc_e->tag,k);
	MemCpy(ptr+k,doc_e2->tag,j+1);
	Free(doc_e->tag);
	doc_e->tag=ptr;
	if (doc->cur_entry==doc_e2) {
	  doc->cur_entry=doc_e;
	  doc->cur_col+=k;
	}
	if (*_best_doc_e==doc_e2) {
	  *_best_doc_e=doc_e;
	  *_best_col=0;
	}
	DocEntryDel(doc,doc_e2);
	k+=j;
	if (k>(right_margin-left_margin+1)<<1)
	  break;
      } else
	break;
    }
  if (doc_e->de_flags & DOCEF_SCROLLING_X)
    k=doc_e->scroll_len;
  return k;
}

U0 DocRecalcXY(CDoc *doc,CDocEntry *doc_e,
	I64 k,I64 left,I64 width,I64 height,I64 left_margin,I64 right_margin,
	I64 x0,I64 y0,I64 *_x,I64 *_y)
{
  I64 i,x=*_x,y=*_y;
  if (doc_e->de_flags & DOCEF_MARGIN_REL_X) {
    if (doc_e->de_flags & DOCEF_LEFT_X)
      x=left_margin-left;
    else if (doc_e->de_flags & DOCEF_RIGHT_X)
      x=right_margin-(k-1)-left;
    else if (doc_e->de_flags & DOCEF_CENTER_X)
      x=(right_margin+left_margin)>>1-k>>1-left;
  } else {
    if (doc_e->de_flags & DOCEF_LEFT_X)
      x=x0;
    else if (doc_e->de_flags & DOCEF_RIGHT_X)
      x=width+x0-k;
    else if (doc_e->de_flags & DOCEF_CENTER_X)
      x=(width+x0-k)>>1;
  }
  i=y;
  if (doc_e->de_flags & DOCEF_PAGE_REL_Y) {
    doc->flags|=DOCF_BWD_MOVEMENT;
    if (doc_e->de_flags & DOCEF_TOP_Y)
      y-=doc_e->page_line_num;
    else if (doc_e->de_flags & DOCEF_BOTTOM_Y)
      y+=doc_e->settings.page_len-doc_e->page_line_num;
    else if (doc_e->de_flags & DOCEF_CENTER_Y)
      y+=doc_e->settings.page_len>>1-doc_e->page_line_num;
  } else {
    doc->flags|=DOCF_BWD_MOVEMENT;
    if (doc_e->de_flags & DOCEF_TOP_Y)
      y=y0;
    else if (doc_e->de_flags & DOCEF_BOTTOM_Y)
      y=height-1+y0;
    else if (doc_e->de_flags & DOCEF_CENTER_Y)
      y=height>>1+y0;
  }
  if (y!=i) {
    doc->page_line_num+=y-i;
    if (doc->page_line_num<0)
      doc->page_line_num=doc_e->settings.page_len+
	    doc->page_line_num%doc_e->settings.page_len;
    else
      doc->page_line_num=doc->page_line_num%doc_e->settings.page_len;
    if (doc_e->settings.header!=DOC_DFT &&
	  doc->page_line_num<doc_e->settings.header) {
      y+=doc_e->settings.header-doc->page_line_num;
      doc->page_line_num=doc_e->settings.header;
    }
    if (doc_e->settings.footer==DOC_DFT) {
      if (doc->page_line_num>=doc_e->settings.page_len) {
	if (doc_e->settings.header==DOC_DFT)
	  doc->page_line_num=0;
	else {
	  doc->page_line_num=doc_e->settings.header;
	  y+=doc_e->settings.header;
	}
      }
    } else {
      if (doc->page_line_num>=
	    doc_e->settings.page_len-doc_e->settings.footer) {
	y+=doc_e->settings.footer;
	if (doc_e->settings.header==DOC_DFT)
	  doc->page_line_num=0;
	else {
	  doc->page_line_num=doc_e->settings.header;
	  y+=doc_e->settings.header;
	}
      }
    }
  }
  *_x=x;
  *_y=y;
}

CDocEntry *DocSplitTag(CDoc *doc,CDocEntry *doc_e,I64 i,I64 x,I64 y,I64 type_u8)
{//Split tag at i, insert DOCT_SOFT_NEW_LINE, DOCT_MARKER or DOCT_CURSOR
  U8 *ptr;
  CDocEntry *doc_e2;
  if (doc_e->type_u8==DOCT_TEXT && i) {
    if (i<StrLen(doc_e->tag)) {
      doc_e2=MAllocIdent(doc_e,doc->mem_task);
      doc_e2->tag=StrNew(doc_e->tag+i,doc->mem_task);
      doc_e2->de_flags=doc_e->de_flags&~DOCEG_HAS_ALLOC|DOCEF_TAG;
      QueIns(doc_e2,doc_e);
      if (doc->cur_entry==doc_e && doc->cur_col>=i) {
	doc->cur_entry=doc_e2;
	doc->cur_col=doc->cur_col-i;
      }
      doc_e->tag[i]=0;
      ptr=StrNew(doc_e->tag,doc->mem_task);
      Free(doc_e->tag);
      doc_e->tag=ptr;
    }
  } else
    doc_e=doc_e->last;
  doc_e2=DocEntryNewBase(doc,type_u8|doc_e->type & 0xFFFFFF00,
	doc_e->de_flags&~DOCEG_HAS_ARG,x,y,doc_e->page_line_num);
  MemCpy(&doc_e2->settings,&doc_e->settings,sizeof(CDocSettings));
  QueIns(doc_e2,doc_e);
  return doc_e2;
}

CDocEntry *DocWordWrapAdd(CDoc *doc,CDocEntry *doc_e,
	I64 *_k,I64 left,I64 right_margin,I64 x,I64 y)
{
  CDocEntry *doc_e2;
  I64 j,i=right_margin+1-(x+left),  //Space left on line
	ii=x+1-doc_e->settings.left_margin;
  if (IsEditableText(doc_e)) {
    if (doc->cur_entry==doc_e->next) {
      if (doc->cur_col==doc_e->next->min_col)
	i--;
    } else {
      if (doc->cur_entry==doc_e && doc->cur_col==i)
	i--;
    }
    if (*_k>i) {
      for (j=i;j>8-ii && j>=0;j--)
	if (doc_e->tag[j]==CH_SPACE || doc_e->tag[j]==CH_SHIFT_SPACE) {
	  i=j+1;
	  break;
	}
      if (0<i<*_k) {
	DocSplitTag(doc,doc_e,i,x,y,DOCT_SOFT_NEW_LINE);
	*_k=StrLen(doc_e->tag);
	return NULL;
      }
    }
    if (*_k==i)
      return NULL;
  }
  if (*_k>=i) {
    doc_e2=doc_e->last;
    if (doc_e2->type_u8!=DOCT_SOFT_NEW_LINE &&
	  doc_e2->type_u8!=DOCT_NEW_LINE &&
	  doc_e2->type_u8!=DOCT_CURSOR_MOVEMENT) {
      doc_e2=DocEntryNewBase(doc,DOCT_SOFT_NEW_LINE|doc_e->type&0xFFFFFF00,
	    DOCEF_WORD_WRAP|doc_e->de_flags&(DOCEF_HIGHLIGHT|DOCG_BL_IV_UL|
	    DOCEF_SKIP|DOCEF_FILTER_SKIP),x,y,doc_e->last->page_line_num);
      MemCpy(&doc_e2->settings,&doc_e->settings,sizeof(CDocSettings));
      QueIns(doc_e2,doc_e->last);
      return doc_e2;
    }
  }
  return NULL;
}

I64 DocTmpAttr(CDoc *doc,CDocEntry *doc_e,I64 cur_u8_attr)
{
  I64 tmp_u32_attr;
  doc_e->de_flags=doc->flags& (DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT) |
	doc_e->de_flags&~(DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT);
  tmp_u32_attr=(cur_u8_attr&0xF0)<<8|
	doc->flags&DOCG_BL_IV_UL|(doc_e->settings.shifted_x&0x1F)<<16|
	(doc_e->settings.shifted_y&0x1F)<<21;
  if (doc_e->de_flags & DOCEF_HAS_BIN && *doc_e->tag=='<')
    tmp_u32_attr.u8[1]|=DOC_COLOR_BIN;
  else
    switch (doc_e->type_u8) {
      case DOCT_SPRITE:
	if (doc_e->de_flags & DOCEF_LEFT_EXP)
	  tmp_u32_attr.u8[1]|=cur_u8_attr&15;
	else if (doc_e->de_flags & DOCEF_LINK)
	  tmp_u32_attr.u8[1]|=DOC_COLOR_LINK;
	else if (doc_e->de_flags & DOCEF_LEFT_MACRO)
	  tmp_u32_attr.u8[1]|=DOC_COLOR_MACRO;
	else if (doc_e->de_flags & (DOCEF_TREE|DOCEF_LST))
	  tmp_u32_attr.u8[1]|=DOC_COLOR_TREE;
	else
	  tmp_u32_attr.u8[1]|=DOC_COLOR_BIN;
	break;
      case DOCT_HTML_CODE:
	tmp_u32_attr.u8[1]|=DOC_COLOR_BIN;
	break;
      case DOCT_LINK:
	tmp_u32_attr.u8[1]|=DOC_COLOR_LINK;
	break;
      case DOCT_MACRO:
	tmp_u32_attr.u8[1]|=DOC_COLOR_MACRO;
	break;
      case DOCT_ANCHOR:
	tmp_u32_attr.u8[1]|=DOC_COLOR_ANCHOR;
	break;
      case DOCT_TREE:
      case DOCT_LST:
	tmp_u32_attr.u8[1]|=DOC_COLOR_TREE;
	break;
      default:
	tmp_u32_attr.u8[1]|=cur_u8_attr&15;
    }
  doc_e->type.u8[1]=tmp_u32_attr.u8[1];
  tmp_u32_attr|=doc_e->type&0xFFFF0000;
  if (doc_e==doc->cur_entry && !(doc->flags&DOCF_DONT_HIGHLIGHT_CURSOR) &&
	doc_e->type_u8!=DOCT_TEXT)
    tmp_u32_attr^=0xFF00;
  doc_e->settings.final_u32_attr=tmp_u32_attr;
  return tmp_u32_attr;
}

public Bool DocRecalc(CDoc *doc,I64 recalc_flags=RECALCt_NORMAL)
{//Recalc and fmt.  Also used by WinMgr to draw on scrn.
  I64 i,ii,j,k,x,x0,y,y0,D,d2,col,col2,best_col=0,best_d=I64_MAX,xx,yy,zz,
	num_entries=0,i_jif,cur_u8_attr,tmp_u32_attr,
	cursor_y=I64_MIN,left_margin,right_margin,y_plot_top,y_plot_bottom,
	top,left,bottom,right,width,height,scroll_x,scroll_y,pix_top,pix_left;
  CDocEntry reg *doc_e,reg *doc_e2,*best_doc_e,*next_clear_found=NULL,
	*added_cursor=NULL;
  CDocBin *tmpb;
  CDocSettings *s;
  Bool del_doc_e,skipped_update,tree_collapsed,same_win,more=FALSE,
	find_cursor=FALSE,blink_flag,full_refresh=TRUE,unlock,clear_holds;
  CTask *win_task,*mem_task;
  CDC *dc;
  U8 *bptr,*ptr,buf[STR_LEN],ch;
  U32 *u32_ptr,*hl;
  I32 *depth_buf=NULL;
  F64 cur_time=tS;
  CWinScroll *vss,*hss;
  CHashDefineStr *tmph;

  if (!doc || doc->doc_signature!=DOC_SIGNATURE_VAL) return FALSE;

    //WinMgr updates all wins $TX,"30",D="WINMGR_FPS"$, 33.33333mS
  if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN && doc->owning_task!=Fs) {
    i_jif=cnts.jiffies+JIFFY_FREQ/250; //4 ms
    while (Bt(&doc->locked_flags,DOClf_LOCKED)) {
      if (cnts.jiffies>=i_jif)
	return FALSE; //Bail-out if doc locked.
      Yield;
    }
  }

  unlock=DocLock(doc);
  if (doc->doc_signature!=DOC_SIGNATURE_VAL) {
    DocUnlock(doc);
    return FALSE;
  }

  win_task=doc->win_task;
  mem_task=doc->mem_task;
  blink_flag=Blink;
  dc=NULL;
  switch [recalc_flags&RECALCG_MASK] {
    case RECALCt_FIND_CURSOR:
      find_cursor=TRUE;
      if (win_task)
	dc=DCAlias(gr.dc2,win_task); //Necessary for sprites
      break;
    case RECALCt_TO_SCRN:
      if (doc->updates_cnt++%(ToI64(winmgr.fps/10)+1) &&
	    !Bt(&doc->flags,DOCf_DO_FULL_REFRESH) &&
	    !(doc->flags&DOCF_BWD_MOVEMENT))
	full_refresh=FALSE;
      if (win_task)
	dc=DCAlias(gr.dc2,win_task);
      break;
  }

  PUSHFD
  CLI
  left	=win_task->win_left;
  right =win_task->win_right;
  width =win_task->win_width;
  scroll_x=win_task->scroll_x;
  scroll_y=win_task->scroll_y;
  top	=win_task->win_top;
  bottom=win_task->win_bottom;
  height=win_task->win_height;
  pix_left  =win_task->pix_left;
  pix_top   =win_task->pix_top;
  left_margin=left;
  right_margin=right;
  POPFD
  if (doc->flags&DOCF_BORDER_DOC) {
    scroll_x=0;
    scroll_y=0;
  }
  best_doc_e=doc->cur_entry;

  if (!(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)) &&
	FilesFindMatch(doc->filename.name,FILEMASK_SRC))
    doc->flags|=DOCF_HIGHLIGHT;
  else
    doc->flags&=~DOCF_HIGHLIGHT;

  x=y=0;
  doc->page_line_num=0;
  if (full_refresh && !find_cursor) {
    doc->x=x;
    doc->y=y;
  }

  hss=&win_task->horz_scroll;
  vss=&win_task->vert_scroll;
  if (doc->flags&DOCF_BORDER_DOC) {
    doc->top_line_num=0;
    doc->line_start_col=0;
    recalc_flags&=~RECALCF_HAS_CURSOR;
    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN)
      doc->settings_head.cur_text_attr=
	    doc->settings_head.dft_text_attr=win_task->border_attr;
  } else {
    if (recalc_flags&RECALCF_HAS_CURSOR && full_refresh) {
      if (Bt(&hss->flags,WSSf_SET_TO_POS)||Bt(&vss->flags,WSSf_SET_TO_POS)) {
	if (!(doc->flags&DOCF_NO_SCROLL_BARS)) {
	  if (Bt(&hss->flags,WSSf_SET_TO_POS)) {
	    doc->line_start_col=hss->pos;
	    LBtr(&hss->flags,WSSf_SET_TO_POS);
	  }
	  if (Bt(&vss->flags,WSSf_SET_TO_POS)) {
	    doc->top_line_num=vss->pos;
	    LBtr(&vss->flags,WSSf_SET_TO_POS);
	  }
	}
	doc->x=doc->line_start_col+width/2;
	doc->y=doc->top_line_num+height/2;
	find_cursor=TRUE;
      }
    }
    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN)
      doc->settings_head.cur_text_attr=
	    doc->settings_head.dft_text_attr=win_task->text_attr;
  }
  x0=doc->line_start_col;
  y0=doc->top_line_num;
  same_win=top	 ==doc->old_win_top &&
	bottom==doc->old_win_bottom &&
	left  ==doc->old_win_left &&
	right ==doc->old_win_right &&
	doc->cur_entry==doc->old_cur_entry &&
	doc->cur_col==doc->old_cur_col;
  if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN) {
    y_plot_top=y0-scroll_y/FONT_HEIGHT;
    y_plot_bottom=y0+height-1-scroll_y/FONT_HEIGHT;
    if (!(doc->flags&DOCF_BORDER_DOC) &&
	  !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
      DocBorderLstDraw(doc);
  }

  if (doc->cur_col<=doc->cur_entry->min_col)
    doc->cur_col=doc->cur_entry->min_col;
  doc_e=doc->head.next;
  doc_e->de_flags&=~(DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT);
  if (doc_e==doc->head.next)
    s=&doc->settings_head;
  else
    s=&doc_e->last->settings;
  doc->flags=doc_e->de_flags& (DOCG_BL_IV_UL|DOCEF_WORD_WRAP) |
	doc->flags&~(DOCG_BL_IV_UL|DOCEF_WORD_WRAP);
  cur_u8_attr=s->cur_text_attr;
  if (doc_e==doc->head.next) {
    doc->flags&=~DOCF_BWD_MOVEMENT;
    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN && full_refresh)
      doc->flags&=~DOCF_HAS_SONG;
  } else
    doc->flags=doc_e->de_flags& DOCEF_HIGHLIGHT |
	  doc->flags&~DOCEF_HIGHLIGHT;

  if (doc->head.next==doc) {
    best_doc_e=doc;
    best_col=0;
    doc->cur_entry=doc;
    doc->cur_col=0;
    doc_e=doc;
  }
  skipped_update= doc_e==doc && doc->head.next!=doc;

  if (full_refresh) {
    doc->min_x=I32_MAX; doc->min_y=I32_MAX;
    doc->max_x=I32_MIN; doc->max_y=I32_MIN;
  }
  while (doc_e!=doc) {
    while (TRUE) {
      del_doc_e=FALSE;
      if (doc_e->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP)) {
	doc_e2=doc_e;
	goto rc_skip;
      }
      MemCpy(&doc_e->settings,s,sizeof(CDocSettings));
      s=&doc_e->settings;
      if (doc_e->de_flags & (DOCEF_TAG_CB|DOCEF_DEFINE) &&
	    !(doc_e->de_flags & DOCEF_LST)) {
	Free(doc_e->tag);
	if (doc_e->de_flags & DOCEF_TAG_CB) {
	  if (doc_e->tag_cb)
	    doc_e->tag=(*doc_e->tag_cb)(doc,doc_e,mem_task);
	  else
	    doc_e->tag=StrNew("",mem_task);
	} else {
	  if (tmph=HashFind(doc_e->define_str,
		win_task->hash_table,HTT_DEFINE_STR))
	    doc_e->tag=StrNew(tmph->data,mem_task);
	  else
	    doc_e->tag=CAlloc(1,mem_task);
	}
	doc_e->max_col=StrLen(doc_e->tag);
	if (doc->cur_entry==doc_e && doc->cur_col>=doc_e->max_col) {
	  if (doc_e->max_col)
	    doc->cur_col=doc_e->max_col-1;
	  else
	    doc->cur_col=0;
	}
      }
      k=DocWordWrapDel(doc,doc_e,full_refresh,same_win,
	    left_margin,right_margin,&best_doc_e,&best_col);
      if (doc_e->de_flags & (DOCEF_LEFT_X|DOCEF_RIGHT_X|DOCEF_CENTER_X|
	    DOCEF_TOP_Y|DOCEF_BOTTOM_Y|DOCEF_CENTER_Y))
	DocRecalcXY(doc,doc_e,k,
	      left,width,height,left_margin,right_margin,x0,y0,&x,&y);
      if (full_refresh && k>0 && doc->flags & DOCF_WORD_WRAP &&
	    (doc_e2=DocWordWrapAdd(doc,doc_e,&k,left,right_margin,x,y)))
	doc_e=doc_e2;
      else
	break;
    }

    if (full_refresh) {
      doc_e->x=x;
      doc_e->y=y;
      doc_e->page_line_num=doc->page_line_num;
      if (x<doc->min_x) doc->min_x=x;
      if (y<doc->min_y) doc->min_y=y;
      if (find_cursor) {
	D=DocCharDist(doc,x,y);
	col=0;
      }
    }
    col2=0;

    tmp_u32_attr=DocTmpAttr(doc,doc_e,cur_u8_attr);
    if (doc_e==doc->cur_entry) {
      cursor_y=doc_e->y;
      if (recalc_flags&RECALCF_ADD_CURSOR && !added_cursor) {
	if (doc_e->type_u8==DOCT_TEXT && 0<doc->cur_col<k &&
	      !(doc_e->de_flags & ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|
	      DOCEF_HIGHLIGHT|DOCEF_SKIP|DOCEF_FILTER_SKIP)) &&
	      !(doc_e->type&DOCG_BL_IV_UL)) {
	  added_cursor=DocSplitTag(doc,doc_e,doc->cur_col,x,y,DOCT_CURSOR);
	  k=StrLen(doc_e->tag);
	} else {
	  added_cursor=doc_e2=DocEntryNewBase(doc,
		DOCT_CURSOR|doc_e->type&0xFFFFFF00,
		doc_e->de_flags&~DOCEG_HAS_ARG,x,y,doc->page_line_num);
	  MemCpy(&doc_e2->settings,&doc_e->settings,sizeof(CDocSettings));
	  if (doc_e->type_u8==DOCT_TEXT && doc->cur_col>=k)
	    QueIns(doc_e2,doc_e);
	  else
	    QueInsRev(doc_e2,doc_e);
	}
      }
    }

    if (doc_e->de_flags & DOCEF_REFRESH_DATA &&
	  (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX ||
	  doc_e->de_flags & DOCEF_LST)) {
      DocDataFmt(doc,doc_e);
      k=StrLen(doc_e->tag);
    }
    if (doc_e->de_flags&DOCEF_TAG) {
      ptr=doc_e->tag;
      if (doc_e->de_flags & DOCEF_TREE) {
	if (k>=2) {
	  if (doc_e->de_flags & DOCEF_CHECKED_COLLAPSED)
	    *ptr++='+';
	  else
	    *ptr++='-';
	  *ptr++=']';
	  ptr=doc_e->tag;
	}
      } else if (doc_e->de_flags & DOCEF_HAS_BIN) {
	if (*ptr=='<' && full_refresh && '0'<=ptr[1]<='9') {
	  ptr=MStrPrint("<%d>",doc_e->bin_num);
	  Free(doc_e->tag);
	  doc_e->tag=StrNew(ptr,mem_task);
	  Free(ptr);
	  ptr=doc_e->tag;
	  k=StrLen(ptr);
	}
      } else if (doc_e->type_u8==DOCT_CHECK_BOX) {
	if (k>=3) {
	  *ptr++='[';
	  if (doc_e->de_flags & DOCEF_CHECKED_COLLAPSED)
	    *ptr++='X';
	  else
	    *ptr++=CH_SPACE;
	  *ptr++=']';
	  ptr=doc_e->tag;
	}
      }
      if (doc_e->de_flags & DOCEF_SCROLLING_X) {
	j=StrLen(doc_e->tag);
	if (j && doc_e->scroll_len) {
	  i_jif=ToI64(cur_time*FONT_WIDTH*DOC_SCROLL_SPEED)%(j*FONT_WIDTH);
	  tmp_u32_attr=tmp_u32_attr & 0xFFE0FF00|
		(FONT_WIDTH-1-i_jif&(FONT_WIDTH-1))<<16;
#assert FONT_WIDTH==8
	  i_jif>>=3;
	  for (k=0;k<doc_e->scroll_len;k++) {
	    ch=ptr[(i_jif+k)%j];
	    if (!Bt(char_bmp_displayable,ch)) ch='.';
	    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		  !(doc_e->de_flags&DOCEF_DONT_DRAW)) {
	      if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+ch);
	      else
		TextChar(win_task,FALSE,x-x0,y-y0,tmp_u32_attr+ch);
	    }
	    x++;
	  }
	}
	if (find_cursor)  {
	  D=DocCharDist(doc,doc_e->x,doc_e->y);
	  col=doc_e->min_col;
	}
	col2=doc_e->scroll_len;	//TODO This is flawed
      } else {
	if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
	      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	  while (ch=*ptr++) {
	    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		  !(doc_e->de_flags&DOCEF_DONT_DRAW))
	      TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+ch);
	    else
	      if (find_cursor) {
		d2=DocCharDist(doc,x,y);
		if (d2<D) {
		  D=d2;
		  col=col2;
		}
	      }
	    col2++;
	    x++;
	  }
	} else {
	  if (doc_e->type_u8==DOCT_TEXT && doc_e->de_flags&DOCEF_HIGHLIGHT)
	    hl=DocHighlight(doc_e,ptr,k,tmp_u32_attr);
	  else
	    hl=NULL;
	  if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		!(doc_e->de_flags&DOCEF_DONT_DRAW)) {
//Technically we should do this for scrolling_x, too.
	    if (y>y_plot_bottom)
	      more=TRUE;
	    else if (y>=y_plot_top) {
	      if (hl)
		TextLenAttrStr(win_task,x-x0,y-y0,k,hl);
	      else
		TextLenStr(win_task,x-x0,y-y0,k,tmp_u32_attr,ptr);
	    }
	    col2+=k;
	    x+=k;
	  } else {
	    if (find_cursor) {
	      while (k--) {
		d2=DocCharDist(doc,x,y);
		if (d2<D) {
		  D=d2;
		  col=col2;
		}
		col2++;
		x++;
	      }
	    } else {
	      col2+=k;
	      x+=k;
	    }
	  }
	  Free(hl);
	}
      }
    }
    switch [doc_e->type_u8] {
      case DOCT_TEXT:
	if (!col2 && !(doc_e->de_flags
	      &(DOCEF_TREE|DOCEF_LST|DOCEF_TAG_CB|DOCEF_DEFINE|
	      DOCEF_AUX_STR|DOCEF_HTML_LINK|DOCEF_BIN_PTR_LINK)))
	  del_doc_e=TRUE;
	break;
      case DOCT_HEX_ED:
	if (doc_e->de_flags&DOCEF_DEREF_DATA &&
	      !(doc_e->de_flags&DOCEF_REMALLOC_DATA))
	  bptr=doc_e->data;
	else
	  bptr=&doc_e->data;
	k=doc_e->hex_ed_width; //columns
	for (i=0;i<doc_e->len;i+=k) {
	  if (doc_e->de_flags & DOCEF_ZERO_BASED)
	    StrPrint(buf,"%08tX ",i);
	  else
	    StrPrint(buf,"%08tX ",bptr);
	  ptr=buf;
	  while (ch=*ptr++) {
	    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		  !(doc_e->de_flags&DOCEF_DONT_DRAW)) {
	      if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+ch);
	      else
		TextChar(win_task,FALSE,x-x0,y-y0,tmp_u32_attr+ch);
	    }
	    if (find_cursor) {
	      d2=DocCharDist(doc,x,y);
	      if (d2<D) {
		D=d2;
		col=i*3;
	      }
	    }
	    x++;
	  }
	  if (i+k>doc_e->len) k=doc_e->len-i;
	  for (j=0;j<k;j++) {
	    StrPrint(buf,"%02tX",*bptr++);
	    ptr=buf;
	    while (ch=*ptr++) {
	      if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		    !(doc_e->de_flags&DOCEF_DONT_DRAW)) {
		if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
		      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		  TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+ch);
		else
		  TextChar(win_task,FALSE,x-x0,y-y0,tmp_u32_attr+ch);
	      }
	      if (find_cursor) {
		d2=DocCharDist(doc,x,y);
		if (d2<D) {
		  D=d2;
		  col=col2;
		}
	      }
	      col2++;
	      x++;
	    }
	    x++;
	  }
	  bptr-=j;
	  x+=(doc_e->hex_ed_width-k)*3;
	  for (j=0;j<k;j++) {
	    ch=*bptr++;
	    if (!Bt(char_bmp_displayable,ch)) ch='.';
	    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		  !(doc_e->de_flags&DOCEF_DONT_DRAW)) {
	      if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+ch);
	      else
		TextChar(win_task,FALSE,x-x0,y-y0,tmp_u32_attr+ch);
	    }
	    if (find_cursor) {
	      d2=DocCharDist(doc,x,y);
	      if (d2<D) {
		D=d2;
		col=col2;
	      }
	    }
	    col2++;
	    x++;
	  }
	  y++;
	  x-=doc_e->hex_ed_width*3+k+9;
	}
	break;
      case DOCT_NEW_LINE:
      case DOCT_SOFT_NEW_LINE:
	if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
	      !(doc_e->de_flags&DOCEF_DONT_DRAW)&&
	      y_plot_top<=y<=y_plot_bottom)
	  TextLenAttr(win_task,x-x0,y-y0,width-(x-x0),cur_u8_attr<<8);
	if (doc_e->de_flags&DOCEF_HIGHLIGHT && s->state==DOCSS_CPP_Z_COMMENT)
	  s->state=DOCSS_NORMAL;
	y++;
	doc->page_line_num++;
rc_start_of_line:
	if (s->left_margin==DOC_DFT)
	  x=s->indent;
	else
	  x=s->indent+s->left_margin;
rc_adjust_xy:
	i=s->indent+s->left_margin;
	if (x<i)
	  x=i;
	if (doc->page_line_num<0)
	  doc->page_line_num=s->page_len+doc->page_line_num%s->page_len;
	else {
	  if (doc->page_line_num>=s->page_len) {
	    doc->page_line_num-=s->page_len;
	    if (doc->page_line_num>=s->page_len) //avoid extra divide
	      doc->page_line_num=doc->page_line_num%s->page_len;
	  }
	}
	if (s->header!=DOC_DFT) {
	  if (doc->page_line_num<s->header) {
	    y+=s->header-doc->page_line_num;
	    doc->page_line_num=s->header;
	    goto rc_start_of_line;
	  }
	}
	if (s->footer==DOC_DFT) {
	  if (doc->page_line_num>=s->page_len) {
	    if (s->header==DOC_DFT)
	      doc->page_line_num=0;
	    else {
	      doc->page_line_num=s->header;
	      y+=s->header;
	    }
	    goto rc_start_of_line;
	  }
	} else {
	  if (doc->page_line_num>=s->page_len-s->footer) {
	    y+=s->footer;
	    if (s->header==DOC_DFT)
	      doc->page_line_num=0;
	    else {
	      doc->page_line_num=s->header;
	      y+=s->header;
	    }
	    goto rc_start_of_line;
	  }
	}
	break;
      case DOCT_TAB:
	k=(x+8) & ~7;
	if (doc_e->de_flags & DOCEF_BORDER_PLOT &&
	      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	  while (x<k) {
	    if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		  !(doc_e->de_flags&DOCEF_DONT_DRAW))
	      TextChar(win_task,TRUE,x-x0,y-y0,tmp_u32_attr+CH_SPACE);
	    if (find_cursor) {
	      d2=DocCharDist(doc,x,y);
	      if (d2<D)
		D=d2;
	    }
	    x++;
	  }
	} else {
	  k-=x;
	  if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
		!(doc_e->de_flags&DOCEF_DONT_DRAW)) {
	    if (y_plot_top<=y<=y_plot_bottom)
	      TextLenStr(win_task,x-x0,y-y0,k,tmp_u32_attr,"        ");
	    x+=k;
	  } else {
	    if (find_cursor) {
	      while (k--) {
		d2=DocCharDist(doc,x,y);
		if (d2<D)
		  D=d2;
		x++;
	      }
	    } else
	      x+=k;
	  }
	}
	break;
      case DOCT_PAGE_BREAK:
	doc->flags|=DOCF_BWD_MOVEMENT;
	y+=s->page_len-doc_e->page_line_num;
	doc->page_line_num=0;
	goto rc_start_of_line;
      case DOCT_CURSOR:
	if (!find_cursor && !(doc->flags & DOCF_NO_CURSOR)) {
	  doc->cur_entry=doc_e->next;
	  doc->cur_col=doc->cur_entry->min_col;
	}
	if (doc_e!=added_cursor)
	  del_doc_e=TRUE;
	break;
      case DOCT_PMT:
	cur_u8_attr=cur_u8_attr&0xF0|DOC_COLOR_PMT;
	if (y==cursor_y) {
	  doc->cur_entry=doc_e->next;
	  doc->cur_col=doc->cur_entry->min_col;
	}
	break;
      case DOCT_CLEAR:
	next_clear_found=doc_e;
	if (doc_e->de_flags&DOCEF_HOLD)
	  clear_holds=TRUE;
	else
	  clear_holds=FALSE;
	break;
      case DOCT_PAGE_LEN:
	s->page_len=doc_e->attr;
	if (doc_e->de_flags & DOCEF_WIN_REL)
	  s->page_len+=height;
	goto rc_adjust_xy;
      case DOCT_LEFT_MARGIN:
	i=doc_e->attr;
	left_margin=left+i;
	s->left_margin=i;
	goto rc_start_of_line;
      case DOCT_RIGHT_MARGIN:
	if (doc_e->de_flags & DOCEF_WIN_REL)
	  i=width-1-doc_e->attr;
	else
	  i=doc_e->attr;
	right_margin=left+i;
	s->right_margin=i;
	goto rc_adjust_xy;
      case DOCT_HEADER:
	s->header=doc_e->attr;
	goto rc_adjust_xy;
      case DOCT_FOOTER:
	s->footer=doc_e->attr;
	goto rc_adjust_xy;
      case DOCT_INDENT:
	if (doc_e->de_flags & DOCEF_LEFT_X)
	  i=doc_e->attr;
	else
	  i=s->indent+doc_e->attr;
	s->indent=i;
	goto rc_start_of_line;
      case DOCT_FOREGROUND:
	cur_u8_attr&=0xF0;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0x0F;
	else
	  cur_u8_attr|=doc_e->attr;
	s->cur_text_attr=cur_u8_attr;
	break;
      case DOCT_BACKGROUND:
	cur_u8_attr&=0x0F;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF0;
	else
	  cur_u8_attr|=doc_e->attr<<4;
	s->cur_text_attr=cur_u8_attr;
	break;
      case DOCT_DFT_FOREGROUND:
	cur_u8_attr&=0xF0;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF;
	else
	  cur_u8_attr|=doc_e->attr;
	s->dft_text_attr=s->dft_text_attr&0xF0|cur_u8_attr&0x0F;
	s->cur_text_attr=cur_u8_attr;
	break;
      case DOCT_DFT_BACKGROUND:
	cur_u8_attr&=0x0F;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF0;
	else
	  cur_u8_attr|=doc_e->attr<<4;
	s->dft_text_attr=s->dft_text_attr&0x0F|cur_u8_attr&0xF0;
	s->cur_text_attr=cur_u8_attr;
	break;
      case DOCT_WORD_WRAP:
	if (doc_e->attr)
	  doc->flags|=DOCF_WORD_WRAP;
	else
	  doc->flags&=~DOCF_WORD_WRAP;
	break;
      case DOCT_HIGHLIGHT:
	if (doc_e->attr)
	  doc->flags|=DOCF_HIGHLIGHT;
	else
	  doc->flags&=~DOCF_HIGHLIGHT;
	break;
      case DOCT_BLINK:
	if (doc_e->attr)
	  doc->flags|=DOCF_BLINK;
	else
	  doc->flags&=~DOCF_BLINK;
	break;
      case DOCT_INVERT:
	if (doc_e->attr)
	  doc->flags|=DOCF_INVERT;
	else
	  doc->flags&=~DOCF_INVERT;
	break;
      case DOCT_UNDERLINE:
	if (doc_e->attr)
	  doc->flags|=DOCF_UNDERLINE;
	else
	  doc->flags&=~DOCF_UNDERLINE;
	break;
      case DOCT_SHIFTED_X:
	s->shifted_x=doc_e->attr;
	break;
      case DOCT_SHIFTED_Y:
	s->shifted_y=doc_e->attr;
	break;
      case DOCT_CURSOR_MOVEMENT:
	doc->flags|=DOCF_BWD_MOVEMENT;
	x+=doc_e->cursor_x_offset;
	if (doc_e->de_flags & DOCEF_PAGE_REL_Y) {
	  i=doc->page_line_num;
	  if (doc_e->de_flags & DOCEF_TOP_Y)
	    doc->page_line_num=0;
	  else if (doc_e->de_flags & DOCEF_BOTTOM_Y)
	    doc->page_line_num=s->page_len-1;
	  else if (doc_e->de_flags & DOCEF_CENTER_Y)
	    doc->page_line_num=s->page_len>>1;
	  y+=doc->page_line_num-i;
	}
	y+=doc_e->cursor_y_offset;
	doc->page_line_num+=doc_e->cursor_y_offset;
	goto rc_adjust_xy;
      case DOCT_SPRITE:
	if (!doc_e->bin_data && doc->flags&DOCEF_HAS_BIN)
	  doc_e->bin_data=DocBinFindNum(doc,doc_e->bin_num);
	if ((tmpb=doc_e->bin_data) &&
	      !tmpb->tag && doc_e->tag && *doc_e->tag)
	  tmpb->tag=StrNew(doc_e->tag,mem_task);
	if (tmpb && dc) {
	  DCRst(dc);
	  dc->flags&=~(DCF_DONT_DRAW|DCF_LOCATE_NEAREST);
	  if (recalc_flags&RECALCG_MASK!=RECALCt_TO_SCRN ||
		doc_e->de_flags&DOCEF_DONT_DRAW)
	    dc->flags|=DCF_DONT_DRAW;
	  bptr=tmpb->data;
	  ii=SpriteTypeMask(bptr);
	  if (ii&1<<SPT_TYPES_NUM) {
	    bptr=gr.empty_sprite;
	    ii=SpriteTypeMask(bptr);
	  }
	  if (ii&(1<<SPT_FLOOD_FILL|1<<SPT_FLOOD_FILL_NOT))
	    i=cur_u8_attr>>4 &0xF ^ win_task->text_attr>>4 & 0xF;
	  else {
	    i=tmp_u32_attr>>12&0xF ^ win_task->text_attr>>4 & 0xF;
	    if (tmp_u32_attr & DOCET_SEL)
	      i^=0xF;
	    if (tmp_u32_attr & DOCET_INVERT)
	      i^=0xF;
	    if (blink_flag &&
		  (doc_e==doc->cur_entry || tmp_u32_attr&DOCET_BLINK))
	      i^=0xF;
	  }
	  dc->color=i;
	  if (find_cursor)
	    dc->flags|=DCF_LOCATE_NEAREST;
	  dc->cur_x=(doc->x-x0)*FONT_WIDTH+pix_left+scroll_x;
	  dc->cur_y=(doc->y-y0)*FONT_HEIGHT+pix_top+scroll_y;
	  dc->cur_z=0;
	  dc->bkcolor=i;
	  if (doc_e->de_flags & DOCEF_FROM_START) {
	    xx=(x-k-x0)*FONT_WIDTH; //TODO: scrolling text is not length k
	    yy=(y-y0)*FONT_HEIGHT;
	    zz=0;
	  } else {
	    xx=(x-x0)*FONT_WIDTH;
	    yy=(y-y0)*FONT_HEIGHT;
	    zz=0;
	  }
	  if (ii&(1<<SPT_MESH|1<<SPT_SHIFTABLE_MESH)) {
	    if (!depth_buf) {
	      DCDepthBufAlloc(dc);
	      depth_buf=dc->depth_buf;
	    } else
	      dc->depth_buf=depth_buf;
	    Mat4x4IdentEqu(dc->r);
	    Mat4x4RotZ(dc->r,cur_time*3.1);
	    Mat4x4RotY(dc->r,cur_time*1.9);
	    Mat4x4RotX(dc->r,cur_time);
	    dc->flags|=DCF_TRANSFORMATION;
	    dc->x=xx;
	    dc->y=yy;
	    dc->z=GR_Z_ALL;
	    xx=0; yy=0; zz=0;
	  }
	  Sprite3(dc,xx,yy,zz,bptr);
	  dc->depth_buf=NULL;
	  dc->flags&=~(DCF_LOCATE_NEAREST|DCF_DONT_DRAW|DCF_TRANSFORMATION);
	  if (dc->nearest_dist<=D) {
	    D=dc->nearest_dist;
	    col=doc_e->min_col;
	  }
	}
	break;
      case DOCT_SONG:
	if (sys_focus_task==win_task &&
	      recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
	      !(doc_e->de_flags&DOCEF_DONT_DRAW)) {
	  if (doc_e->aux_str &&
		(!music.cur_song || StrCmp(music.cur_song,doc_e->aux_str))) {
	    Free(music.cur_song);
	    MusicSettingsRst;
	    music.cur_song=AStrNew(doc_e->aux_str);
	  }
	}
	doc->flags|=DOCF_HAS_SONG;
	break;
      case DOCT_HTML_CODE:
	if (recalc_flags&RECALCF_TO_HTML &&
	      doc_e->de_flags&DOCEF_TAG && doc_e->tag)
	  x-=StrLen(doc_e->tag);
	break;
      case DOCT_TYPES_NUM-1: //nobound switch
      default:
	break;
    }

    if (doc_e->de_flags & DOCEF_HAS_BORDER)
      TextBorder(win_task,doc_e->x-x0,x-x0-1,doc_e->y-y0,y-y0,
	    tmp_u32_attr.u8[1],ToBool(doc_e->de_flags & DOCEF_SOLID_BORDER));
    if (full_refresh) {
      switch (doc_e->type_u8) {
	case DOCT_CHECK_BOX:
	  doc_e->max_col=2;
	  break;
	case DOCT_LST:
	case DOCT_TREE:
	case DOCT_BTTN:
	case DOCT_LINK:
	case DOCT_MENU_VAL:
	case DOCT_MACRO:
	  doc_e->max_col=1;
	  break;
	default:
	  if (doc_e->de_flags & (DOCEF_TREE|DOCEF_LST))
	    doc_e->max_col=1;
	  else
	    doc_e->max_col=col2;
      }

      if (x>doc->max_x) doc->max_x=x;
      if (y>doc->max_y) doc->max_y=y;
      if (D<=best_d && !(doc_e->de_flags&DOCEF_NO_CLICK_ON)) {
	best_d=D;
	best_doc_e=doc_e;
	best_col=col;
      }
      if (doc_e->de_flags & DOCEF_TREE) {
	if (doc_e->de_flags & DOCEF_CHECKED_COLLAPSED)
	  tree_collapsed=TRUE;
	else
	  tree_collapsed=FALSE;
	doc_e2=doc_e->next;
	while (doc_e2!=doc && doc_e2->type_u8!=DOCT_INDENT &&
	      !(doc_e2->de_flags & DOCEF_TREE))
	  doc_e2=doc_e2->next;
	if (doc_e2->type_u8==DOCT_INDENT) {
	  j=i=s->indent;
	  do {
	    if (tree_collapsed)
	      doc_e2->de_flags|=DOCEF_SKIP;
	    else
	      doc_e2->de_flags&=~DOCEF_SKIP;
	    if (doc_e2->type_u8==DOCT_INDENT) {
	      if (doc_e2->de_flags & DOCEF_LEFT_X)
		j=doc_e2->attr;
	      else
		j+=doc_e2->attr;
	    }
	    doc_e2=doc_e2->next;
	  } while (doc_e2!=doc && j>i);
	}
      }
    }

    doc_e2=doc_e->next;
rc_skip:
    while (doc_e2!=doc && doc_e2->de_flags&(DOCEF_SKIP|DOCEF_FILTER_SKIP)) {
      if (doc_e2==doc->cur_entry) {
	doc->cur_entry=doc_e2->next;
	doc->cur_col=doc->cur_entry->min_col;
      }
      if (full_refresh) {
	doc_e2->x=x;
	doc_e2->y=y;
	doc_e2->page_line_num=doc->page_line_num;
	MemCpy(&doc_e2->settings,s,sizeof(CDocSettings));
	doc_e2->type.u8[1]=cur_u8_attr;
	doc_e2->de_flags=doc->flags
	      &(DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT) |
	      doc_e2->de_flags&~(DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT);
      }
      doc_e2=doc_e2->next;
    }

    if (full_refresh) {
      if (del_doc_e) {
	if (!(doc_e->de_flags & (DOCEF_HOLD|DOCEF_FILTER_SKIP))) {
	  if (doc_e==doc->cur_entry) {
	    doc->cur_entry=doc_e2;
	    doc->cur_col=doc_e2->min_col;
	  }
	  if (best_doc_e==doc_e) {
	    best_doc_e=doc_e2;
	    best_col=doc_e2->min_col;  //TODO: might be bug
	  }
	  DocEntryDel(doc,doc_e);
	}
      }
    }
    num_entries++;
    if (!full_refresh && doc_e->y>y_plot_bottom)
      break;
    doc_e=doc_e2;
  }

  if (full_refresh) {
    if (doc->cur_entry==doc && recalc_flags&RECALCF_ADD_CURSOR) {
      doc_e2=DocEntryNewBase(doc,DOCT_CURSOR,,x,y,doc->page_line_num);
      MemCpy(&doc_e2->settings,s,sizeof(CDocSettings));
      QueInsRev(doc_e2,doc);
    }

    if (doc->min_x>doc->max_x) {
      doc->max_x=0;
      doc->min_x=0;
    }
    if (doc->min_y>doc->max_y) {
      doc->max_y=0;
      doc->min_y=0;
    }

    //Update header
    if (!skipped_update) {
      doc_e->x=x;
      doc_e->y=y;
      doc_e->page_line_num=doc->page_line_num;
      MemCpy(&doc_e->settings,s,sizeof(CDocSettings));
      doc_e->type.u8[1]=cur_u8_attr;
      if (find_cursor) {
	D=DocCharDist(doc,x,y);
	if (D<best_d && !(doc_e->de_flags&DOCEF_NO_CLICK_ON)) {
	  best_d=D;
	  best_doc_e=doc_e;
	  best_col=0;
	}
      }
    }
    if (doc->flags & DOCF_SIZE_MIN) {
      if (Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	if (left<0)
	  left=0;
	i=left+doc->max_x-doc->min_x;
	if (i>TEXT_COLS-1)
	  i=TEXT_COLS-1;
	WinHorz(left,i,win_task);
	if (top<0)
	  top=0;
	i=top+doc->max_y-doc->min_y;
	if (i>TEXT_ROWS-1)
	  i=TEXT_ROWS-1;
	WinVert(top,i,win_task);
      } else {
	if (left<1)
	  left=1;
	i=left+doc->max_x-doc->min_x;
	if (i>TEXT_COLS-2)
	  i=TEXT_COLS-2;
	WinHorz(left,i,win_task);
	if (top<1)
	  top=1;
	i=top+doc->max_y-doc->min_y;
	if (i>TEXT_ROWS-2)
	  i=TEXT_ROWS-2;
	WinVert(top,i,win_task);
      }
    }
    if (find_cursor) {
      doc->cur_entry=best_doc_e;
      doc->cur_col=best_col;
      DocFormBwd(doc);
//We need this because text coordinates are used
      if (best_d<FONT_WIDTH)
	best_d=0;
      doc->best_d=best_d;
    }

    if (doc->cur_entry->type_u8!=DOCT_HEX_ED) {
      doc->y=doc->cur_entry->y;
      doc->x=doc->cur_entry->x+doc->cur_col;
    } else {
      doc->y=doc->cur_entry->y+doc->cur_col/3/doc->cur_entry->hex_ed_width;
      x=doc->cur_col%(doc->cur_entry->hex_ed_width*3);
      i=x/doc->cur_entry->hex_ed_width;
      doc->x=doc->cur_entry->x+9;
      if (i<2)
	doc->x+=x>>1*3+x&1;
      else
	doc->x+=doc->cur_entry->hex_ed_width*3+
	      (x-doc->cur_entry->hex_ed_width<<1);
    }
    doc->line=doc->y+1;
    doc->col=doc->x+1;

    if (recalc_flags&RECALCF_HAS_CURSOR) {
      if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN) {
	x=0;
	y=0;
      } else {
	x=scroll_x/FONT_WIDTH;
	y=scroll_y/FONT_HEIGHT;
      }
      if (doc->top_line_num-y+height-1>doc->max_y)
	doc->top_line_num=doc->max_y-(height-1)+y;
      if (doc->top_line_num-y<doc->min_y)
	doc->top_line_num=doc->min_y+y;

      if (doc->y-doc->top_line_num+y>height-1)
	doc->top_line_num=doc->y-(height-1)+y;
      if (doc->y-doc->top_line_num+y<0)
	doc->top_line_num=doc->y+y;

      if (doc->line_start_col-x+width-1>doc->max_x)
	doc->line_start_col=doc->max_x-(width-1)+x;
      if (doc->line_start_col-x<doc->min_x)
	doc->line_start_col=doc->min_x+x;

      if (doc->x-doc->line_start_col+x>width-1)
	doc->line_start_col=doc->x-(width-1)+x;
      if (doc->x-doc->line_start_col+x<0)
	doc->line_start_col=doc->x+x;
    }
  }
  if (recalc_flags&RECALCG_MASK==RECALCt_TO_SCRN &&
	recalc_flags&RECALCF_HAS_CURSOR) {
    x=doc->x-doc->line_start_col+left +scroll_x/FONT_WIDTH;
    y=doc->y-doc->top_line_num+top+scroll_y/FONT_HEIGHT;
    if (0<=x<=right && 0<=y<=bottom &&
	  x<TEXT_COLS && y<TEXT_ROWS &&
	  !(doc->flags&DOCF_HIDE_CURSOR)) {
      u32_ptr=gr.text_base+y*TEXT_COLS+x;
      *u32_ptr|=DOCET_BLINK;
      *u32_ptr^=0xFF00;
    }
    if (full_refresh) {
      if (!(doc->flags&DOCF_NO_SCROLL_BARS)) {
	if (!Bt(&hss->flags,WSSf_SET_TO_POS)) {
	  hss->min=doc->min_x;
	  if (doc->max_x-width+1<hss->min)
	    hss->max=hss->min;
	  else
	    hss->max=doc->max_x-width+1;
	  hss->pos=doc->line_start_col;
	}
	if (!Bt(&vss->flags,WSSf_SET_TO_POS)) {
	  vss->min=doc->min_y;
	  if (doc->max_y-height+1<vss->min)
	    vss->max=vss->min;
	  else
	    vss->max=doc->max_y-height+1;
	  vss->pos=doc->top_line_num;
	}
      }
      LBEqu(&doc->flags,DOCf_MORE,more);
    }
  }
  if (!same_win) {
    doc->old_win_top=top;
    doc->old_win_bottom=bottom;
    doc->old_win_left=left;
    doc->old_win_right=right;
    doc->old_cur_entry=doc->cur_entry;
    doc->old_cur_col=doc->old_cur_col;
  }
  if (doc->flags & DOCF_HAS_SONG)
    LBts(&win_task->task_flags,TASKf_HAS_SONG);
  if (full_refresh) {
    i=num_entries-doc->max_entries;
    if (next_clear_found) {
      DocDelToEntry(doc,next_clear_found,clear_holds);
      DocRecalc(doc,recalc_flags);
    } else if (i>1024) {
      DocDelToNum(doc,i);
      DocRecalc(doc,recalc_flags);
    }
  }
  DCDel(dc);
  Free(depth_buf);
  if (unlock)
    DocUnlock(doc);
  return TRUE;
}
