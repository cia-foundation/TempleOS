#help_index "Debugging"

#define IDET_GO			1
#define IDET_STEP_INTO		2
#define IDET_STEP_OVER		3

class CIDE
{
  I64 type;
  U8 *cur_line,*next_line;
  U8 *call_bpt;
  Bool existing_bpt;
};

U0 IDEWaitParent()
{
  CTask *task=Fs->parent_task;
  while (!IsSuspended(task) &&
	 !Bt(&task->task_flags,TASKf_IDLE) &&
	 !Bt(&task->rflags,RFLAGS_SINGLE_STEP))
    Yield;
}

public CHashFun *DbgFunFind(CDoc *doc,I64 line_offset=0)
{
  U8 st[256],*filename=doc->filename.name;
  I64 line=doc->cur_entry->y+1+line_offset;
  CHashTable *h=Fs->hash_table;
  CHashSrcSym *temph;
  CDbgInfo *dbg;
  while (h) {
    temph=h->next_added;
    while (temph!=h) {
      if (temph->type&(HTT_FUN|HTT_EXPORT_SYS_SYM))
	dbg=temph->debug;
      else
	dbg=NULL;
      if (dbg) {
	StrCpy(st,temph->src_link);
	StrFirstRem(st,":");
	StrLastRem(st,",");
	if (!StrCmp(st,filename)) {
	  if (dbg->min_line<=line<=dbg->max_line)
	    return temph;
	}
      }
      temph=temph->next_added;
    }
    h=h->next;
  }
  return NULL;
}

U0 WatchLocals()
{
  Bool old_preempt=Preempt(OFF);
  CTask *task=Fs->parent_task,*dbg_task;
  CDoc *doc;
  CHashFun *tempf,*tempf1;
  I64 sc,ch,offset,ip;
  F64 timeout;
  U8 **rbp,**ptr;
  while (TRUE) {
    IDEWaitParent;
    if (!TaskValidate(dbg_task=task->dbg_task))
      Exit;
    if (ScanKey(&ch,&sc,TRUE)) {
      timeout=tS+4.0;
      do {
	if (ch==CH_SHIFT_ESC || ch==CH_ESC)
	  Exit;
	if (ScanKey(&ch,&sc,TRUE))
	  timeout=tS+4.0;
	WinMgrSync;
      } while (tS<timeout);
    } else {
      IDEWaitParent;
      if (!TaskValidate(dbg_task=task->dbg_task))
	Exit;
      DocDblBufStart;
      if (doc=DocCur(dbg_task)) {
	if (tempf=DbgFunFind(doc,0)) {
	  "$$WW,1$$";
	  rbp=task->rbp;
	  ip=task->rip;
	  while (TRUE) {
	    tempf1=PSectFind(ip,&offset);
break; //TODO: load reg vars for higher level subroutines
	    if (tempf==tempf1)
	      break;
	    else {
	      if (rbp>=*rbp)
		break;
	      ptr=rbp+1;
	      rbp=*rbp;
	      ip=*ptr;
	      if (!ChkOnStk(rbp,task))
		break;
	    }
	  }
	  if (tempf==tempf1)
	    FunRep(tempf->str,rbp,1,task);
	}
      }
      DocDblBufEnd;
      WinMgrSync;
    }
  }
  Preempt(old_preempt);
}


