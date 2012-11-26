#help_index "Debugging"

Bool ClassRep2(CDoc *doc,U8 *class_name,U8 *_b,I64 depth,I64 max_depth,Bool dynamic,I64 types=HTT_CLASS,I64 offset=0)
{
  I64 i,j,*ptr;
  CMemberList *ml;
  CDocEntry *doc_e;
  Bool unlock;
  CHashClass *tempc,*tempc2;
  if (depth>=max_depth) return TRUE;
  if (!(tempc=HashFind(class_name,Fs->hash_table,types))) {
    DocPutS(doc,"Class Not Found.\n");
    return FALSE;
  }
  if (!ChkPtr(_b) || !ChkPtr(_b(U8 *)+tempc->size)) {
    DocPrintF(doc,"Bad Ptr:%016X\n",_b);
    return FALSE;
  }
  if (tempc->base_class && !ClassRep2(doc,tempc->base_class->str,_b,depth,max_depth,dynamic,types,offset))
    return FALSE;
  unlock=DocLock(doc);
  DocPrintF(doc,"Class:\"%s\"\n",class_name);
  ml=tempc->member_list;
  while (ml) {
    tempc2=ml->member_class;
    ptr=_b(U8 *)+ml->offset;
    DocPrintF(doc,"%08X ",ptr(U8 *)+offset);
    j=tempc2->ptr_cnt;
    while (tempc2->fwd_class)
      tempc2=tempc2->fwd_class;
    tempc2-=tempc2->ptr_cnt;
    if (tempc2->type & HTT_INTERNAL_TYPE) {
      DocPrintF(doc,"$$FG,GREEN$$%-20ts:$$FG$$",ml->str);
      if (j==1 && (tempc2->sub_type==IT_I8 || tempc2->sub_type==IT_U8)) {
	ptr=*ptr;
	if (ChkPtr(ptr)) {
	  if (dynamic) {
	    doc_e=DocPutS(doc,"$$DA-T-P+RD,64,\"%40ts\"$$\n");
	    doc_e->data=ptr;
	    DocDataFmt(doc_e);
	  } else
	    DocPrintF(doc,"%40ts\n",ptr);
	}
      } else if (!j) {
	j=MinI64(ml->dim.total_cnt,32);
	if (tempc2->sub_type==IT_I8 || tempc2->sub_type==IT_U8) {
	  if (j==1) {
	    if (dynamic) {
	      doc_e=DocPutS(doc,"$$DA-T+RD+UD-I64+U8,32,\"%c\"$$\n");
	      doc_e->data=ptr;
	    } else
	      DocPrintF(doc,"%c\n",*ptr(U8 *));
	  } else {
	    if (dynamic) {
	      doc_e=DocPutS(doc,"$$DA-T-P+RD,64,\"%40ts\"$$\n");
	      doc_e->data=ptr;
	      DocDataFmt(doc_e);
	    } else
	      DocPrintF(doc,"%40ts\n",ptr);
	  }
	}
	for (i=0;i<j;i++) {
	  switch (tempc2->sub_type) {
	    case IT_I0:
	    case IT_U0:
	      break;
	    case IT_I8:
	    case IT_U8:
	      if (dynamic) {
		doc_e=DocPutS(doc,"$$DA-T+RD+UD-I64+U8,32,\"%02X\"$$ ");
		doc_e->data=ptr(U8 *)++;
	      } else
		DocPrintF(doc,"%02X ",*ptr(U8 *)++);
	      break;
	    case IT_I16:
	    case IT_U16:
	      if (dynamic) {
		doc_e=DocPutS(doc,"$$DA-T+RD+UD-I64+U16,32,\"%04X\"$$ ");
		doc_e->data=ptr(U16 *)++;
	      } else
		DocPrintF(doc,"%04X ",*ptr(U16 *)++);
	      break;
	    case IT_I32:
	    case IT_U32:
	      if (dynamic) {
		doc_e=DocPutS(doc,"$$DA-T+RD+UD-I64+U32,32,\"%08X\"$$ ");
		doc_e->data=ptr(U32 *)++;
	      } else
		DocPrintF(doc,"%08X ",*ptr(U32 *)++);
	      break;
	    case IT_F64:
	      if (dynamic) {
		doc_e=DocPutS(doc,"$$DA-T+RD+UD,32,\"%16g\"$$ ");
		doc_e->data=ptr(I64 *)++;
	      } else
		DocPrintF(doc,"%16g ",*ptr(I64 *)++);
	      break;
	    default:
	      if (dynamic) {
		doc_e=DocPutS(doc,"$$DA-T+RD+UD,32,\"%016X\"$$ ");
		doc_e->data=ptr(I64 *)++;
	      } else
		DocPrintF(doc,"%016X ",*ptr(I64 *)++);
	  }
	  if (dynamic)
	    DocDataFmt(doc_e);
	}
	if (j<ml->dim.total_cnt)
	  DocPutS(doc,"...");
      } else
	DocPrintF(doc,"%016X",*ptr);
      DocPutS(doc,"\n");
    } else {
      if (depth<2) {
	if (j==1 && !ChkPtr(*ptr))
	  DocPrintF(doc,"%-20ts:%016X\n",ml->str,*ptr);
	else {
	  DocPrintF(doc,"$$TR,\"%s\"$$\n",ml->str);
	  DocPutS(doc,"$$ID,2$$");
	  if (!j)
	    ClassRep2(doc,tempc2->str,ptr,depth+1,max_depth,dynamic);
	  else if (j==1)
	    ClassRep2(doc,tempc2->str,*ptr,depth+1,max_depth,dynamic);
	  DocPutS(doc,"$$ID,-2$$");
	}
      } else
	DocPrintF(doc,"%-20ts\n",ml->str);
    }
    ml=ml->next;
  }
  if (unlock)
    DocUnlock(doc);
  return TRUE;
}

