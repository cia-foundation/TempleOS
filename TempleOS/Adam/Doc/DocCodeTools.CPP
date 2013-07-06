#define C_INDENT_SPACES		2
#define ASM_RENUM_SPACING	5

#define EF_REINDENT	0
#define EF_CHK_COMPILE	1
#define EF_RENUM_ASM	2
#define EF_SLIDER_CTRL	3
#define EF_CH_SC	4

I64 PopUpEdFmt()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPutS(doc,"$$FG,LTBLUE$$$$MU,\"Check Compile\",EF_CHK_COMPILE$$\n"
	"$$MU,\"Reindent CPP.Z Fun\",EF_REINDENT$$\n"
	"$$MU,\"Renum Asm Local @@ Labels for Fun\",EF_RENUM_ASM$$\n"
	"$$MU,\"Insert Mgd Code: Slider Ctrl\",EF_SLIDER_CTRL$$\n"
	"$$MU,\"Insert ASCII/Scan Code Hex Codes for key pressed\",EF_CH_SC$$\n"
	"$$MU,\"CANCEL\",DOCM_CANCEL$$\n\n"
	"$$FG,GREEN$$<ALT-BACKSPACE>$$FG$$ to undo if not happy\n"
	"with the results.\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CRILex
{
  CLex *lx1,*lx2;
  CQueVectU8 *indent;
  I64 depth,exp_depth;
  Bool was_new_line,is_not_cont,one_shot;
};

I64 EdRILex(CRILex *rx)
{
  Bool is_not_cont=FALSE;
  I64 i;
  do {
    Lex(rx->lx1);
    Lex(rx->lx2);

    if (rx->lx1->token=='\n') {
      if (rx->lx2->token==TK_IDENT)
	i=PrsKeyWord(rx->lx2);
      else
	i=-1;
      if (rx->lx2->token==';' || rx->lx2->token=='{' || rx->lx2->token=='}' ||
	   rx->lx2->token==':' || rx->lx2->token==')' && !rx->exp_depth ||
	   i==KW_ELSE || i==KW_CATCH || i==KW_DO)
	is_not_cont=TRUE;
    }

    if (rx->was_new_line && (rx->lx1->token!=':' || rx->lx2->token!=TK_IDENT ||
	  PrsKeyWord(rx->lx2)==KW_DFT)) {
      if (rx->one_shot) {
	QueVectU8Put(rx->indent,rx->lx2->lfns[0].cur_entry->y,rx->depth+3);
	rx->one_shot=FALSE;
      } else
	QueVectU8Put(rx->indent,rx->lx2->lfns[0].cur_entry->y,rx->depth);
    }
    if (rx->lx2->token=='\n')
      rx->was_new_line=TRUE;
    else
      rx->was_new_line=FALSE;
  } while (rx->lx1->token=='\n');
  rx->is_not_cont=is_not_cont;
  return rx->lx1->token;
}

U0 EdRIExp(CRILex *rx)
{
  if (rx->lx1->token=='(') {
    if (!rx->exp_depth++)
      rx->depth+=3;
    EdRILex(rx);
    while (rx->lx1->token && rx->lx1->token!=')')
      EdRIExp(rx);
    if (!--rx->exp_depth)
      rx->depth-=3;
  }
  EdRILex(rx);
}

U0 EdRIStmt(CRILex *rx,Bool indent)
{
  I64 i;
  Bool cont;
  if (rx->lx1->token=='{') {
    rx->depth++;
    EdRILex(rx);
    while (rx->lx1->token && rx->lx1->token!='}')
      EdRIStmt(rx,FALSE);
    rx->depth--;
    EdRILex(rx);
  } else {
    if (indent) rx->depth++;
    do {
      cont=FALSE;
      switch (PrsKeyWord(rx->lx1)) {
	case KW_IF:
	  EdRILex(rx);
	  EdRIExp(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->lx1)==KW_ELSE) {
	    EdRILex(rx);
	    if (PrsKeyWord(rx->lx1)==KW_IF && rx->lx2->token!='\n')
	      EdRIStmt(rx,FALSE);
	    else
	      EdRIStmt(rx,TRUE);
	  }
	  break;
	case KW_TRY:
	  EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->lx1)==KW_CATCH) {
	    EdRILex(rx);
	    EdRIStmt(rx,TRUE);
	  }
	  break;
	case KW_FOR:
	  case KW_WHILE:
	  EdRILex(rx);
	  EdRIExp(rx);
	  EdRIStmt(rx,TRUE);
	  break;
	case KW_DO:
	  EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->lx1)==KW_WHILE) {
	    EdRILex(rx);
	    EdRIExp(rx);
	  }
	  if (rx->lx1->token==';')
	    EdRILex(rx);
	  break;
	case KW_SWITCH:
	  case KW_NOBOUND_SWITCH:
	  EdRILex(rx);
	  EdRIExp(rx);
	  if (rx->lx1->token=='{') {
	    rx->depth++;
	    EdRILex(rx);
	    while (rx->lx1->token && rx->lx1->token!='}') {
	      while (TRUE) {
		i=PrsKeyWord(rx->lx1);
		if (i==KW_CASE || i==KW_DFT) {
		  EdRILex(rx);
		  while (rx->lx1->token && rx->lx1->token!=':')
		    EdRILex(rx);
		  EdRILex(rx);
		} else
		  break;
	      }
	      if (rx->lx1->token)
		EdRIStmt(rx,TRUE);
	    }
	    rx->depth--;
	    EdRILex(rx);
	  }
	  break;
	default:
	  while (rx->lx1->token && rx->lx1->token!=';' && rx->lx1->token!=':') {
	    if (rx->lx2->token=='\n' && !rx->is_not_cont)
	      rx->one_shot=TRUE;
	    EdRILex(rx);
	  }
	  if (rx->lx1->token==':')
	    cont=TRUE;
	  EdRILex(rx);
      }
    } while (cont);
    if (indent) rx->depth--;
  }
}

