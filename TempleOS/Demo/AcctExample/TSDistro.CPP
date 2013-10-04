//The CFG defines are $LK,"Here","FI:::/TempleOS/Demo/AcctExample/TSAdam.CPP"$.

U0 ShowBootFileDate(U8 drv_let)
{
  CDirEntry de;
  U8 *name=MPrint("%c:/Kernel.BIN.C",drv_let);
  if (FileFind(name,&de)) {
    Free(de.full_name);
    "%s %D %T\n",name,de.datetime,de.datetime;
  } else
    "%s\n",name;
  Free(name);
}

U0 ShowBootFiles()
{
  U8 *ptr=TS_HDS;
  while (*ptr)
    ShowBootFileDate(*ptr++);
}

U8 TSPmtDrv()
{
  I64 result;
  ShowBootFiles;
  "Drive:";
  result=ToUpper(GetChar);
  NewLine;
  return result;
}

U0 TSMakeOSInstallPrtBootLoader(U8 drv_let)
{
  drv_let=ToUpper(drv_let);
  Auto(TS_CFG);
  MakeOSInstallPrtBootLoader(drv_let);
  if (StrOcc(TS_MASTER_BOOT_DRVS,drv_let))
    InstallMasterBootLoader(drv_let);
}

U0 SoftBootMenu(U8 drv_let=0)
{
  if (!drv_let)
    drv_let=Prt2Drv(Fs->cur_prt);
  Drv(drv_let);
  Cd("/TempleOS/Kernel");
  Auto(TS_MENU_CFG);
  if (!Cmp("Kernel","Kernel",NULL,':'))
    SoftBoot("Kernel.BIN.Z");
}

U0 TSCopyDrv(U8 src,U8 dst)
{
  U8 buf_s[128],buf_d[128];
  src=ToUpper(src);
  dst=ToUpper(dst);

  Fmt(dst,TRUE,FALSE,PT_FAT32);

  SPrint(buf_s,"%c:/TempleOS",src);
  SPrint(buf_d,"%c:/TempleOS",dst);
  CopyTree(buf_s,buf_d);

  SPrint(buf_s,"%c:/Windows",src);
  SPrint(buf_d,"%c:/Windows",dst);
  CopyTree(buf_s,buf_d);

  SPrint(buf_s,"%c:/Linux",src);
  SPrint(buf_d,"%c:/Linux",dst);
  CopyTree(buf_s,buf_d);

  SPrint(buf_s,"%c:/0000Boot",src);
  SPrint(buf_d,"%c:/0000Boot",dst);
  CopyTree(buf_s,buf_d);

  SPrint(buf_d,"%c:/Temp",dst);
  MkDir(buf_d);

  SPrint(buf_d,"%c:/Temp/ScreenShots",dst);
  MkDir(buf_d);

  DocClear;
  Drv(dst);
  TSMakeOSInstallPrtBootLoader(dst);
}

U0 TSPmtCopyDrv()
{
  U8 src,dst;
  "$$FG,RED$$\nCopy Src Drive:\n$$FG$$";
  src=TSPmtDrv;
  "$$FG,RED$$\nCopy Dst Drive:\n$$FG$$";
  dst=TSPmtDrv;
  TSCopyDrv(src,dst);
}

public U0 FileCursorRem(U8 *filename)
{
  CDoc *doc;
  doc=DocRead(filename,DOCF_PLAIN_TEXT|DOCF_NO_CURSOR);
  DocWrite(doc);
  DocDel(doc);
}

public U0 FileCursorTop(U8 *filename)
{
  CDoc *doc;
  doc=DocRead(filename,DOCF_PLAIN_TEXT);
  doc->cur_entry=doc->root.next;
  DocWrite(doc);
  DocDel(doc);
}

public U0 FilesCursorRem(U8 *mask="*")
{
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F");
  CDirEntry *tempde=FilesFind(mask,fuf_flags),*tempde1;
  while (tempde) {
    tempde1=tempde->next;
    FileCursorRem(tempde->full_name);
    Touch(tempde->full_name,"",,tempde->datetime);
    DirEntryDel(tempde);
    tempde=tempde1;
  }
}

public U0 FilesCursorTop(U8 *mask="*")
{
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F");
  CDirEntry *tempde=FilesFind(mask,fuf_flags),*tempde1;
  while (tempde) {
    tempde1=tempde->next;
    FileCursorTop(tempde->full_name);
    Touch(tempde->full_name,"",,tempde->datetime);
    DirEntryDel(tempde);
    tempde=tempde1;
  }
}

