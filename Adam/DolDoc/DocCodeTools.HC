#help_index "DolDoc/Misc"

U0 EdReplaceTroubleOne(CDoc *doc,U8 *st_original,U8 *st_safe,I64 num,
	Bool to_safe,Bool sel)
{
  U8 buf[STR_LEN];
  StrPrint(buf,st_safe,num);
  if (to_safe)
    EdReplace(doc,st_original,buf,sel);
  else
    EdReplace(doc,buf,st_original,sel);
}

U0 EdReplaceTroubleAll(CDoc *doc,Bool to_safe,Bool sel)
{
  I64 i=0;
  EdReplaceTroubleOne(doc,"#assert" ,"//<@%d@>"  ,i++,to_safe,sel);
  EdReplaceTroubleOne(doc,"#define" ,"//<@%d@>"  ,i++,to_safe,sel);
  EdReplaceTroubleOne(doc,"#include","//<@%d@>"  ,i++,to_safe,sel);
//#if will match #if,#ifdef,#ifndef,#ifaot and #ifjit
  EdReplaceTroubleOne(doc,"#if"     ,"//<@%d@>"  ,i++,to_safe,sel);
  EdReplaceTroubleOne(doc,"#endif"  ,"//<@%d@>"  ,i++,to_safe,sel);
//Convert #exe to union because we want that indent pattern.
  EdReplaceTroubleOne(doc,"#exe"    ,"union @%d@",i++,to_safe,sel);
  EdReplaceTroubleOne(doc,"'{'"     ,"'<@%d@>'"  ,i++,to_safe,sel);
  EdReplaceTroubleOne(doc,"'}'"     ,"'<@%d@>'"  ,i++,to_safe,sel);
}

#define C_INDENT_SPACES		2
#define ASM_RENUM_SPACING	5

#define EF_REINDENT	0
#define EF_CMP_CHK	1
#define EF_RENUM_ASM	2
#define EF_CTRL_SLIDER	3
#define EF_CH_SC	4

I64 PopUpEdFmt()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$LTBLUE$$$$MU,\"Compile Check\",LE=EF_CMP_CHK$$\n"
	"$$MU,\"Reindent HolyC Fun (Beware braces in strings.)\","
	"LE=EF_REINDENT$$\n"
	"$$MU,\"Renum Asm Local @@ Labels for Fun\",LE=EF_RENUM_ASM$$\n"
	"$$MU,\"Insert Template Code: Ctrl Slider\",LE=EF_CTRL_SLIDER$$\n"
	"$$MU,\"Insert ASCII/Scan Code Hex Codes for key pressed\","
	"LE=EF_CH_SC$$\n\n"
	"$$MU,\"CANCEL\",LE=DOCM_CANCEL$$\n\n"
	"$$GREEN$$<ALT-BACKSPACE>$$FG$$ to undo if not happy\n"
	"with the ress.\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CRILex
{
  CCmpCtrl *cc1,*cc2;
  CQueVectU8 *indent;
  I64 depth,exp_depth,one_shot;
  Bool was_new_line,is_not_cont;
};

I64 EdRILex(CRILex *rx)
{
  rx->is_not_cont=FALSE;
  I64 i;
  CLexFile *tmpf;
  do {
    Lex(rx->cc1);
    Lex(rx->cc2);
    i=PrsKeyWord(rx->cc2);
    if (rx->cc1->token=='\n' && rx->cc2->token==';' || rx->cc2->token=='{' ||
	  rx->cc2->token=='}' || rx->cc2->token==':' || rx->cc2->token==')' &&
	  !rx->exp_depth || i==KW_ELSE || i==KW_CATCH || i==KW_DO)
      rx->is_not_cont=TRUE;
    if (rx->was_new_line && (rx->cc1->token!=':' || i==KW_CASE ||
	  i==KW_DFT || i==KW_START || i==KW_END)) {
      tmpf=rx->cc2->lex_include_stk;
      while (tmpf->next)
	tmpf=tmpf->next;
      QueVectU8Put(rx->indent,tmpf->cur_entry->y,rx->depth+rx->one_shot);
      rx->one_shot=0;
    }
    if (rx->cc2->token=='\n')
      rx->was_new_line=TRUE;
    else
      rx->was_new_line=FALSE;
  } while (rx->cc1->token=='\n');
  return rx->cc1->token;
}

