#help_index "Utils;Cmd Line (Typically)"

#define GRF_REPLACE	0
#define GRF_SKIP	1
#define GRF_ALL		2
#define GRF_ED		3
#define GRF_ABORT_FILE	4
#define GRF_SKIP_FILE	5

I64 PopUpReplaceSkipAllCancelEd(U8 *msg)
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"%s",msg);
  DocPrint(doc,"$$CM+LX,2,4$$$$BT,\"REPLACE\",GRF_REPLACE$$"
	"$$CM+LX,22,0$$$$BT,\"SKIP\",GRF_SKIP$$"
	"$$CM+LX,2,4$$$$BT,\"ALL\",GRF_ALL$$"
	"$$CM+LX,22,0$$$$BT,\"ABORT ALL\",DOCM_CANCEL$$"
	"$$CM+LX,2,4$$$$BT,\"EDIT\",GRF_ED$$"
	"$$CM+LX,22,0$$$$BT,\"ABORT FILE\",GRF_ABORT_FILE$$"
	"$$CM+LX,2,4$$$$BT,\"SKIP FILE\",GRF_SKIP_FILE$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}



I64 GrepFile(U8 *pattern,U8 *name,I64 *_fuf_flags,U8 *replace_text)
{
  Bool first_on_line,write_this_file=FALSE,cont=!Bt(_fuf_flags,FUf_CANCEL);
  U8 *src,*dst,*dst2,*name_buf=NULL;
  I64 i,j,plen,rlen,dlen,cnt=0,old_flags,ss_flags;
  CDoc *cur_l,*doc=DocRead(name,DOCF_PLAIN_TEXT_WITH_TABS|DOCF_NO_CURSOR);
  CDocEntry *doc_e;

  if (Bt(_fuf_flags,FUf_IGNORE))
    ss_flags=SFF_IGNORE_CASE;
  else
    ss_flags=0;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS))
    ss_flags|=SFG_WHOLE_LABELS;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS_FRONT))
    ss_flags|=SFF_WHOLE_LABELS_FRONT;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS_BACK))
    ss_flags|=SFF_WHOLE_LABELS_BACK;

  plen=StrLen(pattern);
  if (replace_text)
    rlen=StrLen(replace_text);
  doc_e=doc->root.next;
  while (doc_e!=doc && cont) {
    if (doc_e->type_u8==DOCT_TEXT) {
      src=doc_e->tag;
      first_on_line=TRUE;
      while (src && cont) {
	if (src=StrFind(pattern,src,ss_flags)) {
	  cnt++;
	  if (first_on_line || Bt(_fuf_flags,FUf_REPLACE)) {
	    first_on_line=FALSE;
	    PutFileLink(name,doc_e->y+1,TRUE);
	    name_buf=MPrint("%s,%d",name,doc_e->y+1);
	    if (cur_l=DocPut) {
	      old_flags=cur_l->flags&DOCF_PLAIN_TEXT;
	      cur_l->flags|=DOCF_PLAIN_TEXT;
	    }
	    " %s\n",doc_e->tag;
	    if (cur_l)
	      cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
	  }
	  if (Bt(_fuf_flags,FUf_REPLACE)) {
	    if (Bt(_fuf_flags,FUf_ALL))
	      i=GRF_ALL;
	    else {
	      i=PopUpReplaceSkipAllCancelEd("");
	      if (i<0) {
		LBts(_fuf_flags,FUf_CANCEL);
		cont=FALSE;
		write_this_file=FALSE;
	      } else if (i==GRF_ALL)
		LBts(_fuf_flags,FUf_ALL);
	      else if (i==GRF_ABORT_FILE) {
		cont=FALSE;
		write_this_file=FALSE;
	      } else if (i==GRF_SKIP_FILE)
		cont=FALSE;
	    }
	    if (i==GRF_REPLACE || i==GRF_ALL) {
	      dlen=StrLen(doc_e->tag);
	      dst=MAlloc(dlen+1+rlen-plen);
	      dst2=dst;
	      j=src-doc_e->tag;
	      for (i=0;i<j;i++)
		*dst++=doc_e->tag[i];
	      for (i=0;i<rlen;i++)
		*dst++=replace_text[i];
	      src=dst;
	      for (i=j+plen;i<=dlen;i++)
		*dst++=doc_e->tag[i];
	      Free(doc_e->tag);
	      doc_e->tag=dst2;
	      if (cur_l=DocPut) {
		old_flags=cur_l->flags&DOCF_PLAIN_TEXT;
		cur_l->flags|=DOCF_PLAIN_TEXT;
	      }
	      "%12s,%04d*%s\n",name,doc_e->y+1,dst2;
	      if (cur_l)
		cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
	      write_this_file=TRUE;
	    } else {
	      src++;
	      if (i==GRF_ED) {
		Free(name_buf);
		name_buf=StrNew(doc->filename.name);
		doc->flags&=~DOCF_NO_CURSOR;
		doc->cur_entry=doc_e;
		doc->cur_data_col=doc_e->min_col;
		DocWrite(doc);
		DocDel(doc);
		"Wrote:%s\n",name_buf;
		Ed(name_buf);
		doc=DocRead(name_buf, DOCF_PLAIN_TEXT_WITH_TABS);
		doc_e=doc->cur_entry;
		if (doc_e->last!=doc) doc_e=doc_e->last;
		src=NULL;
		write_this_file=FALSE;
	      }
	    }
	  } else
	    src++;
	  Free(name_buf);
	  name_buf=NULL;
	}
      }
    }
    doc_e=doc_e->next;
  }
  if (write_this_file) {
    DocWrite(doc);
    "Wrote:%s\n",doc->filename.name;
  }
  DocDel(doc);
  return cnt;
}