U0 UncollapseFile(U8 *filename,I64 *)
{
  CDoc *doc=DocRead(filename,DOCF_NO_CURSOR);
  DocCollapse(FALSE,doc);
  DocWrite(doc);
  DocDel(doc);
}

U0 Uncollapse(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+T");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde && !Bt(&fuf_flags,FUf_CANCEL)) {
    UncollapseFile(tempde->full_name,&fuf_flags);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

public U0 CopySongs(U8 threshold='7')
{
  CDoc *s;
  CDocEntry *doc_e;
  CDirEntry *tempde,*tempde1;
  DelTree("/TempleOS/Apps/MusicOrgan/Examples");
  MkDir("/TempleOS/Apps/MusicOrgan/Examples");
  tempde1=tempde=FilesFind("/TempleOS/Accts/TS/TAD/MusicOrgan/*.CPP*");
  while (tempde1) {
    s=DocRead(tempde1->full_name);
    doc_e=s->root.next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[2]>=threshold)
      Copy(tempde1->full_name,"/TempleOS/Apps/MusicOrgan/Examples");
    tempde1=tempde1->next;
  }
  DirLstDel(tempde);
}

U0 DistroPrep()
{
  Del("/TempleOS/Demo/WebLogExample/WebLog.TXT*");
  Del("/TempleOS/Demo/Games/EagleDive.BIN*");
  DelTree("/TempleOS/Accts/*Temp.TXT.Z");
  CopySongs;

//User
  Del("/TempleOS/Accts/User/Registry.CPP.Z");

//Install
  Del("/TempleOS/Accts/Install/Registry.CPP.Z");

//SafeMode
  Del("/TempleOS/Accts/SafeMode/Registry.CPP.Z");

  Touch("/TempleOS/Accts/PersonalMenu.TXT.Z","+T");
//TS
  Touch("/TempleOS/Accts/TS/PersonalMenu.TXT.Z","+T");

  DelTree("/TempleOS/Demo/AcctExample");
  CopyTree("/TempleOS/Accts/TS","/TempleOS/Demo/AcctExample");
  DelTree("/TempleOS/Demo/AcctExample/TAD/MusicOrgan");
  DelTree("/TempleOS/Demo/AcctExample/Wb2");
  Del("/TempleOS/Demo/AcctExample/TAD/Test*");

  Cd("/TempleOS");
  FilesCursorTop;

  DelTree("/TempleOS/Demo/*.BI*");

  Cd("/TempleOS");
  S2T("*","+r+S");
  DocOpt;
  Move(WSD_DEF_FILENAME,WSD_DEF_FILENAME_Z);
}

U0 MakeSymFiles()
{
  CDoc *doc;
  U8 *st;
  if (ts_has_bak_drv) {
    DocClear;
    DocMax;
    Who;
    if (doc=DocPut) {
      st=StrNew(doc->filename.name);
      StrCpy(doc->filename.name,TS_BAK_DRV ":/Wb/Accts/TS/Wb2/SymsAlpha.TXT.Z");
      DocWrite(doc);
      StrCpy(doc->filename.name,st);
      Free(st);
    }
    DocClear;
    DocMax;
    Who("+m");
    if (doc=DocPut) {
      st=StrNew(doc->filename.name);
      StrCpy(doc->filename.name,TS_BAK_DRV ":/Wb/Accts/TS/Wb2/SymsAddress.TXT.Z");
      DocWrite(doc);
      StrCpy(doc->filename.name,st);
      Free(st);
    }
    DocClear;
  }
}

I64 MakeLineCntFile()
{
  U8 *st;
  CDoc *doc;
  I64 result;
  CDocEntry *tree_branch,*start_indent,*end_indent;

  DocClear;
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;

  Cd(TS_MAIN_DRV ":/TempleOS");
  CopyTree(TS_MAIN_DRV ":/TempleOS/Accts/TS",TS_MAIN_DRV ":/Temp/TS");
  DelTree(TS_MAIN_DRV ":/TempleOS/Accts/TS");

  DocMax;
  DocClear;
  result=LineCntRep;
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,TS_MAIN_DRV ":/Temp/TS/Wb2/LineCnt.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  CopyTree(TS_MAIN_DRV ":/Temp/TS",TS_MAIN_DRV ":/TempleOS/Accts/TS");
  DelTree(TS_MAIN_DRV ":/Temp/TS");
  DocClear;

  doc=DocRead(TS_MAIN_DRV ":/TempleOS/Adam/DocDefines.CPP.Z");
  DocTreeFind(doc,"LineCnt",&tree_branch,&start_indent,&end_indent);
  DocCut(doc,start_indent->next,end_indent->last);
  doc->cur_entry=start_indent->next;
  doc->cur_data_col=0;
  DocPrint(doc,"DefinePrint(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",result);

  st=MPrint("%,d",result);
  DefineLoad("DD_TEMPLEOS_LOC",st);
  Free(st);

  DocWrite(doc);
  DocDel(doc);

  "Total LOC:%12,d\n\n",result;
  return result;
}

