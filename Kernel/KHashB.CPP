I64 HashTypeNum(CHash *temph)
{//Return bit num of hash type, limited to just types.
  if (temph)
    return Bsf(temph->type&HTG_TYPE_MASK);
  else
    return -1;
}

I64 HashVal(CHash *temph)
{//Returns most likely desired value.
  switch [HashTypeNum(temph)] {
    case HTt_EXPORT_SYS_SYM:
      return temph(CHashExport *)->val;
    case HTt_IMPORT_SYS_SYM:
      return temph(CHashImport *)->module_base;
    case HTt_DEFINE_STR:
    case HTt_CLASS:
    case HTt_INTERNAL_TYPE:
    case HTt_WORD:
    case HTt_DICT_WORD:
    case HTt_OPCODE:
    case HTt_HELP_FILE:
      return temph;
    case HTt_GLBL_VAR:
      if (temph(CHashGlblVar *)->flags&GVF_EXTERN)
	return &temph(CHashGlblVar *)->data_addr;
      else
	return temph(CHashGlblVar *)->data_addr;
    case HTt_FUN:
      if (Bt(&temph(CHashFun *)->flags,Cf_EXTERN))
	return temph;
      else
	return temph(CHashFun *)->executable_addr;
    case HTt_REG:
      return temph(CHashReg *)->reg_num|temph(CHashReg *)->reg_type<<8;
    case HTt_KEYWORD:
    case HTt_ASM_KEYWORD:
    case HTt_MODULE:
    case HTt_FILE:
    case HTt_FRAME_PTR:
      return temph(CHashGeneric *)->user_data0;

    case -1:		//nobound switch
    case HTt_NUM_TYPES: //nobound switch
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

U0 HashDel(CHashSrcSym *temph)
{//Free a std TempleOS system hash entry.
  if (!temph) return;
  if (!(temph->type&HTT_DICT_WORD))
    Free(temph->str);
  if (temph->type & HTG_SRC_SYM) {
    Free(temph->src_link);
    Free(temph->idx);
    Free(temph->import_name);
    LinkedLstDel(temph->ie_lst);
    if (temph->type & (HTT_FUN | HTT_EXPORT_SYS_SYM))
      Free(temph->dbg_info);
    if (temph->type & (HTT_FUN | HTT_CLASS))
//Assumes code not on heap, so doesn't Free.
    //$LK,"MemberLstDel",A="MN:MemberLstDel"$() is an import to the Kernel module
      MemberLstDel(temph);
    else if (temph->type&HTT_DEFINE_STR)
      Free(temph(CHashDefineStr *)->data);
    else if (temph->type & HTT_GLBL_VAR) {
      if (!(temph(CHashGlblVar *)->flags&GVF_ALIAS))
	Free(temph(CHashGlblVar *)->data_addr);
      LinkedLstDel(temph(CHashGlblVar *)->dim.next);
      if (temph(CHashGlblVar *)->fun_ptr)
	HashDel(temph(CHashGlblVar *)->fun_ptr
	      -temph(CHashGlblVar *)->fun_ptr->ptr_stars_cnt);
    }
  } else if (temph->type & HTT_FILE)
    Free(temph(CHashGeneric *)->user_data0);
  Free(temph);
}

U0 HashTableDel(CHashTable *table)
{//Free std system hash table, calling $LK,"HashDel",A="MN:HashDel"$() on entries.
  I64 i;
  CHashSrcSym *temph,*temph1;
  if (!table) return;
  for (i=0;i<=table->mask;i++) {
    temph=table->body[i];
    while (temph) {
      temph1=temph->next;
      HashDel(temph);
      temph=temph1;
    }
  }
  Free(table->body);
  Free(table);
}

I64 HashTablePurge(CHashTable *table)
{//Eliminate ExportSysSyms that have been usurped.
  I64 i,res=0;
  CHashSrcSym *temph,*temph1,*temph2;
  if (!table) return 0;
  PUSHFD
  CLI	 //Precaution
  for (i=0;i<=table->mask;i++) {
    temph=table->body[i];
    while (temph) {
      temph1=temph->next; //We delete only older ones
      if (temph->type&(HTT_FUN|HTT_GLBL_VAR)) {
	temph2=temph->next; //Older always later in chain
	while (temph2) {
	  if ((temph2->type&HTT_EXPORT_SYS_SYM ||
		temph2->type&HTG_TYPE_MASK==HTT_INVALID) &&
		!StrCmp(temph2->str,temph->str)) {
	    if (temph2->type&HTG_TYPE_MASK==HTT_INVALID)
	      temph2->type=HTT_KEYWORD;//Won't delete HTT_INVALID
	    HashRemDel(temph2,table);
	    res++;
	    break;
	  }
	  temph2=temph2->next;
	}
      }
      temph=temph1;
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
  CLexFile *tempf=cc->lex_include_stk;
  I64 line_num=tempf->line_num+line_num_offset;
  if (line_num<1) line_num=1;
  Free(h->src_link);
  h->src_link=MStrPrint("FL:%s,%d",tempf->full_name,line_num);
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
      HashSrcFileSet(Fs->last_lex,res);
    return res;
  } else
    return NULL;
}

I64 HashLstAdd(U8 *lst,I64 type,CHashTable *table)
{//Add a list to a hash table.
  I64 i=0;
  CHashGeneric *temph;
  if (lst) {
    while (*lst) {
      if (*lst=='@')
	lst++;
      else
	i++;
      temph=CAlloc(sizeof(CHashGeneric));
      temph->user_data0=i-1;
      temph->str=StrNew(lst);
      temph->type=type;
      HashAdd(temph,table);
      while (*lst++);
    }
  }
  return i;
}

I64 HashDefineLstAdd(U8 *dname,I64 type,CHashTable *table)
{//Add define list to a hash table. See $LK,"::/Adam/DolDoc/DocInit.CPP",A="FF:::/Adam/DolDoc/DocInit.CPP,HashDefineLstAdd"$.
  CHashDefineStr *temph;
  if (temph=HashFind(dname,Fs->hash_table,HTT_DEFINE_STR))
    return HashLstAdd(temph->data,type,table);
  else
    return 0;
}

I64 FramePtr(U8 *name,CTask *task=NULL)
{//Find entry in task->hash_table, Return user_data.
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return temph->user_data0;
  else
    return 0;
}

CHashGeneric *FramePtrAdd(U8 *name,I64 val=0,CTask *task=NULL)
{//Add named value to task->hash_table.
  return HashGenericAdd(name,HTT_FRAME_PTR,val,0,0,task);
}

I64 FramePtrSet(U8 *name,I64 val,CTask *task=NULL)
{//Find hash entry in task->hash_table. Change user_data0.
  CHashGeneric *temph;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR))
    return LXchgI64(&temph->user_data0,val);
  else
    return 0;
}

I64 FramePtrDel(U8 *name,CTask *task=NULL)
{//Remove entry and delete.
  CHashGeneric *temph;
  I64 res=0;
  if (!task) task=Fs;
  if (temph=HashFind(name,task->hash_table,HTT_FRAME_PTR)) {
    res=temph->user_data0;
    HashRemDel(temph,task->hash_table);
  }
  return res;
}
