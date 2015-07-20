#help_index "AutoComplete/Dictionary"

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

public U8 *ACDDefGet(U8 *st,I64 def_num=1)
{//MAlloc str holding single dict definition of word.
  CFile *f;
  CHashGeneric *temph;
  U8 *result=NULL,*buf,*in_ptr,
	*st2=MStrUtil(st,SUF_TO_UPPER);
  temph=HashFind(st2,ac.hash_table,HTT_DICT_WORD);
  Free(st2);
  if (temph) {
    if (f=FOpen(ACD_DEF_FILENAME,"r")) {
      buf=MAlloc(ACD_BLK_SIZE*2+1);
      buf[ACD_BLK_SIZE*2]=0; //terminate
      FRBlks(f,buf,temph->user_data0*ACD_BLK_SIZE/BLK_SIZE,
	    ACD_BLK_SIZE*2/BLK_SIZE);
      FClose(f);
      in_ptr=buf;
      while (in_ptr<buf+ACD_BLK_SIZE*2) {
	while (*in_ptr!=ACD_WORD_CHAR && in_ptr<buf+ACD_BLK_SIZE*2)
	  in_ptr++;
	if (*in_ptr++==ACD_WORD_CHAR) {
	  if (!StrICmp(st,in_ptr)) {
	    while (def_num && *in_ptr!=ACD_WORD_CHAR
		  && in_ptr<buf+ACD_BLK_SIZE*2) {
	      if (*in_ptr==ACD_DEF_CHAR) {
		if (!--def_num)
		  break;
		else
		  in_ptr++;
	      } else
		in_ptr++;
	    }
	    if (*in_ptr++==ACD_DEF_CHAR) {
	      result=StrNew(in_ptr);
	      break;
	    }
	  }
	}
      }
      Free(buf);
    }
  }
  return result;
}

public U8 *ACDDefsGet(U8 *st)
{//MAlloc str with all dict definitions of word.
  CFile *f;
  CHashGeneric *temph;
  U8 *result=NULL,*buf,*in_ptr,*in_ptr2,
	*st2=MStrUtil(st,SUF_TO_UPPER);
  temph=HashFind(st2,ac.hash_table,HTT_DICT_WORD);
  Free(st2);
  if (temph) {
    if (f=FOpen(ACD_DEF_FILENAME,"r")) {
      buf=MAlloc(ACD_BLK_SIZE*2+1);
      buf[ACD_BLK_SIZE*2]=0; //terminate
      FRBlks(f,buf,temph->user_data0*ACD_BLK_SIZE/BLK_SIZE,
	    ACD_BLK_SIZE*2/BLK_SIZE);
      FClose(f);
      in_ptr=buf;
      while (in_ptr<buf+ACD_BLK_SIZE*2) {
	while (*in_ptr!=ACD_WORD_CHAR && in_ptr<buf+ACD_BLK_SIZE*2)
	  in_ptr++;
	if (*in_ptr++==ACD_WORD_CHAR) {
	  if (!StrICmp(st,in_ptr)) {
	    in_ptr2=in_ptr;
	    in_ptr--;
	    while (*in_ptr2!=ACD_WORD_CHAR
		  && in_ptr2<buf+ACD_BLK_SIZE*2) {
	      in_ptr2++;
	    }
	    result=MAlloc(in_ptr2+1-in_ptr);
	    MemCpy(result,in_ptr,in_ptr2-in_ptr);
	    result[in_ptr2-in_ptr]=ACD_END_CHAR;
	    break;
	  }
	}
      }
      Free(buf);
    }
  }
  return result;
}

/*Fmt of word lst entry:
  U8 ACD_WORD_CHAR
  U8 word[] with terminating zero
  I16 block;
*/
public U8 *ACDWordPtAt(U8 *st)
{//Point to word in word list.
  I64 i;
  U8 *start=acd.word_lst,*r=start,
	*end=acd.word_lst+acd.word_lst_size;
  if (!st || !*st)
    return acd.word_lst;
  if (acd.word_lst_size) {
    while (start+3<end) {
      r=(start+end)>>1;
      while (TRUE) {
	while (*r!=ACD_WORD_CHAR && r>acd.word_lst)
	  r--;
	if ((r[2]==ACD_WORD_CHAR||r[1]==ACD_WORD_CHAR)&&r-3>acd.word_lst)
	  r--;
	else
	  break;
      }
      if (*r==ACD_WORD_CHAR) {
	i=StrICmp(st,r+1);
	if (i<0)
	  end=r-1;
	else if (i>0)
	  start=r+StrLen(r)+3;
	else
	  return r;
      } else
	break;
    }
    r=(start+end)>>1;
    while (TRUE) {
      while (*r!=ACD_WORD_CHAR && r>acd.word_lst)
	r--;
      if ((r[2]==ACD_WORD_CHAR||r[1]==ACD_WORD_CHAR)&&r-3>acd.word_lst)
	r--;
      else
	break;
    }
    if (*r==ACD_WORD_CHAR && StrICmp(st,r+1)>0)
      r+=StrLen(r)+3;
  }
  if (*r==ACD_WORD_CHAR)
    return r;
  else
    return acd.word_lst;
}

