#help_index "AutoComplete/Dictionary"
acd.has_words=FileFind(ACD_WORD_FILENAME);
acd.has_defs =FileFind(ACD_DEF_FILENAME)||FileFind(ACD_DEF_FILENAME_Z);

public U0 ACDWordsLoad()
{//Put words from word list into hash table.
  I64 size;
  CHashGeneric *temph;
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
	temph=ACAlloc(sizeof(CHashGeneric)+StrLen(st2)+1);
	StrCpy(temph+1,st2);
	Free(st2);
	in_ptr+=StrLen(in_ptr)+1;
	temph->str=temph+1;
	temph->use_cnt=1;
	temph->type=HTT_DICT_WORD;
	d=in_ptr;
	temph->user_data0=*d;
	in_ptr+=2;
	HashAdd(temph,ac.hash_table);
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
	if (Bt(chars_bmp_alpha_numeric,ch))
	  break;
      } else
	return;
    }
    ptr3=ptr;
    ptr2=ptr;
    ptr--;
    while (TRUE) {
      if (ch=*ptr2++) {
	if (Bt(chars_bmp_alpha_numeric,ch))
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
  CDirEntry *tempde,*tempde1;
  try {
    tempde=tempde1=FilesFind(files_find_mask,
	  FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES|FUF_CLUSTER_ORDER);
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

  while (LBts(&ac.flags,ACf_INIT_IN_PROGRESS))
    Yield;
  AutoComplete;

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
  AutoComplete(ON);
}

I64 AutoCompleteSize()
{
  if (ac.hash_table)
    return HashTableSize2(ac.hash_table)+MSize2(acd.word_lst);
  else
    return 0;
}
