I64 HasLower(U8 *src)
{
  U8 ch;
  while (ch=*src++)
    if ('a'<=ch<='z')
     return TRUE;
  return FALSE;
}

CHash *PSectFind(U8 *add,I64 *_offset)
{ //See $LK,"::/SparrowOS/OSMain/Hash1a.CPP.Z","FL:::/SparrowOS/OSMain/Hash1a.CPP.Z,1"$
  CHash *result=NULL;
  CHashExport *tempess;
  CHashTable *h=Fs->hash_table;
  Bool has_lower=FALSE;
  U64 best=0x8000,j;
  if (!ChkPtr(add)) {
    *_offset=best;
    return NULL;
  }
  while (h) {
    tempess=h->next_added;
    while (tempess!=h) {
      if (tempess->type&HTT_FUN)
	j=tempess(CHashFun *)->executable_addr;
      else if (tempess->type&HTT_EXPORT_SYS_SYM)
	j=tempess->val;
      else
	j=0;
      j=add(I64)-j;
      if (tempess->type&HTT_EXPORT_SYS_SYM) {
	if (j<best || j==best && !has_lower) {
	  has_lower=HasLower(tempess->str);
	  best=j;
	  result=tempess;
	}
      } else if (tempess->type&HTT_FUN) {
	if (j<best || j==best &&
	    (result && result->type&HTT_EXPORT_SYS_SYM||!has_lower)) {
	  has_lower=HasLower(tempess->str);
	  best=j;
	  result=tempess;
	}
      }
      tempess=tempess->next_added;
    }
    h=h->next;
  }
  *_offset=best;
  return result;
}

U0 PSectCacheAdd(CHash *temps,U8 *add)
{
  I64 i;
  CDbgInfo *dbg;
  Bool old_preempt;
  CPSectCache *temppc;
  if (temps && temps->type&HTT_FUN &&
      (dbg=temps(CHashFun *)->debug)) {
    old_preempt=Preempt(OFF);
    lock {i=psect_cache_index++;}
    temppc=&psect_cache[i&(PSECT_CACHE_SIZE-1)];
    temppc->base=dbg->body[0];
    if (add<temppc->base)
      temppc->base=add;
    temppc->limit=dbg->body[dbg->max_line-dbg->min_line];
    if (add>=temppc->limit)
      temppc->limit=add(I64)+1;
    i=MinI64(StrLen(temps->str),PSECT_CACHE_STR_LEN-1);
    MemCpy(temppc->str,temps->str,i);
    temppc->str[i]=0;
    temppc->time_stamp=tS;
    Preempt(old_preempt);
  }
}

U8 *PSectCacheFind(U8 *add,I64 *_offset)
{
  I64 i;
  F64 timeout;
  CPSectCache *temppc=psect_cache;
  if (&sys_idle_start<=add<&sys_idle_end) {
    *_offset=add-&sys_idle_start;
    return "SYS_IDLE_START";
  } else {
    timeout=tS+8.0;
    for (i=0;i<PSECT_CACHE_SIZE;i++,temppc++)
      if (temppc->base<=add<temppc->limit &&
	  temppc->time_stamp>timeout) {
	*_offset=add-temppc->base;
	return temppc->str;
      }
    return NULL;
  }
}

U0 SPrintFPSect(U8 *buf,I64 add,I64 field_len)
{
  I64 offset;
  CHashExport *tempess;
  U8 *str,*str2;
  Bool is_fun=FALSE;
  if (add) {
    if (str=PSectCacheFind(add,&offset)) {
      if (!(&sys_idle_start<=add<&sys_idle_end))
	is_fun=TRUE;
    } else  {
      if (tempess=PSectFind(add,&offset)) {
	if (tempess->type&HTT_FUN)
	  is_fun=TRUE;
	PSectCacheAdd(tempess,add);
	str=tempess->str;
      }
    }
    if (str) {
      if (offset<=0xFFFF) {
	if (is_fun) {
	  str2=MSPrintF("&%s",str);
	  if (field_len>7)
	    SPrintF(buf,"%*ts+0x%04X",field_len-7,str2,offset);
	  else
	    SPrintF(buf,"%s+0x%04X",str2,offset);
	  Free(str2);
	} else {
	  if (field_len>7)
	    SPrintF(buf,"%*ts+0x%04X",field_len-7,str,offset);
	  else
	    SPrintF(buf,"%s+0x%04X",str,offset);
	}
      }
      return;
    }
  }
  if (field_len)
    SPrintF(buf,"%*tX",field_len,add);
  else
    SPrintF(buf,"%X",add);
}


