#help_index "DolDoc/Tree"

public Bool DocTreeFind(CDoc *haystack_doc,U8 *needle_path,
  CDocEntry **_tree_entry=NULL,
  CDocEntry **_start_indent=NULL, CDocEntry **_end_indent=NULL)
{//Find tree widget start and end.
  I64 i=0,k=0;
  U8 *st1=StrNew(needle_path),*st2=MAlloc(StrLen(needle_path)+1);
  Bool res=FALSE,unlock_doc=DocLock(haystack_doc);
  CDocEntry *doc_e=haystack_doc->head.next;
  if (_tree_entry) *_tree_entry=haystack_doc;
  if (_start_indent) *_start_indent=haystack_doc;
  if (_end_indent) *_end_indent=haystack_doc;
  while (*st1 && doc_e!=haystack_doc) {
    StrFirstRem(st1,"/",st2);
    if (*st2) {
      while (doc_e!=haystack_doc) {
	if (doc_e->type_u8==DOCT_INDENT)
	  i+=doc_e->attr;
	else if (i==k && doc_e->de_flags&DOCEF_TREE &&
	      !StrCmp(doc_e->tag+3,st2)) {
	  if (*st1)
	    break;
	  else {
	    if (_tree_entry) *_tree_entry=doc_e;
	    i=0;
	    while (doc_e!=haystack_doc && doc_e->type_u8!=DOCT_INDENT)
	      doc_e=doc_e->next;
	    if (doc_e!=haystack_doc) {
	      i=doc_e->attr;
	      if (_start_indent) *_start_indent=doc_e;
	      doc_e=doc_e->next;
	      while (doc_e!=haystack_doc && i>0) {
		if (doc_e->type_u8==DOCT_INDENT) {
		  i+=doc_e->attr;
		  if (i<=0) {
		    if (_end_indent) *_end_indent=doc_e;
		    res=TRUE;
		    break;
		  }
		}
		doc_e=doc_e->next;
	      }
	    }
	    goto ft_done;
	  }
	}
	doc_e=doc_e->next;
      }
      k+=2;
    }
  }
ft_done:
  if (unlock_doc)
    DocUnlock(haystack_doc);
  Free(st1);
  Free(st2);
  return res;
}

public Bool DocTreeFFind(U8 *name,U8 *path)
{//Find tree widget in file.
  CDoc *doc=DocRead(name);
  Bool res=DocTreeFind(doc,path);
  DocDel(doc);
  return res;
}

public Bool DocTreeMake(CDoc *doc,U8 *path)
{//Make tree widget.
  I64 i=0,j=I64_MIN,k=0;
  U8 *st1=StrNew(path),
	*st2=MAlloc(StrLen(path)+1),
	*st3=StrNew(path);
  Bool res=TRUE,unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->head.next;
  doc->cur_entry=doc;
  doc->cur_col=0;
  while (*st1 && doc_e!=doc) {
    StrFirstRem(st1,"/",st2);
    if (*st2) {
      while (doc_e!=doc) {
	if (doc_e->type_u8==DOCT_INDENT) {
	  i+=doc_e->attr;
	  if (i==j) {
	    doc->cur_entry=doc_e;
	    doc->cur_col=0;
	    goto mt_done;
	  }
	} else if (i==k && doc_e->de_flags&DOCEF_TREE &&
	      !StrCmp(doc_e->tag+3,st2)) {
	  Free(st3);
	  st3=StrNew(st1);
	  j=i;
	  if (!*st1)
	    res=FALSE;
	  else
	    break;
	}
	doc_e=doc_e->next;
      }
      k+=2;
    }
  }
mt_done:
  if (res) {
    while (*st3) {
      StrFirstRem(st3,"/",st2);
      if (*st2) {
	DocPrint(doc,"$$TR+C,\"%s\"$$\n$$ID,2$$",st2);
	doc->cur_entry=DocPrint(doc,"$$ID,-2$$");
	doc->cur_col=0;
      }
    }
  }
  if (unlock_doc)
    DocUnlock(doc);
  Free(st1);
  Free(st2);
  Free(st3);
  return res;
}