public U0 IDE(U8 *filename)
//Spawns a child process which has
//access to parents syms. Can be used
//to debug parent.
{
  CTask *task;
  U8 *fn1=DftExt(filename,"CPP.Z"),
     *fn2=FileNameAbs(fn1);

  if (PopUpCancelOk("$$BK,1$$$$FG,RED$$SparrowOS IDE Debugger$$FG$$$$BK,0$$\n"
	"Do not use this on files that run when included.\n"
	"\nTo use:\n"
	"  1) Set a break-point in the editor window.\n"
	"  2) Run program from cmd line.\n\n"
	"$$FG,GREEN$$F5$$FG$$\t\tGo\n"
	"$$FG,GREEN$$F10$$FG$$\t\tStep Over\n"
	"$$FG,GREEN$$F11$$FG$$\t\tStep Into\n"
	"$$FG,GREEN$$F9$$FG$$\t\tToggle Break Point\n"
	"$$FG,GREEN$$SHIFT-F9$$FG$$\tClear All Break Points\n",
	"\n\n\nNote: I don't use this debugger because\n"
	"I'm always doing multitasking stuff, \n"
	"kernel stuff and multicore stuff.  Also,\n"
	"you can't debug code in the window\n"
	"mgr callbacks.  It works on really\n"
	"simple stuff, sort-of.\n\n"
	"I recommend making use of:\n"
	"$$LK,\"progress1\",\"MN:progress1\"$$-$$LK,\"progress4\",\"MN:progress4\"$$,       -- glbl I64 vars shown on wallpaper\n"
	"$$LK,\"DbgNum\",\"MN:DbgNum\"$$() $$LK,\"DbgStr\",\"MN:DbgStr\"$$() $$LK,\"DbgAdd\",\"MN:DbgAdd\"$$() -- raw text output for a time\n"
	"$$LK,\"Beep\",\"MN:Beep\"$$() $$LK,\"Snd\",\"MN:Snd\"$$()               -- sound\n"
	"$$LK,\"AdamLog\",\"MN:AdamLog\"$$() $$LK,\"AdamErr\",\"MN:AdamErr\"$$()        -- printf to adam task's window\n"
	"$$LK,\"SysDbg\",\"MN:SysDbg\"$$() and $$LK,\"IsSysDbg\",\"MN:IsSysDbg\"$$()    -- just a flag\n\n")) {
    Fs->win_top=2;
    Fs->win_bottom=TEXT_ROWS-2;
    Fs->win_left=1;
    Fs->win_right=TEXT_COLS>>1-1;
    TaskDerivedValsUpdate;
    LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);

    task=Spawn(&UserCmdLine,NULL,"Debug",Fs);
    TaskWait(task);
    task->win_top=12;
    task->win_bottom=TEXT_ROWS-2;
    task->win_left=TEXT_COLS>>1+1;
    task->win_right=TEXT_COLS-2;
    TaskDerivedValsUpdate(task);
    XTalk(task,"Ed(\"%s\");\n",fn2);

    Fs->dbg_task=task;

    task=Spawn(&UserCmdLine,NULL,"Watch",Fs);
    TaskWait(task);
    task->win_top=2;
    task->win_bottom=10;
    task->win_left=TEXT_COLS>>1+1;
    task->win_right=TEXT_COLS-2;
    TaskDerivedValsUpdate(task);
    XTalk(task,"WatchLocals;\n");
    ExeFile(fn2);
    Free(fn1);
    Free(fn2);
  }
}


#help_index "Debugging/Breakpoints"
I64 IDELineAdd(CDbgInfo *dbg,I64 line)
{
  I64 max=dbg->max_line-dbg->min_line;
  line-=dbg->min_line;
  if (line<0)
    return IDELineAdd(dbg,dbg->min_line);
  while (!dbg->body[line] && line<=max)
    line++;
  return dbg->body[line];
}

I64 IDERIPLineNum(CDoc *doc,U8 *rip)
{
  U8 st[256],*filename=doc->filename.name;
  I64 line;
  CHashTable *h=Fs->hash_table;
  CHashFun *tempf;
  CDbgInfo *dbg;
  while (h) {
    tempf=h->next_added;
    while (tempf!=h) {
      if (tempf->type&HTT_FUN) {
	if (dbg=tempf->debug) {
	  StrCpy(st,tempf->src_link);
	  StrFirstRem(st,":");
	  StrLastRem(st,",");
	  if (!StrCmp(st,filename)) {
	    for (line=dbg->min_line;line<=dbg->max_line;line++)
	      if (rip>=IDELineAdd(dbg,line) &&
		  rip<IDELineAdd(dbg,line+1))
		return line;
	    }
	}
      }
      tempf=tempf->next_added;
    }
    h=h->next;
  }
  return -1;
}

U0 IDESetStepRange(CDoc *doc)
{
  U8 st[256],*filename=doc->filename.name;
  I64 line=doc->cur_entry->y+1;
  CHashTable *h=Fs->hash_table;
  CHashFun *tempf;
  CIDE *tempi=NamedFramePtr("IDEFrame");
  CDbgInfo *dbg;
  while (h) {
    tempf=h->next_added;
    while (tempf!=h) {
      if (tempf->type&HTT_FUN) {
	if (dbg=tempf->debug) {
	  StrCpy(st,tempf->src_link);
	  StrFirstRem(st,":");
	  StrLastRem(st,",");
	  if (!StrCmp(st,filename)) {
	    if (dbg->min_line<=line<=dbg->max_line) {
	      tempi->cur_line =IDELineAdd(dbg,line);
	      tempi->next_line=IDELineAdd(dbg,line+1);
	      return;
	    }
	  }
	}
      }
      tempf=tempf->next_added;
    }
    h=h->next;
  }
}