U0 TSDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(TS_CDROM_CFG);
  MakeOSInstallPrtBootLoader(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/TempleOS",TS_DISTRO_DIR "/TempleOS");
  CopyTree(TS_MAIN_DRV ":/Windows",TS_DISTRO_DIR "/Windows");
  CopyTree(TS_MAIN_DRV ":/Linux",TS_DISTRO_DIR "/Linux");
  MkDir(TS_DISTRO_DIR "/Temp");
  MkDir(TS_DISTRO_DIR "/Temp/ScreenShots");
  CopyTree(TS_MAIN_DRV ":/0000Boot",TS_DISTRO_DIR "/0000Boot");
}

U0 StdDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSInstallPrtBootLoader(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/TempleOS",TS_DISTRO_DIR "/TempleOS");
  CopyTree(TS_MAIN_DRV ":/Windows",TS_DISTRO_DIR "/Windows");
  CopyTree(TS_MAIN_DRV ":/Linux",TS_DISTRO_DIR "/Linux");
  MkDir(TS_DISTRO_DIR "/Temp");
  DelTree(TS_DISTRO_DIR "/TempleOS/Accts/TS");
  CopyTree(TS_MAIN_DRV ":/0000Boot",TS_DISTRO_DIR "/0000Boot");
  Del(TS_DISTRO_DIR "/0000Boot/OldMBR.BIN.C");
  Del(TS_DISTRO_DIR "/0000Boot/BootMHD2.BIN.C");
}

