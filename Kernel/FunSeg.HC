I64 HasLower(U8 *src)
{
  I64 ch;
  while (ch=*src++)
    if ('a'<=ch<='z')
      return TRUE;
  return FALSE;
}

U0 HashFunSegFind(CHashTable *h,U8 *addr,
	Bool *_has_lower,U64 *_best,CHash **_res)
{
  Bool *has_lower=*_has_lower;
  CHashExport *tmpex;
  U64 i,j,best=*_best;
  CHash *res=*_res;
  for (i=0;i<=h->mask;i++) {
    tmpex=h->body[i];
    while (tmpex) {
      j=0;
      if (tmpex->type&HTT_FUN) {
	if (!Bt(&tmpex(CHashFun *)->flags,Cf_EXTERN) &&
	      !Bt(&tmpex(CHashFun *)->flags,Ff_INTERNAL))
	  j=tmpex(CHashFun *)->exe_addr;
      } else if (tmpex->type&HTT_EXPORT_SYS_SYM)
	j=tmpex->val;
      if (j) {
	j=addr(I64)-j;
	if (0<=j<=best) {
	  if (tmpex->type&HTT_EXPORT_SYS_SYM) {
	    if (j<best || j==best && !has_lower) {
	      has_lower=HasLower(tmpex->str);
	      best=j;
	      res=tmpex;
	    }
	  } else if (tmpex->type&HTT_FUN) {
	    if (j<best || j==best &&
		  (res && res->type&HTT_EXPORT_SYS_SYM||!has_lower)) {
	      has_lower=HasLower(tmpex->str);
	      best=j;
	      res=tmpex;
	    }
	  }
	}
      }
      tmpex=tmpex->next;
    }
  }
  *_has_lower=has_lower;
  *_best=best;
  *_res =res;
}

CHash *FunSegFind(U8 *addr,I64 *_offset)
{//See $LK,"Hash",A="HI:Hash"$.
  CHash *res=NULL;
  Bool has_lower=FALSE;
  CTask *task;
  CHashTable *h;
  CCPU *c;
  U64 i,best=0xFFFF;
  if (!ChkCodePtr(addr)) {
    *_offset=best;
    return NULL;
  }
  if (IsDbgMode)
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      task=c->seth_task;
      do {
	if (!TaskValidate(task)) goto fs_abort_task;
	h=task->hash_table;
	while (h) {
	  HashFunSegFind(h,addr,&has_lower,&best,&res);
	  h=h->next;
	}
	task=task->next_task;
      } while (task!=c->seth_task);
fs_abort_task:
    }
  else {
    h=Fs->hash_table;
    while (h) {
      HashFunSegFind(h,addr,&has_lower,&best,&res);
      h=h->next;
    }
  }
  *_offset=best;
  return res;
}

U0 FunSegCacheAdd(CHash *tmps,U8 *addr)
{
  I64 i;
  CDbgInfo *dbg_info;
  CFunSegCache *tmpfsc;
  if (tmps && tmps->type&HTT_FUN &&
	(dbg_info=tmps(CHashFun *)->dbg_info)) {
    lock i=dbg.fun_seg_cache_index++;
    tmpfsc=&dbg.fun_seg_cache[i&(FUN_SEG_CACHE_SIZE-1)];
    tmpfsc->base=dbg_info->body[0];
    if (addr<tmpfsc->base)
      tmpfsc->base=addr;
    tmpfsc->limit=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line];
    if (addr>=tmpfsc->limit)
      tmpfsc->limit=addr+1;
    i=MinI64(StrLen(tmps->str),FUN_SEG_CACHE_STR_LEN-1);
    MemCpy(tmpfsc->str,tmps->str,i);
    tmpfsc->str[i]=0;
    tmpfsc->time_stamp=tS;
  }
}

