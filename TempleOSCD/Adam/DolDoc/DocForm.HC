#help_index "DolDoc/Form"

U0 DocFormFwd(CDoc *doc,Bool giveup=FALSE)
{
  CDocEntry *doc_e=doc->cur_entry,*doc_e2=doc_e;
  if (doc->flags & DOCF_FORM) {
    if (doc_e==doc) goto ff_recover;
    while (!Bt(doldoc.type_flags_form,doc_e->type_u8) &&
	  !(doc_e->de_flags&DOCEF_LINK) ||
	  doc_e->de_flags&DOCEF_SKIP_IN_FORM) {
      doc_e=doc_e->next;
      if (doc_e==doc) {
ff_recover:
	doc->cur_col=0;
	if (!giveup) {
	  doc->cur_entry=doc_e->last;
	  DocFormBwd(doc,TRUE);
	} else
	  doc->cur_entry=doc;
	return;
      }
    }
  }
  while (doc_e->type_u8==DOCT_INDENT)
    doc_e=doc_e->next;
  if (doc_e!=doc_e2) {
    doc->cur_col=doc_e->min_col;
    doc->cur_entry=doc_e;
  }
}

U0 DocFormBwd(CDoc *doc,Bool giveup=FALSE)
{
  CDocEntry *doc_e=doc->cur_entry,*doc_e2=doc_e;
  if (doc->flags & DOCF_FORM) {
    while (!Bt(doldoc.type_flags_form,doc_e->type_u8) &&
	  !(doc_e->de_flags&DOCEF_LINK) ||
	  doc_e->de_flags&DOCEF_SKIP_IN_FORM) {
      doc_e=doc_e->last;
      if (doc_e==doc) {
	doc->cur_col=0;
	if (!giveup) {
	  doc->cur_entry=doc_e->next;
	  DocFormFwd(doc,TRUE);
	} else
	  doc->cur_entry=doc;
	return;
      }
    }
  }
  while (doc_e->type_u8==DOCT_INDENT)
    doc_e=doc_e->next;
  if (doc_e!=doc_e2) {
    doc->cur_col=doc_e->min_col;
    doc->cur_entry=doc_e;
  }
}

U0 DocDataFmt(CDoc *doc,CDocEntry *doc_e,I64 d=DOCM_CANCEL)
{
  I64 i;
  U8 *ptr,*ptr2;
  CHashDefineStr *tmph;
  if (doc_e->type_u8==DOCT_DATA && doc_e->de_flags&DOCEF_AUX_STR ||
	doc_e->type_u8==DOCT_CHECK_BOX || doc_e->de_flags & DOCEF_LST) {
    if (d==DOCM_CANCEL) {
      if (doc_e->de_flags&DOCEF_DEREF_DATA &&
	    !(doc_e->de_flags&DOCEF_REMALLOC_DATA)) {
	if (!(ptr=doc_e->data)) return;
      } else
	ptr=&doc_e->data;
      switch (doc_e->raw_type) {
	case RT_I0:
	case RT_U0:	d=0;		break;
	case RT_I8:	d=*ptr(I8 *);	break;
	case RT_U8:	d=*ptr(U8 *);	break;
	case RT_I16:	d=*ptr(I16 *);	break;
	case RT_U16:	d=*ptr(U16 *);	break;
	case RT_I32:	d=*ptr(I32 *);	break;
	case RT_U32:	d=*ptr(U32 *);	break;
	default:	d=*ptr(I64 *);
      }
    }
    if (doc_e->type_u8==DOCT_DATA) {
      if (doc_e->de_flags & DOCEF_REMALLOC_DATA) {
	ptr=MStrPrint(doc_e->aux_str,d,doc_e->my_fmt_data);
	i=StrLen(ptr);
	if (!doc_e->data) {
	  doc_e->data=CAlloc(2,doc->mem_task);
	  doc_e->len=MSize(doc_e->data)-2;
	}
	if (doc_e->len+doc_e->min_col>i)
	  MemCpy(doc_e->tag,ptr,i+1);
	else {
	  ptr2=MAlloc(i+8,doc->mem_task);
	  doc_e->len=MSize(ptr2)-doc_e->min_col-2;	 //See $LK,"DataTagWidth",A="FA:::/Adam/DolDoc/DocPlain.HC,DataTagWidth"$
	  MemCpy(ptr2,ptr,i+1);
	  Free(doc_e->tag);
	  doc_e->tag=ptr2;
	}
	Free(ptr);
      } else {
	StrPrint(doc_e->tag,doc_e->aux_str,d,doc_e->my_fmt_data);
	i=StrLen(doc_e->tag);
      }
      if (doc_e->de_flags & DOCEF_HAS_TERMINATOR) {
	doc_e->tag[i++]='_';
	doc_e->tag[i]=0;
      }
      doc_e->max_col=i;
    } else if (doc_e->de_flags & DOCEF_LST) {
      if (doc_e->de_flags & DOCEF_DEFINE && (tmph=HashFind(doc_e->define_str,
	    doc->win_task->hash_table,HTT_DEFINE_STR)) && 0<=d<tmph->cnt) {
	ptr=MStrPrint("[%s]",tmph->sub_idx[d]);
	Free(doc_e->tag);
	doc_e->tag=StrNew(ptr,doc->mem_task);
	Free(ptr);
      } else {
	Free(doc_e->tag);
	doc_e->tag=StrNew("[]",doc->mem_task);
      }
    } else {
      if (d)
	doc_e->de_flags|=DOCEF_CHECKED_COLLAPSED;
      else
	doc_e->de_flags&=~DOCEF_CHECKED_COLLAPSED;
    }
  }
}

