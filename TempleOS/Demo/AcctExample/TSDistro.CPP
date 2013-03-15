//The CFG defines are $LK,"Here","FI:::/TempleOS/Demo/AcctExample/TSAdam.CPP"$.

U0 ShowBootFileDate(U8 drv_let)
{
  CDirEntry de;
  U8 *name=MSPrintF("%c:/Kernel.BIN.C",drv_let);
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

  SPrintF(buf_s,"%c:/TempleOS",src);
  SPrintF(buf_d,"%c:/TempleOS",dst);
  CopyTree(buf_s,buf_d);

  SPrintF(buf_s,"%c:/Windows",src);
  SPrintF(buf_d,"%c:/Windows",dst);
  CopyTree(buf_s,buf_d);

  SPrintF(buf_s,"%c:/Linux",src);
  SPrintF(buf_d,"%c:/Linux",dst);
  CopyTree(buf_s,buf_d);

  SPrintF(buf_s,"%c:/0000Boot",src);
  SPrintF(buf_d,"%c:/0000Boot",dst);
  CopyTree(buf_s,buf_d);

  SPrintF(buf_d,"%c:/Temp",dst);
  MkDir(buf_d);

  SPrintF(buf_d,"%c:/Temp/ScreenShots",dst);
  MkDir(buf_d);

  if (ts_has_bak_drv && dst==TS_BAK_DRV_LET) {
    SPrintF(buf_d,"%c:/Logs",dst);
    MkDir(buf_d);
    SPrintF(buf_d,"%c:/Logs/TEMPLEOSLogs",dst);
    MkDir(buf_d);
  }
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
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F");
  CDirEntry *tempm=FilesFind(mask,fuf_flags),*tempm1;
  while (tempm) {
    tempm1=tempm->next;
    FileCursorRem(tempm->full_name);
    Touch(tempm->full_name,"",,tempm->datetime);
    CDirEntryDel(tempm);
    tempm=tempm1;
  }
}

public U0 FilesCursorTop(U8 *mask="*")
{
  I64 fuf_flags=0;
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F");
  CDirEntry *tempm=FilesFind(mask,fuf_flags),*tempm1;
  while (tempm) {
    tempm1=tempm->next;
    FileCursorTop(tempm->full_name);
    Touch(tempm->full_name,"",,tempm->datetime);
    CDirEntryDel(tempm);
    tempm=tempm1;
  }
}

U0 UncollapseFile(U8 *filename,I64 *)
{
  CDoc *doc=DocRead(filename,DOCF_NO_CURSOR);
  DocCollapse(FALSE,doc);
  DocWrite(doc);
  DocDel(doc);
}

U0 UncollapseLst(CDirEntry *tempm1,I64 *flags)
{
  CDirEntry *tempm2;
  while (tempm1 && !Bt(flags,FUf_CANCEL)) {
    tempm2=tempm1->next;
    if (tempm1->attr & _ATTR_DIR) {
      if (tempm1->sub) {
	"%S %s\n","ST_SCANNING_DIR",tempm1->full_name;
	UncollapseLst(tempm1->sub,flags);
      }
    } else
      UncollapseFile(tempm1->full_name,flags);
    CDirEntryDel(tempm1);
    tempm1=tempm2;
  }
}

U0 Uncollapse(U8 *files_find_mask=FILENAME_MASK_TXT,U8 *fu_flags=NULL)
{
  I64 f=0;
  GetFFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  UncollapseLst(FilesFind(files_find_mask,f),&f);
}

U0 CopySongDir(U8 *files_find_mask,U8 threshold)
{
  CDoc *s;
  CDocEntry *doc_e;
  CDirEntry *tempm,*tempm1;
  tempm1=tempm=FilesFind(files_find_mask);
  while (tempm1) {
    s=DocRead(tempm1->full_name);
    doc_e=s->root.next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[2]>=threshold)
      Copy(tempm1->full_name,"/TempleOS/Apps/MusicOrgan/Examples");
    tempm1=tempm1->next;
  }
  CDirLstDel(tempm);
}

public U0 CopySongs(U8 threshold='7')
{
  DelTree("/TempleOS/Apps/MusicOrgan/Examples");
  MkDir("/TempleOS/Apps/MusicOrgan/Examples");
  CopySongDir("/TempleOS/Accts/TS/TAD/MusicOrgan/*.CPP*",threshold);
}

U0 DistroPrep()
{
  Del("/TempleOS/Demo/WebLogExample/WebLog.TXT*");
  Del("/TempleOS/Demo/GameStarters/EagleDive.BIN*");
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

  DelTree("/TempleOS/Demo/*.BI?");

  Cd("/TempleOS");
  S2T(FILENAME_MASK_SRC,"+r");
  DocOpt;
  Move(WSD_DEF_FILENAME,WSD_DEF_FILENAME_Z);
}

