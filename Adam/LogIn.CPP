#help_index "Misc/Accts"

U0 UserStartUp()
{//Temporary until defined in $LK,"HOME/Adam3eSys.CPP"$.
//Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
}

U0 SrvStartUp()
{//Temporary until defined in $LK,"HOME/Adam3eSys.CPP"$.
//Run each time a srv task is spawned.
  DocTermNew;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
}

U8 *PopUpSysAcct()
{
  U8 *result,*st;
  CDoc *doc=DocNew;
  CDirEntry	*tempde=FilesFind("/Accts/*",FUF_RECURSE|FUF_JUST_DIRS|FUF_FLATTEN_TREE),
		*tempde1=tempde;
  DocPrint(doc,"$$CM+CX-LE,10$$");
  DocSprite(doc,$IB,"<1>",BI=1,BP="::/Misc/StockSprites.TXT,TempleOSLogo"$,"$$SP+SIF,\"\",BI=%d$$");
  DocPrint(doc,"$$CM+LX,0,12$$Select Account.\nCursor down and press <SPACE>.\n");
  while (tempde) {
    if (*tempde->name!='.') {
      st=tempde->full_name;
      if (!StrNCmp(st+2,"/Accts/",7))
	st+=9;
      DocPrint(doc,"$$CM+LX,2,3$$$$BT,\"%s\",LE=%d$$",st,tempde);
    }
    tempde=tempde->next;
  }
  DocPrint(doc,"\n\n\n\nLog-in as USER if you are new and get familiar\n"
	    "before you try to install.\n");
  do tempde=PopUpMenu(doc,DOF_WIN_MAX);
  while (tempde<=0);

  DocDel(doc);
  result=AStrNew(tempde->full_name);
  DirTreeDel(tempde1);
  return result;
}

U0 SysLogin()
{
  U8 *st;
  if (!FileFind(sys_acct)||!StrCmp(sys_acct+2,"/Accts")) {
    st=sys_acct;
    sys_acct=PopUpSysAcct;
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
{//Add code doc tree branch to registry.
  Bool result,unlock_doc;
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (!DocTreeFind(sys_registry_doc,path)) {
    DocTreeMake(sys_registry_doc,path);
    DocPrint(sys_registry_doc,"%s",val);
    if (is_adam_entry) {
      if (Fs==adam_task)
	ExePrint("%s",val);
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
{//Execute doc tree branch in registry.
  AcctCacheRegistry;
  return DocTreeBranchExe(sys_registry_doc,path);
}

public Bool AcctRegWriteBranch(U8 *path,U8 *fmt,...)
{//Rewrite doc tree branch in registry.
  Bool result,unlock_doc;
  CDocEntry *tree_branch,*start_indent,*end_indent;
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  AcctCacheRegistry;
  unlock_doc=DocLock(sys_registry_doc);
  if (result=DocTreeFind(sys_registry_doc,path,&tree_branch,&start_indent,&end_indent))
    DocCut(sys_registry_doc,tree_branch,end_indent);
  DocTreeMake(sys_registry_doc,path);
  DocPrint(sys_registry_doc,"%s",buf);
  if (IsDrvWritable(*sys_acct))
    DocWrite(sys_registry_doc);
  if (unlock_doc)
    DocUnlock(sys_registry_doc);
  Free(buf);
  return result;
}
 
public Bool OneTimePopUp(U8 *_flags,I64 flag_num,U8 *msg)
{//See $LK,"::/Apps/X-Caliber/X-Caliber.CPP"$.
  Bool result=FALSE;
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  if (!Bt(_flags,flag_num)) {
    if (msg) DocPrint(doc,"%s",msg);
    doc_e=DocPrint(doc,"\n$$CB,\"Do not show this msg again.\",LE=1$$");
    DocPrint(doc,"$$CM+CX,0,4$$$$BT,\"OKAY\",LE=1$$\n");
    if (PopUpMenu(doc)==1 && doc_e->de_flags&DOCEF_CHECKED_COLLAPSED) {
      LBts(_flags,flag_num);
      result=TRUE;
    }
    DocDel(doc);
  }
  return result;
}

U0 AcctOneTimePopUp(I64 flag_num,U8 *msg)
{//You're not supposed to make system pop-up flags, only me.
  if (OneTimePopUp(sys_msg_flags,flag_num,msg))
    AcctRegWriteBranch("Adam/SysMsgFlags",
	"sys_msg_flags[0]=0x%X;\n",
	sys_msg_flags[0]);
}

U0 RegistryInit()
{
  U8 buf[136];
  Bool version_present;
  AcctRegSetDftEntry("Adam/SysMsgFlags","sys_msg_flags[0]=0;\n",TRUE);
  StrPrint(buf,"registry_version=%4.3f;\n",os_version);
  version_present=AcctRegSetDftEntry("Adam/SysRegVer",buf,TRUE);
  AcctRegExeBranch("Adam");
  if (registry_version!=os_version) {
    AcctRegWriteBranch("Adam/SysRegVer",buf);
    AcctRegExeBranch("Adam");
  }
}