U0 IDEToggleBpt(CDoc *doc)
{ //F9
  U8 st[512],*filename=doc->filename.name;
  I64 a,line=doc->cur_entry->y+1;
  Bool unlock;
  CHashTable *h=Fs->hash_table;
  CHashFun *tempf;
  CDbgInfo *dbg;
  CDocEntry *doc_e;
  while (h) {
    tempf=h->next_added;
    while (tempf!=h) {
      if (tempf->type&HTT_FUN) {
	if (dbg=tempf->debug) {
	  StrCpy(st,tempf->src_link);
	  StrFirstRem(st,":");
	  StrLastRem(st,",");
	  if (!StrCmp(st,filename)) {
	    if (dbg->min_line<=line<=dbg->max_line) {
	      a=IDELineAdd(dbg,line);
	      unlock=DocLock(doc);
	      doc_e=doc->cur_entry;
	      while (doc_e->last->type_u8!=DOCT_NEW_LINE &&
		  doc_e->last->type_u8!=DOCT_SOFT_NEW_LINE &&
		  doc_e->last!=doc)
		doc_e=doc_e->last;
	      doc->cur_entry=doc_e;
	      doc->cur_data_col=doc_e->min_col;
	      if (!BptR(a,Fs->parent_task)) {
		BptS(a,Fs->parent_task);
		DocPutS(doc,"$$BP$$");
	      } else {
		if (doc_e->type_u8==DOCT_BPT) {
		  doc->cur_entry=doc_e->next;
		  doc->cur_data_col=doc->cur_entry->min_col;
		  DocEntryDel(doc,doc_e);
		}
	      }
	      if (unlock)
		DocUnlock(doc);
	      DocLineNumGoTo(doc,line);
	      return;
	    }
	  }
	}
      }
      tempf=tempf->next_added;
    }
    h=h->next;
  }
}

U0 IDEClearAllBpts(CDoc *doc)
{ //SHIFT-F9
  CDocEntry *doc_e,*doc_e2;
  Bool unlock;
  IDEWaitParent;
  BptsR(Fs->parent_task);
  unlock=DocLock(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (doc_e->type_u8==DOCT_BPT) {
      if (doc->cur_entry==doc_e) {
	doc->cur_entry=doc_e->next;
	doc->cur_data_col=doc->cur_entry->min_col;
      }
      DocEntryDel(doc,doc_e);
    }
    doc_e=doc_e2;
  }
  if (unlock)
    DocUnlock(doc);
}

U0 IDEGo()  //Continue after breakpoint
{ //F5
  CTask *task=Fs->parent_task;
  CIDE *tempi=NamedFramePtr("IDEFrame");
  if (!tempi) {
    tempi=CAlloc(sizeof(CIDE));
    NamedFramePtrAdd("IDEFrame",tempi);
  }
  IDEWaitParent;
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  tempi->cur_line=task->rip;
  tempi->type=IDET_GO;
  LBts(&task->rflags,RFLAGS_SINGLE_STEP);
  Suspend(task,FALSE);
}

U0 IDEStepOver(CDoc *doc)
{ //F10
  CTask *task=Fs->parent_task;
  I64 line;
  U8 *opcode;

  CIDE *tempi=NamedFramePtr("IDEFrame");
  if (!tempi) {
    tempi=CAlloc(sizeof(CIDE));
    NamedFramePtrAdd("IDEFrame",tempi);
  }

  IDEWaitParent;
  line=IDERIPLineNum(doc,task->rip);
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  if (line>0) {
    DocLineNumGoTo(doc,line);
    IDESetStepRange(doc);
  } else {
    tempi->cur_line=NULL;
    tempi->next_line=NULL;
  }
  tempi->type=IDET_STEP_OVER;
  tempi->call_bpt=NULL;

  LBts(&task->rflags,RFLAGS_SINGLE_STEP);
  Suspend(task,FALSE);

  opcode=task->rip;
  if (*opcode==OC_CALL) {
    if (BptS(opcode+5,task)) {
      if (tempi->call_bpt!=opcode+5)
	tempi->existing_bpt=TRUE;
    } else
      tempi->existing_bpt=FALSE;
    tempi->call_bpt=opcode+5;
    LBtr(&task->task_flags,TASKf_DISABLE_BPTS);
    LBtr(&task->rflags,RFLAGS_SINGLE_STEP);
  }
}

U0 IDEStepInto(CDoc *doc)
{ //F11
  CTask *task=Fs->parent_task;
  I64 line;

  CIDE *tempi=NamedFramePtr("IDEFrame");
  if (!tempi) {
    tempi=CAlloc(sizeof(CIDE));
    NamedFramePtrAdd("IDEFrame",tempi);
  }

  IDEWaitParent;
  line=IDERIPLineNum(doc,task->rip);
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  if (line>0) {
    DocLineNumGoTo(doc,line);
    IDESetStepRange(doc);
  } else {
    tempi->cur_line=NULL;
    tempi->next_line=NULL;
  }
  tempi->type=IDET_STEP_INTO;
  LBts(&task->rflags,RFLAGS_SINGLE_STEP);
  Suspend(task,FALSE);
}

