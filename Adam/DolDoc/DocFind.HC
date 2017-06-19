#help_index "DolDoc/Editor"

public Bool DocGoToLine(CDoc *doc,I64 line_num) //one based
{//Nearest to specified line num.  Move cur_entry & center.
  Bool res=FALSE,unlock;
  if (doc) {
    unlock=DocLock(doc);
    doc->x=0;
    doc->y=line_num-1;
    DocRecalc(doc,RECALCt_FIND_CURSOR);
    DocCenter(doc);
    if (doc->cur_entry->y==line_num-1)
      res=TRUE;
    if (unlock)
      DocUnlock(doc);
  }
  return res;
}

public Bool DocFind(CDoc *haystack_doc,I64 start_line_num=I64_MIN,
U8 *needle,I64 match=1)
{//Find str by searching tags. Move cur_entry & center.
  Bool res=FALSE,unlock;
  CDocEntry *doc_e;
  U8 *ptr;
  I64 i;
  if (haystack_doc) {
    unlock=DocLock(haystack_doc);
    if (start_line_num==I64_MIN) {
      res=TRUE;
      doc_e=haystack_doc->head.next;
    } else {
      res=DocGoToLine(haystack_doc,start_line_num);
      doc_e=haystack_doc->cur_entry;
    }
    if (res) {
      if (needle) {
	res=FALSE;
	while (doc_e!=haystack_doc) {
	  if (doc_e->de_flags&DOCEF_TAG && doc_e->tag &&
//TODO: handle multi-DocEntry strs
		(ptr=StrIMatch(needle,doc_e->tag))) {
	    i=ptr-doc_e->tag;
	    if (!--match) {
	      haystack_doc->cur_entry=doc_e;
	      if (i<doc_e->min_col) i=doc_e->min_col;
	      if (i>doc_e->max_col) i=doc_e->max_col;
	      haystack_doc->cur_col=i;
	      res=TRUE;
	      break;
	    }
	  }
	  doc_e=doc_e->next;
	}
      } else
	res=FALSE;
    }
    if (!res) DocBottom(haystack_doc);
    DocCenter(haystack_doc);
    if (unlock)
      DocUnlock(haystack_doc);
  }
  return res;
}

public Bool DocAnchorFind(CDoc *haystack_doc,U8 *needle_str)
{//Find named anchor. Move cur_entry & center.
  Bool res=FALSE,unlock;
  CDocEntry *doc_e;
  if (haystack_doc) {
    unlock=DocLock(haystack_doc);
    doc_e=haystack_doc->head.next;
    if (needle_str)
      while (doc_e!=haystack_doc) {
	if (doc_e->type_u8==DOCT_ANCHOR && doc_e->de_flags & DOCEF_AUX_STR) {
	  if (!StrCmp(needle_str,doc_e->aux_str)) {
	    haystack_doc->cur_entry=doc_e;
	    haystack_doc->cur_col=doc_e->min_col;
	    res=TRUE;
	    break;
	  }
	}
	doc_e=doc_e->next;
      }
    if (!res) DocBottom(haystack_doc);
    DocCenter(haystack_doc);
    if (unlock)
      DocUnlock(haystack_doc);
  }
  return res;
}