U0 EdRIExp(CRILex *rx)
{
  if (rx->cc1->token=='(') {
    if (!rx->exp_depth++)
      rx->depth+=3;
    EdRILex(rx);
    while (rx->cc1->token && rx->cc1->token!=')')
      EdRIExp(rx);
    if (!--rx->exp_depth) {
      rx->depth-=3;
      if (rx->depth<0) rx->depth=0;
    }
  } else if (rx->cc1->token=='[') {
    if (!rx->exp_depth++)
      rx->depth+=3;
    EdRILex(rx);
    while (rx->cc1->token && rx->cc1->token!=']')
      EdRIExp(rx);
    if (!--rx->exp_depth) {
      rx->depth-=3;
      if (rx->depth<0) rx->depth=0;
    }
  }
  EdRILex(rx);
}

U0 EdRIStmt(CRILex *rx,Bool indent)
{
  I64 i;
  Bool cont;
  if (rx->cc1->token=='{') {
    rx->depth++;
    EdRILex(rx);
    while (rx->cc1->token && rx->cc1->token!='}')
      EdRIStmt(rx,FALSE);
    if (--rx->depth<0) rx->depth=0;
    EdRILex(rx);
  } else {
    if (indent) rx->depth++;
    do {
      cont=FALSE;
      switch (PrsKeyWord(rx->cc1)) {
	case KW_IF:
	  EdRILex(rx);
	  EdRIExp(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->cc1)==KW_ELSE) {
	    EdRILex(rx);
	    if (PrsKeyWord(rx->cc1)==KW_IF && rx->cc2->token!='\n')
	      EdRIStmt(rx,FALSE);
	    else
	      EdRIStmt(rx,TRUE);
	  }
	  break;
	case KW_TRY:
	  EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->cc1)==KW_CATCH) {
	    EdRILex(rx);
	    EdRIStmt(rx,TRUE);
	  }
	  break;
	case KW_LOCK:
	  EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  break;
	case KW_FOR:
	case KW_WHILE:
	  EdRILex(rx);
	  EdRIExp(rx);
	  EdRIStmt(rx,TRUE);
	  break;
	case KW_ASM:
	case KW_CLASS:
	case KW_UNION:
	  if (EdRILex(rx)==TK_IDENT)
	    EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  break;
	case KW_DO:
	  EdRILex(rx);
	  EdRIStmt(rx,TRUE);
	  if (PrsKeyWord(rx->cc1)==KW_WHILE) {
	    EdRILex(rx);
	    EdRIExp(rx);
	  }
	  if (rx->cc1->token==';')
	    EdRILex(rx);
	  break;
	case KW_SWITCH:
	  EdRILex(rx);
	  EdRIExp(rx);
	  if (rx->cc1->token=='{') {
	    rx->depth++;
	    EdRILex(rx);
	    i=0;
	    while (rx->cc1->token && rx->cc1->token!='}') {
	      switch (PrsKeyWord(rx->cc1)) {
		case KW_START:
		  rx->depth+=i; i=0;
		  while (EdRILex(rx) && rx->cc1->token!=':');
		  EdRILex(rx);
		  i++;
		  break;
		case KW_END:
		  rx->depth+=i; i=0;
		  if (--rx->depth<0) rx->depth=0;
		  while (EdRILex(rx) && rx->cc1->token!=':');
		  EdRILex(rx);
		  break;
		case KW_CASE:
		case KW_DFT:
		  rx->depth+=i; i=0;
		  while (EdRILex(rx) && rx->cc1->token!=':');
		  EdRILex(rx);
		  break;
		default:
		  if (rx->cc1->token)
		    EdRIStmt(rx,TRUE);
	      }
	    }
	    if (--rx->depth<0) rx->depth=0;
	    EdRILex(rx);
	  }
	  break;
	default:
	  if (rx->cc1->token==TK_IDENT && rx->cc1->hash_entry &&
		rx->cc1->hash_entry->type&(HTT_OPCODE|HTT_ASM_KEYWORD)) {
//	    rx->one_shot=4-rx->depth;
	    do EdRILex(rx);
	    while (rx->cc2->token && rx->cc2->token!='\n');
	    rx->is_not_cont=TRUE;
	  } else {
	    while (rx->cc1->token && rx->cc1->token!=';' &&
		  rx->cc1->token!=':') {
	      if (rx->cc2->token=='\n' && !rx->is_not_cont)
		rx->one_shot=3;
	      EdRILex(rx);
	    }
	    if (rx->cc1->token==':')
	      cont=TRUE;
	    EdRILex(rx);
	  }
      }
    } while (cont && rx->cc1->token!='}');
    if (indent && --rx->depth<0)
      rx->depth=0;
  }
}

