#help_index "WordStat"

/****  Debug code
U0 WSPutMatrix(CWSMatrix *m)
{
  I64 i,j;
  CWSMatrixSide *ms;
  CWS *tempw;

  "n:%d\n",m->n;
  for (i=0;i<ws_num_matrix_words;i++) {
    ms=&m->side[i];
    "col:%d rhits:%d chits:%d\n",i,ms->row_hits,ms->col_hits;
    tempw=ms->next;
    while (tempw!=ms) {
      "substructs:%d entries:%d\n",tempw->substructs,tempw->entries;
      for (j=0;j<tempw->substructs;j++) {
	"(%04X %04X) ",
	  tempw->subs[j].hits,
	  tempw->subs[j].repeats;
      }
      NewLine;
      tempw=tempw->next;
    }
  }

  View;
}
****/

CHashWS *WSHashAdd(U8 *w)
{
  CHashWS *tempw=HashFind(w,ws_hash_table,HTT_WORD);
  if (tempw) {
    tempw->hits++;
    return tempw;
  }
  tempw=ACAlloc(sizeof(CHashWS));
  tempw->str=AStrNew(w);
  tempw->type=HTT_WORD;
  tempw->use_cnt=1;
  tempw->hits=1;
  tempw->num=-1;
  HashAdd(tempw,ws_hash_table);
  ws_num_words++;
  return tempw;
}

U0 WSSidesInit(CWSMatrix *m)
{
  I64 i;
  CWS *tempa;
  m->n=0;
  for (i=0;i<WS_MAX_MATRIX_WORDS;i++) {
    m->side[i].next=m->side[i].last=&m->side[i].next;
    m->side[i].const_minus_one=-1;
    m->side[i].row_hits=0;
    m->side[i].col_hits=0;
    tempa=AMAlloc(sizeof(CWS));
    ws_structs_size+=MSize2(tempa);
    tempa->subs[0].hits=0;
    tempa->subs[0].repeats=ws_num_matrix_words;
    tempa->entries=ws_num_matrix_words;
    tempa->substructs=1;
    QueIns(tempa,m->side[i].last);
  }
}

CWS *WSStructFind(CWSMatrix *m,I64 col,I64 row,
			    I64 *_row_base,I64 *_substruct)
{
  CWS *tempa;
  I64 row_base,i,j;
 
  if (row>=ws_num_matrix_words ||
      col>=ws_num_matrix_words)
    return NULL;
  if (row<ws_num_matrix_words>>1) {
    tempa=m->side[col].next;
    row_base=0;
    j=row;
    while (row>=(i=tempa->entries)) {
      row-=i;
      tempa=tempa->next;
    }
    row_base+=j-row;
    for (*_substruct=0;row>=tempa->subs[*_substruct].repeats;*_substruct=*_substruct+1) {
      row-=tempa->subs[*_substruct].repeats;
      row_base+=tempa->subs[*_substruct].repeats;
    }
  } else {
    row=ws_num_matrix_words-row;
    tempa=m->side[col].last;
    row_base=ws_num_matrix_words;
    j=row;
    while (row>(i=tempa->entries)) {
      row-=i;
      tempa=tempa->last;
    }
    row_base+=j-row;
    for (*_substruct=tempa->substructs-1;row>tempa->subs[*_substruct].repeats;
	 *_substruct=*_substruct-1) {
      row-=tempa->subs[*_substruct].repeats;
      row_base-=tempa->subs[*_substruct].repeats;
    }
    row_base-=tempa->subs[*_substruct].repeats;
  }
  *_row_base=row_base;
  return tempa;
}

public I64 WSGetHits(CWSMatrix *m,I64 col,I64 row)
{
  I64 row_base,substruct;
  CWS *tempa;
  if (row>=ws_num_matrix_words ||
      col>=ws_num_matrix_words)
    return 0;
  tempa=WSStructFind(m,col,row,
		    &row_base,&substruct);
  return tempa->subs[substruct].hits;
}

