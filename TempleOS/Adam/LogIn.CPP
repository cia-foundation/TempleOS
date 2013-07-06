#help_index "Task/Login"

//This is temporary until defined in $LK-A,"HOME/Adam3fSys.CPP"$.
U0 UserStartUp()
{ //Run each time a user a spawned
  UseTermDoc;
  Type("::/TempleOS/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
}

//This is temporary until defined in $LK-A,"HOME/Adam3fSys.CPP"$.
U0 SrvStartUp()
{ //Run each time a srv task is spawned.
  UseTermDoc;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
}

U8 *PopUpSysAcctGet()
{
  U8 *result,*st;
  CDoc *doc=DocNew;
  CDirEntry	*tempm1=FilesFind("/TempleOS/Accts/*",FUF_RECURSE|FUF_JUST_DIRS|FUF_FLATTEN_TREE),
		*tempm=tempm1;
  DocPutS(doc,"$$CM+CX-LE,8$$");
  DocPutSprite(doc,$IB+BP,"<1>",1,"::/TempleOS/Demo/Graphics/Logo.CPP,1"$,"$$SP+SIF,\"\",%d$$");
  DocPutS(doc,"$$CM+LX,0,14$$Select Account.\nCursor down and press <SPACE BAR>.\n");
  while (tempm) {
    if (*tempm->name!='.') {
      st=tempm->full_name;
      if (!StrNCmp(st+2,"/TempleOS/Accts/",10))
	st+=12;
      DocPrintF(doc,"$$CM+LX,2,3$$$$BT,\"%s\",%d$$",st,tempm);
    }
    tempm=tempm->next;
  }
  DocPutS(doc,"\n\n\n\nLog-in as USER if you are new and get familiar\n"
	    "before you try to install.\n");
  do tempm=PopUpMenu(doc,DOF_WIN_MAX);
  while (tempm<0);

  DocDel(doc);
  result=AStrNew(tempm->full_name);
  CDirLstDel(tempm1);
  return result;
}

U0 SysLogin()
{
  U8 *st;
  if (!FileFind(sys_acct)||!StrCmp(sys_acct+2,"/TempleOS/Accts")) {
    st=sys_acct;
    sys_acct=PopUpSysAcctGet;
    Free(st);
  }
}

#define ACCT_REGISTRY_FILENAME "HOME/Registry.CPP.Z"
CDoc *sys_registry_doc=NULL;
I64 sys_msg_flags[1]={0};
F64 registry_version;

Bool AcctCacheRegistry()
{
  Bool old_silent;
  if (!sys_registry_doc) {
    old_silent=Silent(ON);
    sys_registry_doc=DocRead(ACCT_REGISTRY_FILENAME);
    Silent(old_silent);
    return FALSE;
  } else
    return TRUE;
}

public Bool AcctRegSetDftEntry(U8 *path,U8 *val,Bool is_adam_entry=FALSE)
{
  Bool result,unlock_doc;
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (!DocTreeFind(sys_registry_doc,path)) {
    DocTreeMake(sys_registry_doc,path);
    DocPutS(sys_registry_doc,val);
    if (is_adam_entry) {
      if (Fs==adam_task)
	ExePutS(val);
      else
	Adam("%s",val);
    }
    if (IsDrvWritable(*sys_acct))
      DocWrite(sys_registry_doc);
    result=FALSE;
  } else
    result=TRUE;
  if (unlock_doc)
    DocUnlock(sys_registry_doc);
  return result;
}

public I64 AcctRegExeBranch(U8 *path)
{
  AcctCacheRegistry;
  return DocTreeBranchExe(sys_registry_doc,path);
}

public Bool AcctRegWriteBranch(U8 *path,U8 *fmt,...)
{
  Bool result,unlock_doc;
  CDocEntry *tree_branch,*start_indent,*end_indent;
  U8 *buf=SPrintFJoin(NULL,fmt,argc,argv);
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (result=DocTreeFind(sys_registry_doc,path,&tree_branch,&start_indent,&end_indent))
    DocCut(sys_registry_doc,tree_branch,end_indent);
  DocTreeMake(sys_registry_doc,path);
  DocPutS(sys_registry_doc,buf);
  if (IsDrvWritable(*sys_acct))
    DocWrite(sys_registry_doc);
  if (unlock_doc)
    DocUnlock(sys_registry_doc);
  Free(buf);
  return result;
}
 
U0 AcctOneTimePopUp(I64 flag_num,U8 *msg)
{ //You're not supposed to make system pop-up flags, only me.
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  if (!Bt(sys_msg_flags,flag_num)) {
    if (msg) DocPutS(doc,msg);
    doc_e=DocPutS(doc,"\n$$CB+LE,\"Do not show this msg again.\",1$$");
    DocPutS(doc,"$$CM+CX,0,4$$$$BT,\"OKAY\",1$$\n");
    if (PopUpMenu(doc)==1 && doc_e->flags1&DOCEF1_CHECKED_COLLAPSED) {
      LBts(sys_msg_flags,flag_num);
      AcctRegWriteBranch("Adam/TempleOS/SysMsgFlags",
	"sys_msg_flags[0]=0x%X;\n",
	sys_msg_flags[0]);
    }
    DocDel(doc);
  }
}

U0 RegistryInit()
{
  U8 buf[136];
  Bool version_present;
  AcctRegSetDftEntry("Adam/TempleOS/SysMsgFlags","sys_msg_flags[0]=0;\n",TRUE);
  SPrintF(buf,"registry_version=%4.3f;\n",os_version);
  version_present=AcctRegSetDftEntry("Adam/TempleOS/SysRegVer",buf,TRUE);
  AcctRegExeBranch("Adam");
  if (registry_version!=os_version) {
    AcctRegWriteBranch("Adam/TempleOS/SysRegVer",buf);
    AcctRegExeBranch("Adam");
  }
}

#help_index ""
