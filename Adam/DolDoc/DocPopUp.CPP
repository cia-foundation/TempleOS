#help_index "DolDoc/Output;StdOut/DolDoc"
public CTask *PopUpViewDoc(CDoc *doc,I64 dof_flags=0)
{//Pass doc to PopUp win task for viewing.
  U8 *buf=MStrPrint("DocEd(0x%X,0x%X);",doc,dof_flags);
  CTask *task=Spawn(&SrvCmdLine,NULL,"View",,Fs);
  TaskExe(task,NULL,buf,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
  Free(buf);
  return task;
}

U0 PopUpViewPrintEndCB()
{
  DocDel(FramePtr("ViewStrFrame"));
  Exit;
}

public CTask *PopUpViewPrint(U8 *fmt,...)
{//View Print statement in PopUp win task.
  CTask *task;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  CDoc *doc=DocNew;
  DocPrint(doc,buf);
  Free(buf);
  task=PopUpViewDoc(doc);
  FramePtrAdd("ViewStrFrame",doc,task);
  task->task_end_cb=&PopUpViewPrintEndCB;
  return task;
}

#help_index "DolDoc/Input;File/FileNames;StdIn/DolDoc"
public U8 *PopUpPickFile(U8 *dir=NULL)
{//Filename chooser.  Uses $LK,"FileMgr",A="MN:FileMgr"$().
  U8 *result,*st,*st2;
  if (dir)
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",st2);
    Free(st2);
  }
  result=PopUp(st,Fs);
  Free(st);
  return result;
}

public U8 *PopUpPickDir(U8 *dir=NULL)
{//File dir name chooser.  Uses $LK,"FileMgr",A="MN:FileMgr"$().
  U8 *result,*st,*st2;
  if (dir)
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",dir);
  else {
    st2=CurDir;
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",st2);
    Free(st2);
  }
  result=PopUp(st,Fs);
  Free(st);
  return result;
}

public U8 *FileNameForm(U8 *dft=NULL,I64 dof_flags=0,CTask *mem_task=NULL)
{//Text filename form in cur win, not PopUp.
  CEdFileName fn;
  if (dft)
    StrCpy(fn.name,dft);
  else
    *fn.name=0;
  if (DocForm(&fn,,dof_flags))
    return StrNew(fn.name,mem_task);
  else
    return NULL;
}

public U8 *PopUpFileName(U8 *dft=NULL,I64 dof_flags=0)
{//Filename chooser. Uses form, not $LK,"FileMgr",A="MN:FileMgr"$().
  U8 *st=MStrPrint("FileNameForm(\"%Q\",0x%X,Fs->parent_task);",
	dft,dof_flags|DOF_MIN_SIZE),*result=PopUp(st,Fs);
  Free(st);
  return result;
}

#help_index "DolDoc"
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

#help_index "DolDoc/Input;Char/Lists;StdIn/DolDoc"
public U8 *PopUpPickLst(U8 *lst)
{//Prompt for lst entry in PopUp win task.
  I64 i=0;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$LTBLUE$$");
  while (*lst) {
    if (*lst=='@') {//Check for '@' alias lst entry
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
{//Prompt for $LK,"Define",A="HI:Define"$ lst entry in PopUp win task.
  return PopUpPickLst(Define(st));
}

#help_index "DolDoc/Input;StdIn/DolDoc"
public I64 PopUp1(U8 *b1,I64 n1,U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task with one button.
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
{//Make PopUp win task with two buttons.
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
{//Make PopUp win task with OKAY button.
  return PopUp1("OKAY",1,header,footer)>0;
}

public Bool PopUpNoYes(U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task with NO/YES buttons.
  return $WW,0$PopUp2("YES",1,"NO",0,header,footer)>0;
}

public Bool PopUpCancelOk(U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task CANCEL/OKAY buttons.
  return PopUp2("OKAY",1,"CANCEL",0,header,footer)>0;
}

U8 *PopUpGetStr2(U8 *header,CTask *mem_task)
{
  U8 *result,*st;
  if (header)
    "%s",header;
  st=GetStr(,,GSF_WITH_NEW_LINE);
  result=StrNew(st,mem_task);
  Free(st);
  return result;
}

public U8 *PopUpGetStr(U8 *header=NULL)
{//Prompt for text str in PopUp win task.
  U8 *st=MStrPrint("PopUpGetStr2(0x%X,0x%X);",header,Fs),
	*result=PopUp(st,Fs);
  Free(st);
  return result;
}

public I64 PopUpGetI64(U8 *msg,I64 dft,I64 lo=MIN_I64,I64 hi=MAX_I64)
{//Prompt for I64 text expression in PopUp win task.
  U8 *st=MStrPrint("GetI64(0x%X,0x%X,0x%X,0x%X);",msg,dft,lo,hi);
  I64 result=PopUp(st,Fs);
  Free(st);
  return result;
}

public F64 PopUpGetF64(U8 *msg,F64 dft,F64 lo=MIN_F64,F64 hi=MAX_F64)
{//Prompt for F64 text expression in PopUp win task.
  U8 *st=MStrPrint("GetF64(0x%X,0x%X(F64),0x%X(F64),0x%X(F64));",msg,dft,lo,hi);
  F64 result=PopUp(st,Fs)(F64);
  Free(st);
  return result;
}

public I64 PopUpRangeI64(I64 lo,I64 hi,I64 step=1,
	U8 *header=NULL,U8 *footer=NULL)
{//Evenly-spaced I64 range chooser in PopUp win task.
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
{//Evenly-spaced F64 range chooser in PopUp win task.
  F64 d;
  I64 i;
  U8 buf[STR_LEN];
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  DocPrint(doc,"$$LTBLUE$$");
  for (d=lo;d<=hi;d+=step) {
    StrPrint(buf,fmt,d);
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
{//Exp-spaced F64 range chooser in PopUp win task.
  F64 d;
  I64 i;
  U8 buf[STR_LEN];
  CDoc *doc=DocNew;
  if (header)
    DocPrint(doc,"%s",header);
  DocPrint(doc,"$$LTBLUE$$");
  for (d=lo;d<=hi;d*=factor) {
    StrPrint(buf,fmt,d);
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
{//Log-spaced F64 range chooser in PopUp win task.
  return PopUpRangeF64Exp(lo,hi,Exp(Ln(hi/lo)/(steps-1)),fmt,header,footer);
}

#help_index "Task/Srv/Exe"
public I64 AdamFile(U8 *filename,Bool warn_ext=TRUE)
{//Tell adam task to compile and run a file.
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

public I64 PopUpFile(U8 *filename,Bool warn_ext=TRUE,
	CTask *parent=NULL,CTask **_pu_task=NULL)
{//Run code file in PopUp win task.
  Bool okay=TRUE;
  U8 *st,*name=FileNameAbs(filename),
	*name2=DftExt(name,"CPP.Z");
  I64 result=0;
  if (warn_ext &&
	!FilesFindMatch(name2,FILEMASK_JIT) &&
	!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n"))
    okay=FALSE;
  if (okay) {
    st=MStrPrint(
	  "\"$$$$WW+H,1$$$$\";Auto(\"ExeFile2(\\\"%s\\\");\\n\");UserTaskCont;",
	  name2);
    result=PopUp(st,parent,_pu_task);
    Free(st);
  }
  Free(name2);
  Free(name);
  return result;
}