CWSSub *WSNext(CWSSub *s,CWS **_parent)
{
  I64 i=(s(U8 *)-(&(*_parent)->subs)(U8 *))/sizeof(CWSSub)+1;
  if (i>=(*_parent)->substructs) {
    i=0;
    *_parent=(*_parent)->next;
    if ((*_parent)->substructs==-1)
      return NULL;
  }
  return &(*_parent)->subs[i];
}

CWSSub *WSLast(CWSSub *s,CWS **_parent)
{
  I64 i=(s(U8 *)-(&(*_parent)->subs)(U8 *))/sizeof(CWSSub)-1;
  if (i<0) {
    *_parent=(*_parent)->last;
    if ((*_parent)->substructs==-1)
      return NULL;
    i=(*_parent)->substructs-1;
  }
  return &(*_parent)->subs[i];
}

U0 WSRem(CWSSub *s,CWS *parent)
{
  I64 i=(s(U8 *)-(&parent->subs)(U8 *))/sizeof(CWSSub);
  if (i<0 || i>=WS_SUBSTRUCTS_PER_STRUCT || parent->substructs<1)
    return;
  if (parent->substructs==1) {
    QueRem(parent);
    ws_structs_size-=MSize2(parent);
    Free(parent);
  } else {
    parent->entries-=s->repeats;
    parent->substructs--;
    MemCpy(&parent->subs[i],&parent->subs[i+1],
	   (parent->substructs-i)*sizeof(CWSSub));
  }
}

CWSSub *WSIns(CWSSub *s,CWS **_parent,CWSSub *last)
{
  CWS *tempa,*parent=*_parent;
  I64 j,k,i=(last(U8 *)-(&parent->subs)(U8 *))/sizeof(CWSSub)+1;
  if (i>=WS_SUBSTRUCTS_PER_STRUCT) {
    i=0;
    parent=parent->next;
    *_parent=parent;
    if (parent->substructs==-1) {
      tempa=AMAlloc(sizeof(CWS));
      ws_structs_size+=MSize2(tempa);
      QueIns(tempa,parent->last);
      parent=tempa;
      *_parent=parent;
      tempa->entries=0;
      tempa->substructs=0;
    }
  }
  if (parent->substructs==WS_SUBSTRUCTS_PER_STRUCT) {
    tempa=parent->next;
    if (tempa->substructs==-1 || tempa->substructs==WS_SUBSTRUCTS_PER_STRUCT) {
      tempa=AMAlloc(sizeof(CWS));
      ws_structs_size+=MSize2(tempa);
      tempa->entries=0;
      tempa->substructs=0;
      QueIns(tempa,parent);
      k=0;
      for (j=i;j<parent->substructs;j++) {
	MemCpy(&tempa->subs[k],&parent->subs[j],sizeof(CWSSub));
	tempa->entries+=tempa->subs[k++].repeats;
	tempa->substructs++;
      }
      parent->entries-=tempa->entries;
      parent->substructs-=tempa->substructs;
    } else {
      for (j=tempa->substructs-1;j>=0;j--)
	MemCpy(&tempa->subs[j+1],&tempa->subs[j],sizeof(CWSSub));
      MemCpy(&tempa->subs[0],
	&parent->subs[WS_SUBSTRUCTS_PER_STRUCT-1],
	sizeof(CWSSub));
      parent->entries-=tempa->subs[0].repeats;
      parent->substructs--;
      tempa->entries+=tempa->subs[0].repeats;
      tempa->substructs++;
      for (j=WS_SUBSTRUCTS_PER_STRUCT-2;j>=i;j--)
	MemCpy(&parent->subs[j+1],
	       &parent->subs[j],sizeof(CWSSub));
    }
  } else {
    for (j=parent->substructs-1;j>=i;j--)
      MemCpy(&parent->subs[j+1],
	     &parent->subs[j],sizeof(CWSSub));
  }
  parent->entries+=s->repeats;
  parent->substructs++;
  MemCpy(&parent->subs[i],s,sizeof(CWSSub));
  return &parent->subs[i];
}

