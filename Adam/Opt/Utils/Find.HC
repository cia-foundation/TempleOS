#help_index "Cmd Line (Typically)"

#define FND_REPLACE	0
#define FND_SKIP	1
#define FND_ALL		2
#define FND_ED		3
#define FND_ABORT_FILE	4
#define FND_SKIP_FILE	5

I64 PopUpFindMenu()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$CM+LX,2,4$$$$BT,\"REPLACE\",LE=FND_REPLACE$$"
	"$$CM+LX,22,0$$$$BT,\"SKIP\",LE=FND_SKIP$$"
	"$$CM+LX,2,4$$$$BT,\"ALL\",LE=FND_ALL$$"
	"$$CM+LX,22,0$$$$BT,\"ABORT ALL\",LE=DOCM_CANCEL$$"
	"$$CM+LX,2,4$$$$BT,\"EDIT\",LE=FND_ED$$"
	"$$CM+LX,22,0$$$$BT,\"ABORT FILE\",LE=FND_ABORT_FILE$$"
	"$$CM+LX,2,4$$$$BT,\"SKIP FILE\",LE=FND_SKIP_FILE$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 FindFile(U8 *needle_str,U8 *haystack_filename,
	I64 *_fuf_flags,U8 *replace_text)
{//Have you confused with $LK,"FileFind",A="MN:FileFind"$()?
  Bool first_on_line,write_this_file=FALSE,cont=!Bt(_fuf_flags,FUf_CANCEL);
  U8 *src,*dst,*dst2,*name_buf=NULL;
  I64 i,j,plen,rlen,dlen,cnt=0,old_flags,ss_flags;
  CDoc *cur_l,*doc=DocRead(haystack_filename,
	DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  CDocEntry *doc_e;

  if (Bt(_fuf_flags,FUf_IGNORE))
    ss_flags=SFF_IGNORE_CASE;
  else
    ss_flags=0;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS))
    ss_flags|=SFG_WHOLE_LABELS;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS_BEFORE))
    ss_flags|=SFF_WHOLE_LABELS_BEFORE;
  if (Bt(_fuf_flags,FUf_WHOLE_LABELS_AFTER))
    ss_flags|=SFF_WHOLE_LABELS_AFTER;

  plen=StrLen(needle_str);
  if (replace_text)
    rlen=StrLen(replace_text);
  doc_e=doc->head.next;
  while (doc_e!=doc && cont) {
    if (doc_e->type_u8==DOCT_TEXT) {
      src=doc_e->tag;
      first_on_line=TRUE;
      while (src && cont) {
	if (src=StrFind(needle_str,src,ss_flags)) {
	  cnt++;
	  if (first_on_line || Bt(_fuf_flags,FUf_REPLACE)) {
	    first_on_line=FALSE;
	    PutFileLink(haystack_filename,,doc_e->y+1,TRUE);
	    name_buf=MStrPrint("%s,%d",haystack_filename,doc_e->y+1);
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
	      i=FND_ALL;
	    else {
	      i=PopUpFindMenu;
	      if (i<0) {
		LBts(_fuf_flags,FUf_CANCEL);
		cont=FALSE;
		write_this_file=FALSE;
	      } else if (i==FND_ALL)
		LBts(_fuf_flags,FUf_ALL);
	      else if (i==FND_ABORT_FILE) {
		cont=FALSE;
		write_this_file=FALSE;
	      } else if (i==FND_SKIP_FILE)
		cont=FALSE;
	    }
	    if (i==FND_REPLACE || i==FND_ALL) {
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
	      "%12s,%04d*%s\n",haystack_filename,doc_e->y+1,dst2;
	      if (cur_l)
		cur_l->flags= cur_l->flags&~DOCF_PLAIN_TEXT |old_flags;
	      write_this_file=TRUE;
	    } else {
	      src++;
	      if (i==FND_ED) {
		Free(name_buf);
		name_buf=StrNew(doc->filename.name);
		doc->flags&=~DOCF_NO_CURSOR;
		doc->cur_entry=doc_e;
		doc->cur_col=doc_e->min_col;
		DocWrite(doc);
		DocDel(doc);
		"Wrote:%s\n",name_buf;
		Ed(name_buf);
		doc=DocRead(name_buf, DOCF_PLAIN_TEXT_TABS);
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

public I64 Find(U8 *needle_str,U8 *files_find_mask="*",
	U8 *fu_flags=NULL,U8 *replace_text=NULL)
{/*Find occurrences of a string in files.
This does not do regular expressions.
Anyway, it's good for searching and replacing.
Let's say it stands for global replace ;-)

"+r" =recurse
"+i" =ignore case
"+l" =whole labels only.
This will check for a nonlabel character before
and after.  If you have a var, "dd" and don't
want to match words like "Add", you
set this flag and it will see that the characters
before or after "dd" are label characters.
"+lb"=only checks for label chars before.
"+la"=only checks for label chars after.
*/
  I64 cnt=0,fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+i+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~(FUG_FILES_FIND|FUF_IGNORE|FUF_ALL|
	FUF_WHOLE_LABELS|FUF_WHOLE_LABELS_BEFORE|FUF_WHOLE_LABELS_AFTER))
    throw('FUF');
  LBEqu(&fuf_flags,FUf_REPLACE,replace_text);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND);
  fuf_flags&=FUF_ALL|FUF_REPLACE|FUF_IGNORE|FUF_WHOLE_LABELS|
	FUF_WHOLE_LABELS_BEFORE|FUF_WHOLE_LABELS_AFTER;
  while (tmpde && !Bt(&fuf_flags,FUf_CANCEL)) {
    cnt+=FindFile(needle_str,tmpde->full_name,&fuf_flags,replace_text);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  return cnt;
}

public I64 FileOcc(U8 *needle_str,
	U8 *files_find_mask="*",U8 *fu_flags="+r+i+l")
{/*Silently return occurrences of a string in files.
"+r"=recurse
"+i"=ignore case
"+l"=whole labels only.
"+lb"=only checks for label chars before.
"+la"=only checks for label chars after.
*/
  I64 cnt=0;
  Bool old_silent=Silent(TRUE);
  cnt=Find(needle_str,files_find_mask,fu_flags);
  Silent(old_silent);
  return cnt;
}

class CFind
{
  U8	find_text[STR_LEN] format "$$DA-P,"
	"A=\"FIND        :%s\"$$\n";
  U8	replace_text[STR_LEN] format "$$DA-P,"
	"A=\"REPLACE     :%s\"$$\n";
  Bool	replace		format "$$CB,\"REPLACE\"$$\n";
  Bool	match_case	format "$$CB,\"MATCH CASE\"$$\n";
  Bool	whole_labels	format "$$CB,\"WHOLE LABELS\"$$\n";
  U8	filemask[STR_LEN] format "$$DA-P,A=\"FILE MASK   :%s\"$$\n";
  Bool	recurse		format "$$CB,\"RECURSE\"$$\n";
};

I64 FindWiz()
{
  CDoc *doc;
  U8 buf[32],*dir,*st;
  CFind *g=CAlloc(sizeof(CFind));
  I64 res=0;
  g->recurse=TRUE;
  StrCpy(g->filemask,FILEMASK_TXT);
  if (doc=DocPut) {
    StrCpy(g->find_text,doc->find_replace->find_text);
    StrCpy(g->replace_text,doc->find_replace->replace_text);
    g->replace=doc->find_replace->replace;
    g->match_case=doc->find_replace->match_case;
    g->whole_labels=doc->find_replace->whole_labels;
  }
  if (DocForm(g,,0,"$$PURPLE$$$$TX+CX,\"Find\"$$\n$$FG$$")) {
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
	CatPrint(buf,"-i");
      if (!g->recurse)
	CatPrint(buf,"-r");
      if (g->whole_labels)
	CatPrint(buf,"+l");
      if (g->replace)
	st=MStrPrint("\"$$$$WW+H,1$$$$\";Cd(\"%s\");"
	      "Find(\"%Q\",\"%Q\",\"%Q\",\"%Q\");UserTaskCont;",
	      dir,g->find_text,g->filemask,buf,g->replace_text);
      else
	st=MStrPrint("\"$$$$WW+H,1$$$$\";Cd(\"%s\");"
	      "Find(\"%Q\",\"%Q\",\"%Q\");UserTaskCont;",
	      dir,g->find_text,g->filemask,buf);
      res=PopUp(st);
    }
    Free(dir);
  }
  Free(g);
  return res;
}
