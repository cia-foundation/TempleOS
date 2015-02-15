#help_index "Debugging/Dump"
Bool ClassRep2(CDoc *doc,U8 *_d,U8 *class_name=lastclass,I64 depth,I64 max_depth,Bool dynamic,I64 types=HTT_CLASS,I64 offset=0)
{//See $LK,"::/Demo/LastClass.CPP"$.
  I64 i,j,stars,*ptr;
  CMemberLst *ml;
  CDocEntry *doc_e;
  Bool unlock;
  CHashClass *tempc,*tempc2;
  if (depth>=max_depth) return TRUE;
  if (!(tempc=HashFind(class_name,Fs->hash_table,types))) {
    DocPrint(doc,"Class Not Found.\n");
    return FALSE;
  }
  if (!ChkPtr(_d) || !ChkPtr(_d(U8 *)+tempc->size)) {
    DocPrint(doc,"Bad Ptr:%016X\n",_d);
    return FALSE;
  }
  if (tempc->base_class && !ClassRep2(doc,_d,tempc->base_class->str,depth,max_depth,dynamic,types,offset))
    return FALSE;
  unlock=DocLock(doc);
  DocPrint(doc,"Class:\"%s\"\n",class_name);
  ml=tempc->member_lst;
  while (ml) {
    tempc2=ml->member_class;
    ptr=_d(U8 *)+ml->offset;
    DocPrint(doc,"%08X ",ptr(U8 *)+offset);
    stars=tempc2->ptr_stars_cnt;
    tempc2=OptClassFwd(tempc2);
    tempc2-=tempc2->ptr_stars_cnt;
    if (tempc2->type & HTT_INTERNAL_TYPE) {
      DocPrint(doc,"$$GREEN$$%-20ts:$$FG$$",ml->str);
      if (stars==1 && (tempc2->raw_type==RT_I8 || tempc2->raw_type==RT_U8)) {
	ptr=*ptr;
	if (ChkPtr(ptr)) {
	  if (dynamic) {
	    doc_e=DocPrint(doc,"$$DA-TRM-P+RD,64,A=\"%%40ts\"$$\n");
	    doc_e->data=ptr;
	    DocDataFmt(doc,doc_e);
	  } else
	    DocPrint(doc,"%40ts\n",ptr);
	}
      } else if (!stars) {
	j=MinI64(ml->dim.total_cnt,32);
	if (tempc2->raw_type==RT_I8 || tempc2->raw_type==RT_U8) {
	  if (j==1) {
	    if (dynamic) {
	      doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=U8,A=\"%%c\"$$\n");
	      doc_e->data=ptr;
	    } else
	      DocPrint(doc,"%c\n",*ptr(U8 *));
	  } else {
	    if (dynamic) {
	      doc_e=DocPrint(doc,"$$DA-TRM-P+RD,64,A=\"%%40ts\"$$\n");
	      doc_e->data=ptr;
	      DocDataFmt(doc,doc_e);
	    } else
	      DocPrint(doc,"%40ts\n",ptr);
	  }
	}
	for (i=0;i<j;i++) {
	  switch (tempc2->raw_type) {
	    case RT_I0:
	    case RT_U0:
	      break;
	    case RT_I8:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=I8,A=\"%%02X\"$$ ");
		doc_e->data=ptr(I8 *)++;
	      } else
		DocPrint(doc,"%02X ",*ptr(I8 *)++);
	      break;
	    case RT_U8:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=U8,A=\"%%02X\"$$ ");
		doc_e->data=ptr(U8 *)++;
	      } else
		DocPrint(doc,"%02X ",*ptr(U8 *)++);
	      break;
	    case RT_I16:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=I16,A=\"%%04X\"$$ ");
		doc_e->data=ptr(I16 *)++;
	      } else
		DocPrint(doc,"%04X ",*ptr(I16 *)++);
	      break;
	    case RT_U16:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=U16,A=\"%%04X\"$$ ");
		doc_e->data=ptr(U16 *)++;
	      } else
		DocPrint(doc,"%04X ",*ptr(U16 *)++);
	      break;
	    case RT_I32:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=I32,A=\"%%08X\"$$ ");
		doc_e->data=ptr(I32 *)++;
	      } else
		DocPrint(doc,"%08X ",*ptr(I32 *)++);
	      break;
	    case RT_U32:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=U32,A=\"%%08X\"$$ ");
		doc_e->data=ptr(U32 *)++;
	      } else
		DocPrint(doc,"%08X ",*ptr(U32 *)++);
	      break;
	    case RT_F64:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,RT=F64,A=\"%%16g\"$$ ");
		doc_e->data=ptr(F64 *)++;
	      } else
		DocPrint(doc,"%16g ",*ptr(I64 *)++);
	      break;
	    default:
	      if (dynamic) {
		doc_e=DocPrint(doc,"$$DA-TRM+RD+UD,32,A=\"%%016X\"$$ ");
		doc_e->data=ptr(I64 *)++;
	      } else
		DocPrint(doc,"%016X ",*ptr(I64 *)++);
	  }
	  if (dynamic)
	    DocDataFmt(doc,doc_e);
	}
	if (j<ml->dim.total_cnt)
	  DocPrint(doc,"...");
      } else
	DocPrint(doc,"%016X",*ptr);
      DocPrint(doc,"\n");
    } else {
      if (depth<2) {
	if (stars==1 && !ChkPtr(*ptr))
	  DocPrint(doc,"%-20ts:%016X\n",ml->str,*ptr);
	else {
	  DocPrint(doc,"$$TR,\"%s\"$$\n",ml->str);
	  DocPrint(doc,"$$ID,2$$");
	  if (!stars)
	    ClassRep2(doc,ptr,tempc2->str,depth+1,max_depth,dynamic);
	  else if (stars==1)
	    ClassRep2(doc,*ptr,tempc2->str,depth+1,max_depth,dynamic);
	  DocPrint(doc,"$$ID,-2$$");
	}
      } else
	DocPrint(doc,"%-20ts\n",ml->str);
    }
    ml=ml->next;
  }
  if (unlock)
    DocUnlock(doc);
  return TRUE;
}