CWSSub *WSCondense(CWSSub *s,CWS **_parent)
{
  CWSSub *temps,*temps_l,*temps_n;
  CWS *tempa_l,*tempa_n;
  if ((*_parent)->substructs==-1)
    return NULL;
  temps=s;
  tempa_l=*_parent;
  temps_l=WSLast(temps,&tempa_l);
  if (temps_l && temps_l->hits==temps->hits) {
    temps_l->repeats+=temps->repeats;
    tempa_l->entries+=temps->repeats;
    WSRem(temps,*_parent);
    *_parent=tempa_l;
    temps=temps_l;
  }
  tempa_n=*_parent;
  temps_n=WSNext(temps,&tempa_n);
  if (temps_n && temps_n->hits==temps->hits) {
    temps->repeats=temps->repeats+temps_n->repeats;
    (*_parent)->entries=(*_parent)->entries+temps_n->repeats;
    WSRem(temps_n,tempa_n);
  }
  return temps;
}

I64 WSHitsInc(CWSMatrix *m,I64 col,I64 row)
{
  I64 row_base,substruct,repeats;
  CWSSub sn,*temps,*temps1,*temps_l;
  CWS	 *tempa,*tempa1,*tempa_l;

  if (row>=ws_num_matrix_words ||
      col>=ws_num_matrix_words)
    return 0;
  tempa=WSStructFind(m,col,row,&row_base,&substruct);
  temps=&tempa->subs[substruct];
  repeats=temps->repeats;
  if (repeats==1) {
    temps->hits++;
    temps=WSCondense(temps,&tempa);
  } else {
    if (row==row_base) {
      temps->repeats--;
      tempa->entries--;
      sn.hits=1;
      sn.repeats=1;
      tempa_l=tempa;
      if (!(temps_l=WSLast(&tempa->subs[substruct],&tempa_l))) {
	tempa_l=tempa->last; //header
	temps_l=&tempa_l->subs[WS_SUBSTRUCTS_PER_STRUCT-1];
      }
      temps=WSIns(&sn,&tempa_l,temps_l);
      temps=WSCondense(temps,&tempa_l);
    } else if (row==repeats-1+row_base) {
      temps->repeats--;
      tempa->entries--;
      sn.hits=1;
      sn.repeats=1;
      temps=WSIns(&sn,&tempa,temps);
      temps=WSCondense(temps,&tempa);
    } else {
      tempa->entries-=repeats;
      sn.hits=temps->hits+1;
      sn.repeats=1;
      tempa1=tempa;
      temps1=temps;
      temps=WSIns(&sn,&tempa1,temps1);
      temps1->repeats=row-row_base;
      tempa->entries+=temps1->repeats;
      sn.repeats=row_base+repeats-(row+1);
      sn.hits=temps1->hits;
      WSIns(&sn,&tempa1,temps);
    }
  }
  m->n++;
  m->side[col].col_hits++;
  m->side[row].row_hits++;
  return temps->hits;
}

U0 WSSingleFileAdd(U8 *buf,I64 pass)
{
  U8 ch,*ptr=buf,*ptr2,*ptr3;
  CHashWS *cur_ptr,*last_ptr=NULL;
  while (TRUE) {
    while (TRUE) {
      if (ch=*ptr++) {
	if (Bt(alpha_numeric_bitmap,ch))
	  break;
      } else
	return;
    }
    ptr3=ptr;
    ptr2=ptr;
    ptr--;
    while (TRUE) {
      if (ch=*ptr2++) {
	if (Bt(alpha_numeric_bitmap,ch))
	  *ptr3++=ch;
	else if (ch!=CH_CURSOR)
	  break;
      } else {
	ptr2--;
	break;
      }
    }
    *ptr3=0;
    if (pass==1)
      WSHashAdd(ptr);
    else {
      cur_ptr=HashFind(ptr,ws_hash_table,HTT_WORD);
      if (cur_ptr->num<0)
	cur_ptr=NULL;
      else
	if (last_ptr)
	  WSHitsInc(ws_matrix,last_ptr->num,cur_ptr->num);
      last_ptr=cur_ptr;
    }
    ptr=ptr2;
  }
}

I64 WSCompare(CHashWS *e1,CHashWS *e2)
{
  I64 r;
  if (!(r=e2->hits - e1->hits))
    r=StrCmp(e1->str,e2->str);
  return r;
}

