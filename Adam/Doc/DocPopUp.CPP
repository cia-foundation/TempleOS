#help_index "Doc"

public I64 DocEntryRun(CDoc *doc,CDocEntry *doc_e,Bool exited,I64 *_has_action=NULL)
{ //sometimes returns locked, sometimes unlocked
  U8 ch=doc->cmd_U8,*st;
  I64 result=DOCM_CANCEL,has_action=FALSE;
  DocLock(doc);
  if (!exited) {
    if (doc_e->flags1 & DOCEF1_ESC) {
      Msg(MSG_KEY_DOWN,CH_ESC,0,1<<SVCf_DONT_FILTER);
      has_action=TRUE;
      goto er_done;
    } if (doc_e->flags1 & DOCEF1_QUIT) {
      Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0,1<<SVCf_DONT_FILTER);
      has_action=TRUE;
      goto er_done;
    } else if (doc_e->flags1 & DOCEF1_CHECK_COLLAPSABLE) {
      doc_e->flags1^=DOCEF1_CHECKED_COLLAPSED;
      has_action=TRUE;
    }
  }
  try {
    if (ch==CH_SPACE) {
      if (doc_e->flags1 & DOCEF1_LINK && doc->left_click_link) {
	result=(*doc->left_click_link)(doc,doc_e);
	has_action=TRUE;
      }
      if (doc_e->flags1 & DOCEF1_LEFT_EXP) {
	result=doc_e->left_exp;
	has_action=TRUE;
	Msg(MSG_CMD,result,0,1<<SVCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_LEFT_CB && doc_e->left_cb) {
	DocUnlock(doc);
	result=(*doc_e->left_cb)(doc,doc_e);
	has_action=TRUE;
	Msg(MSG_CMD,result,0,1<<SVCf_DONT_FILTER);
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
	has_action=TRUE;
      }
    } else if (ch=='\n') {
      if (doc_e->flags1 & DOCEF1_LINK && doc->right_click_link) {
	result=(*doc->right_click_link)(doc,doc_e);
	has_action=TRUE;
      }
      if (doc_e->flags1 & DOCEF1_RIGHT_EXP) {
	result=doc_e->right_exp;
	has_action=TRUE;
	Msg(MSG_CMD,result,0,1<<SVCf_DONT_FILTER);
      }
      if (doc_e->flags1 & DOCEF1_RIGHT_CB && doc_e->right_cb) {
	DocUnlock(doc);
	result=(*doc_e->right_cb)(doc,doc_e);
	has_action=TRUE;
	Msg(MSG_CMD,result,0,1<<SVCf_DONT_FILTER);
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
	has_action=TRUE;
      }
    }
  }
  catch
    DocBottom(doc);
  doc->cmd_U8=CH_SPACE;
er_done:
  if (_has_action) *_has_action=has_action;
  return result;
}

U8 *EdOverStrikeCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  if (doc->flags & DOCF_OVERSTRIKE)
    *st='O';
  else
    *st='Ä';
  st[1]=0;
  return st;
}

U8 *EdFilterCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  if (doc->find_replace->filter_lines)
    *st='F';
  else
    *st='Ä';
  st[1]=0;
  return st;
}

U8 *EdDollarCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  if (doc->flags & DOCF_IN_DOLLAR)
    *st='$$';
  else
    *st='Ä';
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

U8 *EdDollarTypeCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *src=DefineSub(doc->cur_entry->type_u8,"ST_DOC_CMDS"),
     *st=CAlloc(8,mem_task);
  if (doc->cur_entry==doc)
    src="EOF";
  else if (!src)
    src="ERR";
  SPrint(st,"%-3ts",src);
  return st;
}