public U0 EdFindNext(CDoc *doc)
{//Editor F3 find next, possibly doing replaces.
  Bool unlock=DocLock(doc);
  U8 *ptr,*ptr2,*ptr3;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e=doc_ce;
  I64 sf_flags;
  if (doc->find_replace->match_case)
    sf_flags=0;
  else
    sf_flags=SFF_IGNORE_CASE;
  if (doc->find_replace->whole_labels)
    sf_flags|=SFG_WHOLE_LABELS;
  do {
    if (doc_e!=doc) {
      if (doc_e->de_flags&DOCEF_TAG && doc_e->tag &&
	    !(doc_e->de_flags&(DOCEG_DONT_EDIT|DOCEF_FILTER_SKIP))) {
	if (doc_e->type & DOCET_SEL ||
	      !doc->find_replace->scan_sel_text) {
	  if (doc->find_replace->scan_fwd) {
	    if (doc_e==doc_ce) {
	      ptr=doc_ce->tag+doc->cur_col+1;
	      if (ptr-doc_ce->tag>=doc_ce->max_col)
		goto fn_skip;
	      if (ptr-doc_ce->tag<doc_ce->min_col)
		ptr=doc_ce->tag+doc_ce->min_col;
	    } else
	      ptr=doc_e->tag;
	    if (ptr=StrFind(doc->find_replace->find_text,ptr,sf_flags)) {
	      doc->cur_entry=doc_e;
	      doc->cur_col=ptr-doc_e->tag;
	      if (doc->cur_col>=doc_e->max_col)
		doc->cur_col=doc_e->max_col-1;
	      if (doc->cur_col<doc_e->min_col)
		doc->cur_col=doc_e->min_col;
	      DocCenter(doc);
	      if (unlock)
		DocUnlock(doc);
	      return;
	    }
	  } else {
	    ptr2=NULL;
	    ptr=doc_e->tag+doc_e->min_col;
	    if (doc_e==doc_ce)
	      ptr3=doc_ce->tag+doc->cur_col;
	    else
	      ptr3=doc_e->tag+doc_e->max_col;
	    while (ptr=StrFind(doc->find_replace->find_text,ptr,sf_flags)) {
	      if (ptr>=ptr3)
		break;
	      ptr2=ptr++;
	    }
	    if (ptr2 && ptr2<ptr3) {
	      doc->cur_entry=doc_e;
	      doc->cur_col=ptr2-doc_e->tag;
	      if (doc->cur_col>=doc_e->max_col)
		doc->cur_col=doc_e->max_col-1;
	      if (doc->cur_col<doc_e->min_col)
		doc->cur_col=doc_e->min_col;
	      DocCenter(doc);
	      if (unlock)
		DocUnlock(doc);
	      return;
	    }
	  }
	}
      }
    }
fn_skip:
    if (doc->find_replace->scan_fwd)
      doc_e=doc_e->next;
    else
      doc_e=doc_e->last;
  } while (doc_e!=doc_ce);
  if (unlock)
    DocUnlock(doc);
}

public U0 EdSelAll(CDoc *doc,Bool sel)
{//Set state of $LK,"DOCET_SEL",A="MN:DOCET_SEL"$ on all entries.
  Bool unlock=DocLock(doc);
  CDocEntry *doc_e=doc->head.next;
  while (doc_e!=doc) {
    BEqu(&doc_e->type,DOCEt_SEL,sel);
    doc_e=doc_e->next;
  }
  if (unlock)
    DocUnlock(doc);
}

