#define EF_DONT_RECOMPILE	0
#define EF_RECOMPILE		1
#define EF_RENUM_ASM		2
#define EF_SLIDER_CTRL		3
#define EF_CH_SC		4

I64 PopUpEdFmt()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPutS(doc,"$$FG,PURPLE$$Reformat Function$$FG$$\n"
	"If you enter $$FG,GREEN$$DONT RECOMPILE$$FG$$ the stored\n"
	"source line indentation information will\n"
	"be used.  $$BK,1$$Be sure it is up-to-date!$$BK,0$$\n"
	"For $$FG,GREEN$$#exe$$FG$$ config'ed projects, enter $$FG,GREEN$$DONT RECOMPILE$$FG$$.\n"
	"$$MU,\"Recompile       and autoindent CPP.Z Fun\",EF_RECOMPILE$$\n"
	"$$MU,\"Don't Recompile but autoindent CPP.Z Fun\",EF_DONT_RECOMPILE$$\n\n"
	"$$MU,\"Renum Asm Local @@ Labels for Fun\",EF_RENUM_ASM$$\n\n"
	"$$FG,PURPLE$$Insert Managed Code$$FG$$\n"
	"$$MU,\"Slider Ctrl\",EF_SLIDER_CTRL$$\n\n"
	"$$FG,PURPLE$$Misc$$FG$$\n"
	"$$MU,\"Insert ASCII/Scan Code Hex Codes for key pressed\",EF_CH_SC$$\n\n"
	"$$MU,\"CANCEL\",DOCM_CANCEL$$\n"
	"$$FG,GREEN$$<ALT-BACKSPACE>$$FG$$ to undo if not happy\n"
	"with the results.\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}


#define C_INDENT_SPACES		2
#define ASM_RENUM_SPACING	5

U0 EdRemFunLeadingSpace(CDoc *doc)
{
  Bool unlock=DocLock(doc),
       start_of_line=TRUE;
  U8 ch,*ptr;
  I64 levels=1;
  CDocEntry *doc_e,*doc_e2;

  EdGoToFun(doc,FALSE,FALSE);
  doc_e=doc->cur_entry->next;
  do {
    doc_e2=doc_e->next;
    if (doc_e!=doc && doc_e!=doc->cur_entry &&
      !(doc_e->flags1&(DOCEG1_DONT_EDIT-DOCEF1_SCROLLING_X)) &&
      !(doc_e->flags2&DOCEG2_DONT_EDIT))
      switch (doc_e->type_u8) {
	case DOCT_TEXT:
	  ptr=doc_e->tag;
	  if (start_of_line) {
	    while (*ptr==CH_SPACE)
	      ptr++;
	    if (*ptr)
	      start_of_line=FALSE;
	    ptr=StrNew(ptr,doc->mem_task);
	    Free(doc_e->tag);
	    doc_e->tag=ptr;
	  }
	  if (!*ptr)
	    DocEntryDel(doc,doc_e);
	  else {
	    while (ch=*ptr++)
	      if (ch=='{')
		levels++;
	      else if (ch=='}') {
		if (!--levels)
		  break;
	      }
	    if (!levels) goto ls_done;
	  }
	  break;
	case DOCT_TAB:
	  if (start_of_line)
	    DocEntryDel(doc,doc_e);
	  break;
	case DOCT_NEW_LINE:
	  start_of_line=TRUE;
	  break;
	default:
	  start_of_line=FALSE;
      }
    doc_e=doc_e2;
  } while (doc_e!=doc->cur_entry);
ls_done:
  DocRecalc(doc);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
}


class CRenum
{
  CRenum *next,*last;
  U8 label[sizeof(CEdFindText.find_text)];
};

I64 EdRAGetU8(CDoc *doc)
{
  I64 result=-1;
  while (doc->cur_entry!=doc && doc->cur_entry->type&DOCET_SELECTED && result<0) {
    result=EdCurU8(doc);
    EdCursorRight(doc);
  }
  return result;
}

U0 EdRACollect(CDoc *doc,CRenum *root)
{
  I64 ch,i;
  CRenum *tempr;
  U8 buf[sizeof(CEdFindText.find_text)];
  ch=EdRAGetU8(doc);
  while (ch>=0) {
    if (ch!='@')
      ch=EdRAGetU8(doc);
    else {
      ch=EdRAGetU8(doc);
      if (ch=='@') {
	ch=EdRAGetU8(doc);
	StrCpy(buf,"@@");
	i=2;
	while (ch>=0 && i<sizeof(CEdFindText.find_text)) {
	  if (Bt(alpha_numeric_bitmap,ch))
	    buf[i++]=ch;
	  else
	    break;
	  ch=EdRAGetU8(doc);
	}
	if (i<sizeof(CEdFindText.find_text)) {
	  buf[i++]=0;
	  while (ch>=0 && Bt(white_space_bitmap,ch))
	    ch=EdRAGetU8(doc);
	  if (ch==':') {
	    ch=EdRAGetU8(doc);
	    tempr=MAlloc(sizeof(CRenum));
	    StrCpy(tempr->label,buf);
	    QueIns(tempr,root->last);
	  }
	}
      }
    }
  }
  //This is needed because we moved the
  //cursor and it didn't recalc.
  DocRecalc(doc);
}

U0 EdRenumAsm(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  I64 num=0;
  CRenum root,*tempr,*tempr1;
  U8 buf[sizeof(CEdFindText.find_text)],
  buf2[sizeof(CEdFindText.find_text)];

  QueInit(&root);
  EdSelectFun(doc,TRUE);
  EdRACollect(doc,&root);

  tempr=root.next;
  while (tempr!=&root) {
    tempr1=tempr->next;
    num+=ASM_RENUM_SPACING;
    SPrintF(buf,"@#%02d",num);
    EdReplace(doc,tempr->label,buf,TRUE,TRUE,TRUE);
    Free(tempr);
    tempr=tempr1;
  }

  while (num) {
    SPrintF(buf, "@#%02d",num);
    SPrintF(buf2,"@@%02d",num);
    EdReplace(doc,buf,buf2,TRUE,TRUE,TRUE);
    num-=ASM_RENUM_SPACING;
  }
  EdSelectAll(doc,FALSE);
  DocRecalc(doc);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
}

#define AC_ABORT	-1
#define AC_REINDENT	0
#define AC_RENUM_ASM	1

U0 EdRefmtFun(CDoc *doc,I64 fmt_type)
{
  Bool unlock=DocLock(doc),start_of_line=TRUE;
  CDocEntry *doc_e,*doc_ne;
  I64 i,x,r,action=AC_ABORT,old_line_num;
  U8 *b,*st,*st2,*prj_file;
  CHashSrcSym *temph;
  CIndentInfo *indent;
  CTask *task=NULL;
  CTaskCmd *tempc;

  DocRecalc(doc);
  old_line_num=doc->cur_entry->y+1;

  DocCaptureUndo(doc,TRUE);
  switch (fmt_type) {
    case EF_RECOMPILE:
      if (EdGoToFun(doc,FALSE,FALSE)) {
	if (doc->flags&DOCF_PLAIN_TEXT)
	  DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
	DocWrite(doc);
	task=Spawn(&ServantUserCmdLine,NULL,"Servant",Fs);
	st2=CurDir;
	st=MSPrintF("Cd(\"%s\");",st2);
	tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	Free(st2);
	Free(st);

	task->win_top=Fs->win_top;
	task->win_bottom=Fs->win_bottom;
	task->win_left=Fs->win_left;
	task->win_right=Fs->win_right;
	TaskDerivedValsUpdate(task);

	if (GetRqstResult(tempc,&r)) {
	  st=DirFile(doc->filename.name,,"PRJ.Z"),
	  prj_file=ZorNotZ(st);
	  Free(st);
	  if (FileFind(prj_file)) {
	    st2=DirFile(prj_file),
	    st=MSPrintF("Cd(\"%s\");",st2);
	    Free(st2);
	    tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	    Free(st);
	    st=MSPrintF("\"$$WW,1$$\";Cmp(\"%s\",\"SysTemp\",\"SysTemp\");",prj_file);
	    tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	    Free(st);
	    if (GetRqstResult(tempc,&r))
	      if (!r) {
		tempc=TaskRqstQue(task,Fs,
		"Load(\"SysTemp\",LDF_JUST_LOAD);"
		    "MapFileLoad(\"SysTemp\");",1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
		if (GetRqstResult(tempc,&r))
		  action=AC_REINDENT;
	      }
	    tempc=TaskRqstQue(task,Fs,"Del(\"SysTemp.*\");",1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	    GetRqstResult(tempc,&r);
	  } else {
	    Free(prj_file);
	    st=DirFile(doc->filename.name,"Load","CPP.Z");
	    prj_file=ZorNotZ(st);
	    Free(st);
	    if (FileFind(prj_file))
	      st=MSPrintF("\"$$WW,1$$\";ExeFile(\"%s\",TRUE);",prj_file);
	    else
	      st=MSPrintF("\"$$WW,1$$\";ExeFile(\"%s\",TRUE);",doc->filename.name);
	    tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	    Free(st);
	    if (GetRqstResult(tempc,&r) && r)
	      action=AC_REINDENT;
	  }
	  Free(prj_file);
	}
	if (action==AC_REINDENT)	{
	  DocUnlock(doc);
	  st=MSPrintF("DbgFunFind(0x%X,0);",doc);
	  tempc=TaskRqstQue(task,Fs,st,1<<TASKCf_WAKE_MASTER|1<<TASKCf_FOCUS_MASTER);
	  Free(st);
	  if (GetRqstResult(tempc,&r))
	    temph=r;
	  else
	    action=AC_ABORT;
	  DocLock(doc);
	} else
	  PopUpOk("Has Errors");
      }
      break;
    case EF_DONT_RECOMPILE:
      if (EdGoToFun(doc,FALSE,FALSE)) {
	if (temph=DbgFunFind(doc,0))
	  action=AC_REINDENT;
      }
      break;
    case EF_RENUM_ASM:
      if (EdGoToFun(doc,FALSE,TRUE)) {
	if (EdCurU8(doc)=='{') {
	  EdCursorRight(doc);
	  DocRecalc(doc);
	} else if (EdCurU8(doc)==':') {
	  EdCursorRight(doc);
	  if (EdCurU8(doc)==':')
	    EdCursorRight(doc);
	  DocRecalc(doc);
	}
	action=AC_RENUM_ASM;
      }
      break;
  }

  switch (action) {
    case AC_RENUM_ASM:
      DocUnlock(doc);
      Snd(2000); Sleep(150); Snd(0);
      Sleep(100);
      Snd(2000); Sleep(150); Snd(0);
      DocLock(doc);
      EdRenumAsm(doc);
      break;
    case AC_REINDENT:
      if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM))
	indent=temph->indent;
      else
	indent=NULL;
      if (indent && indent->min_line<=indent->max_line) {
	DocUnlock(doc);
	Snd(2000); Sleep(150); Snd(0);
	Sleep(100);
	Snd(2000); Sleep(150); Snd(0);
	DocLock(doc);
	EdRemFunLeadingSpace(doc);
	DocLineNumGoTo(doc,indent->min_line);
	doc_e=doc->cur_entry;
	while (indent->min_line<=doc_e->y+1<=indent->max_line) {
	  if (doc_e!=doc && doc_e!=doc->cur_entry &&
	    !(doc_e->flags1&(DOCEG1_DONT_EDIT-DOCEF1_SCROLLING_X)) &&
	    !(doc_e->flags2&DOCEG2_DONT_EDIT)) {
	    if (doc_e->type_u8==DOCT_NEW_LINE||doc_e->type_u8==DOCT_SOFT_NEW_LINE)
	      start_of_line=TRUE;
	    else {
	      if (start_of_line) {
		i=indent->body[doc_e->y+1-indent->min_line]*C_INDENT_SPACES;
		x=doc_e->x+1;
		while (i>8) {
		  doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_ne->type=DOCT_TAB | doc->settings_root.dft_text_attr << 8;
		  doc_ne->x=x;
		  doc_ne->y=doc_e->y;
		  doc_ne->page_line_num=doc_e->page_line_num;
		  QueIns(doc_ne,doc_e->last);
		  i-=8;
		  x+=8;
		}
		if (i>0) {
		  b=MAlloc(i+1,doc->mem_task);
		  MemSet(b,CH_SPACE,i);
		  b[i]=0;
		  doc_ne=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
		  doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
		  doc_ne->tag=b;
		  doc_ne->max_col=1;
		  doc_ne->x=x;
		  doc_ne->y=doc_e->y;
		  doc_ne->page_line_num=doc_e->page_line_num;
		  QueIns(doc_ne,doc_e->last);
		}
	      }
	      start_of_line=FALSE;
	    }
	  }
	  doc_e=doc_e->next;
	}
	break;
      }
  }
  DocRecalc(doc);
  DocLineNumGoTo(doc,old_line_num);

  DocUnlock(doc);
  if (!unlock)
    DocLock(doc);
  if (task)
    Kill(task,FALSE);
}

U0 EdPopUpChSC(I64 *_ch,I64 *_sc)
{
  I64 sc;
  "Press A Key\n";
  DocCur->flags|=DOCF_MIN_SIZE;
  do GetMsg(_ch,&sc,1<<MSG_KEY_DOWN);
  while (sc.u8[0]==SC_SHIFT || sc.u8[0]==SC_CTRL || sc.u8[0]==SC_ALT);
  *_sc=sc;
}

U0 EdChSC(CDoc *doc)
{
  I64 ch,sc;
  U8 buf[136];
  SPrintF(buf,"EdPopUpChSC(%d,%d);",&ch,&sc);
  PopUp(buf,Fs);
  if (ch==CH_BACKSPACE)
    DocPrintF(doc,"CH_BACKSPACE,0x%X",sc);
  else if (ch==CH_NEW_LINE)
    DocPrintF(doc,"CH_NEW_LINE,0x%X",sc);
  else if (CH_CTRLA<=ch<=CH_CTRLZ)
    DocPrintF(doc,"CH_CTRL%C,0x%X",ch+'@',sc);
  else if (ch=='$$')
    DocPrintF(doc,"'$$$$',0x%X",sc);
  else if (ch=='\\')
    DocPrintF(doc,"'\\\\',0x%X",sc);
  else if (ch=='\'')
    DocPrintF(doc,"'\\\'',0x%X",sc);
  else if (ch==CH_ESC)
    DocPrintF(doc,"CH_ESC,0x%X",sc);
  else if (ch==CH_SHIFT_ESC)
    DocPrintF(doc,"CH_SHIFT_ESC,0x%X",sc);
  else if (ch==CH_SPACE)
    DocPrintF(doc,"CH_SPACE,0x%X",sc);
  else if (ch==CH_SHIFT_SPACE)
    DocPrintF(doc,"CH_SHIFT_SPACE,0x%X",sc);
  else if (Bt(displayable_chars_bitmap,ch))
    DocPrintF(doc,"'%c',0x%X",ch,sc);
  else
    DocPrintF(doc,"0x%X,0x%X",ch,sc);
}

U0 EdCodeTools(CDoc *doc)
{
  I64 fmt_type=PopUpEdFmt;
  switch (fmt_type) {
    case EF_DONT_RECOMPILE:
    case EF_RECOMPILE:
    case EF_RENUM_ASM:
      EdRefmtFun(doc,fmt_type);
      break;
    case EF_SLIDER_CTRL:
      MgdSliderCtrl(doc);
      break;
    case EF_CH_SC:
      EdChSC(doc);
      break;
  }
}