public I64 DocGetKey(I64 *_sc=NULL)
{
  I64 ch,sc;
  Bool cont,old_preempt=IsPreempt;
  do {
    ch=GetKey(&sc,TRUE);
    DocRecalc(DocPut);
    cont=FALSE;
    if (ch==CH_ESC || ch==CH_SHIFT_ESC) { //Check for exit filter mode
      Preempt(OFF);
      if (DocPut && DocPut->find_replace->filter_lines) {
	EdUndoFilter(DocPut);
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
public Bool DocDo(CDoc *doc,I64 dof_flags=0)
{
  CDoc *old_put_doc    =DocPut,
       *old_display_doc=DocDisplay,
       *old_border_doc =DocBorder,*bdoc;
  CDocEntry *doc_e;
  I64 old_attr=Fs->text_attr,
      old_top =Fs->win_top, old_bottom=Fs->win_bottom,
      old_left=Fs->win_left,old_right =Fs->win_right,
      old_title_src=Fs->title_src;
  Bool result,unlock,old_preempt;
  U8 *old_task_title;
  if (dof_flags&DOF_WIN_MAX)
    WinMax;

  unlock=DocLock(doc);
  doc->win_task=Fs;
  bdoc=DocNew;
  bdoc->flags|=DOCF_BORDER_DOC;
  DocPrint(bdoc,"$$CM+TY+LX+NC,0,-1$$");
  DocPrint(bdoc,"$$TX+RX+BD,\"[X]\"$$");
  DocPrint(bdoc,"$$BK,1$$$$TX+LX+BD,\"MENU\"$$$$BK,0$$");

  old_task_title=StrNew(Fs->task_title);
  if (Fs->title_src!=TTS_LOCKED_CONST) {
    Fs->title_src=TTS_ED_FILENAME;
    MemCpy(Fs->task_title,doc->filename.name,TASK_TITLE_LEN);
  }
  doc_e=DocPrint(bdoc,"$$DA-TRM-P+BD+RD+CX+IV,140,A=\"%%s...\",SCX=16$$");
  doc_e->data=&Fs->task_title;
  DocDataFmt(doc_e);

  if (doc->flags & DOCF_ALLOW_UNDO) {
    DocPrint(bdoc,"$$CM+BY+LX+NC,1,1$$");
    doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM-U64+U32,32,A=\"Undo:%%03d\"$$\n");
    doc_e->data=&doc->undo_cnt;
    DocDataFmt(doc_e);
  }

  DocPrint(bdoc,"$$CM+BY+RX+NC,-30,1$$");
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\"     \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdMoreCB;
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdDollarTypeCB;
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdFilterCB;
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdOverStrikeCB;
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdDollarCB;
  doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM,32,A=\"Line:%%04d \"$$");
  doc_e->data=&doc->line;
  DocDataFmt(doc_e);
  doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM,32,A=\"Col:%%04d\"$$\n");
  doc_e->data=&doc->col;
  DocDataFmt(doc_e);

  DocRecalc(bdoc);
  DocRecalc(doc);
  if (!(dof_flags&DOF_DONT_HOME))
    DocHome(doc);
  old_preempt=Preempt(OFF);
  Fs->border_doc=bdoc;
  if (doc!=old_display_doc)
    doc->parent_doc=old_display_doc;
  Fs->put_doc=Fs->display_doc=doc;
  if (!(dof_flags&DOF_DONT_TEXT_ATTR))
    Fs->text_attr=DOC_ATTR_DFT_TEXT;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (dof_flags&DOF_MIN_SIZE)
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
	DocDataScan(doc_e);
      doc_e=doc_e->next;
    }
  }
  if (unlock)
    DocUnlock(doc);
  Fs->border_doc =old_border_doc;
  Fs->display_doc=old_display_doc;
  Fs->put_doc	 =old_put_doc;
  Fs->text_attr  =old_attr;
  if (Fs->title_src!=TTS_LOCKED_CONST) {
    Fs->title_src  =old_title_src;
    StrCpy(Fs->task_title,old_task_title);
  }
  Free(old_task_title);
  DocDel(bdoc);
  if (dof_flags&DOF_MIN_SIZE) {
    SetWinHorz(old_left,old_right);
    SetWinVert(old_top,old_bottom);
  }
  Preempt(old_preempt);
  return result;
}


