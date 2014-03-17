//The CFG defines are $LK,"Here",A="FI:::/Demo/AcctExample/TSAdam.CPP"$.

U8 TSPmtDrv()
{
  I64 result;
  "Drive (%s):",TS_HDS;
  result=ToUpper(GetChar);
  NewLine;
  return result;
}

U0 TSMakeOSPrtBootInstall(U8 drv_let)
{
  drv_let=ToUpper(drv_let);
  Auto(TS_CFG);
  MakeOSPrtBootInstall(drv_let);
  if (StrOcc(TS_MASTER_BOOT_DRVS,drv_let))
    MasterBootIntall(drv_let);
}

U0 SoftBootMenu(U8 drv_let=0)
{
  if (!drv_let)
    drv_let=Prt2Drv(Fs->cur_prt);
  Drv(drv_let);
  Cd("/Kernel");
  Auto(TS_MENU_CFG);
  if (!Cmp("Kernel","Kernel",NULL,':'))
    SoftBoot("Kernel.BIN.Z");
}

U0 TSCopyDrv(U8 src,U8 dst)
{
  U8 buf_s[128],buf_d[128];
  src=ToUpper(src);
  dst=ToUpper(dst);

  Fmt(dst,,FALSE,PT_FAT32);

  SPrint(buf_s,"%c:/",src);
  SPrint(buf_d,"%c:/",dst);
  CopyTree(buf_s,buf_d);

  DocClear;
  Drv(dst);
  TSMakeOSPrtBootInstall(dst);
}

U0 TSPmtCopyDrv()
{
  I64 src,dst;
  "$$RED$$\nCopy Src Drive:\n$$FG$$";
  src=TSPmtDrv;
  "$$RED$$\nCopy Dst Drive:\n$$FG$$";
  dst=TSPmtDrv;
  TSCopyDrv(src,dst);
}