public Bool EdFindPaired(CDoc *doc,U8 plus,U8 minus,
Bool fwd,Bool abort_on_dbl_colon=FALSE)
{//Find { } or ( ) pair. Move cur_entry & center.
  Bool unlock=DocLock(doc),res=FALSE;
  U8 *ptr;
  I64 ch,levels=0,colons=0,original_col=doc->cur_col;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e=doc_ce,*original_ce=doc_ce;
  if (abort_on_dbl_colon && EdCurU8(doc)==':')
    colons=1;
  else
    colons=0;
  do {
    if (doc_e!=doc) {
      if (doc_e->de_flags&DOCEF_TAG && doc_e->tag &&
	    !(doc_e->de_flags&DOCEF_FILTER_SKIP)) {
	if (fwd) {
	  if (doc_e==doc_ce)
	    ptr=doc_e->tag+doc->cur_col+1;
	  else
	    ptr=doc_e->tag;
	  if (ptr-doc_e->tag<doc_e->min_col)
	    ptr=doc_e->tag+doc_e->min_col;
	  if (ptr-doc_e->tag>=doc_e->max_col)
	    goto pa_skip;
	  while (ch=*ptr++)
	    if (abort_on_dbl_colon && ch==':') {
	      if (++colons==2) {
		doc->cur_entry=doc_e;
		doc->cur_col=ptr-doc_e->tag-1;
		EdCursorLeft(doc);
		res=FALSE;
		goto pa_done;
	      }
	    } else {
	      colons=0;
	      if (ch==plus)
		levels++;
	      else if (ch==minus) {
		if (!levels--) {
		  doc->cur_entry=doc_e;
		  doc->cur_col=ptr-doc_e->tag-1;
		  res=doc->cur_entry!=original_ce ||
			doc->cur_col!=original_col;
		  goto pa_done;
		}
	      }
	    }
	} else {
	  if (doc_e==doc_ce) {
	    ptr=doc_e->tag+doc->cur_col-1;
	    if (ptr-doc_e->tag>=doc_e->max_col)
	      ptr=doc_e->tag+doc_e->max_col-1;
	  } else
	    ptr=doc_e->tag+doc_e->max_col-1;
	  if (ptr-doc_e->tag<doc_e->min_col)
	    goto pa_skip;
	  while (ptr>=doc_e->tag+doc_e->min_col) {
	    ch=*ptr--;
	    if (abort_on_dbl_colon && ch==':') {
	      if (++colons==2) {
		doc->cur_entry=doc_e;
		doc->cur_col=ptr-doc_e->tag+1;
		res=FALSE;
		goto pa_done;
	      }
	    } else {
	      colons=0;
	      if (ch==plus)
		levels++;
	      else if (ch==minus) {
		if (!levels--) {
		  doc->cur_entry=doc_e;
		  doc->cur_col=ptr-doc_e->tag+1;
		  res=doc->cur_entry!=original_ce ||
			doc->cur_col!=original_col;
		  goto pa_done;
		}
	      }
	    }
	  }
	}
      }
    }
pa_skip:
    if (fwd)
      doc_e=doc_e->next;
    else
      doc_e=doc_e->last;
  } while (doc_e!=doc_ce);
pa_done:
  DocRecalc(doc);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
  return res;
}

public Bool EdGoToFun(CDoc *doc,Bool fwd,Bool abort_on_dbl_colon)
{//Move cur_entry to start of cur fun and center.(Shoddy)
  Bool unlock=DocLock(doc),res=FALSE;
  I64 ch,levels,colons;
  if (fwd) {
    levels=0;
    colons=0;
    while (doc->cur_entry!=doc) {
      ch=EdCurU8(doc);
      if (abort_on_dbl_colon && ch==':') {
	if (++colons==2) {
	  EdCursorLeft(doc);
	  break;
	}
      } else {
	colons=0;
	if (ch=='{')
	  levels++;
	else if (ch=='}' && !levels--)
	  break;
      }
      EdCursorRight(doc);
    }
    DocRecalc(doc);
    if (doc->cur_entry!=doc)
      res=TRUE;
  } else {
    while (EdFindPaired(doc,'}','{',FALSE,abort_on_dbl_colon));
    if (doc->cur_entry!=doc) {
      ch=EdCurU8(doc);
      if (abort_on_dbl_colon && ch==':')
	res=TRUE;
      else {
	if (ch=='{')
	  res=TRUE;
      }
    }
  }
  if (unlock)
    DocUnlock(doc);
  return res;
}

