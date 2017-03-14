#help_index "DolDoc/Output;StdOut/DolDoc"
public CTask *PopUpViewDoc(CDoc *doc,I64 dof_flags=0)
{//Pass doc to PopUp win task for viewing.
  U8 *buf=MStrPrint("DocEd(0x%X,0x%X);",doc,dof_flags);
  CTask *task=Spawn(&SrvCmdLine,NULL,"View",,Fs);
  TaskExe(task,NULL,buf,1<<JOBf_EXIT_ON_COMPLETE|1<<JOBf_FREE_ON_COMPLETE);
  Free(buf);
  return task;
}

public CTask *PopUpViewPrint(U8 *fmt,...)
{//View Print stmt in PopUp win task.
  CTask *task=Spawn(&SrvCmdLine,NULL,"View",,Fs);
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  CDoc *doc=DocNew(,task);
  DocPrint(doc,buf);
  Free(buf);
  buf=MStrPrint("DocEd(0x%X);",doc);
  TaskExe(task,NULL,buf,1<<JOBf_EXIT_ON_COMPLETE|1<<JOBf_FREE_ON_COMPLETE);
  Free(buf);
  return task;
}

#help_index "DolDoc/Input;File/FileNames;StdIn/DolDoc"
public U8 *PopUpPickFile(U8 *dir=NULL)
{//Filename chooser.  Uses $LK,"FileMgr",A="MN:FileMgr"$().
  U8 *res,*st,*st2;
  if (dir)
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",dir);
  else {
    st2=DirCur;
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_FILE,Fs->parent_task);",st2);
    Free(st2);
  }
  res=PopUp(st,Fs);
  Free(st);
  return res;
}

public U8 *PopUpPickDir(U8 *dir=NULL)
{//File dir name chooser.  Uses $LK,"FileMgr",A="MN:FileMgr"$().
  U8 *res,*st,*st2;
  if (dir)
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",dir);
  else {
    st2=DirCur;
    st=MStrPrint("Cd(\"%Q\");FileMgr(FM_PICK_DIR,Fs->parent_task);",st2);
    Free(st2);
  }
  res=PopUp(st,Fs);
  Free(st);
  return res;
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
	dft,dof_flags|DOF_SIZE_MIN),*res=PopUp(st,Fs);
  Free(st);
  return res;
}

#help_index "DolDoc"
Bool PopUpCd()
{
  Bool res;
  U8 *st=PopUpPickDir;
  if (st) {
    res=Cd(st);
    Free(st);
  } else
    res=FALSE;
  return res;
}

#help_index "DolDoc/Input;Char/Lists;StdIn/DolDoc"
public I64 PopUpPickLst(U8 *lst)
{//Prompt for lst entry in PopUp win task.
  I64 res,i=0;
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
  res=PopUpMenu(doc);
  DocDel(doc);
  return res;
}

#help_index "DolDoc/Input;Char/Lists;Char/Define;StdIn/DolDoc"
public U8 *PopUpPickDefineSub(U8 *dname)
{//Prompt for $LK,"Define",A="HI:Define"$ lst entry in PopUp win task.
  return PopUpPickLst(Define(dname));
}

