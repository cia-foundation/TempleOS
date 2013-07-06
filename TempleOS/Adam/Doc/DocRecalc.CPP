I64 DocCharDist(CDoc *doc,I64 x,I64 y)
{
#assert FONT_WIDTH==FONT_HEIGHT
  return (SqrI64(doc->x-x)+SqrI64(doc->y-y))*FONT_WIDTH*FONT_WIDTH;
}

U0 LockToPage(CDoc *doc)
{
  I64 win_page_len;
  CDocEntry *doc_e;
  CDocSettings *s;
  doc_e=doc->cur_entry;
  if (doc_e->flags1 & DOCEF1_LOCK_TO_PAGE)  {
    doc->cur_top_line_num=doc_e->y-doc_e->page_line_num;
    s=DocSettingsLast(doc_e);
    win_page_len=doc->win_task->win_height;
    if (doc_e->page_line_num>=win_page_len)
      doc->cur_top_line_num+=s->page_len-win_page_len;
  }
}

public U0 DocCenter(CDoc *doc=NULL,I64 recalc=DOC_RECALC_ALL)
{
  Bool unlock;
  CTask *task;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  task=doc->win_task;
  DocRecalc(doc,FALSE,recalc);
  if (!(doc->flags&DOCF_BORDER_DOC))
    doc->cur_top_line_num=doc->y-(task->win_height+1)>>1;
  LockToPage(doc);
  if (unlock)
    DocUnlock(doc);
}

U0 DocDelToNum(CDoc *doc,I64 num)
{
  CDocEntry *doc_e=doc->root.next,*doc_e2;
  while (num-->0 && doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->flags1 & (DOCEF1_HOLD|DOCEF1_FILTER_SKIP))) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_data_col=doc_e2->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    doc_e=doc_e2;
  }
}

