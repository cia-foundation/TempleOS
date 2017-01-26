#help_index "AutoComplete/Dictionary"
acd.has_words=FileFind(ACD_WORD_FILENAME);
acd.has_defs =FileFind(ACD_DEF_FILENAME)||FileFind(ACD_DEF_FILENAME_Z);

public U0 ACDWordsLoad()
{//Put words from word list into hash table.
  I64 size;
  CHashGeneric *tmph;
  U8 *in_ptr,*in_start,*st2;
  U16 *d;
  acd.num_words=0;
  if (in_ptr=FileRead(ACD_WORD_FILENAME,&size)) {
    in_start=in_ptr;
    Free(acd.word_lst);
    acd.word_lst=AMAlloc(size);
    MemCpy(acd.word_lst,in_start,size);
    acd.word_lst_size=size;

    while (in_ptr<in_start+size) {
      if (*in_ptr==ACD_WORD_CHAR)
	in_ptr++;
      if (*in_ptr) {
	st2=MStrUtil(in_ptr,SUF_TO_UPPER);
	tmph=ACAlloc(sizeof(CHashGeneric)+StrLen(st2)+1);
	StrCpy(tmph+1,st2);
	Free(st2);
	in_ptr+=StrLen(in_ptr)+1;
	tmph->str=tmph+1;
	tmph->use_cnt=1;
	tmph->type=HTT_DICT_WORD;
	d=in_ptr;
	tmph->user_data0=*d;
	in_ptr+=2;
	HashAdd(tmph,ac.hash_table);
	acd.num_words++;
      } else
	in_ptr+=3;
    }
    Free(in_start);
  }
}

#help_index "AutoComplete"
CHashAC *ACHashAdd(U8 *w)
{
  CHashAC *tmpw=HashFind(w,ac.hash_table,HTT_WORD);
  if (tmpw) {
    tmpw->hits++;
    return tmpw;
  }
  tmpw=ACAlloc(sizeof(CHashAC));
  tmpw->str=AStrNew(w);
  tmpw->type=HTT_WORD;
  tmpw->use_cnt=1;
  tmpw->hits=1;
  HashAdd(tmpw,ac.hash_table);
  ac.num_words++;
  return tmpw;
}

U0 ACSingleFileAdd(U8 *buf)
{
  I64 ch;
  U8 *ptr=buf,*ptr2,*ptr3;
  while (TRUE) {
    while (TRUE) {
      if (ch=*ptr++) {
	if (Bt(char_bmp_alpha_numeric,ch))
	  break;
      } else
	return;
    }
    ptr3=ptr;
    ptr2=ptr;
    ptr--;
    while (TRUE) {
      if (ch=*ptr2++) {
	if (Bt(char_bmp_alpha_numeric,ch))
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

U0 ACMainFileLstTraverse(U8 *files_find_mask)
{
  U8 *buf;
  CDirEntry *tmpde,*tmpde1;
  try {
    tmpde=tmpde1=FilesFind(files_find_mask,
	  FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES|FUF_CLUS_ORDER);
    while (tmpde) {
      "%s\n",tmpde->full_name;
      buf=FileRead(tmpde->full_name);
      ACSingleFileAdd(buf);
      Free(buf);
      tmpde=tmpde->next;
    }
  } catch
    Fs->catch_except=TRUE;
  DirTreeDel(tmpde1);
}

U0 ACProgressTask(Bool *_start_flag)
{
  I64 start=blkdev.write_cnt;
  progress1=0;
  progress1_max=(Size(ACD_DEF_FILENAME_Z,"+x+s")+BLK_SIZE-1)>>BLK_SIZE_BITS;
  StrCpy(progress1_desc,"Uncompressing Dictionary");
  start=blkdev.write_cnt;
  *_start_flag=TRUE;
  while (progress1<progress1_max) {
    progress1=blkdev.write_cnt-start;
    Sleep(10);
  }
  *progress1_desc=progress1=progress1_max=0;
}

public U0 ACInit(U8 *mask=NULL)
{//Read files and build AutoComplete statistics.
  Bool start_flag;
  CBlkDev *bd=Let2BlkDev;

  LBtr(&sys_run_level,RLf_AUTO_COMPLETE);
  AutoComplete;
  while (LBts(&ac.flags,ACf_INIT_IN_PROGRESS))
    Yield;
  if (DrvIsWritable && FileFind(ACD_DEF_FILENAME_Z) &&
	!FileFind(ACD_DEF_FILENAME)) {
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
  LBts(&sys_run_level,RLf_AUTO_COMPLETE);
  AutoComplete(ON);
}

I64 AutoCompleteSize()
{
  if (ac.hash_table)
    return HashTableSize2(ac.hash_table)+MSize2(acd.word_lst);
  else
    return 0;
}
