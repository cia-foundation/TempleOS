U0 LexBackupLastChar(CCmpCtrl *cc)
{
  CLexFile *tempf=cc->lex_include_stk;
  tempf->buf_ptr=cc->cur_buf_ptr;
  if (cc->flags & CCF_USE_LAST_U16) {
    tempf->last_U16=cc->last_U16;
    cc->flags&=~CCF_USE_LAST_U16;
  } else
    tempf->last_U16=0;
}

U0 LexPush(CCmpCtrl *cc)
{//Create token-stream save point.
  CLexFile *tempf;
  LexBackupLastChar(cc);
  if (cc->lex_include_stk->last_U16)
    cc->flags|=CCF_USE_LAST_U16;
  tempf=MAllocIdentical(cc->lex_include_stk);
  tempf->next=cc->lex_prs_stk;
  cc->lex_prs_stk=tempf;
}

U0 LexPopRestore(CCmpCtrl *cc)
{//Restore token-stream saved-point.
//Bad things can happen if you cross an #include file boundary.
  CLexFile *tempf=cc->lex_prs_stk;
  cc->cur_buf_ptr=tempf->buf_ptr;
  if (cc->last_U16=tempf->last_U16)
    cc->flags|=CCF_USE_LAST_U16;
  else
    cc->flags&=~CCF_USE_LAST_U16;
  MemCpy(cc->lex_include_stk(U8 *)+sizeof(U8 *),tempf(U8 *)+sizeof(U8 *),
	sizeof(CLexFile)-sizeof(U8 *));
  cc->lex_prs_stk=tempf->next;
  Free(tempf);
}

U0 LexPopNoRestore(CCmpCtrl *cc)
{//Don't restore token-stream saved-point.
  CLexFile *tempf=cc->lex_prs_stk;
  cc->lex_prs_stk=tempf->next;
  Free(tempf);
}

I64 MemberMetaData(U8 *needle_str,CMemberLst *haystack_member_lst)
{//Find meta data name, return meta data val. See $LK,"::/Demo/ClassMeta.CPP"$.
  CMemberLstMeta *meta=haystack_member_lst->meta;
  while (meta) {
    if (!StrCmp(meta->str,needle_str))
      return meta->user_data;
    meta=meta->next;
  }
  return 0;
}

CMemberLstMeta *MemberMetaFind(U8 *needle_str,CMemberLst *haystack_member_lst)
{//Find meta data name, return meta data struct. See $LK,"::/Demo/ClassMeta.CPP"$.
  CMemberLstMeta *meta=haystack_member_lst->meta;
  while (meta) {
    if (!StrCmp(meta->str,needle_str))
      return meta;
    meta=meta->next;
  }
  return NULL;
}

CMemberLst *MemberFind(U8 *needle_str,CHashClass *haystack_class)
{//Find class member. See $LK,"ClassRep",A="MN:ClassRep"$() and $LK,"DocForm",A="MN:DocForm"$().
  I64 i;
  CMemberLst *tempm;
  do {
    tempm=haystack_class->member_lst;
    while (tempm) {
      if (!(i=StrCmp(tempm->str,needle_str))) {
	tempm->use_cnt++;
	return tempm;
      }
      if (i<=0)
	tempm=tempm->left;
      else
	tempm=tempm->right;
    }
  } while (haystack_class=haystack_class->base_class);
  return NULL;
}

CMemberLst *MemberFind2(CHashClass *needle_class,CHashClass *haystack_class)
{//Find class member, just offset<0. See $LK,"ClassRep",A="MN:ClassRep"$() and $LK,"DocForm",A="MN:DocForm"$().
  CMemberLst *tempm;
  tempm=haystack_class->member_lst2;
  while (tempm) {
    if (needle_class==tempm->member_class2)
      return tempm;
    if (needle_class<tempm->member_class2)
      tempm=tempm->left2;
    else
      tempm=tempm->right2;
  }
  return NULL;
}