U0 DocDelToEntry(CDoc *doc,CDocEntry *clear_entry)
{
  CDocEntry *doc_e=doc->root.next,*doc_e2;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (!(doc_e->flags1 & (DOCEF1_HOLD|DOCEF1_FILTER_SKIP))) {
      if (doc_e==doc->cur_entry) {
	doc->cur_entry=doc_e2;
	doc->cur_data_col=doc_e2->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    if (doc_e==clear_entry)
      break;
    doc_e=doc_e2;
  }
}

U0 DocBorderLstDraw(CDoc *doc)
{
  CTask *win_task=doc->win_task;
  I64 i,y=-1,attr=win_task->border_attr<<8;
  CDoc *templ=doc;
  U8 *ptr;
  CD3I64 saved_scroll;
  Bool old_preempt=Preempt(OFF);
  while (templ && templ->doc_signature==DOC_SIGNATURE_VAL) {
    y+=StrLen(templ->desc)+1;
    templ=templ->parent_doc;
  }
  templ=doc;
  if (y>win_task->win_height)
    y=win_task->win_height;
  WinScrollNull(win_task,&saved_scroll);
  while (templ && templ->doc_signature==DOC_SIGNATURE_VAL) {
    ptr=templ->desc;
    i=StrLen(templ->desc);
    ptr+=i;
    attr=win_task->border_attr<<8;
    while (i-- && y>0)
      PlotBorderU8(win_task,attr+*--ptr,-1,--y);
    y--;
    templ=templ->parent_doc;
  }
  WinScrollRestore(win_task,&saved_scroll);
  Preempt(old_preempt);
}

U8 doc_spaces[9]=
{CH_SPACE,CH_SPACE,CH_SPACE,CH_SPACE,CH_SPACE,CH_SPACE,CH_SPACE,CH_SPACE,0};

Bool DocRecalc(CDoc *doc,Bool has_cursor=FALSE,I64 recalc=DOC_RECALC_ALL)
{
  I64 i,ii,j,k,x,x0,y,y0,D,d2,col,col2,best_col=0,best_d=MAX_I64,xx,yy,zz,
	num_entries=0,ijif,cur_u8_attr,temp_u32_attr,
	hide_collapsed[2],
	cursor_y=MIN_I64,left_margin,right_margin,
	y_plot_top,y_plot_bottom,
	top,left,bottom,right,width,height,scroll_x,scroll_y,
	pixel_top,pixel_left;
  CDocEntry reg *doc_e,reg *doc_e2,*best_do_e,*alias,*next_clear_found=NULL;
  CDocBin *tempb;
  CDocSettings *s;
  Bool del_cl,skipped_update,tree_collapsed,
	resize=FALSE,same_win,more=FALSE,find_cursor=FALSE,
	blink_flag,full_refresh=TRUE,unlock;
  CTask *win_task,*mem_task;
  CDC *dc;
  U8 *bptr,*ptr,buf[136],ch;
  U32 *u32_ptr,*hl;
  I32 *depth_buf=NULL;
  F64 cur_time=tT;
  CWinScroll *vss,*hss;
  CHashDefineStr *temph;

  if (!doc || doc->doc_signature!=DOC_SIGNATURE_VAL) return FALSE;

  if (recalc==DOC_RECALC_TO_SCREEN && doc->owning_task!=Fs) {
    ijif=sys_jiffies+JIFFY_FREQ/240;
    while (Bt(&doc->locked_flags,DOClf_LOCKED)) {
      if (sys_jiffies>=ijif)
	return FALSE;
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

  nobound_switch (recalc) {
    case DOC_RECALC_FIND_CURSOR:
      if (win_task)
	dc=DCAlias(gr_dc2,win_task);
      find_cursor=TRUE;
    case DOC_RECALC_ALL:
      doc->recalc_start=doc;
      break;
    case DOC_RECALC_TO_SCREEN:
      if (doc->refresh_cnt++%(ToI64(win_actual_refresh/10)+1) &&
	    !Bt(&doc->flags,DOCf_DO_FULL_REFRESH) &&
	    !(doc->flags&DOCF_BACKWARD_MOVEMENT))
	full_refresh=FALSE;
      doc->recalc_start=doc;
      if (win_task)
	dc=DCAlias(gr_dc2,win_task);
      break;
    case DOC_RECALC_PARTIAL:
      DocRemSoftNewLines(doc,doc->recalc_start);
      break;
  }

  PushFD;
  Cli;
  left	=win_task->win_left;
  right =win_task->win_right;
  width =win_task->win_width;
  top	=win_task->win_top;
  scroll_x=win_task->win_scroll_x;
  scroll_y=win_task->win_scroll_y;
  bottom=win_task->win_bottom;
  height=win_task->win_height;
  pixel_left  =win_task->win_pixel_left;
  pixel_top   =win_task->win_pixel_top;
  left_margin=left;
  right_margin=right;
  PopFD;
  if (doc->flags&DOCF_BORDER_DOC) {
    scroll_x=0;
    scroll_y=0;
  }
  best_do_e=doc->cur_entry;

  if (!(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)) &&
	FilesFindMatch(FILENAME_MASK_SRC,doc->filename.name))
    doc->flags|=DOCF_HIGHLIGHT;
  else
    doc->flags&=~DOCF_HIGHLIGHT;

  x=doc->start_x;
  y=doc->start_y;
  doc->page_line_num=0;
  if (full_refresh) {
    if (!find_cursor) {
      doc->x=x;
      doc->y=y;
    }
    if (LBtr(&win_task->display_flags,DISPLAYf_HAS_BEEN_RESIZED))
      resize=TRUE;
  }

  hss=&win_task->horz_scroll;
  vss=&win_task->vert_scroll;
  if (doc->flags&DOCF_BORDER_DOC) {
    doc->cur_top_line_num=0;
    doc->line_start_col=0;
    has_cursor=FALSE;
    doc->settings_root.cur_text_attr=doc->settings_root.dft_text_attr=win_task->border_attr;
  } else {
    if (has_cursor && full_refresh) {
      if (Bt(&hss->flags,WSSf_SET_TO_POS)||Bt(&vss->flags,WSSf_SET_TO_POS)) {
	if (!(doc->flags&DOCF_NO_SCROLL_BARS)) {
	  if (Bt(&hss->flags,WSSf_SET_TO_POS)) {
	    doc->line_start_col=hss->pos;
	    LBtr(&hss->flags,WSSf_SET_TO_POS);
	  }
	  if (Bt(&vss->flags,WSSf_SET_TO_POS)) {
	    doc->cur_top_line_num=vss->pos;
	    LBtr(&vss->flags,WSSf_SET_TO_POS);
	  }
	}
	doc->x=doc->line_start_col+width/2;
	doc->y=doc->cur_top_line_num+height/2;
	find_cursor=TRUE;
	doc->recalc_start=doc;
      }
    }
    doc->settings_root.cur_text_attr=doc->settings_root.dft_text_attr=win_task->text_attr;
  }
  x0=doc->line_start_col;
  y0=doc->cur_top_line_num;
  same_win=top	 ==doc->old_win_top &&
	bottom==doc->old_win_bottom &&
	left  ==doc->old_win_left &&
	right ==doc->old_win_right &&
	doc->cur_entry==doc->old_cur_entry &&
	doc->cur_data_col==doc->old_cur_data_col;
  if (recalc==DOC_RECALC_TO_SCREEN) {
    y_plot_top=y0-scroll_y/FONT_HEIGHT;
    y_plot_bottom=y0+height-1-scroll_y/FONT_HEIGHT;
    if (!(doc->flags&DOCF_BORDER_DOC) && !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
      DocBorderLstDraw(doc);
  }

  if (doc->cur_data_col<=doc->cur_entry->min_col)
    doc->cur_data_col=doc->cur_entry->min_col;
  if (doc->recalc_start==doc) {
    doc_e=doc->root.next;
    doc_e->flags1&=~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER);
  } else if (!doc->recalc_start)
    doc_e=doc; //skip update
  else {
    doc_e=doc->recalc_start;
    i=doc->ww_lines_back;
    ii=i/2;  //Threshold for stopping on SOFT_CR
    while (i-- && doc_e->type_u8!=DOCT_NEW_LINE &&
	  (i>ii || doc_e->type_u8!=DOCT_SOFT_NEW_LINE) &&
	  doc_e->flags1 & DOCEF1_WORD_WRAP &&
	  doc_e->last!=doc)
      doc_e=doc_e->last;
    x=doc_e->x; y=doc_e->y;
    doc->page_line_num=doc_e->page_line_num;
  }
  if (doc_e==doc->root.next)
    s=&doc->settings_root;
  else
    s=DocSettingsLast(doc_e);
  doc->flags=doc_e->flags1& (DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER) |
	doc->flags&~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER);
  cur_u8_attr=s->cur_text_attr;
  if (doc_e==doc->root.next) {
    doc->flags&=~DOCF_BACKWARD_MOVEMENT;
    if (recalc==DOC_RECALC_TO_SCREEN && full_refresh)
      doc->flags&=~DOCF_HAS_SONG;
  } else
    doc->flags=doc_e->flags1& DOCEF1_HIGHLIGHT |
	  doc->flags&~DOCEF1_HIGHLIGHT;

  if (doc->root.next==doc) {
    best_do_e=doc;
    best_col=0;
    doc->cur_entry=doc;
    doc->cur_data_col=0;
    doc_e=doc;
  }
  skipped_update= doc_e==doc && doc->root.next!=doc;

  if (full_refresh) {
    DocSettlingsRemainingDel(doc,doc_e,s->next);
    doc->min_x=MAX_I32; doc->min_y=MAX_I32;
    doc->max_x=MIN_I32; doc->max_y=MIN_I32;
  }
  while (doc_e!=doc) {
restart:
    del_cl=FALSE;
    if (doc_e->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)) {
      doc_e2=doc_e;
      goto skip;
    }
    if (!full_refresh && doc_e->settings)
      s=doc_e->settings;
    if (doc_e->flags1 & (DOCEF1_TAG_CB|DOCEF1_DEFINE)) {
      Free(doc_e->tag);
      if (doc_e->flags1 & DOCEF1_TAG_CB) {
	if (doc_e->tag_cb)
	  doc_e->tag=(*doc_e->tag_cb)(doc,doc_e,mem_task);
	else
	  doc_e->tag=StrNew("",mem_task);
      } else {
	if (temph=HashFind(doc_e->define_str,win_task->hash_table,HTT_DEFINE_STR))
	  doc_e->tag=StrNew(temph->data,mem_task);
	else
	  doc_e->tag=CAlloc(1,mem_task);
      }
      doc_e->max_col=StrLen(doc_e->tag);
      if (doc->cur_entry==doc_e) {
	if (doc->cur_data_col>=doc_e->max_col) {
	  if (doc_e->max_col)
	    doc->cur_data_col=doc_e->max_col-1;
	  else
	    doc->cur_data_col=0;
	}
      }
    }
    if (Bt(doc_tag_types,doc_e->type_u8))
      k=StrLen(doc_e->tag);
    else
      k=0;
    if (full_refresh) {
word_wrap_start:
      doc_e2=doc_e->next;
      if (doc_e2->type_u8==DOCT_SOFT_NEW_LINE) {
	if (!same_win) {
	  if (doc->cur_entry==doc_e2) {
	    doc->cur_entry=doc_e2->next;
	    doc->cur_data_col=doc->cur_entry->min_col;
	  }
	  if (best_do_e==doc_e2) {
	    best_do_e=doc_e2->next;
	    best_col=0;
	  }
	  DocEntryDel(doc,doc_e2);
	  goto word_wrap_start;
	}
      }
      if (doc_e->type_u8==DOCT_TEXT &&
	    !(doc_e->flags1&DOCEG1_DONT_EDIT) &&
	    !(doc_e->flags2&DOCEG2_DONT_EDIT)) {
	if (doc_e->flags1==doc_e2->flags1 && doc_e->flags2==doc_e2->flags2 &&
	      doc_e->type==doc_e2->type) {
	  j=StrLen(doc_e2->tag);
	  ptr=MAlloc(k+j+1,mem_task);
	  MemCpy(ptr,doc_e->tag,k);
	  MemCpy(ptr+k,doc_e2->tag,j+1);
	  Free(doc_e->tag);
	  doc_e->tag=ptr;
	  if (doc->cur_entry==doc_e2) {
	    doc->cur_entry=doc_e;
	    doc->cur_data_col+=k;
	  }
	  if (best_do_e==doc_e2) {
	    best_do_e=doc_e;
	    best_col=0;
	  }
	  DocEntryDel(doc,doc_e2);
	  k+=j;
	  if (k>(right_margin-left_margin+1)<<1)
	    goto word_wrap2;
	  else
	    goto word_wrap_start;
	}
      }
    }
word_wrap2:
    if (doc_e->flags1 & DOCEF1_SCROLLING_X)
      k=doc_e->scroll_len;
    if (doc_e->flags1 &
	  (DOCEF1_LEFT_X|DOCEF1_RIGHT_X|DOCEF1_CENTER_X|
	  DOCEF1_TOP_Y|DOCEF1_BOTTOM_Y|DOCEF1_CENTER_Y)) {
      if (doc_e->flags1 & DOCEF1_MARGIN_REL_X) {
	if (doc_e->flags1 & DOCEF1_LEFT_X)
	  x=left_margin-left;
	else if (doc_e->flags1 & DOCEF1_RIGHT_X)
	  x=right_margin-(k-1)-left;
	else if (doc_e->flags1 & DOCEF1_CENTER_X)
	  x=(right_margin+left_margin)>>1-k>>1-left;
      } else {
	if (doc_e->flags1 & DOCEF1_LEFT_X)
	  x=x0;
	else if (doc_e->flags1 & DOCEF1_RIGHT_X)
	  x=width+x0-k;
	else if (doc_e->flags1 & DOCEF1_CENTER_X)
	  x=(width+x0-k)>>1;
      }
      i=y;
      if (doc_e->flags1 & DOCEF1_PAGE_REL_Y) {
	doc->flags|=DOCF_BACKWARD_MOVEMENT;
	if (doc_e->flags1 & DOCEF1_TOP_Y)
	  y-=doc_e->page_line_num;
	else if (doc_e->flags1 & DOCEF1_BOTTOM_Y)
	  y+=s->page_len-doc_e->page_line_num;
	else if (doc_e->flags1 & DOCEF1_CENTER_Y)
	  y+=s->page_len>>1-doc_e->page_line_num;
      } else {
	doc->flags|=DOCF_BACKWARD_MOVEMENT;
	if (doc_e->flags1 & DOCEF1_TOP_Y)
	  y=y0;
	else if (doc_e->flags1 & DOCEF1_BOTTOM_Y)
	  y=height-1+y0;
	else if (doc_e->flags1 & DOCEF1_CENTER_Y)
	  y=height>>1+y0;
      }
      if (y!=i) {
	doc->page_line_num+=y-i;
	if (doc->page_line_num<0)
	  doc->page_line_num=s->page_len+doc->page_line_num%s->page_len;
	else
	  doc->page_line_num=doc->page_line_num%s->page_len;
	if (s->header!=DOC_DFT &&
	      doc->page_line_num<s->header) {
	  y+=s->header-doc->page_line_num;
	  doc->page_line_num=s->header;
	}
	if (s->footer==DOC_DFT) {
	  if (doc->page_line_num>=s->page_len) {
	    if (s->header==DOC_DFT)
	      doc->page_line_num=0;
	    else {
	      doc->page_line_num=s->header;
	      y+=s->header;
	    }
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
	  }
	}
      }
    }

    if (full_refresh && k>0) {
      if (doc->flags & DOCF_WORD_WRAP) {
	i=right_margin+1-(x+left);  //Space left on line
	ii=x+1-s->left_margin;
	if (doc_e->type_u8==DOCT_TEXT &&
	      !(doc_e->flags1&DOCEG1_DONT_EDIT) &&
	      !(doc_e->flags2&DOCEG2_DONT_EDIT)) {
	  if (doc->cur_entry==doc_e->next) {
	    if (doc->cur_data_col==doc_e->next->min_col)
	      i--;
	  } else {
	    if (doc->cur_entry==doc_e) {
	      if (doc->cur_data_col==i)
		i--;
	    }
	  }
	  if (k>i) {
	    for (j=i;j>8-ii && j>=0;j--)
	      if (doc_e->tag[j]==CH_SPACE ||
		    doc_e->tag[j]==CH_SHIFT_SPACE) {
		i=j+1;
		break;
	      }
	    if (k>i && i>0) {
	      doc_e2=MAllocIdentical(doc_e,mem_task);
	      doc_e2->tag=StrNew(doc_e->tag+i,mem_task);
	      QueIns(doc_e2,doc_e);
	      if (doc->cur_entry==doc_e && doc->cur_data_col>=i) {
		doc->cur_entry=doc_e2;
		doc->cur_data_col=doc->cur_data_col-i;
	      }
	      doc_e->tag[i]=0;
	      ptr=StrNew(doc_e->tag,mem_task);
	      Free(doc_e->tag);
	      doc_e->tag=ptr;
	      doc_e2=CAlloc(sizeof(CDocEntryBase),mem_task);
	      doc_e2->type=DOCT_SOFT_NEW_LINE | doc_e->type & 0xFFFFFF00;
	      doc_e2->flags1=doc_e->flags1;
	      doc_e2->flags2=doc_e->flags2;
	      doc_e2->x=x;
	      doc_e2->y=y;
	      doc_e2->page_line_num=doc_e->page_line_num;
	      doc_e2->settings=NULL;
	      QueIns(doc_e2,doc_e);
	      k=StrLen(doc_e->tag);
	      goto skip_soft_cr;
	    }
	  }
	  if (k==i)
	    goto skip_soft_cr;
	}
	if (k>=i) {
	  doc_e2=doc_e->last;
	  if (doc_e2->type_u8!=DOCT_SOFT_NEW_LINE &&
		doc_e2->type_u8!=DOCT_NEW_LINE &&
		doc_e2->type_u8!=DOCT_CURSOR_MOVEMENT) {
	    doc_e2=CAlloc(sizeof(CDocEntryBase),mem_task);
	    doc_e2->type=DOCT_SOFT_NEW_LINE | doc_e->type & 0xFFFFFF00;
	    doc_e2->flags1=DOCEF1_WORD_WRAP |
		  doc_e->flags1 & (DOCEF1_LOCK_TO_PAGE|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_CENTER|DOCG_BL_IV_UL|DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP);
	    doc_e2->x=x;
	    doc_e2->y=y;
	    doc_e2->page_line_num=doc_e->last->page_line_num;
	    doc_e2->settings=s;
	    QueIns(doc_e2,doc_e->last);
	    doc_e=doc_e2;
	    goto restart;
	  }
	}
skip_soft_cr:
      }
    }
    if (full_refresh) {
      doc_e->x=x;
      doc_e->y=y;
      doc_e->page_line_num=doc->page_line_num;
      doc_e->settings=s;
      if (x<doc->min_x) doc->min_x=x;
      if (y<doc->min_y) doc->min_y=y;

      if (find_cursor) {
	D=DocCharDist(doc,x,y);
	col=0;
      }
    }
    col2=0;

    doc_e->flags1=doc->flags& (DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER) |
	  doc_e->flags1&~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER);
    temp_u32_attr=(cur_u8_attr&0xF0)<<8|doc->flags&DOCG_BL_IV_UL|(s->shifted_x&0x1F)<<16|(s->shifted_y&0x1F)<<21;
    switch (doc_e->type_u8) {
      case DOCT_SPRITE:
	if (doc_e->flags1 & DOCEF1_LEFT_EXP)
	  temp_u32_attr.u8[1]|=cur_u8_attr&15;
	else if (doc_e->flags1 & DOCEF1_LINK)
	  temp_u32_attr.u8[1]|=DOC_COLOR_LINK;
	else if (doc_e->flags1 & DOCEF1_LEFT_MACRO)
	  temp_u32_attr.u8[1]|=DOC_COLOR_MACRO;
	else if (doc_e->flags1 & DOCEF1_TREE)
	  temp_u32_attr.u8[1]|=DOC_COLOR_TREE;
	else
	  temp_u32_attr.u8[1]|=DOC_COLOR_BIN;
	break;
      case DOCT_HTML_IMG:
      case DOCT_HTML_VIDEO:
      case DOCT_HTML_CODE:
	temp_u32_attr.u8[1]|=DOC_COLOR_BIN;
	break;
      case DOCT_LINK:
	temp_u32_attr.u8[1]|=DOC_COLOR_LINK;
	break;
      case DOCT_MACRO:
	temp_u32_attr.u8[1]|=DOC_COLOR_MACRO;
	break;
      case DOCT_ANCHOR:
	temp_u32_attr.u8[1]|=DOC_COLOR_ANCHOR;
	break;
      case DOCT_TREE:
	temp_u32_attr.u8[1]|=DOC_COLOR_TREE;
	break;
      case DOCT_HIDE_START:
      case DOCT_HIDE_END:
	temp_u32_attr.u8[1]|=DOC_COLOR_HIDDEN;
	break;
      default:
	temp_u32_attr.u8[1]|=cur_u8_attr&15;
    }
    doc_e->type.u8[1]=temp_u32_attr.u8[1];
    temp_u32_attr|=doc_e->type&0xF3FF0000;
    if (doc_e==doc->cur_entry) {
      if (doc_e->type_u8!=DOCT_TEXT)
	temp_u32_attr^=0xFF00;
      cursor_y=doc_e->y;
    }

    if (doc_e->flags1 & DOCEF1_REFRESH_DATA) {
      if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX)
	DocDataFmt(doc_e);
    }

    if (Bt(doc_tag_types,doc_e->type_u8)) {
      ptr=doc_e->tag;
      if (doc_e->flags1 & DOCEF1_TREE) {
	if (k>=2) {
	  if (doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED)
	    *ptr++='+';
	  else
	    *ptr++='-';
	  *ptr++=']';
	  ptr=doc_e->tag;
	}
      } else if (doc_e->flags1 & DOCEF1_HAS_BIN) {
	if (*ptr=='<') {
	  temp_u32_attr=temp_u32_attr & 0xFFFFF000 | DOC_COLOR_BIN<<8;
	  doc_e->type.u8[1]=temp_u32_attr.u8[1];
	  if (doc_e==doc->cur_entry && doc_e->type_u8!=DOCT_TEXT)
	    temp_u32_attr^=0x0F00;
	  if (full_refresh && '0'<=ptr[1]<='9') {
	    SPrintF(buf,"<%d>",doc_e->bin_num);
	    k=StrLen(buf);
	    Free(doc_e->tag);
	    ptr=doc_e->tag=MAlloc(k+1,mem_task);
	    MemCpy(ptr,buf,k+1);
	  }
	}
      } else if (doc_e->type_u8==DOCT_CHECK_BOX) {
	if (k>=3) {
	  *ptr++='[';
	  if (doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED)
	    *ptr++='X';
	  else
	    *ptr++=CH_SPACE;
	  *ptr++=']';
	  ptr=doc_e->tag;
	}
      } else if (doc_e->type_u8==DOCT_HIDE_START) {
	if (k>=2) {
	  *ptr++='<';
	  if (doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED)
	    *ptr++='+';
	  else
	    *ptr++='-';
	  ptr=doc_e->tag;
	}
      } else if (doc_e->type_u8==DOCT_HIDE_END) {
	if (k>=1)
	  *ptr='>';
      }

      if (doc_e->flags1 & DOCEF1_SCROLLING_X) {
	j=StrLen(doc_e->tag);
	if (j && doc_e->scroll_len) {
	  ijif=cur_time*FONT_WIDTH*DOC_SCROLL_SPEED;
	  ijif%=j*FONT_WIDTH;
	  temp_u32_attr=temp_u32_attr & 0xFFE0FF00|
		(FONT_WIDTH-1-ijif&(FONT_WIDTH-1))<<16;
#assert FONT_WIDTH==8
	  ijif>>=3;
	  for (k=0;k<doc_e->scroll_len;k++) {
	    ch=ptr[(ijif+k)%j];
	    if (!Bt(displayable_chars_bitmap,ch)) ch='.';
	    if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	      if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		PlotBorderU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
	      else
		PlotWinU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
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
	if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
	      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	  while (ch=*ptr++) {
	    if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW))
	      PlotBorderU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
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
	  if (doc_e->type_u8==DOCT_TEXT && doc_e->flags1&DOCEF1_HIGHLIGHT)
	    hl=DocHighlight(doc,full_refresh,doc_e,&s,ptr,k,temp_u32_attr);
	  else
	    hl=NULL;
	  if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	    if (y>y_plot_bottom) //Technically we should do this for scrolling_x, too.
	      more=TRUE;
	    else if (y>=y_plot_top) {
	      if (hl)
		PlotAttrStr2(win_task,hl,k,x-x0,y-y0);
	      else
		PlotWinStr2(win_task,ptr,k,temp_u32_attr,x-x0,y-y0);
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
    nobound_switch (doc_e->type_u8) {
      case DOCT_TEXT:
	if (!col2 &&
	      !(doc_e->flags1&(DOCEF1_TREE|DOCEF1_TAG_CB|DOCEF1_DEFINE|DOCEF1_HTML_LINK)) &&
	      !(doc_e->flags2&DOCEF2_BIN_PTR_LINK))
	  del_cl=TRUE;
	break;
      case DOCT_HEX_ED:
	if (doc_e->flags1 & DOCEF1_DATA_IS_PTR)
	  bptr=doc_e->data;
	else
	  bptr=&doc_e->data;
	k=doc_e->hex_ed_width; //columns
	for (i=0;i<doc_e->len;i+=k) {
	  if (doc_e->flags1 & DOCEF1_ZERO_BASED)
	    SPrintF(buf,"%08tX ",i);
	  else
	    SPrintF(buf,"%08tX ",bptr);
	  ptr=buf;
	  while (ch=*ptr++) {
	    if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	      if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		PlotBorderU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
	      else
		PlotWinU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
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
	    SPrintF(buf,"%02tX",*bptr++);
	    ptr=buf;
	    while (ch=*ptr++) {
	      if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
		if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
		      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		  PlotBorderU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
		else
		  PlotWinU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
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
	    if (!Bt(displayable_chars_bitmap,ch)) ch='.';
	    if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	      if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
		    !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER))
		PlotBorderU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
	      else
		PlotWinU8(win_task,temp_u32_attr+ch,x-x0,y-y0);
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
      case DOCT_TAB:
	k=(x+8) & ~7;
	if (doc_e->flags1 & DOCEF1_BORDER_PLOT &&
	      !Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	  while (x<k) {
	    if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW))
	      PlotBorderU8(win_task,temp_u32_attr+CH_SPACE,x-x0,y-y0);
	    if (find_cursor) {
	      d2=DocCharDist(doc,x,y);
	      if (d2<D)
		D=d2;
	    }
	    x++;
	  }
	} else {
	  k-=x;
	  if (recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	    if (y_plot_top<=y<=y_plot_bottom)
	      PlotWinStr2(win_task,doc_spaces,k,temp_u32_attr,x-x0,y-y0);
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
      case DOCT_NEW_LINE:
      case DOCT_SOFT_NEW_LINE:
	if (recalc==DOC_RECALC_TO_SCREEN &&
	      !(doc_e->flags1&DOCEF1_DONT_DRAW)&& y_plot_top<=y<=y_plot_bottom)
	  PlotWinAttr(win_task,width-(x-x0),
		cur_u8_attr<<8,x-x0,y-y0);
	if (doc_e->flags1&DOCEF1_HIGHLIGHT) {
	  if (s->state&7==DOCSS_CPP_Z_COMMENT) {
	    if (full_refresh) {
	      s=DocSettingsIdentical(doc,s);
	      s->state=DOCSS_NORMAL;
	      QueIns(s,doc->settings_root.last);
	    } else
	      s=DocSettingsNext(doc_e->next);
	  } else if (s->state&DOCSSF_BPT) {
	    if (full_refresh) {
	      s=DocSettingsIdentical(doc,s);
	      s->state&=~DOCSSF_BPT;
	      QueIns(s,doc->settings_root.last);
	    } else
	      s=DocSettingsNext(doc_e->next);
	  }
	}
	y++;
	doc->page_line_num++;
start_of_line:
	if (s->left_margin==DOC_DFT)
	  x=s->indent;
	else
	  x=s->indent+s->left_margin;
adjust_xy:
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
	    goto start_of_line;
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
	    goto start_of_line;
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
	    goto start_of_line;
	  }
	}
	break;
      case DOCT_CURSOR_MOVEMENT:
	doc->flags|=DOCF_BACKWARD_MOVEMENT;
	x+=doc_e->cursor_x_offset;
	if (doc_e->flags1 & DOCEF1_PAGE_REL_Y) {
	  i=doc->page_line_num;
	  if (doc_e->flags1 & DOCEF1_TOP_Y)
	    doc->page_line_num=0;
	  else if (doc_e->flags1 & DOCEF1_BOTTOM_Y)
	    doc->page_line_num=s->page_len-1;
	  else if (doc_e->flags1 & DOCEF1_CENTER_Y)
	    doc->page_line_num=s->page_len>>1;
	  y+=doc->page_line_num-i;
	}
	y+=doc_e->cursor_y_offset;
	doc->page_line_num+=doc_e->cursor_y_offset;
	goto adjust_xy;
      case DOCT_PAGE_BREAK:
	doc->flags|=DOCF_BACKWARD_MOVEMENT;
	y+=s->page_len-doc_e->page_line_num;
	doc->page_line_num=0;
	goto start_of_line;
      case DOCT_INDENT:
	if (full_refresh) {
	  if (doc_e->flags1 & DOCEF1_LEFT_X)
	    i=doc_e->attr;
	  else
	    i=s->indent+doc_e->attr;
	  s=DocSettingsIdentical(doc,s);
	  s->indent=i;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto start_of_line;
      case DOCT_PAGE_LEN:
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->page_len=doc_e->attr;
	  if (doc_e->flags1 & DOCEF1_WIN_REL)
	    s->page_len+=height;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto adjust_xy;
      case DOCT_LEFT_MARGIN:
	i=doc_e->attr;
	left_margin=left+i;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->left_margin=i;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto start_of_line;
      case DOCT_RIGHT_MARGIN:
	if (doc_e->flags1 & DOCEF1_WIN_REL)
	  i=width-1-doc_e->attr;
	else
	  i=doc_e->attr;
	right_margin=left+i;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->right_margin=i;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto adjust_xy;
      case DOCT_HEADER:
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->header=doc_e->attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto adjust_xy;
      case DOCT_FOOTER:
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->footer=doc_e->attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	goto adjust_xy;
      case DOCT_FOREGROUND:
	cur_u8_attr&=0xF0;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0x0F;
	else
	  cur_u8_attr|=doc_e->attr;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->cur_text_attr=cur_u8_attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_BACKGROUND:
	cur_u8_attr&=0x0F;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF0;
	else
	  cur_u8_attr|=doc_e->attr<<4;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->cur_text_attr=cur_u8_attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_DFT_FOREGROUND:
	cur_u8_attr&=0xF0;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF;
	else
	  cur_u8_attr|=doc_e->attr;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->dft_text_attr=s->dft_text_attr&0xF0|cur_u8_attr&0x0F;
	  s->cur_text_attr=cur_u8_attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_DFT_BACKGROUND:
	cur_u8_attr&=0x0F;
	if (doc_e->attr==DOC_DFT)
	  cur_u8_attr|=s->dft_text_attr&0xF0;
	else
	  cur_u8_attr|=doc_e->attr<<4;
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->dft_text_attr=s->dft_text_attr&0x0F|cur_u8_attr&0xF0;
	  s->cur_text_attr=cur_u8_attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_PMT:
	cur_u8_attr=cur_u8_attr&0xF0|DOC_COLOR_PMT;
	if (y==cursor_y) {
	  doc->cur_entry=doc_e->next;
	  doc->cur_data_col=doc->cur_entry->min_col;
	}
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
      case DOCT_LOCK_TO_PAGE:
	if (doc_e->attr)
	  doc->flags|=DOCF_LOCK_TO_PAGE;
	else
	  doc->flags&=~DOCF_LOCK_TO_PAGE;
	break;
      case DOCT_LOCK_TO_CENTER:
	if (doc_e->attr)
	  doc->flags|=DOCF_LOCK_TO_CENTER;
	else
	  doc->flags&=~DOCF_LOCK_TO_CENTER;
	break;
      case DOCT_UNDERLINE:
	if (doc_e->attr)
	  doc->flags|=DOCF_UNDERLINE;
	else
	  doc->flags&=~DOCF_UNDERLINE;
	break;
      case DOCT_INVERT:
	if (doc_e->attr)
	  doc->flags|=DOCF_INVERT;
	else
	  doc->flags&=~DOCF_INVERT;
	break;
      case DOCT_BLINK:
	if (doc_e->attr)
	  doc->flags|=DOCF_BLINK;
	else
	  doc->flags&=~DOCF_BLINK;
	break;
      case DOCT_SHIFTED_X:
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->shifted_x=doc_e->attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_SHIFTED_Y:
	if (full_refresh) {
	  s=DocSettingsIdentical(doc,s);
	  s->shifted_y=doc_e->attr;
	  QueIns(s,doc->settings_root.last);
	} else
	  s=DocSettingsNext(doc_e->next);
	break;
      case DOCT_CURSOR:
	if (!find_cursor && !(doc->flags & DOCF_NO_CURSOR)) {
	  doc->cur_entry=doc_e->next;
	  doc->cur_data_col=doc->cur_entry->min_col;
	}
	del_cl=TRUE;
	break;
      case DOCT_SPRITE:
	if (!doc_e->bin_data && doc->flags&DOCEF1_HAS_BIN)
	  doc_e->bin_data=DocBinFindNum(doc,doc_e->bin_num);
	if ((tempb=doc_e->bin_data) && !tempb->tag && doc_e->tag && *doc_e->tag)
	  tempb->tag=StrNew(doc_e->tag,mem_task);
	if (tempb && dc) {
	  DCRst(dc);
	  dc->flags&=~(DCF_DONT_DRAW|DCF_LOCATE_NEAREST);
	  if (recalc!=DOC_RECALC_TO_SCREEN || doc_e->flags1&DOCEF1_DONT_DRAW)
	    dc->flags|=DCF_DONT_DRAW;
	  bptr=tempb->data;
	  ii=SpriteTypeMask(bptr);
	  if (ii&1<<GRST_NUM_TYPES) {
	    bptr=gr_empty_sprite;
	    ii=SpriteTypeMask(bptr);
	  }
	  if (ii&(1<<GRST_FLOOD_FILL|1<<GRST_FLOOD_FILL_NOT))
	    i=cur_u8_attr>>4 &0xF ^ win_task->text_attr>>4 & 0xF;
	  else {
	    i=temp_u32_attr>>12&0xF ^ win_task->text_attr>>4 & 0xF;
	    if (temp_u32_attr & DOCET_SELECTED)
	      i^=0xF;
	    if (temp_u32_attr & DOCET_INVERT)
	      i^=0xF;
	    if (blink_flag && (doc_e==doc->cur_entry || temp_u32_attr&DOCET_BLINK))
	      i^=0xF;
	  }
	  dc->color=i;
	  if (find_cursor)
	    dc->flags|=DCF_LOCATE_NEAREST;
	  dc->nearest_sprite_elem_num=0;
	  dc->nearest_dist=MAX_I64;
	  if (recalc==DOC_RECALC_TO_SCREEN && doc->cur_sprite==bptr) {
	    dc->flags|=DCF_LOCATE_NEAREST;
	    dc->cur_x=ipx;
	    dc->cur_y=ipy;
	  } else {
	    dc->cur_x=(doc->x-x0)*FONT_WIDTH+pixel_left+scroll_x;
	    dc->cur_y=(doc->y-y0)*FONT_HEIGHT+pixel_top+scroll_y;
	  }
	  dc->cur_z=0;
	  dc->bkcolor=i;
	  if (doc_e->flags1 & DOCEF1_FROM_START) {
	    xx=(x-k-x0)*FONT_WIDTH; //TODO: scrolling text is not length k
	    yy=(y-y0)*FONT_HEIGHT;
	    zz=0;
	  } else {
	    xx=(x-x0)*FONT_WIDTH;
	    yy=(y-y0)*FONT_HEIGHT;
	    zz=0;
	  }
	  if (ii&(1<<GRST_MESH|1<<GRST_SHIFTABLE_MESH)) {
	    if (!depth_buf) {
	      DCAllocDepthBuf(dc);
	      depth_buf=dc->depth_buf;
	    } else
	      dc->depth_buf=depth_buf;
	    GrRotZEqu(dc->r,cur_time*3.1);
	    GrRotYEqu(dc->r,cur_time*1.9);
	    GrRotXEqu(dc->r,cur_time);
	    dc->flags|=DCF_TRANSFORMATION;
	    dc->x=xx;
	    dc->y=yy;
	    dc->z=GR_Z_ALL;
	    xx=0; yy=0; zz=0;
	  }
	  SpritePlot3(dc,xx,yy,zz,bptr);
	  dc->depth_buf=NULL;
	  if (recalc==DOC_RECALC_TO_SCREEN && doc->cur_sprite==bptr)
	    doc->cur_sprite_elem_num=dc->nearest_sprite_elem_num;
	  dc->flags&=~(DCF_LOCATE_NEAREST|DCF_DONT_DRAW|DCF_TRANSFORMATION);
	  if (dc->nearest_dist<=D) {
	    D=dc->nearest_dist;
	    col=doc_e->min_col;
	  }
	}
	break;
      case DOCT_BPT:
	if (!(s->state&DOCSSF_BPT)) {
	  if (full_refresh) {
	    s=DocSettingsIdentical(doc,s);
	    s->state|=DOCSSF_BPT;
	    QueIns(s,doc->settings_root.last);
	  } else
	    s=DocSettingsNext(doc_e->next);
	}
	break;
      case DOCT_SONG:
	if (sys_focus_task==win_task && recalc==DOC_RECALC_TO_SCREEN && !(doc_e->flags1&DOCEF1_DONT_DRAW)) {
	  if (doc_e->aux_str && (!cur_song || StrCmp(cur_song,doc_e->aux_str))) {
	    Free(cur_song);
	    MusicSettingsRst;
	    cur_song=AStrNew(doc_e->aux_str);
	  }
	}
	doc->flags|=DOCF_HAS_SONG;
	break;
      case DOCT_CLEAR:
	next_clear_found=doc_e;
	break;
      case DOCT_NUM_TYPES-1: //nobound_switch
      default:
	break;
    }

    if (doc_e->flags1 & DOCEF1_HAS_BORDER) {
      if (doc_e->flags1 & DOCEF1_SOLID_BORDER)
	DrawBorder(win_task,1,doc_e->x-x0,x-x0-1,doc_e->y-y0,y-y0,temp_u32_attr.u8[1],TRUE);
      else
	DrawBorder(win_task,0,doc_e->x-x0,x-x0-1,doc_e->y-y0,y-y0,temp_u32_attr.u8[1],TRUE);
    }
    if (full_refresh) {
      switch (doc_e->type_u8) {
	case DOCT_CHECK_BOX:
	case DOCT_HIDE_START:
	  doc_e->max_col=2;
	  break;
	case DOCT_TREE:
	case DOCT_BUTTON:
	case DOCT_LINK:
	case DOCT_MENU_VAL:
	case DOCT_MACRO:
	case DOCT_HIDE_END:
	  doc_e->max_col=1;
	  break;
	default:
	  if (doc_e->flags1 & DOCEF1_TREE)
	    doc_e->max_col=1;
	  else
	    doc_e->max_col=col2;
      }

      if (x>doc->max_x) doc->max_x=x;
      if (y>doc->max_y) doc->max_y=y;
      if (D<=best_d && !(doc_e->flags1&DOCEF1_NO_CLICK_ON)) {
	best_d=D;
	best_do_e=doc_e;
	best_col=col;
      }
      if (doc_e->flags1 & DOCEF1_TREE) {
	if (doc_e->flags1 & DOCEF1_ALIAS)
	  alias=doc_e;
	else
	  alias=NULL;
	if (doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED)
	  tree_collapsed=TRUE;
	else
	  tree_collapsed=FALSE;
	doc_e2=doc_e->next;
	while (doc_e2!=doc && doc_e2->type_u8!=DOCT_INDENT && !(doc_e2->flags1 & DOCEF1_TREE))
	  doc_e2=doc_e2->next;
	if (doc_e2->type_u8==DOCT_INDENT) {
	  i=s->indent;
	  j=i;
	  do {
	    if (tree_collapsed)
	      doc_e2->flags1|=DOCEF1_SKIP;
	    else
	      doc_e2->flags1&=~DOCEF1_SKIP;
	    if (alias && doc_e2==doc->cur_entry && Bt(doc_tag_types,doc_e2->type_u8)) {
	      Free(alias->tag);
	      alias->tag=MAlloc(StrLen(doc_e2->tag)+8,mem_task);
	      alias->tag[0]='+';
	      alias->tag[1]=']';
	      alias->tag[2]=CH_SPACE;
	      StrCpy(alias->tag+3,doc_e2->tag);
	      alias->max_col=doc_e2->max_col+3;
	    }
	    if (doc_e2->type_u8==DOCT_INDENT) {
	      if (doc_e2->flags1 & DOCEF1_LEFT_X)
		j=doc_e2->attr;
	      else
		j+=doc_e2->attr;
	    }
	    doc_e2=doc_e2->next;
	  } while (doc_e2!=doc && j>i);
	}
      }
      if (doc_e->type_u8 == DOCT_HIDE_START) {
	if (doc_e->flags1 & DOCEF1_ALIAS)
	  alias=doc_e;
	else
	  alias=NULL;
	doc_e2=doc_e->next;
	LBtr(hide_collapsed,0);
	j=1;
	LBEqu(hide_collapsed,j,doc_e->flags1 & DOCEF1_CHECKED_COLLAPSED);
	while (doc_e2!=doc && j>0) {
	  if (Bt(hide_collapsed,j))
	    doc_e2->flags1|=DOCEF1_SKIP;
	  else
	    doc_e2->flags1&=~DOCEF1_SKIP;
	  if (doc_e2->type_u8==DOCT_HIDE_START) {
	    j++;
	    LBEqu(hide_collapsed,j,doc_e2->flags1 & DOCEF1_CHECKED_COLLAPSED);
	    if (Bt(hide_collapsed,j-1))
	      LBts(hide_collapsed,j);
	  } else if (doc_e2->type_u8==DOCT_HIDE_END) {
	    j--;
	    if (Bt(hide_collapsed,j))
	      doc_e2->flags1|=DOCEF1_SKIP;
	    else
	      doc_e2->flags1&=~DOCEF1_SKIP;
	  }
	  doc_e2=doc_e2->next;
	}
      }
    }

    doc_e2=doc_e->next;
skip:
    while (doc_e2!=doc && doc_e2->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)) {
      if (doc_e2==doc->cur_entry) {
	doc->cur_entry=doc_e2->next;
	doc->cur_data_col=doc->cur_entry->min_col;
      }
      if (full_refresh) {
	doc_e2->x=x;
	doc_e2->y=y;
	doc_e2->page_line_num=doc->page_line_num;
	doc_e2->settings=s;
	doc_e2->type.u8[1]=cur_u8_attr;
	doc_e2->flags1=doc->flags& (DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER) |
	      doc_e2->flags1&~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER);
      }
      doc_e2=doc_e2->next;
    }

    if (full_refresh) {
      if (del_cl) {
	if (!(doc_e->flags1 & (DOCEF1_HOLD|DOCEF1_FILTER_SKIP))) {
	  if (doc_e==doc->cur_entry) {
	    doc->cur_entry=doc_e2;
	    doc->cur_data_col=doc_e2->min_col;
	  }
	  if (best_do_e==doc_e) {
	    best_do_e=doc_e2;
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
    if (doc->min_x>doc->max_x) {
      doc->max_x=doc->start_x;
      doc->min_x=doc->start_x;
    }
    if (doc->min_y>doc->max_y) {
      doc->max_y=doc->start_y;
      doc->min_y=doc->start_y;
    }

    //Update header
    if (!skipped_update) {
      doc_e->x=x;
      doc_e->y=y;
      doc_e->page_line_num=doc->page_line_num;
      doc_e->settings=s;
      doc_e->type.u8[1]=cur_u8_attr;
      if (find_cursor) {
	D=DocCharDist(doc,x,y);
	if (D<best_d && !(doc_e->flags1&DOCEF1_NO_CLICK_ON)) {
	  best_d=D;
	  best_do_e=doc_e;
	  best_col=0;
	}
      }
    }
    if (doc->recalc_start==doc && doc->flags & DOCF_MIN_SIZE) {
      if (Bt(&win_task->display_flags,DISPLAYf_NO_BORDER)) {
	if (left<0)
	  left=0;
	i=left+doc->max_x-doc->min_x;
	if (i>TEXT_COLS-1)
	  i=TEXT_COLS-1;
	SetWinHorz(left,i,win_task);
	if (top<0)
	  top=0;
	i=top+doc->max_y-doc->min_y;
	if (i>TEXT_ROWS-1)
	  i=TEXT_ROWS-1;
	SetWinVert(top,i,win_task);
      } else {
	if (left<1)
	  left=1;
	i=left+doc->max_x-doc->min_x;
	if (i>TEXT_COLS-2)
	  i=TEXT_COLS-2;
	SetWinHorz(left,i,win_task);
	if (top<1)
	  top=1;
	i=top+doc->max_y-doc->min_y;
	if (i>TEXT_ROWS-2)
	  i=TEXT_ROWS-2;
	SetWinVert(top,i,win_task);
      }
    }
    if (find_cursor) {
      doc->cur_entry=best_do_e;
      doc->cur_data_col=best_col;
      DocFormBackward(doc);
//We need this because text coordinates are used
      if (best_d<FONT_WIDTH)
	best_d=0;
      doc->best_d=best_d;
    }

    if (doc->cur_entry->type_u8!=DOCT_HEX_ED) {
      doc->y=doc->cur_entry->y;
      doc->x=doc->cur_entry->x+doc->cur_data_col;
    } else {
      doc->y=doc->cur_entry->y+doc->cur_data_col/3/doc->cur_entry->hex_ed_width;
      x=doc->cur_data_col%(doc->cur_entry->hex_ed_width*3);
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

    if (has_cursor) {
      if (recalc==DOC_RECALC_TO_SCREEN) {
	x=0;
	y=0;
      } else {
	x=scroll_x/FONT_WIDTH;
	y=scroll_y/FONT_HEIGHT;
      }
      if (doc->cur_top_line_num-y+height-1>doc->max_y)
	doc->cur_top_line_num=doc->max_y-(height-1)+y;
      if (doc->cur_top_line_num-y<doc->min_y)
	doc->cur_top_line_num=doc->min_y+y;

      if (doc->y-doc->cur_top_line_num+y>height-1)
	doc->cur_top_line_num=doc->y-(height-1)+y;
      if (doc->y-doc->cur_top_line_num+y<0)
	doc->cur_top_line_num=doc->y+y;

      if (doc->cur_entry->flags1 & DOCEF1_LOCK_TO_CENTER)
	DocCenter(doc,DOC_RECALC_PARTIAL);
      LockToPage(doc);

      if (doc->line_start_col-x+width-1>doc->max_x)
	doc->line_start_col=doc->max_x-(width-1)+x;
      if (doc->line_start_col-x<doc->min_x)
	doc->line_start_col=doc->min_x+x;

      if (doc->x-doc->line_start_col+x>width-1)
	doc->line_start_col=doc->x-(width-1)+x;
      if (doc->x-doc->line_start_col+x<0)
	doc->line_start_col=doc->x+x;
    }
    doc->recalc_start=NULL;
  }
  if (recalc==DOC_RECALC_TO_SCREEN && has_cursor) {
    x=doc->x-doc->line_start_col+left +scroll_x/FONT_WIDTH;
    y=doc->y-doc->cur_top_line_num+top+scroll_y/FONT_HEIGHT;
    if (0<=x<=right && 0<=y<=bottom &&
	  x<TEXT_COLS && y<TEXT_ROWS &&
	  !(doc->flags&DOCF_HIDE_CURSOR)) {
      u32_ptr=gr_text_base+y*TEXT_COLS+x;
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
	  vss->pos=doc->cur_top_line_num;
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
    doc->old_cur_data_col=doc->old_cur_data_col;
  }
  if (doc->flags & DOCF_HAS_SONG)
    LBts(&win_task->task_flags,TASKf_HAS_SONG);
  if (full_refresh) {
    i=num_entries-doc->max_entries;
    if (next_clear_found) {
      DocDelToEntry(doc,next_clear_found);
      DocRecalc(doc,has_cursor,recalc);
    } else if (i>1024) {
      DocDelToNum(doc,i);
      DocRecalc(doc,has_cursor,recalc);
    }
  }
  DCDel(dc);
  Free(depth_buf);
  if (unlock)
    DocUnlock(doc);
  return TRUE;
}

public U0 DocInsSafe(CDoc *doc,U8 *st)
{ //This is necessary when hidden text might screw-up
//inserts.  It does them all-at-once.
  Bool unlock;
  I64 old_flags;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  old_flags=doc->flags;
  doc->flags|=DOCF_NO_CURSOR;
  DocPutS(doc,st);
  DocRecalc(doc,FALSE,DOC_RECALC_PARTIAL);
  doc->flags=old_flags;
  if (unlock)
    DocUnlock(doc);
}

public U0 DocBottom(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  doc->cur_entry=doc;
  doc->cur_data_col=0;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocTop(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  doc->cur_entry=doc->root.next;
  doc->cur_data_col=0;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocClear(CDoc *doc=NULL)
{
  Bool unlock;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  DocBottom(doc);
  DocPutS(doc,"$$CL$$");
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public Bool DocCursor(Bool show=OFF,CDoc *doc=NULL)
{
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return FALSE;
  }
  return !LBEqu(&doc->flags,DOCf_HIDE_CURSOR,!show);
}

public Bool DocScroll(Bool val=OFF,CDoc *doc=NULL)
{
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return FALSE;
  }
  return !LBEqu(&doc->flags,DOCf_NO_SCROLL_BARS,!val);
}

public U0 DocCollapse(Bool collapse=TRUE,CDoc *doc=NULL)
{
  CDocEntry *doc_e;
  Bool unlock;
  if (!doc) {
    doc=DocPut;
    if (!doc)
      return;
  }
  unlock=DocLock(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->flags1&DOCEF1_TREE || doc_e->type_u8==DOCT_HIDE_START)
      BEqu(&doc_e->flags1,DOCEf1_CHECKED_COLLAPSED,collapse);
    doc_e=doc_e->next;
  }
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Task"
U0 DocUpdateTaskDocs(CTask *task)
{
  CPrt *p;
  CDoc *doc;
  CD3I64 saved_scroll;
  Bool old_preempt=Preempt(OFF);
  I64 border_attr;
  if (task->border_src==BDS_CUR_PRT && task->cur_prt) {
    border_attr=task->cur_prt->text_attr;
    if (border_attr&15!=border_attr>>4)
      task->border_attr=border_attr;
  }
  if (task->title_src==TTS_TASK_NAME)
    StrCpy(task->task_title,task->task_name);
  if ((doc=DocDisplay(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    if (task->border_src==BDS_ED_FILENAME_PRT) {
      try {
	if (p=Drv2Prt(*doc->filename.name)) {
	  border_attr=p->text_attr;
	  if (border_attr&15!=border_attr>>4)
	    task->border_attr=border_attr;
	}
      } catch
	Fs->catch_except=TRUE;
    }
    if (task->title_src==TTS_ED_FILENAME)
      MemCpy(task->task_title,doc->filename.name,TASK_TITLE_LEN);
    Preempt(old_preempt);
    DocRecalc(doc,TRUE,DOC_RECALC_TO_SCREEN);
    Preempt(OFF);
  }
  if ((doc=DocBorder(task)) && !(doc->flags&DOCF_DONT_SHOW)) {
    WinScrollNull(task,&saved_scroll);
    DocRecalc(doc,FALSE,DOC_RECALC_TO_SCREEN);
    WinScrollRestore(task,&saved_scroll);
  }
  Preempt(old_preempt);
}
#help_index "Doc"
