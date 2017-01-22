I64 HashTypeNum(CHash *tmph)
{//Return bit num of hash type, limited to just types.
  if (tmph)
    return Bsf(tmph->type&HTG_TYPE_MASK);
  else
    return -1;
}

I64 HashVal(CHash *tmph)
{//Returns most likely desired value.
  switch [HashTypeNum(tmph)] {
    case HTt_EXPORT_SYS_SYM:
      return tmph(CHashExport *)->val;
    case HTt_IMPORT_SYS_SYM:
      return tmph(CHashImport *)->module_base;
    case HTt_DEFINE_STR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
    case HTt_WORD:
    case HTt_DICT_WORD:
    case HTt_OPCODE:
    case HTt_HELP_FILE:
      return tmph;
    case HTt_GLBL_VAR:
      if (tmph(CHashGlblVar *)->flags&GVF_EXTERN)
	return &tmph(CHashGlblVar *)->data_addr;
      else
	return tmph(CHashGlblVar *)->data_addr;
    case HTt_FUN:
      if (Bt(&tmph(CHashFun *)->flags,Cf_EXTERN))
	return tmph;
      else
	return tmph(CHashFun *)->exe_addr;
    case HTt_REG:
      return tmph(CHashReg *)->reg_num|tmph(CHashReg *)->reg_type<<8;
    case HTt_KEYWORD:
    case HTt_ASM_KEYWORD:
    case HTt_MODULE:
    case HTt_FILE:
    case HTt_FRAME_PTR:
      return tmph(CHashGeneric *)->user_data0;

    case -1:		//nobound switch
    case HTt_TYPES_NUM: //nobound switch
    default:
      return 0;
  }
}

CHashTable *HashTableNew(I64 size,CTask *mem_task=NULL)
{//New hash table, power-of-two in size.
  CHashTable *table;
  table=CAlloc(sizeof(CHashTable),mem_task);
  table->body=CAlloc(size<<3,mem_task);
  table->mask=size-1;
  return table;
}

