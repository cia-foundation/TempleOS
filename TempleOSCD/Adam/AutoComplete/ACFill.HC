#help_index "AutoComplete/Dictionary"
public U8 *ACDDefGet(U8 *st,I64 def_num=1)
{//MAlloc str holding single dict definition of word.
  CFile *f;
  CHashGeneric *tmph;
  U8 *res=NULL,*buf,*in_ptr,
	*st2=MStrUtil(st,SUF_TO_UPPER);
  tmph=HashFind(st2,ac.hash_table,HTT_DICT_WORD);
  Free(st2);
  if (tmph) {
    if (f=FOpen(ACD_DEF_FILENAME,"r")) {
      buf=MAlloc(ACD_BLK_SIZE*2+1);
      buf[ACD_BLK_SIZE*2]=0; //terminate
      FBlkRead(f,buf,tmph->user_data0*ACD_BLK_SIZE/BLK_SIZE,
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
	      res=StrNew(in_ptr);
	      break;
	    }
	  }
	}
      }
      Free(buf);
    }
  }
  return res;
}

public U8 *ACDDefsGet(U8 *st)
{//MAlloc str with all dict definitions of word.
  CFile *f;
  CHashGeneric *tmph;
  U8 *res=NULL,*buf,*in_ptr,*in_ptr2,
	*st2=MStrUtil(st,SUF_TO_UPPER);
  tmph=HashFind(st2,ac.hash_table,HTT_DICT_WORD);
  Free(st2);
  if (tmph) {
    if (f=FOpen(ACD_DEF_FILENAME,"r")) {
      buf=MAlloc(ACD_BLK_SIZE*2+1);
      buf[ACD_BLK_SIZE*2]=0; //terminate
      FBlkRead(f,buf,tmph->user_data0*ACD_BLK_SIZE/BLK_SIZE,
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
	    res=MAlloc(in_ptr2+1-in_ptr);
	    MemCpy(res,in_ptr,in_ptr2-in_ptr);
	    res[in_ptr2-in_ptr]=ACD_END_CHAR;
	    break;
	  }
	}
      }
      Free(buf);
    }
  }
  return res;
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

U0 ACDFillin(I64 n)
{
  U8 *s;
  I64 len;
  if (0<=n<acd.num_fillins) {
    s=acd.fillins[n]+1;
    len=StrLen(s);
    if (len>ac.partial_len)
      In(s+ac.partial_len);
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

#help_index "AutoComplete"
U0 ACFillIn(I64 n)
{
  U8 *s;
  if (0<=--n<ac.num_fillins) {
    s=ac.fillin_matches[n]->str;
    if (StrLen(s)>ac.partial_len)
      In(s+ac.partial_len);
  }
}

U0 ACMan(I64 n,CTask *parent_task=NULL)
{
  CHashAC *tmpw;
  CHashSrcSym *tmph;
  if (0<=--n<ac.num_fillins && (tmpw=ac.fillin_matches[n]) &&
	(tmph=HashFind(tmpw->str,Fs->hash_table,HTG_SRC_SYM)) &&
	tmph->src_link)
    PopUpEd(tmph->src_link,parent_task);
}