public U0 EdSelFun(CDoc *doc,Bool abort_on_dbl_colon=FALSE)
{//Set $LK,"DOCET_SEL",A="MN:DOCET_SEL"$ on all entries in cur fun.
  Bool unlock=DocLock(doc);
  U8 *ptr;
  I64 ch,levels=0,colons=0;
  CDocEntry *doc_e;
  EdSelAll(doc,FALSE);
  EdGoToFun(doc,FALSE,abort_on_dbl_colon);
  if (EdCurU8(doc)=='{')
    levels--;
  else if (abort_on_dbl_colon && EdCurU8(doc)==':') {
    EdCursorRight(doc);
    if (EdCurU8(doc)==':')
      EdCursorRight(doc);
  }
  doc_e=doc->cur_entry;
  while (doc_e!=doc) {
    doc_e->type|=DOCET_SEL;
    if (doc_e->de_flags&DOCEF_TAG && doc_e->tag) {
      ptr=doc_e->tag;
      if (doc_e==doc->cur_entry)
	ptr+=doc->cur_col;
      while (ch=*ptr++)
	if (abort_on_dbl_colon && ch==':') {
	  if (++colons==2)
	    goto sf_done;
	} else {
	  colons=0;
	  if (ch=='{')
	    levels++;
	  else if (ch=='}' && !levels--)
	    goto sf_done;
	}
    }
    doc_e=doc_e->next;
  }
sf_done:
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#define RSAC_REPLACE	0
#define RSAC_SKIP	1
#define RSAC_ALL	2

I64 PopUpReplaceSkipAllCancel(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"$$CM+LX,1,4$$$$BT,\"REPLACE\",LE=RSAC_REPLACE$$"
	"$$CM+LX,17,0$$$$BT,\"SKIP\",LE=RSAC_SKIP$$"
	"$$CM+LX,1,3$$$$BT,\"ALL\",LE=RSAC_ALL$$"
	"$$CM+LX,17,0$$$$BT,\"CANCEL\",LE=DOCM_CANCEL$$\n");
  if (footer) DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 EdFindReplace(CDoc *doc)
{
  Bool found,unlock;
  I64 cmd,i,j,plen,rlen,dlen,res=-1,sf_flags;
  U8 *src,*dst,*dst2;
  CDocEntry *doc_ce,*doc_e,*doc_marker=NULL;
  if (doc->find_replace->pmt)
    cmd=RSAC_REPLACE;
  else
    cmd=RSAC_ALL;
  if (!doc->find_replace->pmt || DocForm(doc->find_replace)) {
    res=0;
    unlock=DocLock(doc);
    if (doc->find_replace->match_case || doc->find_replace->local_var)
      sf_flags=0;
    else
      sf_flags=SFF_IGNORE_CASE;
    if (doc->find_replace->whole_labels || doc->find_replace->local_var)
      sf_flags|=SFG_WHOLE_LABELS;

    if (i=doc->find_replace->filter_lines) {
      doc_ce=doc->head.next;
      while (doc_ce!=doc) {
	if (doc_ce->de_flags&DOCEF_TAG && doc_ce->tag &&
	      !(doc_ce->de_flags&DOCEF_FILTER_SKIP) &&
	      StrFind(doc->find_replace->find_text,doc_ce->tag,sf_flags)) {
	  doc_ce->type|=DOCET_SEL;
	  res++;
	} else
	  doc_ce->type&=~DOCET_SEL;
	doc_ce=doc_ce->next;
      }

      doc_ce=doc->head.next;
      while (doc_ce!=doc) {
	if (!(doc_ce->de_flags&DOCEF_FILTER_SKIP)) {
	  found=FALSE;

	  doc_e=doc_ce;
	  while (doc_e!=doc && doc_e->y>doc_ce->y-i) {
	    if (doc_e->type&DOCET_SEL) {
	      found=TRUE;
	      break;
	    } else
	      doc_e=doc_e->last;
	  }

	  if (!found) {
	    doc_e=doc_ce;
	    while (doc_e!=doc && doc_e->y<doc_ce->y+i) {
	      if (doc_e->type&DOCET_SEL) {
		found=TRUE;
		break;
	      } else
		doc_e=doc_e->next;
	    }
	  }

	  if (!found)
	    doc_ce->de_flags|=DOCEF_FILTER_SKIP;
	}

	doc_ce=doc_ce->next;
      }
      EdSelAll(doc,FALSE);
      goto fr_unlock_done;
    }

    if (doc->find_replace->local_var)
      EdSelFun(doc);

    if (!doc->find_replace->replace && !doc->find_replace->local_var) {
      EdFindNext(doc);
      goto fr_unlock_done;
    }
    plen=StrLen(doc->find_replace->find_text);
    if (!plen)
      goto fr_unlock_done;
    rlen=StrLen(doc->find_replace->replace_text);
    if (doc->head.next!=doc) {
      doc_e=doc_marker=DocSplitTag(doc,doc->cur_entry,doc->cur_col,
	    doc->cur_entry->x+doc->cur_col,doc->cur_entry->y,DOCT_MARKER);
      do {
	if (doc_e==doc) {
	  if (doc->find_replace->scan_fwd)
	    doc_e=doc_e->next;
	  else
	    doc_e=doc_e->last;
	  if (doc_e==doc_marker)
	    break;
	}
	if (doc_e->type_u8==DOCT_TEXT &&
	      !(doc_e->de_flags&(DOCEG_DONT_EDIT|DOCEF_FILTER_SKIP)) &&
	      (doc_e->type & DOCET_SEL ||
	      !doc->find_replace->scan_sel_text&&
	      !doc->find_replace->local_var)) {
	  src=doc_e->tag;
	  while (src) {
	    src=StrFind(doc->find_replace->find_text,src,sf_flags);
	    if (src) {
	      doc->cur_col=src-doc_e->tag;
	      doc->cur_entry=doc_e;
	      if (cmd!=RSAC_ALL)
		DocCenter(doc);
	      if (cmd!=RSAC_ALL) {
		DocUnlock(doc);
		cmd=PopUpReplaceSkipAllCancel("");
		DocLock(doc);
		if (cmd<0)
		  goto fr_unlock_done;
	      }
	      doc_e=doc->cur_entry;
	      src=doc->cur_col+doc_e->tag;
	      if (cmd==RSAC_REPLACE || cmd==RSAC_ALL) {
		dlen=StrLen(doc_e->tag);
		doc_e->max_col=dlen+rlen-plen;
		dst=MAlloc(doc_e->max_col+1,doc->mem_task);
		dst2=dst;
		j=src-doc_e->tag;
		for (i=0;i<j;i++)
		  *dst++=doc_e->tag[i];
		for (i=0;i<rlen;i++)
		  *dst++=doc->find_replace->replace_text[i];
		src=dst;
		for (i=j+plen;i<=dlen;i++)
		  *dst++=doc_e->tag[i];
		Free(doc_e->tag);
		doc_e->tag=dst2;
		doc->cur_col=src-doc_e->tag;
		doc->cur_entry=doc_e;
		if (cmd!=RSAC_ALL) {
		  DocRemSoftNewLines(doc,doc->cur_entry);
		  DocRecalc(doc);
		}
		doc_e=doc->cur_entry;
		src=doc->cur_col+doc_e->tag;
		res++;
	      } else
		src++;
	    }
	  }
	}
	if (doc->find_replace->scan_fwd)
	  doc_e=doc_e->next;
	else
	  doc_e=doc_e->last;
      } while (doc_e!=doc_marker);
    }
fr_unlock_done:
    if (doc_marker)
      DocEntryDel(doc,doc_marker);
    DocRemSoftNewLines(doc,NULL);
    DocRecalc(doc);
    DocCenter(doc);
    if (unlock)
      DocUnlock(doc);
  }
  return res;
}

public I64 EdReplace(CDoc *doc,U8 *find,U8 *replace,
Bool sel=TRUE,Bool match_case=TRUE,Bool whole_labels=FALSE)
{//Find & replace using editor's cmd.
  CEdFindText old_find_replace;
  Bool unlock;
  I64 i,res=-1;
  if (!doc) return -1;
  unlock=DocLock(doc);
  MemCpy(&old_find_replace,doc->find_replace,sizeof(CEdFindText));
  MemSet(doc->find_replace,0,sizeof(CEdFindText));
  i=StrLen(find);
  if (i<sizeof(CEdFindText.find_text)) {
    MemCpy(doc->find_replace->find_text,find,i+1);
    i=StrLen(replace);
    if (i<sizeof(CEdFindText.replace_text)) {
      MemCpy(doc->find_replace->replace_text,replace,i+1);
      doc->find_replace->replace=TRUE;
      doc->find_replace->scan_sel_text=sel;
      doc->find_replace->match_case=match_case;
      doc->find_replace->whole_labels=whole_labels;
      doc->find_replace->pmt=FALSE;
      res=EdFindReplace(doc);
    }
  }
  MemCpy(doc->find_replace,&old_find_replace,sizeof(CEdFindText));
  if (unlock)
    DocUnlock(doc);
  return res;
}

class CEdLineGoTo
{
  I64 line format "$$DA,A=\"Go to Line:%d\"$$";
};

U0 EdGoToLine(CDoc *doc)
{//Prompt with form and go to line num.
  CEdLineGoTo gtl;
  gtl.line=1;
  if (DocForm(&gtl))
    DocGoToLine(doc,gtl.line);
}