U8 *FunSegCacheFind(U8 *addr,I64 *_offset)
{
  I64 i;
  F64 timeout;
  CFunSegCache *tmpfsc=dbg.fun_seg_cache;
  if (addr==SYS_IDLE_PT) {
    *_offset=0;
    return "SYS_IDLE_PT";
  } else {
    timeout=tS+8.0;
    for (i=0;i<FUN_SEG_CACHE_SIZE;i++,tmpfsc++)
      if (tmpfsc->base<=addr<tmpfsc->limit &&
	    tmpfsc->time_stamp>timeout) {
	*_offset=addr-tmpfsc->base;
	return tmpfsc->str;
      }
    return NULL;
  }
}

U0 StrPrintFunSeg(U8 *buf,I64 addr,I64 field_len,I64 flags)
{
  I64 offset;
  CHashExport *tmpex;
  U8 *str,*str2;
  Bool is_fun=FALSE;
  if (!(flags&PRTF_TRUNCATE))
    field_len=0;
  if (addr) {
    if (str=FunSegCacheFind(addr,&offset)) {
      if (addr!=SYS_IDLE_PT)
	is_fun=TRUE;
    } else  {
      if (tmpex=FunSegFind(addr,&offset)) {
	if (tmpex->type&HTT_FUN)
	  is_fun=TRUE;
	FunSegCacheAdd(tmpex,addr);
	str=tmpex->str;
      }
    }
    if (str) {
      if (offset>0xFFFF) offset=0xFFFF;
      if (flags&PRTF_COMMA) {
	if (is_fun) {
	  str2=MStrPrint("&%s",str);
	  if (!field_len)
	    StrCpy(buf,str2);
	  else if (flags&PRTF_LEFT_JUSTIFY && StrLen(str2)<field_len)
	    StrCpy(buf,str2);
	  else
	    StrPrint(buf,"%*ts",field_len,str2);
	  Free(str2);
	} else {
	  if (!field_len)
	    StrCpy(buf,str);
	  else if (flags&PRTF_LEFT_JUSTIFY && StrLen(str)<field_len)
	    StrCpy(buf,str);
	  else
	    StrPrint(buf,"%*ts",field_len,str);
	}
      } else {
	if (is_fun) {
	  str2=MStrPrint("&%s",str);
	  if (field_len && field_len>7) {
	    if (flags&PRTF_LEFT_JUSTIFY && StrLen(str2)<field_len-7)
	      StrPrint(buf,"%s+0x%04X",str2,offset);
	    else
	      StrPrint(buf,"%*ts+0x%04X",field_len-7,str2,offset);
	  } else
	    StrPrint(buf,"%s+0x%04X",str2,offset);
	  Free(str2);
	} else {
	  if (field_len && field_len>7) {
	    if (flags&PRTF_LEFT_JUSTIFY && StrLen(str)<field_len-7)
	      StrPrint(buf,"%s+0x%04X",str,offset);
	    else
	      StrPrint(buf,"%*ts+0x%04X",field_len-7,str,offset);
	  } else
	    StrPrint(buf,"%s+0x%04X",str,offset);
	}
      }
      return;
    }
  }
  if (flags&PRTF_COMMA)
    StrCpy(buf,".");
  else if (flags&PRTF_TRUNCATE && field_len)
    StrPrint(buf,"%*tX",field_len,addr);
  else
    StrPrint(buf,"%X",addr);
}

I64 SrcLineNum(U8 *addr,I64 cnt=1)
{//linenum for src of addr.
  CHashSrcSym *tmph;
  I64 cur_line,first_line,last_line,num_lines,offset;
  CDbgInfo *dbg_info;
  U32 *body;
  U8 *src,*src2;
  if (tmph=FunSegFind(addr,&offset)) {
    if (tmph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (dbg_info=tmph->dbg_info) {
	num_lines=dbg_info->max_line-dbg_info->min_line+1;
	body=dbg_info->body;

	//find first nonzero
	first_line=0;
	while (!body[first_line]) {
	  first_line++;
	  if (first_line>=num_lines)
	    return -1;
	}

	//find last nonzero
	last_line=num_lines-1;
	while (!body[last_line] && last_line>first_line)
	  last_line--;

	  //interpolate to guess line num
	cur_line=ClampI64(ToF64(addr-body[first_line])*(last_line-first_line+1)/
	      (body[last_line]-body[first_line]+1),first_line,last_line);

	//retreat while too high
	while ((!body[cur_line] || body[cur_line]>=addr) && cur_line>first_line)
	  cur_line--;

	  //advance while to low
	while ((!body[cur_line] || body[cur_line]<addr) && cur_line<last_line)
	  cur_line++;

	if (addr<body[cur_line]+cnt)
	  return cur_line+dbg_info->min_line;

      } else if (tmph->src_link) {
	src =StrNew(tmph->src_link);
	src2=StrNew(tmph->src_link);
	StrLastRem(src,",",src2);
	cur_line=Str2I64(src2);
	Free(src);
	Free(src2);
	return cur_line;
      }
    }
  }
  return -1;
}

