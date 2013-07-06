I64 HasLower(U8 *src)
{
  U8 ch;
  while (ch=*src++)
    if ('a'<=ch<='z')
     return TRUE;
  return FALSE;
}

CHash *PSectFind(U8 *add,I64 *_offset)
{ //See $LK,"::/TempleOS/Kernel/Hash1a.CPP","FL:::/TempleOS/Kernel/Hash1a.CPP,1"$
  CHash *result=NULL;
  CHashExport *tempess;
  Bool has_lower=FALSE;
  CHashTable *h=Fs->hash_table;
  U64 best=0xFFFF,j;
  if (!ChkCodePtr(add)) {
    *_offset=best;
    return NULL;
  }
  while (h) {
    tempess=h->next_added;
    while (tempess!=h) {
      j=0;
      if (tempess->type&HTT_FUN) {
	if (!Bt(&tempess(CHashFun *)->flags,Cf_EXTERN) &&
	      !Bt(&tempess(CHashFun *)->flags,Ff_INTERNAL))
	  j=tempess(CHashFun *)->executable_addr;
      } else if (tempess->type&HTT_EXPORT_SYS_SYM)
	j=tempess->val;
      if (j) {
	j=add(I64)-j;
	if (0<=j<=best) {
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
  if (add==SYS_IDLE_PT) {
    *_offset=0;
    return "SYS_IDLE_PT";
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
      if (add!=SYS_IDLE_PT)
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
      if (offset>0xFFFF) offset=0xFFFF;
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
  I64 cur_line,first_line,last_line,num_lines,offset;
  CDbgInfo *dbg;
  U8 **body,*src,*src2;
  if (temph=PSectFind(add,&offset)) {
    if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM)) {
      if (dbg=temph->debug) {
	num_lines=dbg->max_line-dbg->min_line+1;
	body=dbg->body;
 
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
	cur_line=LimitI64(ToF64(add-body[first_line])*(last_line-first_line+1)/
		 (body[last_line]-body[first_line]+1),first_line,last_line);

	//retreat while too high
	while ((!body[cur_line] || body[cur_line]>=add) && cur_line>first_line)
	  cur_line--;

	//advance while to low
	while ((!body[cur_line] || body[cur_line]<add) && cur_line<last_line)
	  cur_line++;

	if (add<body[cur_line]+cnt)
	  return cur_line+dbg->min_line;

      } else if (temph->src_link) {
	src =StrNew(temph->src_link);
	src2=StrNew(temph->src_link);
	StrLastRem(src,",",src2);
	cur_line=A2I(src2);
	Free(src);
	Free(src2);
	return cur_line;
      }
    }
  }
  return -1;
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
  U8 *filename,*st,*st2;
  I64 linenum;
  if (filename=SrcFileName(add,cnt)) {
    linenum=SrcLineNum(add,cnt);
    if (linenum<1)
      linenum=1;
    st2=MSPrintF("FL:%s,%d",filename,linenum);
    Free(filename);
    st=StrNew(st2,mem_task);
    Free(st2);
    return st;
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

U0 E(U8 *add,I64 cnt=512,I64 ed_flags=0)
{//Edit src at add
  U8 *st;
  if (st=SrcEdLink(add,cnt)) {
    if (IsRaw)
      EdLiteFileLine(st);
    else
      Ed(st,ed_flags);
    Free(st);
  }
}

U0 Man(U8 *st)
//Edit src code for sym
{
  U8 **st2;
  CHashSrcSym *temph;
  if (IsRaw) {
    if ((temph=HashFind(st,Fs->hash_table,HTG_SRC_SYM)) && temph->src_link)
      EdLiteFileLine(temph->src_link);
  } else {
    st2=MSPrintF("MN:%s",st);
    Ed(st2);
    Free(st2);
  }
}
