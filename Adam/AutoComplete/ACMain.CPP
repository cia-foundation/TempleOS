#help_index "AutoComplete"

CHashAC *ACHashAdd(U8 *w)
{
  CHashAC *tempw=HashFind(w,ac.hash_table,HTT_WORD);
  if (tempw) {
    tempw->hits++;
    return tempw;
  }
  tempw=ACAlloc(sizeof(CHashAC));
  tempw->str=AStrNew(w);
  tempw->type=HTT_WORD;
  tempw->use_cnt=1;
  tempw->hits=1;
  HashAdd(tempw,ac.hash_table);
  ac.num_words++;
  return tempw;
}

U0 ACSingleFileAdd(U8 *buf)
{
  I64 ch;
  U8 *ptr=buf,*ptr2,*ptr3;
  while (TRUE) {
    while (TRUE) {
      if (ch=*ptr++) {
	if (Bt(chars_bitmap_alpha_numeric,ch))
	  break;
      } else
	return;
    }
    ptr3=ptr;
    ptr2=ptr;
    ptr--;
    while (TRUE) {
      if (ch=*ptr2++) {
	if (Bt(chars_bitmap_alpha_numeric,ch))
	  *ptr3++=ch;
	else if (ch!=CH_CURSOR)
	  break;
      } else {
	ptr2--;
	break;
      }
    }
    *ptr3=0;
    ACHashAdd(ptr);
    ptr=ptr2;
  }
}

I64 ACSkipCrap(U8 *src,I64 len)
{
  I64 j;
  j=len-1;
  while (j>=0) {
    if (Bt(chars_bitmap_alpha_numeric,src[j]))
      break;
    else
      j--;
  }
  return j+1;
}

I64 ACPriorWordInStr(U8 *src,U8 *dst,I64 len,I64 buf_size)
{
  I64 i,j=0,k;
  i=len-1;
  while (i>=0)
    if (!Bt(chars_bitmap_alpha_numeric,src[i]))
      break;
    else
      i--;
  if (i>=-1 && len>0)
    for (k=i+1;k<len && j<buf_size-1;k++)
      dst[j++]=src[k];
  dst[j]=0;
  return i+1;
}

U0 ACFillIn(I64 n)
{
  U8 *s;
  if (0<=--n<ac.num_fillins) {
    s=ac.fillin_matches[n]->str;
    if (StrLen(s)>ac.partial_len)
      Auto(s+ac.partial_len);
  }
}

U0 ACMan(I64 n,CTask *parent_task=NULL)
{
  CHashAC *tempw;
  CHashSrcSym *temph;
  Bool old_preempt=Preempt(OFF);
  if (0<=--n<ac.num_fillins && (tempw=ac.fillin_matches[n]) &&
	(temph=HashFind(tempw->str,Fs->hash_table,HTG_SRC_SYM)) &&
	temph->src_link) {
    Preempt(old_preempt);
    PopUpEd(temph->src_link,parent_task);
  }
  Preempt(old_preempt);
}

U0 ACFillInAdd(CHashAC *tempw)
{
  I64 k;
  if (ac.num_fillins<AC_MAX_FILLINS || tempw->hits>ac.fillin_hits[ac.num_fillins-1]) {
    for (k=ac.num_fillins-1;k>=0;k--) {
      if (tempw->hits<=ac.fillin_hits[k])
	break;
      else {
	ac.fillin_matches[k+1]=ac.fillin_matches[k];
	ac.fillin_hits[k+1]   =ac.fillin_hits[k];
      }
    }
    ac.fillin_matches[k+1]=tempw;
    ac.fillin_hits[k+1]   =tempw->hits;
    if (ac.num_fillins<AC_MAX_FILLINS)
      ac.num_fillins++;
  }
}