#help_index "Doc/Input"
public Bool DocFormDo(U8 *_d,U8 *class_name=lastclass,
    I64 dof_flags=0,U8 *header=NULL,U8 *footer=NULL)
//Supply a class name that has fmtstr definitions
//and this routine will allow user input.
{
  CMemberLst *ml;
  CDocEntry *doc_e;
  U8 *fmtstr;
  CHashClass *tempc;
  CDoc *doc;
  Bool result=FALSE;
  I64 old_border_src=Fs->border_src,has_action;
  if (!(tempc=HashFind(class_name,Fs->hash_table,HTT_CLASS)))
    return FALSE;
  doc=DocNew;
  StrCpy(doc->desc,"Form");
  if (header) DocPrint(doc,"%s",header);
  doc->flags|=DOCF_OVERSTRIKE|DOCF_FORM;
  if (dof_flags&DOF_MIN_SIZE)
    doc->flags|=DOCF_MIN_SIZE;
  ml=tempc->member_lst;
  while (ml) {
    if (fmtstr=MemberMetaData("fmtstr",ml)) {
      doc_e=DocPrint(doc,"%s",fmtstr);
      doc_e->data=_d+ml->offset;
      doc_e->my_fmt_data=MemberMetaData("fmtdata",ml);
      DocDataFmt(doc_e);
    }
    ml=ml->next;
  }
  if (footer) DocPrint(doc,"%s",footer);
  if (doc->root.next!=doc)	{
    Fs->border_src=BDS_CONST;
    DocRecalc(doc);
    if (DocDo(doc,dof_flags)) {
      doc_e=doc->cur_entry;
      result=TRUE;
      if (doc_e!=doc) {
	if (DocEntryRun(doc,doc_e,TRUE,&has_action)==DOCM_CANCEL && has_action)
	  result=FALSE;
	DocUnlock(doc);
      }
    }
  }
  DocDel(doc);
  Fs->border_src=old_border_src;
  return result;
}

U0 DocMenuDoEndTaskCB()
{
  WinToTop;
  throw;
}

public I64 DocMenuDo(CDoc *m,I64 dof_flags=0)
{ //returns m unlocked
  U8 *old_end_cb=Fs->task_end_cb;
  Bool old_break_shift_esc=LBts(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC);
  CDocEntry *doc_e;
  I64 old_border_src=Fs->border_src,result=DOCM_CANCEL,has_action;
  Fs->task_end_cb=&DocMenuDoEndTaskCB;
  try {
    if (m) {
      StrCpy(m->desc,"Menu");
      Fs->border_src=BDS_CONST;
md_restart:
      if (DocDo(m,dof_flags)) {
	doc_e=m->cur_entry;
	if (doc_e!=m) {
	  result=DocEntryRun(m,doc_e,TRUE,&has_action);
	  DocUnlock(m);
	  if (!has_action) {
	    result=DOCM_CANCEL;
	    dof_flags|=DOF_DONT_HOME;
	    goto md_restart;
	  }
	}
      }
    }
  } catch {
    if (!Fs->except_ch) {
      if (!(dof_flags & DOF_INTERCEPT_TASK_END))
	Exit;
      Fs->catch_except=TRUE;
    }
  }
  LBEqu(&Fs->task_flags,TASKf_BREAK_TO_SHIFT_ESC,old_break_shift_esc);
  Fs->border_src=old_border_src;
  Fs->task_end_cb=old_end_cb;
  return result;
}