I64 WSSkipCrap(U8 *src,I64 len)
{
  I64 j;
  j=len-1;
  while (j>=0) {
    if (Bt(alpha_numeric_bitmap,src[j]))
      break;
    else
      j--;
  }
  return j+1;
}

I64 WSPriorWordInStr(U8 *src,U8 *dst,I64 len,I64 buf_size)
{
  I64 i,j=0,k;
  Bool cont=TRUE;
  i=len-1;
  while (i>=0 && cont) {
    if (!Bt(alpha_numeric_bitmap,src[i]))
      cont=FALSE;
    else
      i--;
  }
  if (i>=-1 && len>0)
    for (k=i+1;k<len && j<buf_size-1;k++)
      dst[j++]=src[k];
  dst[j]=0;
  return i+1;
}


Bool WSPartialMatch(U8 *partial,U8 *complete)
{
  I64 i,l=StrLen(partial);
  for (i=0;i<l;i++)
    if (partial[i]!=complete[i])
      return FALSE;
  return TRUE;
}

U0 WSFillIn(I64 n)
{
  U8 *s;
  I64 len;
  if (0<=--n<ws_num_fillins) {
    s=ws_sorted_words[ws_fillin_matches[n]]->str;
    len=StrLen(s);
    if (len>ws_prtial_len)
      Auto(s+ws_prtial_len);
  }
}

U0 WSMan(I64 n,CTask *parent_task=NULL)
{
  U8 *buf;
  CHashWS *tempw;
  CHashSrcSym *temph;
  Bool old_preempt=Preempt(OFF);
  if (0<=--n<ws_num_fillins) {
    if (tempw=ws_sorted_words[ws_fillin_matches[n]]) {
      if (temph=HashFind(tempw->str,Fs->hash_table,HTG_SRC_SYM)) {
	if (temph->src_link) {
	  buf=MPrint("Ed(\"%s\");",temph->src_link);
	  Preempt(old_preempt);
	  PopUp(buf,parent_task);
	  Free(buf);
	} else if (temph->type&HTT_DEFINE_STR && temph(CHashDefineStr *)->caller)
	  E(temph(CHashDefineStr *)->caller,DFT_ADDR_LINK_BIN_SIZE);
      }
    }
  }
  Preempt(old_preempt);
}

U0 WSFillInAdd(I64 i,I64 j,I64 old_num_fillins)
{
  Bool cont;
  I64 k;
  if (ws_num_fillins<WS_MAX_FILLINS || i>ws_fillin_scores[ws_num_fillins-1] ||
      i==ws_fillin_scores[ws_num_fillins-1] && ws_sorted_words[j]->hits>ws_fillin_hits[ws_num_fillins-1]) {
    cont=TRUE;
    for (k=ws_num_fillins-1;k>=old_num_fillins && cont;k--)
      if (i<=ws_fillin_scores[k] || i==ws_fillin_scores[k] && ws_sorted_words[j]->hits<=ws_fillin_hits[k]) {
	cont=FALSE;
	k++; //offset k--
      } else {
	ws_fillin_scores[k+1]=ws_fillin_scores[k];
	ws_fillin_matches[k+1]=ws_fillin_matches[k];
	ws_fillin_hits[k+1]=ws_fillin_hits[k];
      }
    if (ws_num_fillins<WS_MAX_FILLINS)
      ws_num_fillins++;
    ws_fillin_scores[k+1]=i;
    ws_fillin_hits[k+1]=ws_sorted_words[j]->hits;
    ws_fillin_matches[k+1]=j;
  }
}