CQueVectU8 *EdRICode(CDoc *doc)
{
  CQueVectU8 *result;
  CRILex *rx=CAlloc(sizeof(CRILex));

  rx->lx1=LexNew(NULL,LF_KEEP_NEW_LINES|LF_DONT_FREE_BUF,doc->filename.name);
  Free(rx->lx1->lfns[0].name);
  LexAttachDoc(rx->lx1,&rx->lx1->lfns[0],doc,,doc->cur_entry,doc->cur_data_col);

  rx->lx2=LexNew(NULL,LF_KEEP_NEW_LINES|LF_DONT_FREE_BUF,doc->filename.name);
  Free(rx->lx2->lfns[0].name);
  LexAttachDoc(rx->lx2,&rx->lx2->lfns[0],doc,,doc->cur_entry,doc->cur_data_col);

  rx->indent=QueVectU8New(doc->cur_entry->y);

  Lex(rx->lx1);
  EdRIStmt(rx,FALSE);

  LexDel(rx->lx1);
  LexDel(rx->lx2);
  result=rx->indent;
  Free(rx);
  return result;
}

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

U0 EdCodeTools2(CDoc *doc,I64 tool_action)
{
  Bool okay,unlock=DocLock(doc),start_of_line=TRUE;
  CDocEntry *doc_e,*doc_ne;
  I64 i,start_y,end_y,x,r,goto_line_num;
  U8 *b,*st,*st2,*prj_file;
  CTask *task=NULL;
  CSrvCmd *tempc;
  CQueVectU8 *indent;

  DocRecalc(doc);
  goto_line_num=doc->cur_entry->y+1;

  DocCaptureUndo(doc,TRUE);
  switch (tool_action) {
    case EF_CHK_COMPILE:
      okay=FALSE;
      if (doc->flags&DOCF_PLAIN_TEXT)
	DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
      DocWrite(doc);
      task=Spawn(&SrvUserCmdLine,NULL,"Srv",,Fs);
      st2=CurDir;
      st=MSPrintF("Cd(\"%s\");",st2);
      tempc=TaskExeStrQue(task,Fs,st,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
      Free(st2);
      Free(st);
      SetWinHorz(Fs->win_left,Fs->win_right, task);
      SetWinVert(Fs->win_top, Fs->win_bottom,task);
      if (ScanResult(tempc,&r)) {
	st=DirFile(doc->filename.name,,"PRJ.Z"),
	      prj_file=FileNameAbs(st,FUF_Z_OR_NOT_Z);
	Free(st);
	if (FileFind(prj_file)) {
	  st2=DirFile(prj_file),
	  st=MSPrintF("Cd(\"%s\");",st2);
	  Free(st2);
	  tempc=TaskExeStrQue(task,Fs,st,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER|1<<SVCf_FREE_ON_COMPLETE);
	  Free(st);
	  st=MSPrintF("\"$$WW,1$$\";Cmp(\"%s\",\"SysTemp\",\"SysTemp\");",prj_file);
	  tempc=TaskExeStrQue(task,Fs,st,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
	  Free(st);
	  if (ScanResult(tempc,&r))
	    if (!r) {
	      tempc=TaskExeStrQue(task,Fs,
		    "Load(\"SysTemp\",LDF_JUST_LOAD);",1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
	      if (ScanResult(tempc,&r))
		okay=TRUE;
	    }
	  tempc=TaskExeStrQue(task,Fs,"Del(\"SysTemp.*\");",1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
	  ScanResult(tempc,&r);
	} else {
	  Free(prj_file);
	  st=DirFile(doc->filename.name,"Load","CPP.Z");
	  prj_file=FileNameAbs(st,FUF_Z_OR_NOT_Z);
	  Free(st);
	  if (FileFind(prj_file))
	    st=MSPrintF("\"$$WW,1$$\";ExeFile(\"%s\",TRUE);",prj_file);
	  else
	    st=MSPrintF("\"$$WW,1$$\";ExeFile(\"%s\",TRUE);",doc->filename.name);
	  tempc=TaskExeStrQue(task,Fs,st,1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
	  Free(st);
	  if (ScanResult(tempc,&r) && r)
	    okay=TRUE;
	}
	Free(prj_file);
      }
      if (!okay) {
	PopUpOk("Has Errors");
	while (LBts(&sys_semas[SYS_SEMA_FIX],0))
	  Yield;
	ToFileLine(sys_fix_file_line,&st,&i);
	LBtr(&sys_semas[SYS_SEMA_FIX],0);
	if (!StrCmp(st,doc->filename.name))
	  goto_line_num=i;
	Free(st);
      }
      break;
    case EF_REINDENT:
      if (EdGoToFun(doc,FALSE,FALSE)) {
	start_y=doc->cur_entry->y;
	indent=EdRICode(doc);
	DocUnlock(doc);
	Snd(2000); Sleep(150); Snd(0);
	Sleep(100);
	Snd(2000); Sleep(150); Snd(0);
	DocLock(doc);
	EdRemFunLeadingSpace(doc);
	DocLineNumGoTo(doc,start_y+1);
	doc_e=doc->cur_entry;
	end_y=start_y+indent->total_cnt;
	while (start_y<=doc_e->y<end_y) {
	  if (doc_e!=doc && doc_e!=doc->cur_entry &&
		!(doc_e->flags1&(DOCEG1_DONT_EDIT-DOCEF1_SCROLLING_X)) &&
		!(doc_e->flags2&DOCEG2_DONT_EDIT)) {
	    if (doc_e->type_u8==DOCT_NEW_LINE||doc_e->type_u8==DOCT_SOFT_NEW_LINE)
	      start_of_line=TRUE;
	    else {
	      if (start_of_line) {
		i=QueVectU8Get(indent,doc_e->y)*C_INDENT_SPACES;
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
		  doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
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
	QueVectU8Del(indent);
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
	DocUnlock(doc);
	Snd(2000); Sleep(150); Snd(0);
	Sleep(100);
	Snd(2000); Sleep(150); Snd(0);
	DocLock(doc);
	EdRenumAsm(doc);
      }
      break;
  }

  DocRecalc(doc);
  DocLineNumGoTo(doc,goto_line_num);

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
  DocPut->flags|=DOCF_MIN_SIZE;
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
  else if (ch=='\n')
    DocPrintF(doc,"'\n',0x%X",sc);
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
  I64 tool_action=PopUpEdFmt;
  switch (tool_action) {
    case EF_CHK_COMPILE:
    case EF_REINDENT:
    case EF_RENUM_ASM:
      EdCodeTools2(doc,tool_action);
      break;
    case EF_SLIDER_CTRL:
      MgdSliderCtrl(doc);
      break;
    case EF_CH_SC:
      EdChSC(doc);
      break;
  }
}