#help_index "DolDoc/Input;StdIn/DolDoc"
public I64 PopUp1(U8 *b1,I64 n1,U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task with one bttn.
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
{//Make PopUp win task with two bttns.
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
{//Make PopUp win task with OKAY bttn.
  return PopUp1("OKAY",1,header,footer)>0;
}

public Bool PopUpNoYes(U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task with NO/YES bttns.
  return $WW,0$PopUp2("YES",1,"NO",0,header,footer)>0;
}

public Bool PopUpCancelOk(U8 *header=NULL,U8 *footer=NULL)
{//Make PopUp win task CANCEL/OKAY bttns.
  return PopUp2("OKAY",1,"CANCEL",0,header,footer)>0;
}

U8 *PopUpGetStr2(U8 *header,CTask *mem_task)
{
  U8 *res,*st;
  if (header)
    "%s",header;
  st=GetStr(,,GSF_WITH_NEW_LINE);
  res=StrNew(st,mem_task);
  Free(st);
  return res;
}

public U8 *PopUpGetStr(U8 *header=NULL)
{//Prompt for text str in PopUp win task.
  U8 *st=MStrPrint("PopUpGetStr2(0x%X,0x%X);",header,Fs),
	*res=PopUp(st,Fs);
  Free(st);
  return res;
}

public I64 PopUpGetI64(U8 *msg,I64 dft,I64 lo=I64_MIN,I64 hi=I64_MAX)
{//Prompt for I64 text expression in PopUp win task.
  U8 *st=MStrPrint("GetI64(0x%X,0x%X,0x%X,0x%X);",msg,dft,lo,hi);
  I64 res=PopUp(st,Fs);
  Free(st);
  return res;
}

public F64 PopUpGetF64(U8 *msg,F64 dft,F64 lo=F64_MIN,F64 hi=F64_MAX)
{//Prompt for F64 text expression in PopUp win task.
  U8 *st=MStrPrint("GetF64(0x%X,0x%X(F64),0x%X(F64),0x%X(F64));",msg,dft,lo,hi);
  F64 res=PopUp(st,Fs)(F64);
  Free(st);
  return res;
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

#help_index "Job/Exe;Task/Job/Exe;Compiler"
public I64 AdamFile(U8 *filename,Bool warn_ext=TRUE)
{//Make adam_task execute file.
  Bool okay=TRUE;
  U8 *name=FileNameAbs(filename),
	*name2=ExtDft(name,"HC.Z");
  I64 res=0;
  if (warn_ext &&
	!FilesFindMatch(name2,FILEMASK_JIT) &&
	!PopUpCancelOk(ST_WARN_ST "Not .HC File\n\n"))
    okay=FALSE;
  if (okay)
    res=Adam("#include \"%s\";",name2);
  Free(name2);
  Free(name);
  return res;
}

public I64 PopUpFile(U8 *filename,Bool warn_ext=TRUE,
	CTask *parent=NULL,CTask **_pu_task=NULL)
{//$LK,"ExeFile2",A="MN:ExeFile2"$() in $LK,"PopUp",A="MN:PopUp"$ task. Cont as User.
  Bool okay=TRUE;
  U8 *st,*name=FileNameAbs(filename),
	*name2=ExtDft(name,"HC.Z");
  I64 res=0;
  if (warn_ext &&
	!FilesFindMatch(name2,FILEMASK_JIT) &&
	!PopUpCancelOk(ST_WARN_ST "Not .HC File\n\n"))
    okay=FALSE;
  if (okay) {
    st=MStrPrint(
	  "\"$$$$WW+H,1$$$$\";In(\"ExeFile2(\\\"%s\\\",CCF_CMD_LINE);\\n\");"
	  "UserTaskCont;",
	  name2);
    res=PopUp(st,parent,_pu_task);
    Free(st);
  }
  Free(name2);
  Free(name);
  return res;
}

public I64 PopUpRunFile(U8 *filename,I64 ccf_flags=0,...)
{//$LK,"ExeFile",A="MN:ExeFile"$() with args using $LK,"LastFun",A="MN:LastFun"$() in $LK,"PopUp",A="MN:PopUp"$ task.
  U8 *st,*name=FileNameAbs(filename),
	*name2=ExtDft(name,"HC.Z");
  I64 res=0;
  st=MStrPrint(
	"\"$$$$WW+H,1$$$$\";ExeFile2(\"%s\",0x%X);LastFun(0x%X,0x%X);",
	name2,ccf_flags,argc,argv);
  res=PopUp(st,Fs);
  Free(st);
  Free(name2);
  Free(name);
  return res;
}