U0 WSPutChoices(CDoc *focus_l,CDocEntry *doc_e,CTask *focus_task)
{
  I64 i,j,k,data_col;
  U8 ch=0,*buf,*buf1,*src=NULL,*st;
  CHashWS *w2;
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
  data_col=WSPriorWordInStr(src,buf,data_col,256);
  ws_prtial_len=StrLen(buf);
  data_col=WSSkipCrap(src,data_col);
  data_col=WSPriorWordInStr(src,buf1,data_col,256);

  if (ws_cur_word && !StrCmp(ws_cur_word,buf))
    goto pc_done;

  st=ws_cur_word;
  ws_cur_word=AStrNew(buf);
  Free(st);
  if (!IsSysDeterministic)
    Preempt(ON);
  if (StrLen(buf1)&&ws_hash_table)
    w2=HashFind(buf1,ws_hash_table,HTT_WORD);
  else
    w2=NULL;
  ws_num_fillins=0;
  if (w2 && w2->num<ws_num_matrix_words)
    for (j=0;j<ws_num_matrix_words && tS<timeout_time;j++)
      if (WSPartialMatch(ws_cur_word,ws_sorted_words[j]->str)) {
	i=WSGetHits(ws_matrix,w2->num,ws_sorted_words[j]->num);
	WSFillInAdd(i,j,0);
      }
  k=ws_num_fillins;
  for (j=0;j<ws_num_words && tS<timeout_time;j++)
    if (WSPartialMatch(ws_cur_word,ws_sorted_words[j]->str))
      WSFillInAdd(0,j,k);
  Preempt(OFF);

pc_done:
  DocRst(doc,TRUE);
  doc->flags|=DOCF_MIN_SIZE;
  DocCursor;
  Fs->text_attr=LTBLUE<<4+WHITE;
  Fs->border_src =BDS_CONST;
  Fs->border_attr=LTBLUE<<4+BLACK;
  DocPrint(doc,"Word:%s\n",ws_cur_word);
  for (i=0;i<ws_num_fillins;i++) {
    st=ws_sorted_words[ws_fillin_matches[i]]->str;
    ch=CH_SPACE;
    if (TaskValidate(focus_task)) {
      if (temph=HashFind(st,focus_task->hash_table,HTG_SRC_SYM))
	if (temph->src_link ||
	    temph->type&HTT_DEFINE_STR && temph(CHashDefineStr *)->caller)
	  ch='*';
    }
    DocPrint(doc,"%cF%02d %s\n",ch,i+1,st);
  }
  WSDDictWordsAdd(doc);

  DocRecalc(doc);
  if (!LBts(&Fs->display_flags,DISPLAYf_SHOW))
    WinZBufUpdate;

  Free(src);
  Free(buf);
  Free(buf1);
}

U0 WSTask(I64)
{
  CTask *focus_task,*original_focus_task;
  CDoc *doc;
  CDocEntry *doc_e;
  TaskInitExt;
  UseTermDoc;
  SetWinHorz(51,Fs->win_right);
  LBts(&Fs->display_flags,DISPLAYf_WIN_ON_TOP);
  do {
    Sleep(333);
    if (Bt(&ws_flags,WSf_ENABLE)) {
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
	  while (doc_e->last->type_u8 !=DOCT_NEW_LINE && doc_e->last!=doc)
	    doc_e=doc_e->last;
	  if (Bt(&ws_flags,WSf_ENABLE))
	    WSPutChoices(doc,doc_e,focus_task);
	  else
	    DocUnlock(doc);
	} else
	  DocUnlock(doc);
      }
    }
  } while (!ScanKey);
  ws_task=NULL;
}

U0 WSMainFileLstTraverse(U8 *files_find_mask,I64 pass)
{
  I64 i,cnt;
  CDirEntry **flst=NULL;
  U8 *buf;
  try {
    flst=FilesOrder(FilesFind(files_find_mask,
	FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES),&cnt);
    for (i=0;i<cnt;i++) {
      "%s\n",flst[i]->full_name;
      buf=TextFileRead(flst[i]->full_name);
      WSSingleFileAdd(buf,pass);
      Free(buf);
      DirEntryDel(flst[i]);
    }
  } catch
    Fs->catch_except=TRUE;
  Free(flst);
}

U0 WSMatrixDel(CWSMatrix *m)
{
  I64 i;
  CWS *tempw,*tempw1,*tempw2;
  if (!m) return;
  for (i=0;i<WS_MAX_MATRIX_WORDS;i++) {
    tempw1=&m->side[i];
    tempw=tempw1->next;
    while (tempw!=tempw1) {
      tempw2=tempw->next;
      Free(tempw);
      tempw=tempw2;
    }
  }
  Free(m);
}