U0 MakeStdDistro()
{
  StdDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeTSDistro()
{
  TSDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 TSWebFile(U8 *name)
{
  U8 *name2;
  CDirEntry de;
  if (FileFind(name,&de)) {
    Free(de.full_name);
    "File:%s\n",name;
    UncollapseFile(name,NULL);
    Touch(name,"",,de.datetime);
    name2=ChgExt(name,"html");
    ToHtml(name,name2,
	"<title>The Temple Operating System</title>\n"
	"<meta name=\"keywords\" content=\"Operating System,64-Bit,64 Bit,Temple,OS,TempleOS,Free,Open Source,Public Domain,x86_64\">\n"
	);
    Touch(name2,"",,de.datetime);
    Free(name2);
  }
}

U0 MakeWebSite()
{
  CDirEntry *tempde,*tempde2;
  if (ts_has_bak_drv) {
    DocClear;
    Cd(TS_BAK_DRV ":/Wb");
    DelTree(TS_BAK_DRV ":/Wb/Accts/TS/TAD");
    Del(TS_BAK_DRV ":/Wb/Accts/TS/*");
    Touch(TS_BAK_DRV ":/Wb/Accts/TS/Wb2/index.TXT.Z","",,GetDateTime);
    tempde=tempde2=FilesFind("*",FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT);
    while (tempde) {
      TSWebFile(tempde->full_name);
      tempde=tempde->next;
    }
    DirLstDel(tempde2);
    Drv(TS_MAIN_DRV_LET);
  }
}

U0 MakeWebSitePartial()
{
  CDirEntry *tempde,*tempde2;
  if (ts_has_bak_drv) {
    DelTree(TS_BAK_DRV ":/Wb/Accts/TS/Wb2");
    CopyTree(TS_MAIN_DRV ":/TempleOS/Accts/TS/Wb2",
	     TS_BAK_DRV ":/Wb/Accts/TS/Wb2");
    DelTree(TS_BAK_DRV ":/Uncompressed/Accts/TS/Wb2");
    CopyTree(TS_MAIN_DRV ":/TempleOS/Accts/TS/Wb2",
	     TS_BAK_DRV ":/Uncompressed/Accts/TS/Wb2");
    tempde=tempde2=FilesFind(TS_BAK_DRV ":/Wb/Accts/TS/Wb2/*",FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT);
    while (tempde) {
      TSWebFile(tempde->full_name);
      tempde=tempde->next;
    }
    DirLstDel(tempde2);
    Drv(TS_MAIN_DRV_LET);
  }
}

U0 ToWeb(U8 *name)
{
  U8 *st,*st2;
  if (ts_has_bak_drv) {
    st=FileNameAbs(name);
    if (!StrNCmp(st+1,":/TempleOS/",12)) {
      st2=MPrint(TS_BAK_DRV":/Wb/%s",st+13);
      "%s\n",st2;
      Copy(name,st2);
      TSWebFile(st2);
      Free(st2);
    }
    Free(st);
  }
}

U0 TSUncompress()
{
  U8 *st;
  CDirEntry *tempde,*tempde1;
  if (ts_has_bak_drv) {
    DelTree(TS_BAK_DRV ":/Uncompressed");
    CopyTree(TS_MAIN_DRV ":/TempleOS",TS_BAK_DRV ":/Uncompressed");
    tempde=FilesFind(TS_BAK_DRV ":/Uncompressed/*.Z",FUF_RECURSE|FUF_JUST_FILES);
    while (tempde) {
      tempde1=tempde->next;
      st=StrNew(tempde->full_name);
      StrLastRem(st,".");
      Move(tempde->full_name,st);
      Free(st);
      DirEntryDel(tempde);
      tempde=tempde1;
    }
    FilesCursorRem(TS_BAK_DRV ":/Uncompressed/*");
  }
}

U0 UpdateISODocDefines()
{
  I64 size;
  size=10*FileSize(TS_BAK_DRV ":/Wb/TempleOSCD.ISO")/1024/1024;
  DefinePrint("DD_TEMPLEOSCD_SIZE",
    "Download $TX+D,"TempleOS V1.05","DD_OS_NAME_VERSION"$ - Std. Distro (%0.1f MB)",0.1*size);
  size=10*FileSize(TS_BAK_DRV ":/Wb/TempleOSTS.ISO")/1024/1024;
  DefinePrint("DD_TEMPLEOSTS_SIZE",
    "Download $TX+D,"TempleOS V1.05","DD_OS_NAME_VERSION"$ - T.S. Company Internal Distro (%0.1f MB)",0.1*size);
}

UpdateISODocDefines;

U0 MakeDistrosAndBackUp()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  progress4=*progress4_desc=0;
  progress4_max=14;
  WordStat;
  WinMax;
  WinBorder;
  ChkAll; progress4++;
  if (ts_has_bak_drv) {
    MakeLineCntFile; progress4++;
    TSCopyDrv(TS_MAIN_DRV_LET,TS_BAK_DRV_LET); progress4++;
    DelTree (TS_BAK_DRV  ":/Wb"); progress4++;
    CopyTree(TS_MAIN_DRV ":/TempleOS",TS_BAK_DRV ":/Wb"); progress4++;
    DocClear;

    MakeTSDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSTS.ISO"); progress4++;

    MakeStdDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSCD.ISO"); progress4++;

    UpdateISODocDefines;

    Cd(TS_MAIN_DRV ":/TempleOS");
    if (LinkChk)
      throw;
    progress4++;
    MakeSymFiles; progress4++;
    MakeWebSite; progress4++;
    TSUncompress; progress4++;
  }
  Drv(TS_MAIN_DRV_LET);
  ChkAll; progress4++;
  progress4=progress4_max=0;
  SettingsPop;
}

public U0 TSMount()
{
  if (!sys_auto_mount_ide_cnt) {
    Auto(CFG_HARD_DRVS "\n");
    Mount;
  }
}

U0 TSCopy()
{
  if (ts_cid==CID_DFT) {//Dangerous! don't do on my main machine
    DelTree(TS_MAIN_DRV ":/TempleOS");
    CopyTree(TS_CDROM_DRV ":/",TS_MAIN_DRV ":/");
    TSMakeOSInstallPrtBootLoader(TS_MAIN_DRV_LET);
    DelTree(TS_BAK_DRV ":/TempleOS");
    CopyTree(TS_CDROM_DRV ":/",TS_BAK_DRV ":/");
    TSMakeOSInstallPrtBootLoader(TS_BAK_DRV_LET);
    Reboot;
  }
}

U0 TSPrtDsk()
{
  CTask *task=SpawnUser;
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  if (ts_cid==CID_DFT) {//Dangerous! don't do on my main machine
    ATARep(FALSE,TRUE,&root);
    ATAIDDrvs(root,*sys_acct,&ata_drv,&atapi_drv);
    if (ata_drv) {
      XTalkWait(task,"PrtDsk;\nY%C2p%d\nYYY",TS_MAIN_DRV_LET,ata_drv->num);
      XTalkWait(task,"%d\nY\nY\n",(prt_dsk_dsk_size-PRT_HEADER)/2);
    }
    Kill(task);
  }
}


public U0 TSUpgrade()
{
  if (ts_cid==CID_DFT) {//Dangerous! don't do on my main machine
    TSMount;
    TSCopy;
  }
}

public U0 TSInstall()
{
  if (ts_cid==CID_DFT) {//Dangerous! don't do on my main machine
    TSPrtDsk;
    TSCopy;
  }
}