U0 ACPutChoices(CDoc *focus_l,CDocEntry *doc_e,CTask *focus_task)
{
  I64 i,data_col;
  U8 *buf,*buf1,*src=NULL,*st;
  CHashAC *tempw;
  F64 timeout_time=tS+0.5;
  CHashSrcSym *temph;
  CDoc *doc=DocPut;

  src=DocScanLine(focus_l,doc_e,&data_col);
  DocUnlock(focus_l);
  i=StrLen(src);
  buf =MAlloc(MaxI64(i+1,256));
  buf1=MAlloc(MaxI64(i+1,256));
  if (data_col==-1)
    data_col=0;
  data_col=ACPriorWordInStr(src,buf,data_col,256);
  ac.partial_len=StrLen(buf);
  data_col=ACSkipCrap(src,data_col);
  data_col=ACPriorWordInStr(src,buf1,data_col,256);

  if (!ac.cur_word || StrCmp(ac.cur_word,buf)) {
    st=ac.cur_word;
    ac.cur_word=AStrNew(buf);
    Free(st);
    ac.num_fillins=0;
    if (*ac.cur_word) {
      Preempt(ON);
      for (i=0;i<=ac.hash_table->mask;i++) {
	tempw=ac.hash_table->body[i];
	while (tempw && tS<timeout_time) {
	  if (!MemCmp(ac.cur_word,tempw->str,StrLen(ac.cur_word)))
	    ACFillInAdd(tempw);
	  tempw=tempw->next;
	}
      }
      Preempt(OFF);
    }
    DocRst(doc,TRUE);
    doc->flags|=DOCF_MIN_SIZE;
    DocCursor;
    Fs->border_src=BDS_CONST;
    Fs->text_attr =Fs->border_attr=LTGRAY<<4+BLUE;
    if (ac.cur_word && *ac.cur_word) {
      DocPrint(doc,"$$PURPLE$$Word:%s$$FG$$\n",ac.cur_word);
      for (i=0;i<ac.num_fillins;i++) {
	st=ac.fillin_matches[i]->str;
	DocPrint(doc,"$$GREEN$$F%02d$$FG$$ ",i+1);
	if (TaskValidate(focus_task) &&
	      (temph=HashFind(st,focus_task->hash_table,HTG_SRC_SYM)) &&
	      temph->src_link) {
	  if (temph->type&HTF_PUBLIC)
	    DocPrint(doc,"$$RED$$");
	  DocPrint(doc,"$$TX+UL+L+PU,\"%Q\",A=\"%s\"$$$$FG$$\n",st,temph->src_link);
	} else
	  DocPrint(doc,"%s\n",st);
      }
      if (acd.has_words)
	ACDDictWordsAdd(doc,ac.cur_word);
    } else if (FileFind("::/Doc/StandBy.TXT.Z"))
      DocType(doc,"::/Doc/StandBy.TXT.Z",0);

    DocRecalc(doc);
  }

  if (!LBts(&Fs->display_flags,DISPLAYf_SHOW))
    WinZBufUpdate;

  Free(src);
  Free(buf);
  Free(buf1);
}

U0 ACTask(I64)
{
  CTask *focus_task,*original_focus_task;
  CDoc *doc;
  CDocEntry *doc_e;
  I64 ch;
  DocTermNew;
  WinHorz(51,Fs->win_right);
  LBts(&Fs->display_flags,DISPLAYf_WIN_ON_TOP);
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_BORDER-WIF_SELF_IP_L-WIF_SELF_IP_R-WIG_DBL_CLICK;
  Free(ac.cur_word);
  ac.cur_word=NULL;
  while (TRUE) {
    original_focus_task=focus_task=sys_focus_task;
    while (TaskValidate(focus_task) && Bt(&focus_task->task_flags,TASKf_INPUT_FILTER_TASK))
      focus_task=focus_task->parent_task;
    if (TaskValidate(focus_task) && (doc=DocPut(focus_task)) &&
	  focus_task!=Fs && Bt(&focus_task->display_flags,DISPLAYf_SHOW)) {
      DocLock(doc);
      if (TaskValidate(focus_task) && original_focus_task==sys_focus_task &&
	    doc && doc==DocPut(focus_task) && (doc_e=doc->cur_entry)) {
	if (doc_e==doc) doc_e=doc_e->last;
	while (doc_e->last!=doc && (doc_e->type_u8==DOCT_NEW_LINE ||
	      doc_e->type_u8==DOCT_SOFT_NEW_LINE))
	  doc_e=doc_e->last;
	while (doc_e->last->type_u8!=DOCT_NEW_LINE && doc_e->last!=doc)
	  doc_e=doc_e->last;
	ACPutChoices(doc,doc_e,focus_task);
      } else
	DocUnlock(doc);
    }
    Sleep(333);
    if (ScanMsg(&ch,,1<<MSG_KEY_DOWN) && (ch==CH_ESC||ch==CH_SHIFT_ESC))
      break;
    doc=DocPut;
    DocLock(doc);
    if (doc->cur_entry->de_flags & DOCEF_LINK) {
      '' CH_SPACE;
      doc->cur_entry=doc;
    }
    DocUnlock(doc);
  }
  ac.task=NULL;
}