#help_index "Doc"
public I64 PopUp(U8 *msg,CTask *parent=NULL,CTask **_pu_task=NULL)
{
  I64 result;
  CSrvCmd *tempc;
  Bool old_preempt=Preempt(OFF);
  CTask *task=Spawn(&SrvUserCmdLine,NULL,"System",,parent);
  if (!parent) {
    TaskExe(task,parent,msg,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
    if (_pu_task) *_pu_task=task;
    Preempt(old_preempt);
    return 0;
  } else {
    Fs->popup_task=task;
    tempc=TaskExe(task,parent,msg,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
    if (_pu_task) *_pu_task=task;
    ScanResult(tempc,&result);
    Fs->popup_task=NULL;
    Kill(task);
    if (_pu_task) *_pu_task=NULL;
    Preempt(old_preempt);
    return result;
  }
}

#help_index "Doc/Output"
public CTask *PopUpViewDoc(CDoc *doc,I64 dof_flags=0)
{
  U8 *buf=MPrint("Preempt(ON);DocDo(0x%X,0x%X);",doc,dof_flags);
  CTask *task=Spawn(&SrvUserCmdLine,NULL,"View",,Fs);
  TaskExe(task,NULL,buf,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
  Free(buf);
  return task;
}

U0 PopUpViewStrTaskEndCB()
{
  DocDel(FramePtr("ViewStrFrame"));
  Exit;
}

public CTask *PopUpViewStr(U8 *msg)
{
  CTask *task;
  CDoc *doc=DocNew;
  DocPrint(doc,msg);
  task=PopUpViewDoc(doc);
  FramePtrAdd("ViewStrFrame",doc,task);
  task->task_end_cb=&PopUpViewStrTaskEndCB;
  return task;
}

#help_index "Doc/Input"
public I64 PopUpMenu(CDoc *doc,I64 dof_flags=0)
{
  doc->flags|=DOCF_MIN_SIZE | DOCF_FORM;
  return SrvUser("DocMenuDo(0x%X,0x%X);",doc,dof_flags);
}

public Bool PopUpForm(U8 *_d,U8 *class_name=lastclass,I64 dof_flags=DOF_MIN_SIZE,
    U8 *header=NULL,U8 *footer=NULL)
{
  return SrvUser("DocFormDo(0x%X,0x%X,0x%X,0x%X,0x%X);",_d,class_name,
    dof_flags,header,footer);
}

#help_index "Doc/Input;File/FileNames"
public U8 *PopUpPickFile(U8 *dir=NULL)
{
  U8 *result,*st,*st2;
  if (dir)
    st=MPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",st2);
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
    st=MPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",st2);
    Free(st2);
  }
  result=PopUp(st,Fs);
  Free(st);
  return result;
}

public U8 *FileNameForm(U8 *dft=NULL,I64 dof_flags=0,CTask *mem_task=NULL)
{
  CEdFileName fn;
  if (dft)
    StrCpy(fn.name,dft);
  else
    *fn.name=0;
  if (DocFormDo(&fn,,dof_flags))
    return StrNew(fn.name,mem_task);
  else
    return NULL;
}

public U8 *PopUpFileName(U8 *dft=NULL,I64 dof_flags=0)
{
  U8 *st=MPrint("FileNameForm(\"%Q\",0x%X,Fs->parent_task);",dft,dof_flags|DOF_MIN_SIZE),
     *result=PopUp(st,Fs);
  Free(st);
  return result;
}

#help_index "Doc"
Bool PopUpCd()
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
public U8 *PopUpPickLst(U8 *lst)
{
  I64 i=0;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$LTBLUE$$");
  while (*lst) {
    if (*lst=='@') { //Check for '@' alias lst entry
      i--;
      lst++;
    }
    DocPrint(doc,"$$MU,\"%s\",LE=%d$$\n",lst,i++);
    lst+=StrLen(lst)+1;
  }
  DocPrint(doc,"\n$$MU,\"CANCEL\",LE=DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public U8 *PopUpPickDefineSub(U8 *st)
{
  return PopUpPickLst(Define(st));
}

public I64 PopUp1(U8 *b1,I64 n1,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i,l1=StrLen(b1);
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"$$CM+CX,%d,4$$$$BT,\"%s\",LE=%d$$\n",-l1/2,b1,n1);
  if (footer) DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 PopUp2(U8 *b1,I64 n1,U8 *b2,I64 n2,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i,l1=StrLen(b1),l2=StrLen(b2),y;
  CDoc *doc=DocNew;
  if (header) {
    DocPrint(doc,"%s",header);
    y=4;
  } else {
    DocPrint(doc,"%*s\n",l1+l2+10,"");
    y=3;
  }
  DocPrint(doc,"$$CM+CX,%d,%d$$$$BT,\"%s\",LE=%d$$",-(l1+l2+3)>>1,y,b1,n1);
  DocPrint(doc,"$$CM+CX,%d,0$$$$BT,\"%s\",LE=%d$$\n" ,-(l1+l2+3)>>1+l1+6,b2,n2);
  if (footer) DocPrint(doc,"%s",footer);
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

U8 *DoPopUpMGetS(U8 *header,CTask *mem_task)
{
  U8 *result,*st;
  if (header)
    "%s",header;
  st=MGetS(GETSF_WITH_NEW_LINE);
  result=StrNew(st,mem_task);
  Free(st);
  return result;
}

public U8 *PopUpMGetS(U8 *header=NULL)
{
  U8 *st=MPrint("DoPopUpMGetS(0x%X,0x%X);",header,Fs),
     *result=PopUp(st,Fs);
  Free(st);
  return result;
}

public I64 PopUpI64(U8 *msg,I64 dft,I64 lo=MIN_I64,I64 hi=MAX_I64)
{
  U8 *st=MPrint("PmtI64(0x%X,0x%X,0x%X,0x%X);",msg,dft,lo,hi);
  I64 result=PopUp(st,Fs);
  Free(st);
  return result;
}

public I64 PopUpRangeI64(I64 lo,I64 hi,I64 step=1,U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  DocPrint(doc,"$$LTBLUE$$");
  for (i=lo;i<=hi;i+=step)
    DocPrint(doc,"$$MU,\"%d\",LE=%d$$\n",i,i);
  if (footer)
    DocPrint(doc,"%s",footer);
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
    DocPrint(doc,"%s",header);
  DocPrint(doc,"$$LTBLUE$$");
  for (d=lo;d<=hi;d+=step) {
    SPrint(buf,fmt,d);
    DocPrint(doc,"$$MU,\"%s\",LE=0x%X$$\n",buf,d);
  }
  if (footer)
    DocPrint(doc,"%s",footer);
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
    DocPrint(doc,"%s",header);
  DocPrint(doc,"$$LTBLUE$$");
  for (d=lo;d<=hi;d*=factor) {
    SPrint(buf,fmt,d);
    DocPrint(doc,"$$MU,\"%s\",LE=0x%X$$\n",buf,d);
  }
  if (footer)
    DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i(F64);
}

public F64 PopUpRangeF64Log(F64 lo,F64 hi,I64 steps,
    U8 *fmt="%9.4f",U8 *header=NULL,U8 *footer=NULL)
{
  return PopUpRangeF64Exp(lo,hi,Exp(Ln(hi/lo)/(steps-1)),fmt,header,footer);
}

#help_index "Task/Srv"
public I64 AdamFile(U8 *filename,Bool warn_ext=TRUE)
{
  Bool okay=TRUE;
  U8 *name=FileNameAbs(filename),
     *name2=DftExt(name,"CPP.Z");
  I64 result=0;
  if (warn_ext &&
      !FilesFindMatch(name2,FILEMASK_JIT) &&
      !PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n"))
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
      !FilesFindMatch(name2,FILEMASK_JIT) &&
      !PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n"))
    okay=FALSE;
  if (okay) {
    st=MPrint("\"$$$$WW+H,1$$$$\";Auto(\"ExeFile2(\\\"%s\\\");\\n\");UserTaskCont;",name2);
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
    DocPrint(doc,"%s",header);
  if (allow_transparent)
    DocPrint(doc,"$$MU,\"TRANSPARENT\",LE=COLOR_TRANSPARENT$$\n");
  for (i=0;i<16;i++)
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-8Z__\",LE=%d$$$$BG$$\n",i,i,"ST_COLORS",i);
  if (allow_dft)
    DocPrint(doc,"$$BG,WHITE$$$$MU,\"DEFAULT\",LE=DOC_DFT$$$$BG$$");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

public I64 PopUpColorLighting(U8 *header=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  DocPrint(doc,"\nSingle-Sided means both sides same color."
	      "\nDouble-Sided means back side darker."
	      "\n"
	      "\nHalf-Range means 0 to [0,7]"
	      "\n                 [8,15]-8 to [8,15]"
	      "\nFull-Range means 0 to [0,7] to [0,7]+8"
	      "\n");
  DocPrint(doc,"\n$$LTPURPLE$$Single-Sided Full-Range Double-Sided Full-Range\n$$BLACK$$");
  for (i=0;i<8;i++) {
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$ ",i,i,"ST_COLORS",i);
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$\n",i,i,"ST_COLORS",ROPF_TWO_SIDED+i);
  }
  DocPrint(doc,"\n$$LTPURPLE$$Single-Sided Half-Range Double-Sided Half-Range\n$$BLACK$$");
  for (i=0;i<16;i++) {
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$ ",i,i,"ST_COLORS",ROPF_HALF_RANGE_COLOR+i);
    DocPrint(doc,"$$BG,%d$$$$MU,\"__%-19Z__\",LE=%d$$$$BG$$\n",i,i,"ST_COLORS",ROPF_TWO_SIDED+ROPF_HALF_RANGE_COLOR+i);
  }
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CTransformForm
{
  F64 x_scale	fmtstr "$$DA-TRM,32,A=\"X Scale :%12.6f\"$$\n";
  F64 y_scale	fmtstr "$$DA-TRM,32,A=\"Y Scale :%12.6f\"$$\n";
  F64 z_scale	fmtstr "$$DA-TRM,32,A=\"Z Scale :%12.6f\"$$\n";
  F64 é_z	fmtstr "\nAngles are in degrees\n$$DA-TRM,32,A=\"Z-Axis  :%12.6f\"$$\n";
  F64 é_y	fmtstr "$$DA-TRM,32,A=\"Y-Axis  :%12.6f\"$$\n";
  F64 é_x	fmtstr "$$DA-TRM,32,A=\"X-Axis  :%12.6f\"$$\n";
  F64 x_offset	fmtstr "\n$$DA-TRM,32,A=\"X Offset:%12.6f\"$$\n";
  F64 y_offset	fmtstr "$$DA-TRM,32,A=\"Y Offset:%12.6f\"$$\n";
  F64 z_offset	fmtstr "$$DA-TRM,32,A=\"Z Offset:%12.6f\"$$\n";
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
  if (PopUpForm(tf)) {
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
  CDoc *doc=DocPut(task);
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
      GrRect(dc,i*PU_SPACING,j*PU_SPACING+FramePtr("DitherDoFrame",task),PU_SPACING,PU_SPACING);
    }
  i=(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH;
  if (doc->max_x>i) i=doc->max_x;
  SetWinHorz(task->win_left,task->win_left+i,task);
  SetWinVert(task->win_top,task->win_top+doc->max_y+(17*PU_SPACING+FONT_WIDTH-1)/FONT_WIDTH,task);
}

I64 DitherColorDo(U8 *header=NULL)
{
  I64 msg_code,p1,p2,result=-1,i,j;
  DocClear;
  "%s",header;
  DocRecalc(DocPut);
  FramePtrAdd("DitherDoFrame",(DocPut->max_y+1)*FONT_HEIGHT);
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_DOWN|1<<MSG_KEY_DOWN);
    if (msg_code==MSG_IP_L_DOWN) {
      msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP);
      i=p1/PU_SPACING;
      j=(p2-FramePtr("DitherDoFrame"))/PU_SPACING;
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
  FramePtrDel("DitherDoFrame");
  return result;
}

public I64 PopUpColorDither(U8 *header=NULL)
{
  U8 buf[136];
  SPrint(buf,"DitherColorDo(0x%X);",header);
  return PopUp(buf,Fs);
}
