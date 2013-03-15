#help_index "Doc"

public Bool DocTreeFind(CDoc *doc,U8 *path,
  CDocEntry **_tree_entry=NULL,
  CDocEntry **_start_indent=NULL, CDocEntry **_end_indent=NULL)
{
  I64 i=0,k=0;
  U8 *st1=StrNew(path),*st2=MAlloc(StrLen(path)+1);
  Bool result=FALSE,unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  if (_tree_entry) *_tree_entry=doc;
  if (_start_indent) *_start_indent=doc;
  if (_end_indent) *_end_indent=doc;
  while (*st1 && doc_e!=doc) {
    StrFirstRem(st1,"/",st2);
    if (*st2) {
      while (doc_e!=doc) {
	if (doc_e->type_u8==DOCT_INDENT)
	  i+=doc_e->attr;
	else if (i==k && doc_e->flags1&DOCEF1_TREE && !StrCmp(doc_e->tag+3,st2)) {
	   if (*st1)
	     break;
	   else {
	     if (_tree_entry) *_tree_entry=doc_e;
	     i=0;
	     while (doc_e!=doc && doc_e->type_u8!=DOCT_INDENT)
	       doc_e=doc_e->next;
	     if (doc_e!=doc) {
	       i=doc_e->attr;
	       if (_start_indent) *_start_indent=doc_e;
	       doc_e=doc_e->next;
	       while (doc_e!=doc && i>0) {
		 if (doc_e->type_u8==DOCT_INDENT) {
		   i+=doc_e->attr;
		   if (i<=0) {
		     if (_end_indent) *_end_indent=doc_e;
		     result=TRUE;
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
    DocUnlock(doc);
  Free(st1);
  Free(st2);
  return result;
}


public Bool DocTreeMake(CDoc *doc,U8 *path)
{
  I64 i=0,j=MIN_I64,k=0;
  U8 *st1=StrNew(path),
     *st2=MAlloc(StrLen(path)+1),
     *st3=StrNew(path);
  Bool result=TRUE,unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  doc->cur_entry=doc;
  doc->cur_data_col=0;
  while (*st1 && doc_e!=doc) {
    StrFirstRem(st1,"/",st2);
    if (*st2) {
      while (doc_e!=doc) {
	if (doc_e->type_u8==DOCT_INDENT) {
	  i+=doc_e->attr;
	  if (i==j) {
	    doc->cur_entry=doc_e;
	    doc->cur_data_col=0;
	    goto mt_done;
	  }
	} else if (i==k && doc_e->flags1&DOCEF1_TREE && !StrCmp(doc_e->tag+3,st2)) {
	   Free(st3);
	   st3=StrNew(st1);
	   j=i;
	   if (!*st1)
	     result=FALSE;
	   else
	     break;
	}
	doc_e=doc_e->next;
      }
      k+=2;
    }
  }
mt_done:
  if (result) {
    while (*st3) {
      StrFirstRem(st3,"/",st2);
      if (*st2) {
	DocPrintF(doc,"$$TR+C,\"%s\"$$\n$$ID,2$$",st2);
	doc->cur_entry=DocPutS(doc,"$$ID,-2$$");
	doc->cur_data_col=0;
      }
    }
  }
  if (unlock_doc)
    DocUnlock(doc);
  Free(st1);
  Free(st2);
  Free(st3);
  return result;
}

#help_index "Doc/Compiler;Compiler"
public I64 ExeDoc(CDoc *doc,Bool just_load=FALSE)
{
  I64 result;
  Bool okay=TRUE,unlock_doc=DocLock(doc);
  CLex *lx;
  lx=LexNew("",LF_DONT_FREE_BUF,FileNameAbs(sys_temp_filename));
  if (just_load)
    lx->flags|=LF_JUST_LOAD;
  if (Fs->last_lex!=&Fs->next_lex)
    lx->opts=Fs->last_lex->opts;
  QueIns(lx,Fs->last_lex);
  LexIncludeDoc(lx,NULL,doc);
  try {
    Lex(lx);
    result=ExeCmdLine(lx);
  } catch {
    if (Fs->except_argc>0) {
      if (Fs->except_argv[0]==EXCEPT_COMPILER ||
      Fs->except_argv[0]==EXCEPT_BREAK) {
	Fs->catch_except=TRUE;
	okay=FALSE;
	result=0;
      }
    }
  }
  QueRem(lx);
  if (okay)
    LexDel(lx); //TODO: can crash
  if (unlock_doc)
    DocUnlock(doc);
  return result;
}


public I64 DocTreeBranchExe(CDoc *doc,U8 *path)
{
  CDoc *doc2;
  Bool unlock_doc=DocLock(doc);
  CDocEntry *tree_branch,*start_indent,*end_indent;
  I64 result=0;
  if (DocTreeFind(doc,path,&tree_branch,&start_indent,&end_indent)) {
    doc2=DocCopy(doc,tree_branch,end_indent);
    result=ExeDoc(doc2);
    DocDel(doc2);
  }
  if (unlock_doc)
    DocUnlock(doc);
  return result;
}

public I64 ExeTreeBranch(U8 *registry_file,U8 *path)
{
  I64 result;
  CDoc *doc=DocRead(registry_file);
  result=DocTreeBranchExe(doc,path);
  DocDel(doc);
  return result;
}

#help_index "Doc"