U0 DocDataScan(CDoc *doc,CDocEntry *doc_e)
{
  I64 i,d;
  U8 *ptr,*ptr1,*ptr2;
  CHashDefineStr *tmph;
  if (doc_e->type_u8==DOCT_DATA && doc_e->de_flags&DOCEF_AUX_STR ||
	doc_e->type_u8==DOCT_CHECK_BOX || doc_e->de_flags & DOCEF_LST) {
    if (doc_e->de_flags&DOCEF_DEREF_DATA &&
	  !(doc_e->de_flags&DOCEF_REMALLOC_DATA)) {
      if (!(ptr=doc_e->data)) return;
    } else
      ptr=&doc_e->data;
    if (doc_e->type_u8==DOCT_DATA) {
      i=StrLen(doc_e->tag);
      if (doc_e->de_flags & DOCEF_HAS_TERMINATOR)
	doc_e->tag[--i]=0;
      if (i>doc_e->len+doc_e->min_col)
	doc_e->tag[doc_e->len+doc_e->min_col]=0;
      if (RT_I8<=doc_e->raw_type<=RT_U32) {
	StrScan(doc_e->tag,doc_e->aux_str,&d,doc_e->my_fmt_data);
	if (doc_e->de_flags & DOCEF_HAS_TERMINATOR)
	  doc_e->tag[i]='_';
      } else if (RT_I64<=doc_e->raw_type<=RT_UF64) {
	if (doc_e->de_flags & DOCEF_REMALLOC_DATA) {
	  ptr=MAlloc(i-doc_e->min_col+8,doc->mem_task);
	  MemCpy(ptr,doc_e->tag+doc_e->min_col,i-doc_e->min_col+1);
	  Free(doc_e->data);
	  doc_e->data=ptr;
	  doc_e->len=MSize(ptr)-1;
	} else
	  StrScan(doc_e->tag,doc_e->aux_str,ptr,doc_e->my_fmt_data);
	if (doc_e->de_flags & DOCEF_HAS_TERMINATOR)
	  doc_e->tag[i]='_';
	return;
      }
    } else if (doc_e->de_flags & DOCEF_LST) {
      d=0;
      if (doc_e->tag && doc_e->de_flags & DOCEF_DEFINE &&
	    (tmph=HashFind(doc_e->define_str,
	    doc->win_task->hash_table,HTT_DEFINE_STR))) {
	ptr1=ptr2=StrNew(doc_e->tag);
	if (*ptr2=='[') {
	  ptr2++;
	  i=StrLen(ptr2);
	  if (ptr2[i-1]==']')
	    ptr2[i-1]=0;
	}
	d=LstMatch(ptr2,tmph->data);
	Free(ptr1);
      }
    } else {
      if (doc_e->de_flags & DOCEF_CHECKED_COLLAPSED)
	d=TRUE;
      else
	d=FALSE;
    }
    switch (doc_e->raw_type) {
      case RT_I8:
      case RT_U8:
	*ptr(U8 *)=d;
      case RT_I0:
      case RT_U0:
	break;
      case RT_I16:
      case RT_U16:
	*ptr(U16 *)=d;
	break;
      case RT_I32:
      case RT_U32:
	*ptr(U32 *)=d;
	break;
      default:
	*ptr(I64 *)=d;
    }
  }
}

