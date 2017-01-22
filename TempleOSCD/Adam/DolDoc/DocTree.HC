#help_index "DolDoc"

public Bool DocTreeFind(CDoc *haystack_doc,U8 *needle_path,
  CDocEntry **_tree_entry=NULL,
  CDocEntry **_start_indent=NULL, CDocEntry **_end_indent=NULL)
{//Find registry-like tree widget start and end.
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

public Bool DocTreeMake(CDoc *doc,U8 *path)
{//Make registry-like tree widget.
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

public I64 DocTreeBranchExe(CDoc *doc,U8 *path)
{//JIT Compile and execute code in doc tree branch.
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

public I64 ExeTreeBranch(U8 *registry_file,U8 *path)
{//JIT Compile and execute a specified branch in a specified file.
  I64 res;
  CDoc *doc=DocRead(registry_file);
  res=DocTreeBranchExe(doc,path);
  DocDel(doc);
  return res;
}