public U0 ClassRep(U8 *_d,U8 *class_name=lastclass,I64 max_depth=2,Bool fun=FALSE,I64 offset=0)
{//Displays members of a record by using the compiler's info.
  CDoc *doc;
  Bool old_preempt;
  if (IsRaw) {
    doc=DocNew;
  } else {
    DocMax;
    old_preempt=Preempt(OFF);
    doc=DocPut;
  }
  if (fun)
    ClassRep2(doc,_d,class_name,0,max_depth,FALSE,HTT_FUN,offset);
  else
    ClassRep2(doc,_d,class_name,0,max_depth,FALSE,HTT_CLASS,offset);
  DocPrint(doc,"\n");
  DocRecalc(doc);
  if (IsRaw) {
    DocDump(doc,100000);
    DocDel(doc);
  } else
    Preempt(old_preempt);
}

public U0 ClassRepD(U8 *_d,U8 *class_name=lastclass,I64 max_depth=2,Bool fun=FALSE,I64 offset=0)
{//Dynamic ClassRep. Uses hex_ed widgit for live changes.
  CDoc *doc;
  Bool old_preempt;
  if (IsRaw) {
    doc=DocNew;
  } else {
    DocMax;
    old_preempt=Preempt(OFF);
    doc=DocPut;
  }
  if (fun)
    ClassRep2(doc,_d,class_name,0,max_depth,TRUE,HTT_FUN,offset);
  else
    ClassRep2(doc,_d,class_name,0,max_depth,TRUE,HTT_CLASS,offset);
  DocPrint(doc,"\n");
  DocRecalc(doc);
  if (IsRaw) {
    DocDump(doc,100000);
    DocDel(doc);
  } else
    Preempt(old_preempt);
}

U0 UpdateRegVarImg(CHashFun *tempf,U8 *_b,CTask *task)
{
  CMemberLst *ml;
  CHashClass *tempc;
  ml=tempf->member_lst;
  while (ml) {
    if (ml->reg!=REG_NONE) {
      tempc=OptClassFwd(ml->member_class);
      MemCpy(_b+ml->offset,TaskRegAddr(task,ml->reg),tempc->size);
    }
    ml=ml->next;
  }
}

public U0 FunRep(U8 *st,U8 *rbp=NULL,I64 max_depth=2,CTask *task=NULL)
{//Shows names and vals of a fun's local vars using compiler's info.
  I64 size;
  U8 *img;
  CHashFun *tempf=HashFind(st,Fs->hash_table,HTT_FUN);
  CMemberLst *tempm;
  if (tempf) {
    if (rbp) {
      if (task) {
	//tempf->size is negative.  It's the bottom
	//of the fun local var space relative to RBP .
	size=tempf->arg_cnt*8-tempf->size+16;

	img=MAlloc(size);
	MemCpy(img,rbp+tempf->size,size);
	UpdateRegVarImg(tempf,img-tempf->size,task);
	ClassRep(img-tempf->size,st,max_depth,TRUE,rbp-img+tempf->size);
	Free(img);
      } else
	ClassRep(rbp,st,max_depth,TRUE);
    } else {
      tempm=tempf->member_lst;
      while (tempm) {
	if (0<=tempm->reg<NUM_REGS)
	  "%08Z %s\n",tempm->reg,"ST_U64_REGS",tempm->str;
	else
	  "%08tX %s\n",tempm->offset,tempm->str;
	tempm=tempm->next;
      }
      "%08tX Stk Size\n",tempf->size;
    }
  }
}

#help_index "Debugging/Unassemble"
public U0 Uf(U8 *st)
{//Unassembles a named fun
  I64 i;
  CHashSrcSym *tempf;
  CDbgInfo *dbg_info;
  if (tempf=HashFind(st,Fs->hash_table,HTT_FUN|HTT_EXPORT_SYS_SYM)) {
    if (tempf->type&HTT_FUN)
      FunRep(st);
    if (dbg_info=tempf->dbg_info) {
      i=dbg_info->body[dbg_info->max_line+1-dbg_info->min_line]-dbg_info->body[0];
      Un(dbg_info->body[0],i);
      "Code Size:%04X\n",i;
    } else
      U(HashVal(tempf));
  }
}