public Bool WordStat(Bool val=OFF)
{
  Bool old_wordstat=FALSE,
	 old_preempt=Preempt(OFF);
  if (val) {
    if (TaskValidate(ws_task))
      old_wordstat=TRUE;
    else {
      ws_task=Spawn(&WSTask,NULL,"WordStat");
      TaskWait(ws_task);
    }
    if (!LBts(&ws_flags,WSf_ENABLE))
      WinToTop(ws_task);
  } else {
    LBtr(&ws_flags,WSf_ENABLE);
    if (TaskValidate(ws_task)) {
      old_wordstat=TRUE;
      Kill(ws_task);
      while (TaskValidate(ws_task))
	Yield;
      ws_task=NULL;
    }
  }
  Preempt(old_preempt);
  return old_wordstat;
}

U0 WSProgressTask(Bool *_start_flag)
{
  I64 start=sys_dsk_writes;
  TaskInitExt; //needed for disk access
  progress1=0;
  progress1_max=(FileSizeExpanded(WSD_DEF_FILENAME_Z)+BLK_SIZE-1)>>BLK_SIZE_BITS;
  StrCpy(progress1_desc,"Uncompressing Dictionary");
  start=sys_dsk_writes;
  *_start_flag=TRUE;
  while (progress1<progress1_max) {
    progress1=sys_dsk_writes-start;
    Sleep(10);
  }
  *progress1_desc=progress1=progress1_max=0;
}

public U0 WSInit(U8 *mask=NULL)
{
  I64 i,j=0;
  Bool start_flag;
  CHashWS *tempw;
  CBlkDev *bdev=Drv2BlkDev;

  while (LBts(&ws_flags,WSf_INIT_IN_PROGRESS))
    Yield;

  WordStat;

  if (IsDrvWritable && FileFind(WSD_DEF_FILENAME_Z) && !FileFind(WSD_DEF_FILENAME)) {
    if (bdev->type!=BDT_RAM) {
      start_flag=FALSE;
      Spawn(&WSProgressTask,&start_flag);
      while (!start_flag)
	Yield;
    }
    Move(WSD_DEF_FILENAME_Z,WSD_DEF_FILENAME);
  }

  HashTableDel(ws_hash_table);
  ws_hash_table=HashTableNew(0x4000,adam_task);

  ws_num_words=0;
  ws_num_matrix_words=0;
  ws_structs_size=0;
  Free(ws_cur_word);
  ws_cur_word=NULL;

  "\n\nPass #1\n";
  if (mask) WSMainFileLstTraverse(mask,1);
  Free(ws_sorted_words);
  ws_sorted_words=AMAlloc(sizeof(U8 *)*ws_num_words);

  tempw=ws_hash_table->next_added;
  while (tempw!=ws_hash_table) {
    ws_sorted_words[j++]=tempw;
    tempw=tempw->next_added;
  }
  QSortI64(ws_sorted_words,ws_num_words,&WSCompare);
  for (i=0;i<ws_num_words;i++)
    ws_sorted_words[i]->num=i;
  if (ws_num_words<=WS_MAX_MATRIX_WORDS)
    ws_num_matrix_words=ws_num_words;
  else
    ws_num_matrix_words=WS_MAX_MATRIX_WORDS;

  WSMatrixDel(ws_matrix);
  ws_matrix=AMAlloc(sizeof(CWSMatrix));
  WSSidesInit(ws_matrix);

  "\n\nPass #2\n";
  if (mask) WSMainFileLstTraverse(mask,2);

  WSDWordsLoad;
  WordStat(ON);
  LBtr(&ws_flags,WSf_INIT_IN_PROGRESS);
}

I64 WordStatSize()
{
  if (ws_hash_table)
    return HashTableSize2(ws_hash_table)+ws_structs_size+
	   MSize2(ws_sorted_words)+MSize2(ws_matrix)+MSize2(ws_dict_word_lst);
  else
    return 0;
}

#help_index ""