U0 IDEEdTask(I64)
{
  CTask *task=Fs->parent_task,
	*ed_task=NULL,*dbg_task;
  U8 *rip=task->rip,
     *st,*rip_filename,*bpt_filename;
  CIDE *tempi,*tempi1;

  CDoc *doc;
  CBpt *tempb;
  if (rip_filename=SrcFileName(rip)) {
    ed_task=Spawn(&UserCmdLine,NULL,"Debug",task);
    TaskWait(ed_task);
    st=MSPrintF("Preempt(OFF);Ed(\"%s\");\n",rip_filename);
    XTalkWait(ed_task,st);
    Free(st);
    doc=DocCur(ed_task);
    if (TaskValidate(dbg_task=task->dbg_task)) {
      ed_task->win_top=dbg_task->win_top;
      ed_task->win_bottom=dbg_task->win_bottom;
      ed_task->win_left=dbg_task->win_left;
      ed_task->win_right=dbg_task->win_right;
      TaskDerivedValsUpdate(ed_task);
      if (tempi=NamedFramePtr("IDEFrame",dbg_task)) {
	tempi1=MAllocIdentical(tempi,ed_task);
	NamedFramePtrAdd("IDEFrame",tempi1);
      }
      Kill(dbg_task);
    }
    task->dbg_task=ed_task;
    tempb=task->bpt_list;
    while (tempb) {
      if (bpt_filename=SrcFileName(tempb->addr)) {
	if (!StrCmp(rip_filename,bpt_filename)) {
	  DocLineNumGoTo(doc,SrcLineNum(tempb->addr));
	  DocPutS(doc,"$$BP$$");
	}
	Free(bpt_filename);
      }
      tempb=tempb->next;
    }
    DocLineNumGoTo(doc,SrcLineNum(rip));
    Free(rip_filename);
    StrCpy(ed_task->task_title,"Debug Task");
  }
}

U0 IDEResumeDbg()
{
  CTask *dbg_task=Fs->dbg_task;
  CIDE *tempi=NamedFramePtr("IDEFrame",dbg_task);
  U8 *opcode;
  if (!tempi) {
    Spawn(&IDEEdTask,NULL,"Resume Dbg",Fs);
    Suspend(Fs);
  } else
    switch (tempi->type) {
      case IDET_GO:
	if (tempi->cur_line) {
	  LBtr(&Fs->task_flags,TASKf_DISABLE_BPTS);
	  LBtr(&Fs->rflags,RFLAGS_SINGLE_STEP);
	  if (Fs->rip==tempi->cur_line) {//Jmp to self
	    tempi->cur_line=NULL;
	    Spawn(&IDEEdTask,NULL,"Resume Dbg",Fs);
	    Suspend(Fs);
	  } else
	    tempi->cur_line=NULL;
	} else {
	  Spawn(&IDEEdTask,NULL,"Resume Dbg",Fs);
	  Suspend(Fs);
	}
	break;
      case IDET_STEP_INTO:
	if (!tempi->cur_line ||
	    Fs->rip<tempi->cur_line || Fs->rip>=tempi->next_line) {
	  Spawn(&IDEEdTask,NULL,"Resume Dbg",Fs);
	  Suspend(Fs);
	} else
	  Suspend(Fs,FALSE);
	break;
      case IDET_STEP_OVER:
	LBts(&Fs->rflags,RFLAGS_SINGLE_STEP);
	LBts(&Fs->task_flags,TASKf_DISABLE_BPTS);
	if (Fs->rip==tempi->call_bpt) {
	  if (!tempi->existing_bpt)
	    BptR(Fs->rip,Fs,FALSE);
	  tempi->call_bpt=NULL;
	}
	if (!tempi->cur_line ||
	    Fs->rip<tempi->cur_line || Fs->rip>=tempi->next_line) {
	  Spawn(&IDEEdTask,NULL,"Resume Dbg",Fs);
	  Suspend(Fs);
	} else {
	  opcode=Fs->rip;
	  if (*opcode==OC_CALL) {
	    if (BptS(opcode+5,Fs,FALSE)) {
	      if (tempi->call_bpt!=opcode+5)
		tempi->existing_bpt=TRUE;
	    } else
	      tempi->existing_bpt=FALSE;
	    tempi->call_bpt=opcode+5;
	    LBtr(&Fs->task_flags,TASKf_DISABLE_BPTS);
	    LBtr(&Fs->rflags,RFLAGS_SINGLE_STEP);
	  }
	  Suspend(Fs,FALSE);
	}
	break;
    }
}

ext[EXT_RESUME_DBG]=&IDEResumeDbg;

#help_index ""
