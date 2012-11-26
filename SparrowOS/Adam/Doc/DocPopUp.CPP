public I64 DocEntryRun(CDoc *doc,CDocEntry *doc_e,Bool exited)
{ //sometimes returns locked, sometimes unlocked
  U8 ch=doc->cmd_U8,*st;
  I64 result=-1;
  DocLock(doc);
  if (!exited) {
    if (doc_e->flags1 & DOCEF1_ESC) {
      Msg(MSG_KEY_DOWN,CH_ESC,0,1<<TASKCf_DONT_FILTER);
      return 0;
    } if (doc_e->flags1 & DOCEF1_QUIT) {
      Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0,1<<TASKCf_DONT_FILTER);
      return 0;
    } else if (doc_e->flags1 & DOCEF1_CHECK_COLLAPSABLE)
      doc_e->flags1^=DOCEF1_CHECKED_COLLAPSED;
  }
  doc->best_d=MAX_I64;
  try {
    if (ch==CH_SPACE) {
      if (doc_e->flags1 & DOCEF1_LINK && doc->left_click_link)
	result=(*doc->left_click_link)(doc,doc_e);
      if (doc_e->flags1 & DOCEF1_LEFT_EXP) {
	result=doc_e->left_exp;
	Msg(MSG_CMD,result,0,1<<TASKCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_LEFT_CB && doc_e->left_cb) {
	DocUnlock(doc);
	result=(*doc_e->left_cb)(doc,doc_e);
	Msg(MSG_CMD,result,0,1<<TASKCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_LEFT_MACRO) {
	if (doc_e->flags1 & DOCEF1_POPUP) {
	  st=StrNew(doc_e->left_macro);
	  DocUnlock(doc);
	  PopUp(st,Fs);
	  Free(st);
	} else {
	  if (doc_e->flags1 & DOCEF1_LEFT_AUTO)
	    AutoStr("%s",doc_e->left_macro);
	  else
	    Auto("%s",doc_e->left_macro);
	}
      }
    } else if (ch==CH_NEW_LINE) {
      if (doc_e->flags1 & DOCEF1_LINK && doc->right_click_link)
	result=(*doc->right_click_link)(doc,doc_e);
      if (doc_e->flags1 & DOCEF1_RIGHT_EXP) {
	result=doc_e->right_exp;
	Msg(MSG_CMD,result,0,1<<TASKCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_RIGHT_CB && doc_e->right_cb) {
	DocUnlock(doc);
	result=(*doc_e->right_cb)(doc,doc_e);
	Msg(MSG_CMD,result,0,1<<TASKCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_RIGHT_MACRO) {
	if (doc_e->flags1 & DOCEF1_POPUP) {
	  st=StrNew(doc_e->right_macro);
	  DocUnlock(doc);
	  PopUp(st,Fs);
	  Free(st);
	} else {
	  if (doc_e->flags1 & DOCEF1_RIGHT_AUTO)
	    AutoStr("%s",doc_e->right_macro);
	  else
	    Auto("%s",doc_e->right_macro);
	}
      }
    }
  }
  catch
    DocBottom(doc);
  doc->cmd_U8=CH_SPACE;
  return result;
}

U8 *EdOverStrikeCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  *st=doc->flags & DOCF_OVERSTRIKE ? 'O':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdFilterCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  *st=doc->find_replace->filter_lines ? 'F':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdDollarCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  *st=doc->flags & DOCF_IN_DOLLAR ? '$$':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdMoreCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  if (doc->flags&DOCF_MORE)
    StrCpy(st,"MoreÄ");
  else
    StrCpy(st,"ÄÄÄÄÄ");
  return st;
}

U8 *EdQuotesCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  CLex *lx=mem_task->last_lex;
  U8 *st=MAlloc(8,mem_task);
  *st=lx->flags & LF_IN_QUOTES ? '"':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdBracesCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  CLex *lx=mem_task->last_lex;
  U8 *st=MAlloc(8,mem_task);
  *st=lx->braces_cnt ? '}':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdSemicolonCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  CLex *lx=mem_task->last_lex;
  U8 *st=MAlloc(8,mem_task);
  *st=lx->statement_cnt ? ';':CH_SINGLE_HORZ;
  st[1]=0;
  return st;
}

U8 *EdDollarTypeCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *src=DefineSub(doc->cur_entry->type_u8,"ST_DOC_CMDS"),
     *st=CAlloc(8,mem_task);
  if (doc->cur_entry==doc)
    src="EOF";
  else if (!src)
    src="ERR";
  SPrintF(st,"%-3ts",src);
  return st;
}