U0 MemberAdd(CCmpCtrl *cc,CMemberLst *tempm,CHashClass *tempc,I64 mode)
{
  U8 *st=tempm->str;
  CMemberLst **tempm1,*tempm2;
  CHashClass *tempc2;
  if (MemberFind(st,tempc) && StrCmp(st,"pad") &&
	StrCmp(st,"reserved") && StrCmp(st,"_anon_"))
    LexExcept(cc,"Duplicate member at ");
  if (mode==PRS1B_LOCAL_VAR) {
    tempc2=tempm->member_class-tempm->member_class->ptr_stars_cnt;
    if (Bt(&cc->opts,OPTf_WARN_DUP_TYPES) && MemberFind2(tempc2,tempc))
      LexWarn(cc,"Duplicate type at ");
  } else
    tempc2=NULL;
  tempm->member_class2=tempc2;

  tempm1=&tempc->member_lst;
  while (tempm2=*tempm1) {
    if (StrCmp(tempm2->str,st)<=0)
      tempm1=&tempm2->left;
    else
      tempm1=&tempm2->right;
  }
  *tempm1=tempm;

  if (tempc2) {
    tempm1=&tempc->member_lst2;
    while (tempm2=*tempm1) {
      if (tempc2<tempm2->member_class2)
	tempm1=&tempm2->left2;
      else if (tempc2>tempm2->member_class2)
	tempm1=&tempm2->right2;
      else {
	tempm1=NULL;
	break;
      }
    }
    if (tempm1)
      *tempm1=tempm;
  }

  tempm->left=NULL;
  tempm->right=NULL;
  tempm->left2=NULL;
  tempm->right2=NULL;
  tempm2=tempc->last_in_member_lst;
  tempm2->next=tempc->last_in_member_lst=tempm;
}

CMemberLst *MemberLstNew(I64 _reg)
{
  CMemberLst *result=CAlloc(sizeof(CMemberLst));
  result->reg=_reg;
  return result;
}

U0 MemberLstDel(CHashClass *tempc)
{
  CMemberLst *tempm,*tempm1;
  CMemberLstMeta *temp_meta,*temp_meta1;
  tempm=tempc->member_lst;
  while (tempm) {
    tempm1=tempm->next;
    Free(tempm->str);
    LinkedLstDel(tempm->dim.next);
    if (tempm->flags & MLF_STR_DFT_AVAILABLE)
      Free(tempm->dft_val);
    if (tempm->flags & MLF_FUN)
      HashDel(tempm->fun_ptr-tempm->fun_ptr->ptr_stars_cnt);
    temp_meta=tempm->meta;
    while (temp_meta) {
      temp_meta1=temp_meta->next;
      Free(temp_meta->str);
      if (temp_meta->flags&MLMF_IS_STR)
	Free(temp_meta->user_data);
      Free(temp_meta);
      temp_meta=temp_meta1;
    }
    Free(tempm);
    tempm=tempm1;
  }
  tempc->size=0;
  tempc->last_in_member_lst=&tempc->member_lst;
  tempc->member_lst=NULL;
  tempc->member_lst2=NULL;
  tempc->member_cnt=0;
  if (tempc->type&HTT_FUN)
    tempc(CHashFun *)->arg_cnt=0;
}

I64 MemberLstSize(CHashClass *tempc)
{
  CMemberLst *tempm;
  CMemberLstMeta *temp_meta;
  I64 result=0;
  tempm=tempc->member_lst;
  while (tempm) {
    result+=MSize2(tempm->str);
    result+=LinkedLstSize(tempm->dim.next);
    if (tempm->flags & MLF_STR_DFT_AVAILABLE)
      result+=MSize2(tempm->dft_val);
    if (tempm->flags & MLF_FUN)
      result+=HashEntrySize2(tempm->fun_ptr-tempm->fun_ptr->ptr_stars_cnt);
    temp_meta=tempm->meta;
    while (temp_meta) {
      result+=MSize2(temp_meta->str);
      if (temp_meta->flags&MLMF_IS_STR)
	result+=MSize2(temp_meta->user_data);
      result+=MSize2(temp_meta);
      temp_meta=temp_meta->next;
    }
    result+=MSize2(tempm);
    tempm=tempm->next;
  }
  return result;
}

U8 *LexExtStr(CCmpCtrl *cc,I64 *_size=NULL,Bool lex_next=TRUE)
{//Lex $LK,"TK_STR",A="MN:TK_STR"$'s to one combined str. _size includes terminator.
  I64 len=cc->cur_str_len,len1,len2;
  U8 *st=cc->cur_str,*st1,*st2;
  cc->cur_str=NULL;
  while (cc->token==TK_STR) {
    st1=st;
    len1=len;
    if (!lex_next && LexGetChar(cc)!='\\') {
      cc->flags|=CCF_USE_LAST_U16;
      break;
    }
    if (Lex(cc)==TK_STR) {
      len2=cc->cur_str_len;
      st2=cc->cur_str;
      cc->cur_str=NULL;
      len=len1+len2-1;
      st=MAlloc(len);
      if (len1>1)
	MemCpy(st,st1,len1-1);
      MemCpy(st+len1-1,st2,len2);
      Free(st1);
      Free(st2);
    }
  }
  if (_size) *_size=len;
  return st;
}
