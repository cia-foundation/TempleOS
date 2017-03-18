#help_index "DolDoc/Output;StdOut/DolDoc"

public Bool View()
{//Go live for user interaction until <ESC> or <SHIFT-ESC>.
  I64 ch;
  do ch=DocGetKey;
  while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  return ch==CH_ESC;
}

#help_index "DolDoc"
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

U8 *EdAutoSaveCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{
  CDoc *doc=doc_e->user_data;
  U8 *st=MAlloc(8,mem_task);
  if (doc->flags & DOCF_AUTO_SAVE)
    *st='S';
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
  StrPrint(st,"%-3ts",src);
  return st;
}

public Bool DocEd(CDoc *doc,I64 dof_flags=0)
{//Live for user interaction. End on <ESC> or <SHIFT-ESC>.
  CDoc *old_put_doc	  =DocPut,
	*old_display_doc=DocDisplay,
	*old_border_doc =DocBorder,*bdoc;
  CDocEntry *doc_e;
  I64 old_attr=Fs->text_attr,
	old_top =Fs->win_top, old_bottom=Fs->win_bottom,
	old_left=Fs->win_left,old_right =Fs->win_right,
	old_title_src=Fs->title_src;
  Bool res,unlock;
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
    MemCpy(Fs->task_title,doc->filename.name,STR_LEN-1);
  }
  doc_e=DocPrint(bdoc,"$$DA-TRM-P+BD+RD+CX+IV,LEN=STR_LEN-1,"
	"A=\"%%s...\",SCX=16$$");
  doc_e->data=&Fs->task_title;
  DocDataFmt(bdoc,doc_e);

  if (doc->flags & DOCF_ALLOW_UNDO) {
    DocPrint(bdoc,"$$CM+BY+LX+NC,1,1$$");
    doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM,RT=U32,A=\"Undo:%%03d\"$$\n");
    doc_e->data=&doc->undo_cnt;
    DocDataFmt(bdoc,doc_e);
  }

  DocPrint(bdoc,"$$CM+BY+RX+NC,-31,1$$");
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
  doc_e->tag_cb=&EdAutoSaveCB;
  doc_e=DocPrint(bdoc,"$$TX+BD+TC,\" \"$$");
  doc_e->user_data=doc;
  doc_e->tag_cb=&EdDollarCB;
  doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM,A=\"Line:%%04d \"$$");
  doc_e->data=&doc->line;
  DocDataFmt(bdoc,doc_e);
  doc_e=DocPrint(bdoc,"$$DA+BD+RD-TRM,A=\"Col:%%04d\"$$\n");
  doc_e->data=&doc->col;
  DocDataFmt(bdoc,doc_e);

  DocRecalc(bdoc);
  DocRecalc(doc);
  if (!(dof_flags&DOF_DONT_HOME))
    DocTop(doc);
  Fs->border_doc=bdoc;
  if (doc!=old_display_doc)
    doc->parent_doc=old_display_doc;
  Fs->put_doc=Fs->display_doc=doc;
  if (!(dof_flags&DOF_DONT_TEXT_ATTR))
    Fs->text_attr=DOC_ATTR_DFT_TEXT;
  if (!(dof_flags&DOF_DONT_SHOW)) {
    LBts(&Fs->display_flags,DISPLAYf_SHOW);
    WinZBufUpdate;
  }
  if (dof_flags&DOF_SIZE_MIN)
    doc->flags|=DOCF_SIZE_MIN;

  DocUnlock(doc);
  if (!(dof_flags&DOF_DONT_WINMGR_SYNC)) {
    Refresh(2,TRUE);
    if (doc->flags&DOCF_SIZE_MIN)
      Refresh(2,TRUE);
  }
  res=View;

  DocLock(doc);
  if (res) {
    doc_e=doc->head.next;
    while (doc_e!=doc) {
      if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_CHECK_BOX)
	DocDataScan(doc,doc_e);
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
  if (dof_flags&DOF_SIZE_MIN) {
    WinHorz(old_left,old_right);
    WinVert(old_top,old_bottom);
  }
  return res;
}

#help_index "DolDoc/Cmd Line (Typically);Cmd Line (Typically)"
public Bool Ed(U8 *link_st,I64 edf_dof_flags=0)
{//Invoke document editor.
  U8 *filename,*needle_str;
  I64 i,num;
  Bool cont,res=FALSE;
  CDoc *doc;

  switch (i=EdLinkCvt(link_st,&filename,&needle_str,&num,edf_dof_flags)) {
    case -1:
      break;
    case LK_DEF:
      doc=DocNew;
      doc->desc='DictDef';
      ACDDefsPut(doc,filename,num);
      goto ej_doc;
    case LK_HELP_INDEX:
      doc=DocNew;
      doc->desc='HelpIndx';
      DocHelpIdx(doc,filename);
ej_doc:
      if (!(edf_dof_flags&EDF_BAIL)) {
	DocEd(doc);
	DocDel(doc);
      }
      if (!(edf_dof_flags&EDF_WAS_WRITE))
	res=TRUE;
      break;
    default:
      if (IsRaw)
	res=EdLite(filename,num,edf_dof_flags);
      else {
	cont=TRUE;
	if (!(edf_dof_flags&EDF_BAIL) && !(LK_DOC<=i<=LK_DOC_LINE) &&
	      !FilesFindMatch(filename,FILEMASK_TXT) &&
	      !PopUpCancelOk(ST_WARN_ST "Not Text File\n\n"))
	  cont=FALSE;
	if (cont)
	  res=DocFileEd(i,filename,needle_str,&num,edf_dof_flags);
      }
  }
  Free(filename);
  Free(needle_str);
  return res;
}

public Bool Plain(U8 *filename,I64 edf_dof_flags=0)
{//Edit document in plain text mode, so dollar signs are not special.
  Bool res;
  U8 *st=MStrPrint("PI:%s",filename);
  res=Ed(st,edf_dof_flags);
  Free(st);
  return res;
}

#help_index "DolDoc;Job/Exe;Task/Job/Exe"
public I64 PopUpEd(U8 *filename,CTask *parent=NULL,CTask **_pu_task=NULL)
{//Create PopUp win task and edit a doc.
  U8 *st=MStrPrint("Ed(\"%Q\");",filename);
  I64 res=PopUp(st,parent,_pu_task);
  Free(st);
  return res;
}