public I64 DocGetKey(I64 *_sc=NULL)
{
  I64 ch,sc;
  Bool cont,old_preempt=IsPreempt;
  do {
    ch=GetKey(&sc,TRUE);
    DocRecalc(DocCur);
    cont=FALSE;
    if (ch==CH_ESC || ch==CH_SHIFT_ESC) { //Check for exit filter mode
      Preempt(OFF);
      if (DocCur && DocCur->find_replace->filter_lines) {
	EdUndoFilter(DocCur);
	cont=TRUE;
      }
      Preempt(old_preempt);
    }
  } while (cont);
  if (_sc) *_sc=sc;
  return ch;
}

#help_index "Doc/Output"
public Bool View()
{
  I64 ch;
  do ch=DocGetKey;
  while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  return ch==CH_ESC;
}

#help_index "Doc"
public Bool DocDo(CDoc *doc,I64 doc_flags=0)
{
  CDoc *old_doc=DocCur,*old_aux_doc=DocAux,*aux;
  CDocEntry *doc_e;
  I64 old_attr=Fs->text_attr,
      old_top =Fs->win_top, old_bottom=Fs->win_bottom,
      old_left=Fs->win_left,old_right =Fs->win_right;
  Bool result,unlock,old_preempt;

  if (doc_flags&DOF_WIN_MAX)
    WinMax;

  unlock=DocLock(doc);
  doc->win_task=Fs;
  aux=DocNew;
  aux->flags|=DOCF_AUX_DOC;
  aux->settings_root.dft_text_attr=Fs->border_attr;
  DocPutS(aux,"$$CM+TY+LX+NC,0,-1$$");
  if (Bt(&Fs->display_flags,DISPLAYf_HAS_CLOSE_WIN))
    DocPutS(aux,"$$TX+RX+BD,\"[X]\"$$");
  DocPutS(aux,"$$BK,1$$$$TX+LX+BD,\"MENU\"$$$$BK,0$$");

  doc_e=DocPutS(aux,"$$DA-T-P+BD+RD+CX+SCX+IV,256,\"%s...\",16$$");
  doc_e->data=&doc->filename.name;
  DocDataFmt(doc_e);

  if (doc->flags & DOCF_ALLOW_UNDO) {
    DocPutS(aux,"$$CM+BY+LX+NC,1,1$$");
    doc_e=DocPutS(aux,"$$DA+BD+RD-T,32,\"Undo:%03d\"$$\n");
    doc_e->data=&doc->undo_cnt;
    DocDataFmt(doc_e);
  }

  DocPutS(aux,"$$CM+BY+RX+NC,-30,1$$");
  doc_e=DocPutS(aux,"$$TX+BD+TC,\"     \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdMoreCB;
  doc_e=DocPutS(aux,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdDollarTypeCB;
  doc_e=DocPutS(aux,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdFilterCB;
  doc_e=DocPutS(aux,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdOverStrikeCB;
  doc_e=DocPutS(aux,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdDollarCB;
  doc_e=DocPutS(aux,"$$DA+BD+RD-T,32,\"Line:%04d \"$$");
  doc_e->data=&doc->line;
  DocDataFmt(doc_e);
  doc_e=DocPutS(aux,"$$DA+BD+RD-T,32,\"Col:%04d\"$$\n");
  doc_e->data=&doc->col;
  DocDataFmt(doc_e);

  DocRecalc(aux);
  DocRecalc(doc);
  if (!(doc_flags&DOF_DONT_HOME))
    DocHome(doc);
  old_preempt=Preempt(OFF);
  DocQueRem(old_aux_doc);
  DocQueInsRev(aux,Fs->next_doc);
  if (doc!=old_doc)
    doc->parent_doc=old_doc;
  DocQueRem(old_doc);
  DocQueIns(doc,Fs->last_doc);
  Fs->text_attr=doc->settings_root.dft_text_attr;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (doc_flags&DOF_MIN_SIZE)
    doc->flags|=DOCF_MIN_SIZE;

  Preempt(old_preempt);
  DocUnlock(doc);
  WinMgrSync(2,TRUE);
  if (doc->flags&DOCF_MIN_SIZE)
    WinMgrSync(2,TRUE);
  result=View;

  Preempt(OFF);
  DocLock(doc);
  if (result) {
    doc_e=doc->root.next;
    while (doc_e!=doc) {
      if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX)
	DocDataGet(doc_e);
      doc_e=doc_e->next;
    }
  }
  if (unlock)
    DocUnlock(doc);

  DocQueRem(aux);
  DocQueInsRev(old_aux_doc,Fs->next_doc);
  DocDel(aux);
  DocQueRem(doc);
  DocQueIns(old_doc,Fs->last_doc);
  Fs->text_attr=old_attr;
  if (doc_flags&DOF_MIN_SIZE) {
    Fs->win_top=old_top; Fs->win_bottom=old_bottom;
    Fs->win_left=old_left; Fs->win_right=old_right;
    TaskDerivedValsUpdate;
  }
  Preempt(old_preempt);
  return result;
}


#help_index "Doc/Input"
public Bool DocFormDo(U8 *D,U8 *class_name,
    I64 doc_flags=0,U8 *header=NULL,U8 *footer=NULL)
//Supply a class name that has fmtstr definitions
//and this routine will allow user input.
{
  CMemberList *ml;
  CDocEntry *doc_e;
  U8 *fmtstr;
  CHashClass *tempc;
  CDoc *doc;
  Bool result=FALSE;
  if (!(tempc=HashFind(class_name,Fs->hash_table,HTT_CLASS)))
    return FALSE;
  doc=DocNew;
  StrCpy(doc->desc,"Form");
  if (header) DocPutS(doc,header);
  doc->flags|=DOCF_OVERSTRIKE|DOCF_FORM;
  if (doc_flags&DOF_MIN_SIZE)
    doc->flags|=DOCF_MIN_SIZE;
  ml=tempc->member_list;
  while (ml) {
    if (fmtstr=MemberMetaData("fmtstr",ml)) {
      doc_e=DocPutS(doc,fmtstr);
      doc_e->data=D+ml->offset;
      doc_e->my_fmt_data=MemberMetaData("fmtdata",ml);
      DocDataFmt(doc_e);
    }
    ml=ml->next;
  }
  if (footer) DocPutS(doc,footer);
  if (doc->root.next!=doc)	{
    doc->flags&=~(DOCF_ATTR_BY_PRT | DOCF_ATTR_BY_FILENAME);
    DocRecalc(doc);
    if (DocDo(doc,doc_flags)) {
      doc_e=doc->cur_entry;
      if (doc_e!=doc) {
	DocEntryRun(doc,doc_e,TRUE);
	DocUnlock(doc);
      }
      result=TRUE;
    }
  }
  DocDel(doc);
  return result;
}

U0 DocMenuDoEndTaskCB()
{
  WinToTop;
  throw(EXCEPT_LOCAL);
}

public I64 DocMenuDo(CDoc *m,I64 doc_flags=0)
{ //returns m unlocked
  U8 *old_end_cb=Fs->task_end_cb;
  Bool old_break_shift_esc=LBts(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC);
  CDocEntry *doc_e;
  I64 old_doc_flags=m->flags,result=DOCM_CANCEL;
  Fs->task_end_cb=&DocMenuDoEndTaskCB;
  try {
    if (m) {
      StrCpy(m->desc,"Menu");
      m->flags&=~(DOCF_ATTR_BY_PRT | DOCF_ATTR_BY_FILENAME);
      if (DocDo(m,doc_flags)) {
	doc_e=m->cur_entry;
	if (doc_e!=m) {
	  result=DocEntryRun(m,doc_e,TRUE);
	  DocUnlock(m);
	}
      }
    }
  } catch {
    if (Fs->except_argc==1 && Fs->except_argv[0]==EXCEPT_LOCAL) {
      if (!(doc_flags & DOF_INTERCEPT_TASK_END))
	Exit;
      Fs->catch_except=TRUE;
    }
  }
  m->flags|=old_doc_flags&(DOCF_ATTR_BY_PRT | DOCF_ATTR_BY_FILENAME);
  LBEqu(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC,old_break_shift_esc);
  Fs->task_end_cb=old_end_cb;
  return result;
}

#help_index "Doc"
public I64 PopUp(U8 *msg,CTask *parent=NULL,CTask **_pu_task=NULL)
{
  I64 result=0;
  Bool old_preempt=Preempt(OFF);
  CTaskCmd *tempc;
  CTask *task=Spawn(&ServantUserCmdLine,NULL,"System",parent);
  if (!parent) {
    TaskRqstQue(task,parent,msg,1<<TASKCf_EXIT_ON_COMPLETE);
    if (_pu_task) *_pu_task=task;
    Preempt(old_preempt);
    return 0;
  } else {
    Fs->popup_task=task;
    tempc=TaskRqstQue(task,parent,msg,
      1<<TASKCf_WAKE_MASTER | 1<<TASKCf_FOCUS_MASTER);
    if (_pu_task) *_pu_task=task;
    GetRqstResult(tempc,&result);
    Fs->popup_task=NULL;
    Kill(task);
    if (_pu_task) *_pu_task=NULL;
    Preempt(old_preempt);
    return result;
  }
}

#help_index "Doc/Output"
public CTask *PopUpViewDoc(CDoc *doc,I64 doc_flags=0)
{
  U8 *buf=MSPrintF("if (!IsSysDeterministic) Preempt(ON);DocDo(0x%X,0x%X);",doc,doc_flags);
  CTask *task=Spawn(&ServantUserCmdLine,NULL,"View",Fs);
  TaskRqstQue(task,NULL,buf,1<<TASKCf_EXIT_ON_COMPLETE);
  Free(buf);
  return task;
}

U0 PopUpViewStrTaskEndCB()
{
  DocDel(NamedFramePtr("ViewStrFrame"));
  Exit;
}

public CTask *PopUpViewStr(U8 *msg)
{
  CTask *task;
  CDoc *doc=DocNew;
  DocPrintF(doc,msg);
  task=PopUpViewDoc(doc);
  NamedFramePtrAdd("ViewStrFrame",doc,task);
  task->task_end_cb=&PopUpViewStrTaskEndCB;
  return task;
}

#help_index "Doc/Input"
public I64 PopUpMenu(CDoc *doc,I64 doc_flags=0)
{
  doc->flags|=DOCF_MIN_SIZE | DOCF_FORM;
  return ServantUser("DocMenuDo(0x%X,0x%X);",doc,doc_flags);
}

public Bool PopUpForm(U8 *D,U8 *class_name,I64 doc_flags=DOF_MIN_SIZE,
    U8 *header=NULL,U8 *footer=NULL)
{
  return ServantUser("DocFormDo(0x%X,0x%X,0x%X,0x%X,0x%X);",D,class_name,
    doc_flags,header,footer);
}

#help_index "Doc/Input;File/FileNames"
public U8 *PopUpPickFile(U8 *dir=NULL)
{
  U8 *result,*st,*st2;
  if (dir)
    st=MSPrintF("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MSPrintF("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",st2);
    Free(st2);
  }
  result=PopUp(st,Fs);
  Free(st);
  return result;
}

public U8 *PopUpPickDir(U8 *dir=NULL)
{
  U8 *result,*st,*st2;
  if (dir)
    st=MSPrintF("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MSPrintF("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",st2);
    Free(st2);
  }
  result=PopUp(st,Fs);
  Free(st);
  return result;
}

public U8 *FileNameForm(U8 *dft=NULL,I64 doc_flags=0,CTask *mem_task=NULL)
{
  CEdFileName fn;
  if (dft)
    StrCpy(fn.name,dft);
  else
    *fn.name=0;
  if (DocFormDo(fn.name,"CEdFileName",doc_flags))
    return StrNew(fn.name,mem_task);
  else
    return NULL;
}

public U8 *PopUpFileName(U8 *dft=NULL,I64 doc_flags=0)
{
  U8 *st=MSPrintF("FileNameForm(\"%Q\",0x%X,Fs->parent_task);",dft,doc_flags|DOF_MIN_SIZE),
     *result=PopUp(st,Fs);
  Free(st);
  return result;
}

#help_index "Doc"
public Bool PopUpCd()
{
  Bool result;
  U8 *st=PopUpPickDir;
  if (st) {
    result=Cd(st);
    Free(st);
  } else
    result=FALSE;
  return result;
}

#help_index "Doc/Input"
public U8 *PopUpPickListEntry(U8 *lst)
{
  I64 i=0;
  CDoc *doc=DocNew;
  while (*lst) {
    if (*lst=='@') { //Check for '@' alias list entry
      i--;
      lst++;
    }
    DocPrintF(doc,"$$MU,\"%s\",0x%08X$$\n",
	lst,i++);
    lst+=StrLen(lst)+1;
  }
  DocPutS(doc,"\n$$MU,\"CANCEL\",DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public U8 *PopUpPickDefineSub(U8 *st)
{
  return PopUpPickListEntry(Define(st));
}

public I64 PopUp1(U8 *b1,I64 n1,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i,l1=StrLen(b1);
  CDoc *doc=DocNew;
  if (header) DocPutS(doc,header);
  DocPrintF(doc,"$$CM+CX,%d,4$$$$BT,\"%s\",%d$$\n",-l1/2,b1,n1);
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 PopUp2(U8 *b1,I64 n1,U8 *b2,I64 n2,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i,l1=StrLen(b1),l2=StrLen(b2),y;
  CDoc *doc=DocNew;
  if (header) {
    DocPutS(doc,header);
    y=4;
  } else {
    DocPrintF(doc,"%*s\n",l1+l2+10,"");
    y=3;
  }
  DocPrintF(doc,"$$CM+CX,%d,%d$$$$BT,\"%s\",%d$$",-(l1+l2+3)>>1,y,b1,n1);
  DocPrintF(doc,"$$CM+CX,%d,0$$$$BT,\"%s\",%d$$\n" ,-(l1+l2+3)>>1+l1+6,b2,n2);
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public Bool PopUpOk(U8 *header=NULL,U8 *footer=NULL)
{
  return PopUp1("OKAY",1,header,footer)>0;
}

public Bool PopUpNoYes(U8 *header=NULL,U8 *footer=NULL)
{
  return $WW,0$PopUp2("YES",1,"NO",0,header,footer)>0;
}

public Bool PopUpCancelOk(U8 *header=NULL,U8 *footer=NULL)
{
  return PopUp2("OKAY",1,"CANCEL",0,header,footer)>0;
}


#define RSAC_REPLACE	0
#define RSAC_SKIP	1
#define RSAC_ALL	2

public I64 PopUpReplaceSkipAllCancel(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPutS(doc,header);
  DocPutS(doc,"$$CM+LX,1,4$$$$BT,\"REPLACE\",RSAC_REPLACE$$"
	"$$CM+LX,17,0$$$$BT,\"SKIP\",RSAC_SKIP$$"
	"$$CM+LX,1,3$$$$BT,\"ALL\",RSAC_ALL$$"
	"$$CM+LX,17,0$$$$BT,\"CANCEL\",DOCM_CANCEL$$\n");
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U8 *DoPopUpMGetS(U8 *header,CTask *mem_task)
{
  U8 *result,*st;
  if (header)
    "" header;
  st=MGetS(GETSF_WITH_NEW_LINE);
  result=StrNew(st,mem_task);
  Free(st);
  return result;
}

public U8 *PopUpMGetS(U8 *header=NULL)
{
  U8 *st=MSPrintF("DoPopUpMGetS(0x%X,0x%X);",header,Fs),
     *result=PopUp(st,Fs);
  Free(st);
  return result;
}

public I64 PopUpRangeI64(I64 lo,I64 hi,I64 step=1,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPutS(doc,header);
  for (i=lo;i<=hi;i+=step)
    DocPrintF(doc,"$$MU,\"%d\",%d$$\n",i,i);
  if (footer)
    DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public F64 PopUpRangeF64(F64 lo,F64 hi,F64 step,
    U8 *fmt="%9.4f",U8 *header=NULL,U8 *footer=NULL)
{
  F64 d;
  I64 i;
  U8 buf[136];
  CDoc *doc=DocNew;
  if (header)
    DocPutS(doc,header);
  for (d=lo;d<=hi;d+=step) {
    SPrintF(buf,fmt,d);
    DocPrintF(doc,"$$MU,\"%s\",0x%X$$\n",buf,d);
  }
  if (footer)
    DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i(F64);
}

public F64 PopUpRangeF64Exp(F64 lo,F64 hi,F64 factor,
    U8 *fmt="%9.4f",U8 *header=NULL,U8 *footer=NULL)
{
  F64 d;
  I64 i;
  U8 buf[136];
  CDoc *doc=DocNew;
  if (header)
    DocPutS(doc,header);
  for (d=lo;d<=hi;d*=factor) {
    SPrintF(buf,fmt,d);
    DocPrintF(doc,"$$MU,\"%s\",0x%X$$\n",buf,d);
  }
  if (footer)
    DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i(F64);
}

public F64 PopUpRangeF64Log(F64 lo,F64 hi,I64 steps,
    U8 *fmt="%9.4f",U8 *header=NULL,U8 *footer=NULL)
{
  return PopUpRangeF64Exp(lo,hi,Exp(Ln(hi/lo)/(steps-1)),fmt,header,footer);
}

#help_index "Task/Rqsts"
public I64 AdamFile(U8 *filename,Bool warn_ext=TRUE)
{
  Bool okay=TRUE;
  U8 *name=FileNameAbs(filename),
     *name2=DftExt(name,"CPP.Z");
  I64 result=0;
  if (warn_ext &&
      !WildCardMatch("*.CPP.Z",name2) &&
      !PopUpCancelOk(ST_WARN_ST "Not .CPP.Z File\n\n"))
    okay=FALSE;
  if (okay)
    result=Adam("#include \"%s\";",name2);
  Free(name2);
  Free(name);
  return result;
}
#help_index "Doc"

public I64 PopUpFile(U8 *filename,Bool warn_ext=TRUE,CTask *parent=NULL,CTask **_pu_task=NULL)
{
  Bool okay=TRUE;
  U8 *st,*name=FileNameAbs(filename),
     *name2=DftExt(name,"CPP.Z");
  I64 result=0;
  if (warn_ext &&
      !WildCardMatch("*.CPP.Z",name2) &&
      !PopUpCancelOk(ST_WARN_ST "Not .CPP.Z File\n\n"))
    okay=FALSE;
  if (okay) {
    st=MSPrintF("\"$$$$WW+H,1$$$$\";Auto(\"ExeFile(\\\"%s\\\");\\n\");UserTaskCont;",name2);
    result=PopUp(st,parent,_pu_task);
    Free(st);
  }
  Free(name2);
  Free(name);
  return result;
}

#help_index "Graphics/Input"
public I64 PopUpColor(U8 *header=NULL,Bool allow_transparent=TRUE,Bool allow_dft=TRUE)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPutS(doc,header);
  if (allow_transparent)
    DocPutS(doc,"$$BG,WHITE$$$$MU,\"TRANSPARENT\",COLOR_TRANSPARENT$$$$BG$$\n");
  DocPutS(doc,"$$FM,BLACK$$");
  for (i=0;i<16;i++)
    DocPrintF(doc,"$$BG,%d$$$$MU,\"__%-8Z__\",%d$$$$BG$$\n",i,i,"ST_COLORS",i);
  if (allow_dft)
    DocPutS(doc,"$$BG,WHITE$$$$MU,\"DEFAULT\",DOC_DFT$$$$BG$$");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 PopUpLightingColor(U8 *header=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPutS(doc,header);
  DocPutS(doc,"\n$$FM,BLACK$$Single-Sided Full-Range Double-Sided Full-Range\n");
  for (i=0;i<8;i++) {
    DocPrintF(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",%d$$$$BG$$ ",i,i,"ST_COLORS",i);
    DocPrintF(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",%d$$$$BG$$\n",i,i,"ST_COLORS",ROPF_TWO_SIDED+i);
  }
  DocPutS(doc,"\nSingle-Sided Half-Range Double-Sided Half-Range\n");
  for (i=0;i<16;i++) {
    DocPrintF(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",%d$$$$BG$$ ",i,i,"ST_COLORS",ROPF_HALF_RANGE_COLOR+i);
    DocPrintF(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",%d$$$$BG$$\n",i,i,"ST_COLORS",ROPF_TWO_SIDED+ROPF_HALF_RANGE_COLOR+i);
  }
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CTransformForm
{
  F64 x_scale	fmtstr "$$DA-T,32,\"X Scale :%12.6f\"$$\n";
  F64 y_scale	fmtstr "$$DA-T,32,\"Y Scale :%12.6f\"$$\n";
  F64 z_scale	fmtstr "$$DA-T,32,\"Z Scale :%12.6f\"$$\n";
  F64 é_z	fmtstr "\nAngles are in degrees\n$$DA-T,32,\"Z-Axis  :%12.6f\"$$\n";
  F64 é_y	fmtstr "$$DA-T,32,\"Y-Axis  :%12.6f\"$$\n";
  F64 é_x	fmtstr "$$DA-T,32,\"X-Axis  :%12.6f\"$$\n";
  F64 x_offset	fmtstr "\n$$DA-T,32,\"X Offset:%12.6f\"$$\n";
  F64 y_offset	fmtstr "$$DA-T,32,\"Y Offset:%12.6f\"$$\n";
  F64 z_offset	fmtstr "$$DA-T,32,\"Z Offset:%12.6f\"$$\n";
};

Bool PopUpTransform(I64 *r)
{
  CTransformForm *tf=MAlloc(sizeof(CTransformForm));
  tf->x_scale=1.0;
  tf->y_scale=1.0;
  tf->z_scale=1.0;
  tf->é_x=0.0;
  tf->é_y=0.0;
  tf->é_z=0.0;
  tf->x_offset=0.0;
  tf->y_offset=0.0;
  tf->z_offset=0.0;
  if (PopUpForm(tf,"CTransformForm")) {
    MemSet(r,0,16*sizeof(I64));
    r[0] =tf->x_scale*gr_scale;
    r[5] =tf->y_scale*gr_scale;
    r[10]=tf->z_scale*gr_scale;
    r[15]=gr_scale;

    GrRotZEqu(r,tf->é_z*ã/180.0);
    GrRotYEqu(r,tf->é_y*ã/180.0);
    GrRotXEqu(r,tf->é_x*ã/180.0);

    r[3] =tf->x_offset*gr_scale;
    r[7] =tf->y_offset*gr_scale;
    r[11]=tf->z_offset*gr_scale;
    return TRUE;
  } else
    return FALSE;
}


#define PU_SPACING	10
U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,i1,j1;
  CDoc *doc=DocCur(task);
  for (j=0;j<17;j++)
    for (i=0;i<17;i++) {
      if (j<16)
	j1=j;
      else
	j1=COLOR_TRANSPARENT;
      if (i<16)
	i1=i;
      else
	i1=COLOR_TRANSPARENT;
      dc->color=j1+i1<<16+ROPF_DITHER;
      GrRect(dc,i*PU_SPACING,j*PU_SPACING+NamedFramePtr("DitherDoFrame",task),PU_SPACING,PU_SPACING);
    }
  i=(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH;
  if (doc->max_x>i) i=doc->max_x;
  task->win_right=task->win_left+i;
  task->win_bottom=task->win_top+doc->max_y+(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH;
}

I64 DitherColorDo(U8 *header=NULL)
{
  I64 msg_code,p1,p2,result=-1,i,j;
  DocClear;
  "" header;
  DocRecalc(DocCur);
  NamedFramePtrAdd("DitherDoFrame",(DocCur->max_y+1)*FONT_HEIGHT);
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_DOWN|1<<MSG_KEY_DOWN);
    if (msg_code==MSG_IP_L_DOWN) {
      msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP);
      i=p1/PU_SPACING;
      j=(p2-NamedFramePtr("DitherDoFrame"))/PU_SPACING;
      if (p1>=0 && p2>=0 && 0<=j<17 && 0<=i<17) {
	if (j==16) j=COLOR_TRANSPARENT;
	if (i==16) i=COLOR_TRANSPARENT;
	result=j+i<<16+ROPF_DITHER;
	goto dcd_done;
      }
    }
  } while (!(msg_code==MSG_KEY_DOWN && (p1==CH_ESC || p1==CH_SHIFT_ESC)));
  do GetMsg(&p1,&p2,1<<MSG_KEY_UP);
  while (!p1);
dcd_done:
  SettingsPop;
  NamedFramePtrDel("DitherDoFrame");
  return result;
}

public I64 PopUpDitherColor(U8 *header=NULL)
{
  U8 buf[136];
  SPrintF(buf,"DitherColorDo(0x%X);",header);
  return PopUp(buf,Fs);
}


U0 PopUpSnap(CSnap *g)
{
  F64 d;
  I64 i=PopUpRangeI64(1,16,1,"Snap\n");
  if (i>=1) {
    d=PopUpRangeF64Exp(1.0/16.0,4.0,2.0,"%5.3f","Pointer Speed Scaler\n");
    if (d>0)
      SetSnap(g,i,d);
  }
}

#help_index "Doc"