U0 ACDDictWordsAdd(CDoc *doc,U8 *st)
{
  I64 i;
  U8 *ptr;
  if (st && *st && (ptr=ACDWordPtAt(st))) {
    for (i=0;i<ACD_MAX_FILLINS;i++) {
      if (*ptr++!=ACD_WORD_CHAR)
	break;
      if (i)
	DocPrint(doc,"\n");
      acd.fillins[i]=ptr-1;
      DocPrint(doc,"$$GREEN$$'%d'$$FG$$ %-23ts",i,ptr);
      ptr+=StrLen(ptr)+3;
    }
    acd.num_fillins=i;
  }
}

U0 ACDFillin(I64 n)
{
  U8 *s;
  I64 len;
  if (0<=n<acd.num_fillins) {
    s=acd.fillins[n]+1;
    len=StrLen(s);
    if (len>ac.partial_len)
      Auto(s+ac.partial_len);
  }
}

public U0 ACDDefsPut(CDoc *doc=NULL,U8 *st,I64 num=-1)
{//Put to doc a dictionary definition(s) of a word.
  U8 *st2,*st3;
  I64 ch,i=0;
  if (!st) return;
  if (*st==ACD_WORD_CHAR)
    st++;
  DocPrint(doc,"$$WW,1$$$$RED$$%s:$$FG$$\n\n",st);
  if (num<0) {
    if (st3=ACDDefsGet(st)) {
      st2=st3;
      while (ch=*st2++) {
	switch (ch) {
	  case ACD_WORD_CHAR:
	    break;
	  case ACD_DEF_CHAR:
	    DocPrint(doc,"$$GREEN$$(%d)$$FG$$ %s\n",
		  ++i,st2);
	    break;
	  case ACD_PRONUNCIATION_CHAR:
	    DocPrint(doc,"$$LTGREEN$$%s$$FG$$\n",st2);
	    break;
	  case ACD_POS_CHAR:
	    DocPrint(doc,"$$BLACK$$%s$$FG$$\n",st2);
	    break;
	  case ACD_EXTRA_CHAR:
	    DocPrint(doc,"$$LTBLUE$$%s$$FG$$\n",st2);
	    break;
	}
	st2+=StrLen(st2)+1;
      }
      Free(st3);
    }
  } else {
    while (st2=ACDDefGet(st,++i)) {
      if (i==num)
	DocPrint(doc,"$$GREEN$$(%d)$$FG$$ %s\n",
	      i,st2);
      Free(st2);
    }
  }
}

U0 ACDPopUpDef(U8 *st,I64 num=-1,CTask *parent=NULL)
{
  U8 *buf;
  buf=MStrPrint("ACDDefsPut(DocPut,\"%s\",%d);View;",st,num);
  PopUp(buf,parent);
  Free(buf);
}

U0 ACDDef(I64 n,CTask *parent=NULL)
{
  if (0<=n<acd.num_fillins)
    ACDPopUpDef(acd.fillins[n],-1,parent);
}

U0 ACFindMisspelled(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_e=doc->cur_entry;
  I64 col=doc->cur_col,col2,ch;
  do {
    if (doc_e!=doc && doc_e->de_flags&DOCEF_TAG) {
      while (col<doc_e->max_col) {
	while (col<doc_e->max_col && !Bt(chars_bmp_word,doc_e->tag[col]))
	  col++;
	if (col<doc_e->max_col) {
	  col2=col;
	  while (col2<doc_e->max_col && Bt(chars_bmp_word,doc_e->tag[col2]))
	    col2++;
	  ch=doc_e->tag[col2];
	  doc_e->tag[col2]=0;
	  if (StrICmp(&doc_e->tag[col],ACDWordPtAt(&doc_e->tag[col])+1)) {
	    doc_e->tag[col2]=ch;
	    doc->cur_entry=doc_e;
	    doc->cur_col=col;
	    goto fm_done;
	  }
	  doc_e->tag[col2]=ch;
	  col=col2;
	}
      }
    }
    doc_e=doc_e->next;
    col=doc_e->min_col;
  } while (doc_e!=doc->cur_entry);
fm_done:
  if (unlock)
    DocUnlock(doc);
}