#help_index "DolDoc/Input;StdIn/DolDoc"
public Bool DocForm(U8 *_d,U8 *class_name=lastclass,
    I64 dof_flags=0,U8 *header=NULL,U8 *footer=NULL)
{//User input. Supply a class name that has format definitions.
//See $LK,"::/Demo/DolDoc/Form.HC"$ and $LK,"::/Demo/LastClass.HC"$.
  CMemberLst *ml;
  CDocEntry *doc_e;
  U8 *format;
  CHashClass *tmpc,*tmpc2;
  CDoc *doc;
  Bool res=FALSE;
  I64 old_border_src=Fs->border_src,has_action;
  if (!(tmpc=HashFind(class_name,Fs->hash_table,HTT_CLASS)))
    return FALSE;
  doc=DocNew;
  doc->desc='Form';
  if (header) DocPrint(doc,"%s",header);
  doc->flags|=DOCF_OVERSTRIKE|DOCF_FORM;
  if (dof_flags&DOF_SIZE_MIN)
    doc->flags|=DOCF_SIZE_MIN;
  ml=tmpc->member_lst_and_root;
  while (ml) {
    if ((format=MemberMetaData("format",ml)) &&
	  (doc_e=DocPrint(doc,"%s",format))) {
      tmpc2=ml->member_class;
      if ((doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_LST ||
	    doc_e->type_u8==DOCT_CHECK_BOX) && !tmpc2->ptr_stars_cnt) {
	tmpc2=OptClassFwd(tmpc2);
	tmpc2-=tmpc2->ptr_stars_cnt;
	if (tmpc2->type & HTT_INTERNAL_TYPE) {
	  if (ml->dim.next) { //Array
	    if (tmpc2->raw_type==RT_U8 &&
		  LBtr(&doc_e->de_flags,&DOCEf_DFT_LEN)) {
	      doc_e->len=ml->dim.total_cnt;
	      if (doc_e->de_flags&DOCEF_HAS_TERMINATOR)
		doc_e->len--;
	      Free(doc_e->tag);  //See $LK,"DataTagWidth",A="FA:::/Adam/DolDoc/DocPlain.HC,DataTagWidth"$
	      doc_e->tag=MAlloc(doc_e->len+doc_e->min_col+2,
		    doc->mem_task); //+2 because "_\0"
	    }
	  } else if (LBtr(&doc_e->de_flags,DOCEf_DFT_RAW_TYPE))
	    doc_e->raw_type=tmpc2->raw_type;
	}
      }
      if (doc_e->de_flags&DOCEF_REMALLOC_DATA) {
	doc_e->user_data=_d+ml->offset;
	doc_e->data=*doc_e->user_data(U8 **);
      } else
	doc_e->data=_d+ml->offset;
      doc_e->my_fmt_data=MemberMetaData("data",ml);
      DocDataFmt(doc,doc_e);
    }
    ml=ml->next;
  }
  if (footer) DocPrint(doc,"%s",footer);
  if (doc->head.next!=doc)	{
    Fs->border_src=BDS_CONST;
    DocRecalc(doc);
    if (DocEd(doc,dof_flags)) {
      doc_e=doc->cur_entry;
      res=TRUE;
      if (doc_e!=doc) {
	if (DocEntryRun(doc,doc_e,TRUE,&has_action)==DOCM_CANCEL && has_action)
	  res=FALSE;
	DocUnlock(doc);
      }
    }
  }
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->de_flags&DOCEF_REMALLOC_DATA) {
      *doc_e->user_data(U8 **)=doc_e->data;
      doc_e->data=NULL;
    }
    doc_e=doc_e->next;
  }
  DocDel(doc);
  Fs->border_src=old_border_src;
  return res;
}

U0 DocMenuEndTaskCB()
{
  WinToTop;
  throw;
}

public I64 DocMenu(CDoc *m,I64 dof_flags=0)
{//Run menu chooser doc. Returns menu doc unlocked.
  U8 *old_end_cb=Fs->task_end_cb;
  Bool old_break_shift_esc=LBts(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC);
  CDocEntry *doc_e;
  I64 old_border_src=Fs->border_src,res=DOCM_CANCEL,has_action;
  Fs->task_end_cb=&DocMenuEndTaskCB;
  try {
    if (m) {
      m->desc='Menu';
      Fs->border_src=BDS_CONST;
dm_restart:
      if (DocEd(m,dof_flags)) {
	doc_e=m->cur_entry;
	if (doc_e!=m) {
	  res=DocEntryRun(m,doc_e,TRUE,&has_action);
	  DocUnlock(m);
	  if (!has_action) {
	    res=DOCM_CANCEL;
	    dof_flags|=DOF_DONT_HOME;
	    goto dm_restart;
	  }
	}
      }
    }
  } catch {
    if (!Fs->except_ch) {
      if (!(dof_flags & DOF_INTERCEPT_TASK_END))
	Exit;
      Fs->catch_except=TRUE;
    }
  }
  LBEqu(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC,old_break_shift_esc);
  Fs->border_src=old_border_src;
  Fs->task_end_cb=old_end_cb;
  return res;
}

public I64 PopUpMenu(CDoc *doc,I64 dof_flags=0)
{//Run menu chooser doc in PopUp win task.
  doc->flags|=DOCF_SIZE_MIN | DOCF_FORM;
  return PopUpPrint("DocMenu(0x%X,0x%X);",doc,dof_flags);
}

public Bool PopUpForm(U8 *_d,U8 *class_name=lastclass,
	I64 dof_flags=DOF_SIZE_MIN,U8 *header=NULL,U8 *footer=NULL)
{//See $LK,"::/Demo/DolDoc/Form.HC"$ and $LK,"::/Demo/LastClass.HC"$.
  return PopUpPrint("DocForm(0x%X,0x%X,0x%X,0x%X,0x%X);",_d,class_name,
	dof_flags,header,footer);
}