U0 TSDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(TS_CDROM_CFG);
  MakeOSInstallPrtBootLoader(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  Fmt(TS_IMG_DRV_LET,TRUE,FALSE,PT_NATIVE);
  Drv(TS_IMG_DRV_LET);
  CopyTree(TS_MAIN_DRV ":/TempleOS","/TempleOS");
  CopyTree(TS_MAIN_DRV ":/Windows","/Windows");
  CopyTree(TS_MAIN_DRV ":/Linux","/Linux");
  MkDir("/Temp");
  MkDir("/Temp/ScreenShots");
  CopyTree(TS_MAIN_DRV ":/0000Boot","/0000Boot");
}

U0 MakeSymFiles()
{
  CDoc *doc;
  U8 *st;
  if (ts_has_bak_drv) {
    DocClear;
    DocMax;
    HashRep(Fs->hash_table,HTG_ALL,NULL,"-p",0,NULL);
    if (doc=DocCur) {
      st=StrNew(doc->filename.name);
      StrCpy(doc->filename.name,TS_BAK_DRV ":/Wb/Accts/TS/Wb2/SymsAlpha.TXT.Z");
      DocWrite(doc);
      StrCpy(doc->filename.name,st);
      Free(st);
    }
    DocClear;
    DocMax;
    HashRep(Fs->hash_table,HTG_ALL,NULL,"-p+m",0,NULL);
    if (doc=DocCur) {
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

  DocMax;
  DocClear;
  Cd(TS_MAIN_DRV ":/TempleOS");
  result=LineCntRep;
  if (doc=DocCur) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,TS_MAIN_DRV ":/TempleOS/Accts/TS/Wb2/LineCnt.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;

  doc=DocRead(TS_MAIN_DRV ":/TempleOS/Adam/DocDefines.CPP.Z");
  DocTreeFind(doc,"LineCnt",&tree_branch,&start_indent,&end_indent);
  DocCut(doc,start_indent->next,end_indent->last);
  doc->cur_entry=start_indent->next;
  doc->cur_data_col=0;
  DocPrintF(doc,"DefinePrintF(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",result);

  st=MSPrintF("%,d",result);
  DefineLoad("DD_TEMPLEOS_LOC",st);
  Free(st);

  DocWrite(doc);
  DocDel(doc);

  "Total LOC:%12,d\n\n",result;
  return result;
}

U0 StdDistroPrep(U8 *auto_cfg)
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(auto_cfg);
  MakeOSInstallPrtBootLoader(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  Fmt(TS_IMG_DRV_LET,TRUE,FALSE,PT_NATIVE);
  Drv(TS_IMG_DRV_LET);
  CopyTree(TS_MAIN_DRV ":/TempleOS","/TempleOS");
  CopyTree(TS_MAIN_DRV ":/Windows","/Windows");
  CopyTree(TS_MAIN_DRV ":/Linux","/Linux");
  MkDir("/Temp");
  DelTree("/TempleOS/Accts/TS");
  CopyTree(TS_MAIN_DRV ":/0000Boot","/0000Boot");
  Del("/0000Boot/OldMBR.BIN.C");
  Del("/0000Boot/BootMHD2.BIN.C");
}

U0 MakeTSDistro()
{
  TSDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_IMG_DRV ":/*",NULL,BOOTCD_BIN_Z);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeStdDistro()
{
  StdDistroPrep(STD_DISTRO_CDROM_CFG);
  CDFileCreate(TS_ISO_NAME,TS_IMG_DRV ":/*",NULL,BOOTCD_BIN_Z);
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
  CDirEntry *tempm,*tempm2;
  if (ts_has_bak_drv) {
    DocClear;
    Cd(TS_BAK_DRV ":/Wb");
    tempm=tempm2=FilesFind(FILENAME_MASK_TXT,FUF_RECURSE|FUF_FLATTEN_TREE|FUF_JUST_FILES);
    while (tempm) {
      TSWebFile(tempm->full_name);
      tempm=tempm->next;
    }
    CDirLstDel(tempm2);
    Drv(TS_MAIN_DRV_LET);
  }
}

U0 ToWeb(U8 *name)
{
  U8 *st,*st2;
  if (ts_has_bak_drv) {
    st=FileNameAbs(name);
    if (!StrNCmp(st+1,":/TempleOS/",12)) {
      st2=MSPrintF(TS_BAK_DRV":/Wb/%s",st+13);
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
  CDirEntry *tempm,*tempm1;
  if (ts_has_bak_drv) {
    DelTree(TS_BAK_DRV ":/Uncompressed");
    CopyTree(TS_MAIN_DRV ":/TempleOS",TS_BAK_DRV ":/Uncompressed");
    tempm=FilesFind(TS_BAK_DRV ":/Uncompressed/*.Z",FUF_RECURSE|FUF_FLATTEN_TREE|FUF_JUST_FILES);
    while (tempm) {
      tempm1=tempm->next;
      st=StrNew(tempm->full_name);
      StrLastRem(st,".");
      Move(tempm->full_name,st);
      Free(st);
      CDirEntryDel(tempm);
      tempm=tempm1;
    }
    FilesCursorRem(TS_BAK_DRV ":/Uncompressed/" FILENAME_MASK_TXT);
  }
}

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
    ATAIDDrvs(root,dft_drv,&ata_drv,&atapi_drv);
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