public U0 ClassRep(U8 *class_name,U8 *_b,I64 max_depth=2,Bool fun=FALSE,I64 offset=0)
//Displays members of a record
//by using the compiler's info.
{
  CDoc *doc;
  Bool old_preempt;
  if (IsRaw) {
    doc=DocNew;
  } else {
    DocMax;
    old_preempt=Preempt(OFF);
    doc=DocCur;
  }
  if (fun)
    ClassRep2(doc,class_name,_b,0,max_depth,FALSE,HTT_FUN,offset);
  else
    ClassRep2(doc,class_name,_b,0,max_depth,FALSE,HTT_CLASS,offset);
  DocPutS(doc,"\n");
  DocRecalc(doc);
  if (IsRaw) {
    DocDump(doc,100000);
    DocDel(doc);
  } else
    Preempt(old_preempt);
}

public U0 ClassRepD(U8 *class_name,U8 *_b,I64 max_depth=2,Bool fun=FALSE,I64 offset=0)
//Dynamic classrep (uses hex_ed widgit for live changes)
{
  CDoc *doc;
  Bool old_preempt;
  if (IsRaw) {
    doc=DocNew;
  } else {
    DocMax;
    old_preempt=Preempt(OFF);
    doc=DocCur;
  }
  if (fun)
    ClassRep2(doc,class_name,_b,0,max_depth,TRUE,HTT_FUN,offset);
  else
    ClassRep2(doc,class_name,_b,0,max_depth,TRUE,HTT_CLASS,offset);
  DocPutS(doc,"\n");
  DocRecalc(doc);
  if (IsRaw) {
    DocDump(doc,100000);
    DocDel(doc);
  } else
    Preempt(old_preempt);
}

U0 UpdateRegVarImg(CHashFun *tempf,U8 *_b,CTask *task)
{
  CMemberList *ml;
  CHashClass *tempc;
  ml=tempf->member_list;
  while (ml) {
    if (ml->register!=CREG_NONE) {
      tempc=ml->member_class;
      while (tempc->fwd_class)
	tempc=tempc->fwd_class;
      MemCpy(_b+ml->offset,TaskRegAdd(task,ml->register),tempc->size);
    }
    ml=ml->next;
  }
}

public U0 FunRep(U8 *st,U8 *rbp=NULL,I64 max_depth=2,CTask *task=NULL)
//Shows names and vals of a fun's
//local vars using info the compiler
//has.
{
  I64 size;
  U8 *img;
  CHashFun *tempf=HashFind(st,Fs->hash_table,HTT_FUN);
  CMemberList *tempm;
  if (tempf) {
    if (rbp) {
      if (task) {
	//tempf->size is negative.  It's the bottom
	//of the fun local var space relative to RBP .
	size=tempf->arg_cnt*8-tempf->size+16;

	img=MAlloc(size);
	MemCpy(img,rbp+tempf->size,size);
	UpdateRegVarImg(tempf,img-tempf->size,task);
	ClassRep(st,img-tempf->size,max_depth,TRUE,rbp-img+tempf->size);
	Free(img);
      } else
	ClassRep(st,rbp,max_depth,TRUE);
    } else {
      tempm=tempf->member_list;
      while (tempm) {
	if (0<=tempm->register<NUM_REGS)
	  "%08Z %s\n",tempm->register,"ST_U64_REGS",tempm->str;
	else
	  "%08tX %s\n",tempm->offset,tempm->str;
	tempm=tempm->next;
      }
      "%08tX Stk Size\n",tempf->size;
    }
  }
}