I64 SrcLineNum(U8 *add,I64 cnt=1)
{
  CHashSrcSym *temph;
  I64 i,ii,j,offset,best=-1,d,best_d;
  CDbgInfo *dbg;
  U8 **body,*src,*src2;
  if (temph=PSectFind(add,&offset)) {
    if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (dbg=temph->debug) {
	j=dbg->max_line-dbg->min_line+1;
	body=dbg->body;
	best_d=MAX_I64;
	for (i=0;i<j;i++) {
	  if (0<body[i]<=add<body[i]+cnt) {
	    ii=i+1;
	    while (!body[ii])
	      ii++;
	    if (add<body[ii]) {
	      d=add(I64)-body[i];
	      if (d<best_d) {
		best_d=d;
		best=i+dbg->min_line;
	      }
	    }
	  }
	}
      } else if (temph->src_link && !offset) {
	src =StrNew(temph->src_link);
	src2=StrNew(temph->src_link);
	StrLastRem(src,",",src2);
	best=A2I(src2);
	Free(src);
	Free(src2);
      }
    }
  }
  return best;
}

U8 *SrcFileName(U8 *add,I64 cnt=1,CTask *mem_task=NULL)
{
  CHashSrcSym *temph;
  I64 i,j,ii,offset,best=NULL,d,best_d;
  CDbgInfo *dbg;
  U8 **body,*src;
  if (temph=PSectFind(add,&offset)) {
    if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (dbg=temph->debug) {
	j=dbg->max_line-dbg->min_line+1;
	body=dbg->body;
	best_d=MAX_I64;
	for (i=0;i<j;i++) {
	  if (0<body[i]<=add<body[i]+cnt) {
	    ii=i+1;
	    while (!body[ii])
	      ii++;
	    if (add<body[ii]) {
	      d=add(I64)-body[i];
	      if (d<best_d) {
		best_d=d;
		best=temph->src_link;
	      }
	    }
	  }
	}
      } else
	best=temph->src_link;
    }
  }
  if (best) {
    src=StrNew(best,mem_task);
    StrFirstRem(src,":");
    StrLastRem(src,",");
    return src;
  } else
    return NULL;
}

U8 *SrcEdLink(U8 *add,I64 cnt=1,CTask *mem_task=NULL)
{
  U8 *filename,buf[512];
  I64 linenum=SrcLineNum(add,cnt);
  if (linenum>=0) {
    if (filename=SrcFileName(add,cnt)) {
      SPrintF(buf,"FL:%s,%d",filename,linenum);
      return StrNew(buf,mem_task);
    }
  }
  return NULL;
}


Bool PutSrcLink(U8 *add,I64 cnt=1,U8 *buf=NULL)
{
  U8 *src;
  if (src=SrcEdLink(add,cnt)) {
    if (buf)
      SPrintF(buf,"$$LK,\"%P\",\"%s\"$$",add,src);
    else
      "$$LK,\"%P\",\"%s\"$$",add,src;
    Free(src);
    return TRUE;
  } else if (buf)
    *buf=0;
  return FALSE;
}

U0 EdAddSrc(U8 *add,I64 cnt=1,I64 ed_flags=0)
{
  U8 *src;
  if (src=SrcEdLink(add,cnt)) {
    EdJoin(src,ed_flags);
    Free(src);
  }
}