U0 ACMainFileLstTraverse(U8 *files_find_mask)
{
  U8 *buf;
  CDirEntry *tempde,*tempde1;
  try {
    tempde=tempde1=FilesFind(files_find_mask,FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES|FUF_CLUSTER_ORDER);
    while (tempde) {
      "%s\n",tempde->full_name;
      buf=FileRead(tempde->full_name);
      ACSingleFileAdd(buf);
      Free(buf);
      tempde=tempde->next;
    }
  } catch
    Fs->catch_except=TRUE;
  DirTreeDel(tempde1);
}

public Bool AutoComplete(Bool val=OFF)
{//Turn AutoComplete OFF (or ON).
  Bool	old_autocomplete=FALSE,
	old_preempt=Preempt(OFF);
  if (val) {
    while (Bt(&ac.flags,ACf_INIT_IN_PROGRESS))
      Sleep(10);
    if (TaskValidate(ac.task))
      old_autocomplete=TRUE;
    else {
      ac.task=Spawn(&ACTask,NULL,"AutoComplete");
      TaskWait(ac.task);
    }
    WinToTop(ac.task);
  } else {
    if (TaskValidate(ac.task)) {
      old_autocomplete=TRUE;
      Kill(ac.task);
      while (TaskValidate(ac.task))
	Yield;
      ac.task=NULL;
    }
  }
  Preempt(old_preempt);
  return old_autocomplete;
}

U0 ACProgressTask(Bool *_start_flag)
{
  I64 start=dsk.write_cnt;
  progress1=0;
  progress1_max=(Size(ACD_DEF_FILENAME_Z,"+x+s")+BLK_SIZE-1)>>BLK_SIZE_BITS;
  StrCpy(progress1_desc,"Uncompressing Dictionary");
  start=dsk.write_cnt;
  *_start_flag=TRUE;
  while (progress1<progress1_max) {
    progress1=dsk.write_cnt-start;
    Sleep(10);
  }
  *progress1_desc=progress1=progress1_max=0;
}

public U0 ACInit(U8 *mask=NULL)
{//Read files and build AutoComplete statistics.
  Bool start_flag;
  CBlkDev *bd=Let2BlkDev;

  while (LBts(&ac.flags,ACf_INIT_IN_PROGRESS))
    Yield;
  AutoComplete;

  if (IsDrvWritable && FileFind(ACD_DEF_FILENAME_Z) && !FileFind(ACD_DEF_FILENAME)) {
    if (bd->type!=BDT_RAM) {
      start_flag=FALSE;
      Spawn(&ACProgressTask,&start_flag);
      while (!start_flag)
	Yield;
    }
    Move(ACD_DEF_FILENAME_Z,ACD_DEF_FILENAME);
  }

  HashTableDel(ac.hash_table);
  ac.hash_table=HashTableNew(2048,adam_task);

  ac.num_words=0;
  Free(ac.cur_word);
  ac.cur_word=NULL;

  if (mask)
    ACMainFileLstTraverse(mask);

  ACDWordsLoad;
  LBtr(&ac.flags,ACf_INIT_IN_PROGRESS);
  AutoComplete(ON);
}

I64 AutoCompleteSize()
{
  if (ac.hash_table)
    return HashTableSize2(ac.hash_table)+MSize2(acd.word_lst);
  else
    return 0;
}