Bool DocTreeWriteJoin(CDoc *doc,U8 *path,Bool write,U8 *fmt,I64 argc,I64 *argv)
{//Rewrite doc tree branch.
  CDocEntry *tree_branch,*start_indent,*end_indent;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  Bool res,unlock_doc=DocLock(doc);
  if (res=DocTreeFind(doc,path,
	&tree_branch,&start_indent,&end_indent)) {
    DocCut(doc,start_indent->next,end_indent->last);
    doc->cur_entry=start_indent->next;
    doc->cur_col=doc->cur_entry->min_col;
  } else
    DocTreeMake(doc,path);
  DocPrint(doc,"%s",buf);
  if (write && DrvIsWritable(*doc->filename.name))
    DocWrite(doc);
  if (unlock_doc)
    DocUnlock(doc);
  Free(buf);
  return res;
}

Bool DocTreeAppendJoin(CDoc *doc,U8 *path,Bool write,U8 *fmt,I64 argc,I64 *argv)
{//Append to doc tree branch.
  CDocEntry *tree_branch,*start_indent,*end_indent;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  Bool res,unlock_doc=DocLock(doc);
  if (res=DocTreeFind(doc,path,
	&tree_branch,&start_indent,&end_indent)) {
    doc->cur_entry=end_indent;
    doc->cur_col=doc->cur_entry->min_col;
  } else
    DocTreeMake(doc,path);
  DocPrint(doc,"%s",buf);
  if (write && DrvIsWritable(*doc->filename.name))
    DocWrite(doc);
  if (unlock_doc)
    DocUnlock(doc);
  Free(buf);
  return res;
}

public Bool DocTreeWrite(CDoc *doc,U8 *path,Bool write=TRUE,U8 *fmt,...)
{//Rewrite doc tree branch.
  return DocTreeWriteJoin(doc,path,write,fmt,argc,argv);
}

public Bool DocTreeAppend(CDoc *doc,U8 *path,Bool write=TRUE,U8 *fmt,...)
{//Append to doc tree branch.
  return DocTreeAppendJoin(doc,path,write,fmt,argc,argv);
}

public Bool DocTreeFWrite(U8 *name,U8 *path,U8 *fmt,...)
{//Rewrite doc tree branch in file.
  CDoc *doc=DocRead(name);
  Bool res=DocTreeWriteJoin(doc,path,TRUE,fmt,argc,argv);
  DocDel(doc);
  return res;
}

public Bool DocTreeFAppend(U8 *name,U8 *path,U8 *fmt,...)
{//Append to doc tree branch in file.
  CDoc *doc=DocRead(name);
  Bool res=DocTreeAppendJoin(doc,path,TRUE,fmt,argc,argv);
  DocDel(doc);
  return res;
}

#help_index "DolDoc/Compiler;Compiler"
public I64 ExeDoc(CDoc *doc,I64 ccf_flags=0)
{//JIT Compile and execute a document.
  I64 res;
  Bool okay=TRUE,unlock_doc=DocLock(doc);
  CCmpCtrl *cc=CmpCtrlNew(,ccf_flags|CCF_DONT_FREE_BUF);
  if (Fs->last_cc!=&Fs->next_cc)
    cc->opts=Fs->last_cc->opts;
  QueIns(cc,Fs->last_cc);
  LexAttachDoc(cc,,doc);
  try {
    Lex(cc);
    res=ExeCmdLine(cc);
  } catch {
    if (Fs->except_ch=='Compiler' || Fs->except_ch=='Break') {
      Fs->catch_except=TRUE;
      okay=FALSE;
      res=0;
    }
  }
  QueRem(cc);
  if (okay)
    CmpCtrlDel(cc); //TODO: can crash
  if (unlock_doc)
    DocUnlock(doc);
  return res;
}

#help_index "DolDoc/Tree;DolDoc/Compiler;Compiler"
public I64 DocTreeExe(CDoc *doc,U8 *path)
{//Execute doc tree branch.
  CDoc *doc2;
  Bool unlock_doc=DocLock(doc);
  CDocEntry *tree_branch,*start_indent,*end_indent;
  I64 res=0;
  if (DocTreeFind(doc,path,&tree_branch,&start_indent,&end_indent)) {
    doc2=DocCopy(doc,tree_branch,end_indent);
    res=ExeDoc(doc2);
    DocDel(doc2);
  }
  if (unlock_doc)
    DocUnlock(doc);
  return res;
}

public I64 DocTreeFExe(U8 *name,U8 *path)
{//Execute doc tree branch in file.
  I64 res;
  CDoc *doc=DocRead(name);
  res=DocTreeExe(doc,path);
  DocDel(doc);
  return res;
}