CQueVectU8 *EdRICode(CDoc *doc)
{
  CQueVectU8 *res;
  CRILex *rx=CAlloc(sizeof(CRILex));

  rx->cc1=CmpCtrlNew(,CCF_KEEP_NEW_LINES|CCF_DONT_FREE_BUF,doc->filename.name);
  Free(rx->cc1->lex_include_stk->full_name);
  LexAttachDoc(rx->cc1,rx->cc1->lex_include_stk,doc,,
	doc->cur_entry,doc->cur_col);

  rx->cc2=CmpCtrlNew(,CCF_KEEP_NEW_LINES|CCF_DONT_FREE_BUF,doc->filename.name);
  Free(rx->cc2->lex_include_stk->full_name);
  LexAttachDoc(rx->cc2,rx->cc2->lex_include_stk,doc,,
	doc->cur_entry,doc->cur_col);

  rx->indent=QueVectU8New(doc->cur_entry->y);

  Lex(rx->cc1);
  EdRIStmt(rx,FALSE);

  CmpCtrlDel(rx->cc1);
  CmpCtrlDel(rx->cc2);
  res=rx->indent;
  Free(rx);
  return res;
}

U0 EdRemFunLeadingSpace(CDoc *doc)
{
  Bool unlock=DocLock(doc),
	start_of_line=TRUE;
  U8 *ptr;
  I64 ch,levels=1;
  CDocEntry *doc_e,*doc_e2;

  EdGoToFun(doc,FALSE,FALSE);
  doc_e=doc->cur_entry->next;
  do {
    doc_e2=doc_e->next;
    if (doc_e!=doc && doc_e!=doc->cur_entry &&
	  !(doc_e->de_flags&(DOCEG_DONT_EDIT-DOCEF_SCROLLING_X)))
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
  I64 res=-1;
  while (doc->cur_entry!=doc &&
	doc->cur_entry->type&DOCET_SEL && res<0) {
    res=EdCurU8(doc);
    EdCursorRight(doc);
  }
  return res;
}

U0 EdRACollect(CDoc *doc,CRenum *head)
{
  I64 ch,i;
  CRenum *tmpr;
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
	  if (Bt(char_bmp_alpha_numeric,ch))
	    buf[i++]=ch;
	  else
	    break;
	  ch=EdRAGetU8(doc);
	}
	if (i<sizeof(CEdFindText.find_text)) {
	  buf[i++]=0;
	  while (ch>=0 && Bt(char_bmp_white_space,ch))
	    ch=EdRAGetU8(doc);
	  if (ch==':') {
	    ch=EdRAGetU8(doc);
	    tmpr=MAlloc(sizeof(CRenum));
	    StrCpy(tmpr->label,buf);
	    QueIns(tmpr,head->last);
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
  CRenum head,*tmpr,*tmpr1;
  U8	buf[sizeof(CEdFindText.find_text)],
	buf2[sizeof(CEdFindText.find_text)];

  QueInit(&head);
  EdSelFun(doc,TRUE);
  EdRACollect(doc,&head);

  tmpr=head.next;
  while (tmpr!=&head) {
    tmpr1=tmpr->next;
    num+=ASM_RENUM_SPACING;
    StrPrint(buf,"@#%02d",num);
    EdReplace(doc,tmpr->label,buf,TRUE,TRUE,TRUE);
    Free(tmpr);
    tmpr=tmpr1;
  }

  while (num) {
    StrPrint(buf, "@#%02d",num);
    StrPrint(buf2,"@@%02d",num);
    EdReplace(doc,buf,buf2,TRUE,TRUE,TRUE);
    num-=ASM_RENUM_SPACING;
  }
  EdSelAll(doc,FALSE);
  DocRecalc(doc);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
}

U0 EdCodeTools2(CDoc *doc,I64 tool_action,Bool beep=TRUE)
{
  Bool okay,unlock=DocLock(doc),start_of_line=TRUE;
  CDocEntry *doc_e,*doc_ne;
  I64 i,start_y,end_y,x,r,goto_line_num;
  U8 *b,*st,*st2,*prj_file;
  CTask *task=NULL;
  CJob *tmpc;
  CQueVectU8 *indent;

  DocRecalc(doc);
  goto_line_num=doc->cur_entry->y+1;

  DocCaptureUndo(doc,TRUE);
  switch (tool_action) {
    case EF_CMP_CHK:
      okay=FALSE;
      if (doc->flags&DOCF_PLAIN_TEXT)
	DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
      DocWrite(doc);
      task=Spawn(&SrvCmdLine,NULL,"Srv",,Fs);
      st2=DirCur;
      st=MStrPrint("Cd(\"%s\");",st2);
      tmpc=TaskExe(task,Fs,st,1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
      Free(st2);
      Free(st);
      WinHorz(Fs->win_left,Fs->win_right, task);
      WinVert(Fs->win_top, Fs->win_bottom,task);
      if (JobResScan(tmpc,&r)) {
	st=DirFile(doc->filename.name,,"PRJ.Z");
	prj_file=FileNameAbs(st,FUF_Z_OR_NOT_Z);
	Free(st);
	if (FileFind(prj_file)) {
	  st2=DirFile(prj_file),
		st=MStrPrint("Cd(\"%s\");",st2);
	  Free(st2);
	  tmpc=TaskExe(task,Fs,st,1<<JOBf_WAKE_MASTER|
		1<<JOBf_FOCUS_MASTER|1<<JOBf_FREE_ON_COMPLETE);
	  Free(st);
	  st=MStrPrint("\"$$WW,1$$\";Cmp(\"%s\",\"SysTmp\",\"SysTmp\");",
		prj_file);
	  tmpc=TaskExe(task,Fs,st,1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
	  Free(st);
	  if (JobResScan(tmpc,&r))
	    if (!r) {
	      tmpc=TaskExe(task,Fs,
		    "Load(\"SysTmp\",LDF_JUST_LOAD);",
		    1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
	      if (JobResScan(tmpc,&r))
		okay=TRUE;
	    }
	  tmpc=TaskExe(task,Fs,"Del(\"SysTmp.*\");",
		1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
	  JobResScan(tmpc,&r);
	} else {
	  Free(prj_file);
	  st=DirFile(doc->filename.name,"Load","HC.Z");
	  prj_file=FileNameAbs(st,FUF_Z_OR_NOT_Z);
	  Free(st);
	  if (FileFind(prj_file))
	    st=MStrPrint("\"$$WW,1$$\";ExeFile(\"%s\",CCF_JUST_LOAD);",prj_file);
	  else
	    st=MStrPrint("\"$$WW,1$$\";ExeFile(\"%s\",CCF_JUST_LOAD);",
		  doc->filename.name);
	  tmpc=TaskExe(task,Fs,st,1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
	  Free(st);
	  if (JobResScan(tmpc,&r) && r)
	    okay=TRUE;
	}
	Free(prj_file);
      }
      if (!okay) {
	PopUpOk("Has Errors");
	while (LBts(&sys_semas[SEMA_FIX],0))
	  Yield;
	ToFileLine(dbg.fix_file_line,&st,&i);
	LBtr(&sys_semas[SEMA_FIX],0);
	if (!StrCmp(st,doc->filename.name))
	  goto_line_num=i;
	Free(st);
      }
      break;
    case EF_REINDENT:
      start_y=doc->cur_entry->y;
      EdReplaceTroubleAll(doc,TRUE,FALSE);
      DocGoToLine(doc,start_y+1);
      if (EdGoToFun(doc,FALSE,FALSE)) {
	start_y=doc->cur_entry->y;
	indent=EdRICode(doc);
	DocUnlock(doc);
	if (beep) {
	  Snd(86); Sleep(150); Snd;
	  Sleep(100);
	  Snd(86); Sleep(150); Snd;
	}
	DocLock(doc);
	EdRemFunLeadingSpace(doc);
	DocGoToLine(doc,start_y+1);
	doc_e=doc->cur_entry;
	end_y=start_y+indent->total_cnt;
	while (start_y<=doc_e->y<end_y) {
	  if (doc_e!=doc && doc_e!=doc->cur_entry &&
		!(doc_e->de_flags&(DOCEG_DONT_EDIT-DOCEF_SCROLLING_X))) {
	    if (doc_e->type_u8==DOCT_NEW_LINE||
		  doc_e->type_u8==DOCT_SOFT_NEW_LINE)
	      start_of_line=TRUE;
	    else {
	      if (start_of_line) {
		i=QueVectU8Get(indent,doc_e->y)*C_INDENT_SPACES;
		x=doc_e->x+1;
		while (i>8) {
		  doc_ne=DocEntryNewBase(doc,
			DOCT_TAB|doc->settings_head.dft_text_attr<<8,,
			x,doc_e->y,doc_e->page_line_num);
		  MemCpy(&doc_ne->settings,
			&doc_e->settings,sizeof(CDocSettings));
		  QueIns(doc_ne,doc_e->last);
		  i-=8;
		  x+=8;
		}
		if (i>0) {
		  b=MAlloc(i+1,doc->mem_task);
		  MemSet(b,CH_SPACE,i);
		  b[i]=0;
		  doc_ne=DocEntryNewBase(doc,
			DOCT_TEXT|doc->settings_head.dft_text_attr<<8,,
			x,doc_e->y,doc_e->page_line_num);
		  doc_ne->tag=b;
		  doc_ne->max_col=1;
		  MemCpy(&doc_ne->settings,
			&doc_e->settings,sizeof(CDocSettings));
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
      start_y=doc->cur_entry->y;
      EdReplaceTroubleAll(doc,FALSE,FALSE);
      DocGoToLine(doc,start_y+1);
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
	if (beep) {
	  Snd(86); Sleep(150); Snd;
	  Sleep(100);
	  Snd(86); Sleep(150); Snd;
	}
	DocLock(doc);
	EdRenumAsm(doc);
      }
      break;
  }

  DocRecalc(doc);
  DocGoToLine(doc,goto_line_num);

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
  DocPut->flags|=DOCF_SIZE_MIN;
  do GetMsg(_ch,&sc,1<<MSG_KEY_DOWN);
  while (sc.u8[0]==SC_SHIFT || sc.u8[0]==SC_CTRL || sc.u8[0]==SC_ALT);
  *_sc=sc;
}

U0 EdChSC(CDoc *doc)
{
  I64 ch,sc;
  U8 buf[STR_LEN];
  StrPrint(buf,"EdPopUpChSC(%d,%d);",&ch,&sc);
  PopUp(buf,Fs);
  if (ch==CH_BACKSPACE)
    DocPrint(doc,"CH_BACKSPACE,0x%X",sc);
  else if (ch=='\n')
    DocPrint(doc,"'\n',0x%X",sc);
  else if (CH_CTRLA<=ch<=CH_CTRLZ)
    DocPrint(doc,"CH_CTRL%C,0x%X",ch+'@',sc);
  else if (ch=='$$')
    DocPrint(doc,"'$$$$',0x%X",sc);
  else if (ch=='\\')
    DocPrint(doc,"'\\\\',0x%X",sc);
  else if (ch=='\'')
    DocPrint(doc,"'\\\'',0x%X",sc);
  else if (ch==CH_ESC)
    DocPrint(doc,"CH_ESC,0x%X",sc);
  else if (ch==CH_SHIFT_ESC)
    DocPrint(doc,"CH_SHIFT_ESC,0x%X",sc);
  else if (ch==CH_SPACE)
    DocPrint(doc,"CH_SPACE,0x%X",sc);
  else if (ch==CH_SHIFT_SPACE)
    DocPrint(doc,"CH_SHIFT_SPACE,0x%X",sc);
  else if (Bt(char_bmp_displayable,ch))
    DocPrint(doc,"'%c',0x%X",ch,sc);
  else
    DocPrint(doc,"0x%X,0x%X",ch,sc);
}

U0 EdCodeTools(CDoc *doc)
{
  I64 tool_action=PopUpEdFmt;
  switch (tool_action) {
    case EF_CMP_CHK:
    case EF_REINDENT:
    case EF_RENUM_ASM:
      EdCodeTools2(doc,tool_action);
      break;
    case EF_CTRL_SLIDER:
      TemplateCtrlSlider(doc);
      break;
    case EF_CH_SC:
      EdChSC(doc);
      break;
  }
}