public U0 CursorRemFile(U8 *filename)
{
  CDoc *doc;
  doc=DocRead(filename,DOCF_PLAIN_TEXT|DOCF_NO_CURSOR);
  DocWrite(doc);
  DocDel(doc);
}
public U0 CursorRem(U8 *files_find_mask="*")
{
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F+O");
  CDirEntry *tempde=FilesFind(files_find_mask,fuf_flags),*tempde1=tempde;
  while (tempde) {
    CursorRemFile(tempde->full_name);
    Touch(tempde->full_name,"",,tempde->datetime);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

public U0 CursorTopFile(U8 *filename)
{
  CDoc *doc;
  doc=DocRead(filename,DOCF_PLAIN_TEXT);
  doc->cur_entry=doc->root.next;
  DocWrite(doc);
  DocDel(doc);
}
public U0 CursorTop(U8 *files_find_mask="*")
{
  I64 fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+T+f+F+O");
  CDirEntry *tempde=FilesFind(files_find_mask,fuf_flags),*tempde1=tempde;
  while (tempde) {
    CursorTopFile(tempde->full_name);
    Touch(tempde->full_name,"",,tempde->datetime);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
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
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+T+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  CDirEntry *tempde=FilesFind(files_find_mask,fuf_flags),*tempde1=tempde;
  while (tempde && !Bt(&fuf_flags,FUf_CANCEL)) {
    UncollapseFile(tempde->full_name,&fuf_flags);
    Touch(tempde->full_name,"",,tempde->datetime);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

public U0 CopySongs(U8 threshold='7')
{
  CDoc *s;
  CDocEntry *doc_e;
  CDirEntry *tempde=FilesFind("/Accts/TS/TAD/MusicOrgan/*.CPP*",FUF_CLUSTER_ORDER),*tempde1=tempde;
  DelTree("/Apps/MusicOrgan/Examples");
  MkDir("/Apps/MusicOrgan/Examples");
  while (tempde) {
    s=DocRead(tempde->full_name);
    doc_e=s->root.next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[2]>=threshold)
      Copy(tempde->full_name,"/Apps/MusicOrgan/Examples");
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

U0 DistroPrep()
{
  Del("/Demo/WebLogExample/WebLogRep.TXT*");
  Del("/Demo/ReverseFileExample/PoemForeward.TXT*");
  Del("/Demo/Games/EagleDive.BIN*");
  DelTree("/Accts/*Temp.TXT.Z");
  CopySongs;

  DelTree("/Temp");
  MkDir("/Temp");
  MkDir("/Temp/ScreenShots");

  //User
  Del("/Accts/User/Registry.CPP.Z");
  //Install
  Del("/Accts/Install/Registry.CPP.Z");

  Touch("/Accts/PersonalMenu.TXT.Z","+T");
  //TS
  Touch("/Accts/TS/PersonalMenu.TXT.Z","+T");

  DelTree("/Demo/AcctExample");
  CopyTree("/Accts/TS","/Demo/AcctExample");
  DelTree("/Demo/AcctExample/TAD/MusicOrgan");
  DelTree("/Demo/AcctExample/Wb2");
  Del("/Demo/AcctExample/TAD/Test*");

  CursorTop("/*");
  DelTree("/Demo/*.BI*");
  S2T("/*","+r+S");
  DocOpt("/*");
  Move(WSD_DEF_FILENAME,WSD_DEF_FILENAME_Z);
}

U0 MakeBootDrvFile()
{
  try
    PrtChk(Drv2Prt(TS_BOOT_DRV_LET));
  catch {
    Fs->catch_except=TRUE;
    Auto(CFG_BOOT_DRV "\n");
    Mount;
  }
  CBlkDev *bdev=Drv2BlkDev(TS_BOOT_DRV_LET);
  Fmt(TS_BOOT_DRV_LET,,FALSE,PT_REDSEA);

  MkDir(TS_BOOT_DRV ":/Compiler");
  Copy(TS_MAIN_DRV ":/Compiler/Compiler.BIN.Z",	TS_BOOT_DRV ":/Compiler");
  Copy(TS_MAIN_DRV ":/Compiler/OpCodes.TXT.Z",	TS_BOOT_DRV ":/Compiler");
  Copy(TS_MAIN_DRV ":/Compiler/CmpDefs.HPP.Z",	TS_BOOT_DRV ":/Compiler");
  Copy(TS_MAIN_DRV ":/Compiler/Cmp.HPP.Z",	TS_BOOT_DRV ":/Compiler");

  MkDir(TS_BOOT_DRV ":/Kernel");
  Copy(TS_MAIN_DRV ":/Kernel/*.HPP*",		TS_BOOT_DRV ":/Kernel");
  Copy(TS_MAIN_DRV ":/Kernel/Adam1*",		TS_BOOT_DRV ":/Kernel");
  CopyTree(TS_MAIN_DRV ":/Kernel/Dsk",		TS_BOOT_DRV ":/Kernel/Dsk");

  MkDir(TS_BOOT_DRV ":/Adam");
  Copy(TS_MAIN_DRV ":/Accts/TS/DbgAdam2.CPP.Z",	TS_BOOT_DRV ":/Adam/Adam2.CPP.Z");
  Copy(TS_MAIN_DRV ":/Adam/AdamExt.HPP.Z",	TS_BOOT_DRV ":/Adam");
  Copy(TS_MAIN_DRV ":/Adam/Math2a.CPP.Z",	TS_BOOT_DRV ":/Adam");
  Copy(TS_MAIN_DRV ":/Adam/Training.CPP.Z",	TS_BOOT_DRV ":/Adam");
  Copy(TS_MAIN_DRV ":/Adam/Utils2a.CPP.Z",	TS_BOOT_DRV ":/Adam");
  Copy(TS_MAIN_DRV ":/Adam/MathODE.CPP.Z",	TS_BOOT_DRV ":/Adam");

  FileWrite(TS_MAIN_DRV ":" CFG_BOOT_DRV_FILE,bdev->RAM_dsk,(bdev->max_blk+1)<<BLK_SIZE_BITS);
}

U0 MakeSymFiles()
{
  CDoc *doc;
  U8 *st;
  if (ts_has_bak_drv) {
    DocClear;
    DocMax;
    HashTablePurge(adam_task->hash_table);
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

I64 MakeLineRepFile()
{
  U8 *st;
  CDoc *doc;
  I64 result,official;
  CDocEntry *tree_branch,*start_indent,*end_indent;

  DocClear;
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;

  Cd(TS_MAIN_DRV ":/");
  if (ts_has_ram_drv) {
    DelTree(TS_RAM_DRV ":/TS");
    CopyTree(TS_MAIN_DRV ":/Accts/TS",TS_RAM_DRV ":/TS");
    DelTree(TS_MAIN_DRV ":/Accts/TS");
  }

  DocMax;
  DocClear;
  result=LineRep;
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,TS_RAM_DRV ":/TS/Wb2/LineRep.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
  official=LineRep(TS_MAIN_DRV ":/Accts/*")+
	   LineRep(TS_MAIN_DRV ":/Adam/*")+
	   LineRep(TS_MAIN_DRV ":/Compiler/*","-S+T")+
	   LineRep(TS_MAIN_DRV ":/Kernel/*");
  DocClear;
  if (ts_has_ram_drv) {
    CopyTree(TS_RAM_DRV ":/TS",TS_MAIN_DRV ":/Accts/TS");
    DelTree(TS_RAM_DRV ":/TS");
  }

  doc=DocRead(TS_MAIN_DRV ":/Adam/DocDefines.CPP.Z");
  DocTreeFind(doc,"LineRep",&tree_branch,&start_indent,&end_indent);
  DocCut(doc,start_indent->next,end_indent->last);
  doc->cur_entry=start_indent->next;
  doc->cur_data_col=0;

  DocPrint(doc,"DefinePrint(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",result);
  DocPrint(doc,"DefinePrint(\"DD_TEMPLEOS_LOC_OFFICIAL\",\"%,d\");\n",official);
  st=MPrint("%,d",result);
  DefineLoad("DD_TEMPLEOS_LOC",st);
  Free(st);
  st=MPrint("%,d",official);
  DefineLoad("DD_TEMPLEOS_LOC_OFFICIAL",st);
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
  MakeOSPrtBootInstall(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/",TS_DISTRO_DIR "/");
}

U0 DbgDistroPrep()
{
  if (ts_has_boot_drv) {
    Drv(TS_MAIN_DRV_LET);
    DistroPrep;
    MakeBootDrvFile;
    Auto(TS_CDROM_DBG_CFG);
    MakeOSPrtBootInstall(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
    DelTree(TS_DISTRO_DIR);
    CopyTree(TS_MAIN_DRV ":/",TS_DISTRO_DIR "/");
    Del(TS_MAIN_DRV ":" CFG_BOOT_DRV_FILE);
  }
}

U0 StdDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSPrtBootInstall(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/",TS_DISTRO_DIR "/");
  DelTree(TS_DISTRO_DIR "/Accts/TS");
  Del(TS_DISTRO_DIR "/0000Boot/OldMBR.BIN.C");
  Del(TS_DISTRO_DIR "/0000Boot/BootMHD2.BIN.C");
}

U0 LiteDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSPrtBootInstall(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/",TS_DISTRO_DIR "/");
  DelTree(TS_DISTRO_DIR "/Accts/TS");
  DelTree(TS_DISTRO_DIR "/Linux");
  DelTree(TS_DISTRO_DIR "/Windows");
  Del(TS_DISTRO_DIR "/0000Boot/OldMBR.BIN.C");
  Del(TS_DISTRO_DIR "/0000Boot/BootMHD2.BIN.C");
  Del(TS_DISTRO_DIR "/Adam/WordStat/WSDefs.DAT*");
  Del(TS_DISTRO_DIR "/Misc/PCIDevices.TXT.Z");
  Del(TS_DISTRO_DIR "/Misc/Bible.TXT.Z");
}

U0 UltraDistroPrep()
{
  Drv(TS_MAIN_DRV_LET);
  DistroPrep;
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSPrtBootInstall(TS_MAIN_DRV_LET,TS_CDROM_DRV_LET);
  DelTree(TS_DISTRO_DIR);
  CopyTree(TS_MAIN_DRV ":/",TS_DISTRO_DIR "/");
  DelTree(TS_DISTRO_DIR "/Accts/TS");
  DelTree(TS_DISTRO_DIR "/Linux");
  DelTree(TS_DISTRO_DIR "/Windows");
  DelTree(TS_DISTRO_DIR "/Apps");
  DelTree(TS_DISTRO_DIR "/Demo");
  Del(TS_DISTRO_DIR "/0000Boot/OldMBR.BIN.C");
  Del(TS_DISTRO_DIR "/0000Boot/BootMHD2.BIN.C");
  Del(TS_DISTRO_DIR "/Adam/WordStat/*.DAT*");
  DelTree(TS_DISTRO_DIR "/Misc/Tour");
  Del(TS_DISTRO_DIR "/Misc/PCIDevices.TXT.Z");
  Del(TS_DISTRO_DIR "/Misc/Bible.TXT.Z");
  Del(TS_DISTRO_DIR "/Adam/God/Vocab.TXT.Z");
}

U0 MakeTSDistro()
{
  TSDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeDbgDistro()
{
  DbgDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeStdDistro()
{
  StdDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeLiteDistro()
{
  LiteDistroPrep;
  CDFileCreate(TS_ISO_NAME,TS_DISTRO_DIR "/*",,TS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv(TS_MAIN_DRV_LET);
}

U0 MakeUltraDistro()
{
  UltraDistroPrep;
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
  CDirEntry *tempde,*tempde1;
  if (ts_has_bak_drv) {
    DocClear;
    Cd(TS_BAK_DRV ":/Wb");
    DelTree(TS_BAK_DRV ":/Wb/Accts/TS/TAD");
    Del(TS_BAK_DRV ":/Wb/Accts/TS/*");
    Touch(TS_BAK_DRV ":/Wb/Accts/TS/Wb2/index.TXT.Z","",,Now);
    Touch(TS_BAK_DRV ":/Wb/Accts/TS/Wb2/Downloads.TXT.Z","",,Now);
    Touch(TS_BAK_DRV ":/Wb/Accts/TS/Wb2/TempleOS.TXT.Z","",,Now);
    tempde=tempde1=FilesFind("*",FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT|FUF_CLUSTER_ORDER);
    while (tempde) {
      TSWebFile(tempde->full_name);
      tempde=tempde->next;
    }
    DirLstDel(tempde1);
    Drv(TS_MAIN_DRV_LET);
  }
}

U0 FreshenWebDir(U8 *dirname)
{
  CDirEntry *tempde,*tempde1;
  U8 *src,*dst;

  src=MPrint(TS_BAK_DRV ":/Wb%s/*",dirname);
  Del(src);
  Free(src);
  src=MPrint(TS_MAIN_DRV ":%s/*",dirname);
  dst=MPrint(TS_BAK_DRV  ":/Wb%s",dirname);
  Copy(src,dst);
  Free(src);
  Free(dst);

  src=MPrint(TS_BAK_DRV ":/Uncompressed%s/*",dirname);
  Del(src);
  Free(src);
  src=MPrint(TS_MAIN_DRV ":%s/*",dirname);
  dst=MPrint(TS_BAK_DRV  ":/Uncompressed%s",dirname);
  Copy(src,dst);
  Free(src);
  Free(dst);

  src=MPrint(TS_BAK_DRV ":/Wb%s/*",dirname);
  tempde=tempde1=FilesFind(src,FUF_JUST_FILES|FUF_JUST_TXT|FUF_CLUSTER_ORDER);
  while (tempde) {
    TSWebFile(tempde->full_name);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
}

U0 MakeWebSitePartial()
{
  if (ts_has_bak_drv) {
    FreshenWebDir("/Accts/TS/Wb2");
    FreshenWebDir("/Accts/TS/Wb2/Rants/TAD");
    FreshenWebDir(TS_BLOG_DIR);
    Drv(TS_MAIN_DRV_LET);
  }
}

U0 TSUncompress()
{
  U8 *st;
  CDirEntry *tempde,*tempde1;
  if (ts_has_bak_drv) {
    DelTree(TS_BAK_DRV ":/Uncompressed");
    CopyTree(TS_MAIN_DRV ":/",TS_BAK_DRV ":/Uncompressed");
    tempde=tempde1=FilesFind(TS_BAK_DRV ":/Uncompressed/*.Z",FUF_RECURSE|FUF_JUST_FILES|FUF_CLUSTER_ORDER);
    while (tempde) {
      st=StrNew(tempde->full_name);
      StrLastRem(st,".");
      Move(tempde->full_name,st);
      Free(st);
      tempde=tempde->next;
    }
    DirLstDel(tempde1);
    CursorRem(TS_BAK_DRV ":/Uncompressed/*");
  }
}

U0 UpdateISODocDefines()
{
  DefinePrint("DD_TEMPLEOSTS_SIZE",
    "Download $TX,"TempleOS V2.03",D="DD_OS_NAME_VERSION"$ - T.S. Company Internal Distro (%0.1f MB)",
    0.1*(10*Size(TS_BAK_DRV ":/Wb/TempleOSTS.ISO","+s")/1024/1024));

  DefinePrint("DD_TEMPLEOSDBG_SIZE",
    "Download $TX,"TempleOS V2.03",D="DD_OS_NAME_VERSION"$ - Debug Distro (%0.1f MB)",
    0.1*(10*Size(TS_BAK_DRV ":/Wb/TempleOSDbg.ISO","+s")/1024/1024));

  DefinePrint("DD_TEMPLEOSCD_SIZE",
    "Download $TX,"TempleOS V2.03",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1f MB)",
    0.1*(10*Size(TS_BAK_DRV ":/Wb/TempleOSCD.ISO","+s")/1024/1024));

  DefinePrint("DD_TEMPLEOSLITE_SIZE",
    "Download $TX,"TempleOS V2.03",D="DD_OS_NAME_VERSION"$ - Lite Distro (%0.1f MB)",
    0.1*(10*Size(TS_BAK_DRV ":/Wb/TempleOSLite.ISO","+s")/1024/1024));

  DefinePrint("DD_TEMPLEOSULTRA_SIZE",
    "Download $TX,"TempleOS V2.03",D="DD_OS_NAME_VERSION"$ - UltraLite Distro (%0.1f MB)",
    0.1*(10*Size(TS_BAK_DRV ":/Wb/TempleOSUltra.ISO","+s")/1024/1024));
}

UpdateISODocDefines;

public U0 MakeDistrosAndBackUp()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  progress4=*progress4_desc=0;
  progress4_max=19;
  WordStat;
  WinMax;
  WinBorder;
  ChkAll; progress4++;
  if (ts_has_bak_drv) {
    MakeLineRepFile; progress4++;
    TSCopyDrv(TS_MAIN_DRV_LET,TS_BAK_DRV_LET); progress4++;
    DelTree (TS_BAK_DRV  ":/Wb"); progress4++;
    CopyTree(TS_MAIN_DRV ":/",TS_BAK_DRV ":/Wb"); progress4++;
    DocClear;

    MakeTSDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSTS.ISO"); progress4++;

    MakeDbgDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSDbg.ISO"); progress4++;

    MakeStdDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSCD.ISO"); progress4++;

    MakeLiteDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSLite.ISO"); progress4++;

    MakeUltraDistro; progress4++;
    DocClear;
    Copy(TS_ISO_NAME,TS_BAK_DRV ":/Wb/TempleOSUltra.ISO"); progress4++;

    UpdateISODocDefines;

    Cd(TS_MAIN_DRV ":/");
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
    Fmt(TS_MAIN_DRV_LET,,FALSE);
    CopyTree(TS_CDROM_DRV ":/",TS_MAIN_DRV ":/");
    TSMakeOSPrtBootInstall(TS_MAIN_DRV_LET);
    Fmt(TS_BAK_DRV_LET,,FALSE);
    CopyTree(TS_CDROM_DRV ":/",TS_BAK_DRV ":/");
    TSMakeOSPrtBootInstall(TS_BAK_DRV_LET);
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