U0 HashDel(CHashSrcSym *tmph)
{//Free a std TempleOS system hash entry.
  if (!tmph) return;
  if (!(tmph->type&HTT_DICT_WORD))
    Free(tmph->str);
  if (tmph->type & HTG_SRC_SYM) {
    Free(tmph->src_link);
    Free(tmph->idx);
    Free(tmph->import_name);
    LinkedLstDel(tmph->ie_lst);
    if (tmph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      Free(tmph->dbg_info);
    if (tmph->type & (HTT_FUN | HTT_CLASS))
//Assumes code not on heap, so doesn't Free.
    //$LK,"ClassMemberLstDel",A="MN:ClassMemberLstDel"$() is an import to the Kernel module
      ClassMemberLstDel(tmph);
    else if (tmph->type&HTT_DEFINE_STR)
      Free(tmph(CHashDefineStr *)->data);
    else if (tmph->type & HTT_GLBL_VAR) {
      if (!(tmph(CHashGlblVar *)->flags&GVF_ALIAS))
	Free(tmph(CHashGlblVar *)->data_addr);
      LinkedLstDel(tmph(CHashGlblVar *)->dim.next);
      if (tmph(CHashGlblVar *)->fun_ptr)
	HashDel(tmph(CHashGlblVar *)->fun_ptr
	      -tmph(CHashGlblVar *)->fun_ptr->ptr_stars_cnt);
    }
  } else if (tmph->type & HTT_FILE)
    Free(tmph(CHashGeneric *)->user_data0);
  Free(tmph);
}

U0 HashTableDel(CHashTable *table)
{//Free std system hash table, calling $LK,"HashDel",A="MN:HashDel"$() on entries.
  I64 i;
  CHashSrcSym *tmph,*tmph1;
  if (!table) return;
  for (i=0;i<=table->mask;i++) {
    tmph=table->body[i];
    while (tmph) {
      tmph1=tmph->next;
      HashDel(tmph);
      tmph=tmph1;
    }
  }
  Free(table->body);
  Free(table);
}

I64 HashTablePurge(CHashTable *table)
{//Eliminate ExportSysSyms that have been usurped.
  I64 i,res=0;
  CHashSrcSym *tmph,*tmph1,*tmph2;
  if (!table) return 0;
  PUSHFD
  CLI	 //Precaution
  for (i=0;i<=table->mask;i++) {
    tmph=table->body[i];
    while (tmph) {
      tmph1=tmph->next; //We delete only older ones
      if (tmph->type&(HTT_FUN|HTT_GLBL_VAR)) {
	tmph2=tmph->next; //Older always later in chain
	while (tmph2) {
	  if ((tmph2->type&HTT_EXPORT_SYS_SYM ||
		tmph2->type&HTG_TYPE_MASK==HTT_INVALID) &&
		!StrCmp(tmph2->str,tmph->str)) {
	    if (tmph2->type&HTG_TYPE_MASK==HTT_INVALID)
	      tmph2->type=HTT_KEYWORD;//Won't delete HTT_INVALID
	    HashRemDel(tmph2,table);
	    res++;
	    break;
	  }
	  tmph2=tmph2->next;
	}
      }
      tmph=tmph1;
    }
  }
  POPFD
  return res;
}

CHashGeneric *HashGenericAdd(U8 *name,I64 type,
	I64 u0=0,I64 u1=0,I64 u2=0,CTask *task=NULL)
{//Add any type to task hash_table, 3 user_data values.
  if (!task) task=Fs;
  CHashGeneric *res=CAlloc(sizeof(CHashGeneric),task);
  res->type=type;
  res->user_data0=u0;
  res->user_data1=u1;
  res->user_data2=u2;
  res->str=StrNew(name,task);
  HashAdd(res,task->hash_table);
  return res;
}

U0 HashSrcFileSet(CCmpCtrl *cc,CHashSrcSym *h,I64 line_num_offset=0)
{//Set $LK,"CHashSrcSym",A="MN:CHashSrcSym"$ link and help_index by cur cc pos.
  CLexFile *tmpf=cc->lex_include_stk;
  I64 line_num=tmpf->line_num+line_num_offset;
  if (line_num<1) line_num=1;
  Free(h->src_link);
  h->src_link=MStrPrint("FL:%s,%d",tmpf->full_name,line_num);
  if (Bt(&cc->opts,OPTf_KEEP_PRIVATE))
    h->type|=HTF_PRIVATE;
  Free(h->idx);
  if (cc->cur_help_idx && *cc->cur_help_idx)
    h->idx=StrNew(cc->cur_help_idx);
  else
    h->idx=NULL;
}

CHashGeneric *HashPublic(U8 *st,I64 mask,Bool val=TRUE)
{//Mark a hash entry as public and $LK,"HashSrcFileSet",A="MN:HashSrcFileSet"$().
  CHashGeneric *res;
  if (res=HashFind(st,Fs->hash_table,mask)) {
    if (val)
      res->type|=HTF_PUBLIC;
    else
      res->type&=~HTF_PUBLIC;
    if (res->type&HTG_SRC_SYM)
      HashSrcFileSet(Fs->last_cc,res);
    return res;
  } else
    return NULL;
}

I64 HashLstAdd(U8 *lst,I64 type,CHashTable *table)
{//Add a list to a hash table.
  I64 i=0;
  CHashGeneric *tmph;
  if (lst) {
    while (*lst) {
      if (*lst=='@')
	lst++;
      else
	i++;
      tmph=CAlloc(sizeof(CHashGeneric));
      tmph->user_data0=i-1;
      tmph->str=StrNew(lst);
      tmph->type=type;
      HashAdd(tmph,table);
      while (*lst++);
    }
  }
  return i;
}

I64 HashDefineLstAdd(U8 *dname,I64 type,CHashTable *table)
{//Add define list to a hash table. See $LK,"::/Adam/DolDoc/DocInit.HC",A="FF:::/Adam/DolDoc/DocInit.HC,HashDefineLstAdd"$.
  CHashDefineStr *tmph;
  if (tmph=HashFind(dname,Fs->hash_table,HTT_DEFINE_STR))
    return HashLstAdd(tmph->data,type,table);
  else
    return 0;
}

I64 FramePtr(U8 *name,CTask *task=NULL)
{//Find entry in task->hash_table, Return user_data.
  CHashGeneric *tmph;
  if (!task) task=Fs;
  if (tmph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return tmph->user_data0;
  else
    return 0;
}

CHashGeneric *FramePtrAdd(U8 *name,I64 val=0,CTask *task=NULL)
{//Add named value to task->hash_table.
  return HashGenericAdd(name,HTT_FRAME_PTR,val,0,0,task);
}

I64 FramePtrSet(U8 *name,I64 val,CTask *task=NULL)
{//Find hash entry in task->hash_table. Change user_data0.
  CHashGeneric *tmph;
  if (!task) task=Fs;
  if (tmph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return LXchgI64(&tmph->user_data0,val);
  else
    return 0;
}

I64 FramePtrDel(U8 *name,CTask *task=NULL)
{//Remove entry and delete.
  CHashGeneric *tmph;
  I64 res=0;
  if (!task) task=Fs;
  if (tmph=HashFind(name,task->hash_table,HTT_FRAME_PTR)) {
    res=tmph->user_data0;
    HashRemDel(tmph,task->hash_table);
  }
  return res;
}