public I64 Grep(U8 *pattern,U8 *files_find_mask="*",U8 *fu_flags=NULL,U8 *replace_text=NULL)
/*
This does not do regular expressions--my bad.
Anyway, it's good for searching and replacing.
Let's say it stands for global replace ;-)

"+r" =recurse
"+i" =ignore case
"+l" =whole labels only.
      This will check for a nonlabel character before
      and after.  If you have a var, "dd" and don't
      want to match words like "Add", you
      set this flag and it will see that the characters
      before or after "ll" are label characters.
"+lf"=only checks for label chars in front
"+lb"=only checks for label chars behind (after).
*/

{
  I64 cnt=0,fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+i+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_IGNORE|FUF_ALL|FUF_WHOLE_LABELS|FUF_WHOLE_LABELS_FRONT|FUF_WHOLE_LABELS_BACK))
    throw(EXCEPT_FUF,1);
  LBEqu(&fuf_flags,FUf_REPLACE,replace_text);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_ALL|FUF_REPLACE|FUF_IGNORE|FUF_WHOLE_LABELS|FUF_WHOLE_LABELS_FRONT|FUF_WHOLE_LABELS_BACK;
  while (tempde && !Bt(&fuf_flags,FUf_CANCEL)) {
    cnt+=GrepFile(pattern,tempde->full_name,&fuf_flags,replace_text);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
  return cnt;
}

public I64 FileOcc(U8 *pattern,U8 *files_find_mask="*",U8 *fu_flags="+r+i+l")
{/*
"+r"=recurse
"+i"=ignore case
"+l"=whole labels only.
"+lf"=only checks for label chars in front
"+lb"=only checks for label chars behind (after).
*/
  I64 cnt=0;
  Bool old_silent=Silent(TRUE);
  cnt=Grep(pattern,files_find_mask,fu_flags);
  Silent(old_silent);
  return cnt;
}

class CGrep
{
  U8	find_text[136]	    fmtstr "$$DA-P,135,\"FIND        :%s\"$$\n";
  U8	replace_text[136]   fmtstr "$$DA-P,135,\"REPLACE     :%s\"$$\n";
  Bool	replace	    fmtstr "$$CB,\"REPLACE\"$$\n";
  Bool	match_case	    fmtstr "$$CB,\"MATCH CASE\"$$\n";
  Bool	whole_labels	    fmtstr "$$CB,\"WHOLE LABELS\"$$\n";
  U8	filemask[136]	    fmtstr "$$DA-P,135,\"FILE MASK   :%s\"$$\n";
  Bool	recurse	    fmtstr "$$CB,\"RECURSE\"$$\n";
};

I64 GrepWiz()
{
  CDoc *doc;
  U8 buf[32],*dir,*st;
  CGrep *g=CAlloc(sizeof(CGrep));
  I64 result=0;
  g->recurse=TRUE;
  StrCpy(g->filemask,FILEMASK_TXT);
  if (doc=DocPut) {
    StrCpy(g->find_text,doc->find_replace->find_text);
    StrCpy(g->replace_text,doc->find_replace->replace_text);
    g->replace=doc->find_replace->replace;
    g->match_case=doc->find_replace->match_case;
    g->whole_labels=doc->find_replace->whole_labels;
  }
  if (DocFormDo(g,,0,"$$FG,PURPLE$$$$TX+CX,\"Grep\"$$\n$$FG$$")) {
    if (doc) {
      StrCpy(doc->find_replace->find_text,g->find_text);
      StrCpy(doc->find_replace->replace_text,g->replace_text);
      doc->find_replace->replace=g->replace;
      doc->find_replace->match_case=g->match_case;
      doc->find_replace->whole_labels=g->whole_labels;
    }
    dir=PopUpPickDir;
    if (*dir) {
      *buf=0;
      if (g->match_case)
	StrCat(buf,"-i");
      if (!g->recurse)
	StrCat(buf,"-r");
      if (g->whole_labels)
	StrCat(buf,"+l");
      if (g->replace)
	st=MPrint("\"$$$$WW+H,1$$$$\";Cd(\"%s\");Grep(\"%Q\",\"%Q\",\"%Q\",\"%Q\");UserTaskCont;",dir,g->find_text,g->filemask,buf,g->replace_text);
      else
	st=MPrint("\"$$$$WW+H,1$$$$\";Cd(\"%s\");Grep(\"%Q\",\"%Q\",\"%Q\");UserTaskCont;",dir,g->find_text,g->filemask,buf);
      result=PopUp(st);
    }
    Free(dir);
  }
  Free(g);
  return result;
}

#help_index ""
