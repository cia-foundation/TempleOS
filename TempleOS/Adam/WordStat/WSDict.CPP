#help_index "WordStat/Dictionary"

public U0 WSDWordsLoad()
{
  I64 size;
  CHashGeneric *temph;
  U8 *in_ptr,*in_start,*st2;
  U16 *d;
  ws_dict_num_words=0;
  if (in_ptr=FileRead(WSD_WORD_FILENAME,&size)) {
    in_start=in_ptr;
    Free(ws_dict_word_lst);
    ws_dict_word_lst=AMAlloc(size);
    MemCpy(ws_dict_word_lst,in_start,size);
    ws_dict_word_lst_size=size;

    while (in_ptr<in_start+size) {
      if (*in_ptr==WSD_WORD_CHAR)
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
	HashAdd(temph,ws_hash_table);
	ws_dict_num_words++;
      } else
	in_ptr+=3;
    }
    Free(in_start);
  }
}


public U8 *WSDGetDef(U8 *st,I64 def_num=1)
{
  CFile *f;
  CHashGeneric *temph;
  U8 *result=NULL,*buf,*in_ptr,
     *st2=MStrUtil(st,SUF_TO_UPPER);
  temph=HashFind(st2,ws_hash_table,HTT_DICT_WORD);
  Free(st2);
  if (temph) {
    if (f=FOpen(WSD_DEF_FILENAME,"r")) {
      buf=MAlloc(WSD_BLK_SIZE*2+1);
      buf[WSD_BLK_SIZE*2]=0; //terminate
      FRBlks(f,buf,temph->user_data0*WSD_BLK_SIZE/BLK_SIZE,WSD_BLK_SIZE*2/BLK_SIZE);
      FClose(f);
      in_ptr=buf;
      while (in_ptr<buf+WSD_BLK_SIZE*2) {
	while (*in_ptr!=WSD_WORD_CHAR && in_ptr<buf+WSD_BLK_SIZE*2)
	  in_ptr++;
	if (*in_ptr++==WSD_WORD_CHAR) {
	  if (!StrICmp(st,in_ptr)) {
	    while (def_num && *in_ptr!=WSD_WORD_CHAR
		   && in_ptr<buf+WSD_BLK_SIZE*2) {
	      if (*in_ptr==WSD_DEF_CHAR) {
		if (!--def_num)
		  break;
		else
		  in_ptr++;
	      } else
		in_ptr++;
	    }
	    if (*in_ptr++==WSD_DEF_CHAR) {
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

public U8 *WSDGetWord(U8 *st)
{
  CFile *f;
  CHashGeneric *temph;
  U8 *result=NULL,*buf,*in_ptr,*in_ptr2,
     *st2=MStrUtil(st,SUF_TO_UPPER);
  temph=HashFind(st2,ws_hash_table,HTT_DICT_WORD);
  Free(st2);
  if (temph) {
    if (f=FOpen(WSD_DEF_FILENAME,"r")) {
      buf=MAlloc(WSD_BLK_SIZE*2+1);
      buf[WSD_BLK_SIZE*2]=0; //terminate
      FRBlks(f,buf,temph->user_data0*WSD_BLK_SIZE/BLK_SIZE,WSD_BLK_SIZE*2/BLK_SIZE);
      FClose(f);
      in_ptr=buf;
      while (in_ptr<buf+WSD_BLK_SIZE*2) {
	while (*in_ptr!=WSD_WORD_CHAR && in_ptr<buf+WSD_BLK_SIZE*2)
	  in_ptr++;
	if (*in_ptr++==WSD_WORD_CHAR) {
	  if (!StrICmp(st,in_ptr)) {
	    in_ptr2=in_ptr;
	    in_ptr--;
	    while (*in_ptr2!=WSD_WORD_CHAR
		   && in_ptr2<buf+WSD_BLK_SIZE*2) {
	      in_ptr2++;
	    }
	    result=MAlloc(in_ptr2+1-in_ptr);
	    MemCpy(result,in_ptr,in_ptr2-in_ptr);
	    result[in_ptr2-in_ptr]=WSD_END_CHAR;
	    break;
	  }
	}
      }
      Free(buf);
    }
  }
  return result;
}


/*
Fmt of word lst entry:
  U8 WSD_WORD_CHAR
  U8 word[] with terminating zero
  I16 block;
*/
public U8 *WSDPtAtWord(U8 *st)
{
  I64 i;
  U8 *start=ws_dict_word_lst,*r=start,
     *end=ws_dict_word_lst+ws_dict_word_lst_size;
  if (!st || !*st)
    return ws_dict_word_lst;
  if (ws_dict_word_lst_size) {
    while (start+3<end) {
      r=(start+end)>>1;
      while (TRUE) {
	while (*r!=WSD_WORD_CHAR && r>ws_dict_word_lst)
	  r--;
	if ((r[2]==WSD_WORD_CHAR||r[1]==WSD_WORD_CHAR)&&r-3>ws_dict_word_lst)
	  r--;
	else
	  break;
      }
      if (*r==WSD_WORD_CHAR) {
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
      while (*r!=WSD_WORD_CHAR && r>ws_dict_word_lst)
	r--;
      if ((r[2]==WSD_WORD_CHAR||r[1]==WSD_WORD_CHAR)&&r-3>ws_dict_word_lst)
	r--;
      else
	break;
    }
    if (*r==WSD_WORD_CHAR && StrICmp(st,r+1)>0)
      r+=StrLen(r)+3;
  }
  if (*r==WSD_WORD_CHAR)
    return r;
  else
    return ws_dict_word_lst;
}


U0 WSDDictWordsAdd(CDoc *doc,U8 *st=NULL)
{
  I64 i;
  U8 *ptr;
  if (!st)
    st=ws_cur_word;
  if (st) {
    if (ptr=WSDPtAtWord(st)) {
      for (i=0;i<WSD_MAX_FILLINS;i++) {
	if (*ptr++!=WSD_WORD_CHAR)
	  break;
	if (i)
	  DocPrint(doc,"\n");
	wsd_fillins[i]=ptr-1;
	DocPrint(doc," '%d' %s",i,ptr);
	ptr+=StrLen(ptr)+3;
      }
      wsd_num_fillins=i;
    }
  }
}

U0 WSDFillin(I64 n)
{
  U8 *s;
  I64 len;
  if (0<=n<wsd_num_fillins) {
    s=wsd_fillins[n]+1;
    len=StrLen(s);
    if (len>ws_prtial_len)
      Auto(s+ws_prtial_len);
  }
}


public U0 WSDDisplayDefs(CDoc *doc,U8 *st,I64 num=-1)
{
  U8 *st2,*st3,ch;
  I64 i=0;
  if (!st) return;
  if (*st==WSD_WORD_CHAR)
    st++;
  DocPrint(doc,"$$WW,1$$$$FG,RED$$%s:$$FG$$\n\n",st);
  if (num<0) {
    if (st3=WSDGetWord(st)) {
      st2=st3;
      while (ch=*st2++) {
	switch (ch) {
	  case WSD_WORD_CHAR:
	    break;
	  case WSD_DEF_CHAR:
	    DocPrint(doc,"$$FG,GREEN$$(%d)$$FG$$ %s\n",
	      ++i,st2);
	    break;
	  case WSD_PRONUNCIATION_CHAR:
	    DocPrint(doc,"$$FG,LTGREEN$$%s$$FG$$\n",st2);
	    break;
	  case WSD_POS_CHAR:
	    DocPrint(doc,"$$FG,BLACK$$%s$$FG$$\n",st2);
	    break;
	  case WSD_EXTRA_CHAR:
	    DocPrint(doc,"$$FG,LTBLUE$$%s$$FG$$\n",st2);
	    break;
	}
	st2+=StrLen(st2)+1;
      }
      Free(st3);
    }
  } else {
    while (st2=WSDGetDef(st,++i)) {
      if (i==num)
	DocPrint(doc,"$$FG,GREEN$$(%d)$$FG$$ %s\n",
	  i,st2);
      Free(st2);
    }
  }
}

U0 WSDPopUpDef(U8 *st,I64 num=-1,CTask *parent=NULL)
{
  U8 *buf;
  buf=MPrint("WSDDisplayDefs(DocPut,\"%s\",%d);View;",st,num);
  PopUp(buf,parent);
  Free(buf);
}

U0 WSDDef(I64 n,CTask *parent=NULL)
{
  if (0<=n<wsd_num_fillins)
    WSDPopUpDef(wsd_fillins[n],-1,parent);
}

U0 WSFindMisspelled(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_e=doc->cur_entry;
  I64 col=doc->cur_data_col,col2,ch;
  do {
    if (doc_e!=doc && Bt(doc_tag_types,doc_e->type_u8)) {
      while (col<doc_e->max_col) {
	while (col<doc_e->max_col && !Bt(word_bitmap,doc_e->tag[col]))
	  col++;
	if (col<doc_e->max_col) {
	  col2=col;
	  while (col2<doc_e->max_col && Bt(word_bitmap,doc_e->tag[col2]))
	    col2++;
	  ch=doc_e->tag[col2];
	  doc_e->tag[col2]=0;
	  if (StrICmp(&doc_e->tag[col],WSDPtAtWord(&doc_e->tag[col])+1)) {
	    doc_e->tag[col2]=ch;
	    doc->cur_entry=doc_e;
	    doc->cur_data_col=col;
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

#help_index ""