public U0 Uf(U8 *st)
{//Unassemble named fun
  I64 i;
  CHashFun *tempf=HashFind(st,Fs->hash_table,HTT_FUN);
  CDbgInfo *dbg;
  if (tempf) {
    FunRep(st);
    if (dbg=tempf->debug) {
      i=dbg->body[dbg->max_line-dbg->min_line+1]-tempf->executable_addr;
      Un(tempf->executable_addr,i);
      "Code Size:%04X\n",i;
    } else
      U(tempf->executable_addr);
  }
}

#help_index "Debugging/Breakpoints"
CBpt *BptFind(CTask *task,U8 *a,Bool rem=FALSE)
{//Rems
  CBpt *result=NULL,*tempb,*tempb1,*tempb2;
  PushFD;
  Cli;
  tempb1=&task->bpt_list;
  tempb=task->bpt_list;
  while (tempb) {
    tempb2=tempb->next;
    if (tempb->addr==a) {
      result=tempb;
      if (rem)
	tempb1->next=tempb2;
      else
	tempb1=&tempb->next;
    } else
      tempb1=&tempb->next;
    tempb=tempb2;
  }
  PopFD;
  return result;
}

public Bool BptS(U8 *a,CTask *task=NULL,Bool live=TRUE)	//set Bpt
//Sets a breakpoint
//Honestly, I never use this.  I put
//output statements to debug, partly
//because breakpoints don't work well.
{
  CBpt *tempb;
  Bool result=TRUE;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (!(tempb=BptFind(task,a,FALSE))) {
    tempb=CAlloc(sizeof(CBpt),task);
    tempb->addr=a;
    tempb->val=*a;
    result=FALSE;
    tempb->next=task->bpt_list;
    task->bpt_list=tempb;
    if (task==Fs && live)
      *a=OC_BPT;
  }
  PopFD;
  return result;
}

U0 CtrlAltD(I64)
{
  if (!IsInDbg) {
    if (Fs==Gs->idle_task)
      BptS(sys_winmgr_task->rip,sys_winmgr_task);
    else
      BptS(*ctrl_alt_let_ret_addr);
  }
}
SetCtrlAltLetCB('D',&CtrlAltD,"Sys/Enter Debugger",TRUE);

public Bool BptR(U8 *a,CTask *task=NULL,Bool live=TRUE,Bool rem=TRUE)
//Rems one breakpoint
{
  CBpt *tempb;
  Bool result=FALSE;
  if (!task) task=Fs;
  PushFD;
  Cli;
  if (tempb=BptFind(task,a,rem)) {
    if (task==Fs && live)
      *tempb->addr=tempb->val;
    result=TRUE;
    if (rem)
      Free(tempb);
  }
  PopFD;
  return result;
}

public U0 BptsR(CTask *task=NULL,Bool live=TRUE)	//clear Bpts
//Rems all breakpoints
{
  CBpt *tempb,*tempb1;
  if (!task) task=Fs;
  PushFD;
  Cli;
  tempb=task->bpt_list;
  task->bpt_list=NULL;
  while (tempb) {
    tempb1=tempb->next;
    if (task==Fs && live)
      *tempb->addr=tempb->val;
    Free(tempb);
    tempb=tempb1;
  }
  PopFD;
}

public U0 Go2(CTask *task=NULL)
//Do a $LK,"Step","MN:Step"$() first after a Bpt.
{
  if (!task) task=Fs;
  if (BptFind(task,task->rip))
    "\nDo one of the following, first:\n"
    ">Step;\t\t\t//Single step.\n"
    ">BptsR;\t\t\t//Reset (clear) all break points.\n\n"
    "After resuming, <CTRL-ALT-T> changes focus task.\n"
    "After resuming, <CTRL-ALT-F> flushes screen VGA cache.\n\n";
  else {
    LBtr(&task->task_flags,TASKf_DISABLE_BPTS);
    LBtr(&task->rflags,RFLAGS_SINGLE_STEP);
    Suspend(task,FALSE);
    if (task==Fs)
      Go1;
    else
      Exit;
  }
}

public U0 Go3(CTask *task=NULL)
//Rems all breakpoints and go.
{
  if (!task) task=Fs;
  BptsR(task);
  WinFocus(sys_dbg_focus_task);
  VGAFlush;
  Go2(task);
}

public U0 Step(CTask *task=NULL)
//Execute single assembly instruction.
{
  if (!task) task=Fs;
  LBts(&task->task_flags,TASKf_DISABLE_BPTS);
  LBts(&task->rflags,RFLAGS_SINGLE_STEP);
  Suspend(task,FALSE);
  if (task==Fs)
    Go1;
  else
    Exit;
}

#help_index ""