U8 *SrcFileName(U8 *addr,I64 cnt=1,CTask *mem_task=NULL)
{//MAlloc filename for src of addr.
  CHashSrcSym *tmph;
  I64 i,j,ii,offset,best=NULL,d,best_d;
  U32 *body;
  CDbgInfo *dbg_info;
  U8 *src;
  if ((tmph=FunSegFind(addr,&offset)) &&
	tmph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
    if (dbg_info=tmph->dbg_info) {
      j=dbg_info->max_line-dbg_info->min_line+1;
      body=dbg_info->body;
      best_d=I64_MAX;
      for (i=0;i<j;i++) {
	if (0<body[i]<=addr<body[i]+cnt) {
	  ii=i+1;
	  while (!body[ii])
	    ii++;
	  if (addr<body[ii]) {
	    d=addr(I64)-body[i];
	    if (d<best_d) {
	      best_d=d;
	      best=tmph->src_link;
	    }
	  }
	}
      }
    } else
      best=tmph->src_link;
  }
  if (best) {
    src=StrNew(best,mem_task);
    StrFirstRem(src,":");
    StrLastRem(src,",");
    return src;
  } else
    return NULL;
}

U8 *SrcEdLink(U8 *addr,I64 cnt=1,CTask *mem_task=NULL)
{//MAlloc file,line link to src of addr.
  U8 *filename,*st,*st2;
  I64 linenum;
  if (filename=SrcFileName(addr,cnt)) {
    linenum=SrcLineNum(addr,cnt);
    if (linenum<1)
      linenum=1;
    st2=MStrPrint("FL:%s,%d",filename,linenum);
    Free(filename);
    st=StrNew(st2,mem_task);
    Free(st2);
    return st;
  }
  return NULL;
}

Bool PutSrcLink(U8 *addr,I64 cnt=1,U8 *buf=NULL)
{//Put to StdOut a $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ file,line link to src of addr.
  U8 *src;
  if (src=SrcEdLink(addr,cnt)) {
    if (buf)
      StrPrint(buf,"$$LK,\"%p\",A=\"%s\"$$",addr,src);
    else
      "$$LK,\"%p\",A=\"%s\"$$",addr,src;
    Free(src);
    return TRUE;
  } else if (buf)
    *buf=0;
  return FALSE;
}

Bool E(U8 *addr,I64 cnt=512,I64 edf_dof_flags=0)
{//Edit src at addr.
  U8 *st;
  Bool res=FALSE;
  if (st=SrcEdLink(addr,cnt)) {
    if (IsRaw)
      res=EdLiteFileLine(st,edf_dof_flags);
    else
      res=Ed(st,edf_dof_flags);
    Free(st);
  }
  return res;
}

Bool Man(U8 *st,I64 edf_dof_flags=0)
{//Owner's manual for symbol.  Edit src code for symbol.
  Bool res=FALSE;
  U8 **st2;
  CHashSrcSym *tmph;
  if (IsRaw) {
    if ((tmph=HashFind(st,Fs->hash_table,HTG_SRC_SYM)) && tmph->src_link)
      res=EdLiteFileLine(tmph->src_link,edf_dof_flags);
  } else {
    st2=MStrPrint("MN:%s",st);
    res=Ed(st2,edf_dof_flags);
    Free(st2);
  }
  return res;
}
