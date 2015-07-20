//The CFG defines are $LK,"::/Home/TOSCfg.CPP",A="FI:::/Demo/AcctExample/TOSCfg.CPP"$.

U8 TOSGetDrv()
{
  I64 result;
  "Drive (%s):",TOS_HDS;
  result=ToUpper(GetChar);
  '\n';
  return result;
}

U0 TOSMakeOSDrvBootInstall(U8 drv_let)
{
  drv_let=ToUpper(drv_let);
  Auto(TOS_CFG);
  MakeOSDrvBootInstall(drv_let);
  if (StrOcc(TOS_MASTER_BOOT_DRVS,drv_let))
    MasterBootIntall(drv_let);
}

U0 TOSCopyDrv(U8 src,U8 dst)
{
  U8 buf_s[STR_LEN],buf_d[STR_LEN];
  src=ToUpper(src);
  dst=ToUpper(dst);

  Fmt(dst,,FALSE,DVT_FAT32);

  StrPrint(buf_s,"%c:/",src);
  StrPrint(buf_d,"%c:/",dst);
  CopyTree(buf_s,buf_d);

  DocClear;
  Drv(dst);
  TOSMakeOSDrvBootInstall(dst);
}

U0 TOSPromptAndCopyDrv()
{
  I64 src,dst;
  "$$RED$$\nCopy Src Drive:\n$$FG$$";
  src=TOSGetDrv;
  "$$RED$$\nCopy Dst Drive:\n$$FG$$";
  dst=TOSGetDrv;
  TOSCopyDrv(src,dst);
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
  DirTreeDel(tempde1);
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
  DirTreeDel(tempde1);
}

public U0 CopySongs(U8 threshold='7')
{
  CDoc *s;
  CDocEntry *doc_e;
  CDirEntry *tempde=FilesFind("::/Home/Psalmody/*.CPP*",FUF_CLUSTER_ORDER),
	*tempde1=tempde;
  DelTree("/Apps/Psalmody/Examples");
  MkDir("/Apps/Psalmody/Examples");
  while (tempde) {
    s=DocRead(tempde->full_name);
    doc_e=s->root.next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[2]>=threshold)
      Copy(tempde->full_name,"/Apps/Psalmody/Examples");
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}

U0 DistroPrep()
{
  Del("/Demo/WebLogExample/WebLogRep.TXT*");
  Del("/Demo/ReverseFileExample/PoemForeward.TXT*");
  Del("/Home/NumBible.TXT.Z");
  DelTree("/Home/*Temp.TXT.Z");
  if (FileFind("::/Home/Psalmody"))
    CopySongs;

  DelTree("/Temp");
  MkDir("/Temp");
  MkDir("/Temp/ScreenShots");

  Touch("/PersonalMenu.TXT.Z","+T");
  Touch("/Home/PersonalMenu.TXT.Z","+T");

  DelTree("/Demo/AcctExample");
  CopyTree("/Home","/Demo/AcctExample");
  DelTree("/Demo/AcctExample/Archive");
  DelTree("/Demo/AcctExample/Budget");
  DelTree("/Demo/AcctExample/Psalmody");
  DelTree("/Demo/AcctExample/Wb2");
  Del("/Demo/AcctExample/Test*");

  //Once in a while, do these:
  //  SortHeaders;
  //  DocOpt("/*");
  //  DblSpaceScan("/*");

  CursorRem("/*");
  DelTree("/Demo/*.BI*");
  S2T("/*","+r+S");
  DocOpt("/*");
  Move(ACD_DEF_FILENAME,ACD_DEF_FILENAME_Z);
}

U0 MakeBootRescueDrvFile()
{
  try
	  Let2Drv('A');
  catch {
    Fs->catch_except=TRUE;
    Auto(CFG_BOOT_RESCUE_DRV "\n");
    Mount;
  }
  CBlkDev *bd=Let2BlkDev('A');
  Fmt('A',,FALSE,DVT_REDSEA);

  MkDir("A:/Compiler");
  Copy("C:/Compiler/Compiler.BIN.Z",	"A:/Compiler");
  Copy("C:/Compiler/OpCodes.TXT.Z",	"A:/Compiler");
  Copy("C:/Compiler/CompilerA.HPP.Z",	"A:/Compiler");
  Copy("C:/Compiler/CompilerB.HPP.Z",	"A:/Compiler");

  MkDir("A:/Kernel");
  Copy("C:/Kernel/*.HPP*",		"A:/Kernel");
  CopyTree("C:/Kernel/Dsk",		"A:/Kernel/Dsk");

  Copy("C:/Home/Wb2/Files/DbgAdam.CPP.Z","A:/Adam.CPP.Z");

  MkDir("A:/Adam");
  Copy("C:/Adam/AExts.HPP.Z",	"A:/Adam");
  Copy("C:/Adam/AMath.CPP.Z",	"A:/Adam");
  Copy("C:/Adam/Training.CPP.Z","A:/Adam");
  Copy("C:/Adam/AUtils.CPP.Z",	"A:/Adam");
  Copy("C:/Adam/AMathODE.CPP.Z","A:/Adam");

  FileWrite("C:" CFG_BOOT_RESCUE_DRV_FILE,
	bd->RAM_dsk,(bd->max_blk+1)<<BLK_SIZE_BITS);
}

U0 MakeSymFiles()
{
  CDoc *doc;
  U8 *st;

  DocClear;
  DocMax;
  HashTablePurge(adam_task->hash_table);
  Who;
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,"D:/Wb/Home/Wb2/SymsAlpha.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
  DocMax;
  Who("+m");
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,"D:/Wb/Home/Wb2/SymsAddress.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
}

I64 MakeLineRepFile()
{
  U8 *st;
  CDoc *doc;
  I64 result,official;
  CDocEntry *tree_branch,*start_indent,*end_indent;

  DocClear;
  Drv('C');
  DistroPrep;

  Cd("C:/");
  DelTree("B:/TOS");
  CopyTree("C:/Home","B:/TOS");
  DelTree("C:/Home");

  DocMax;
  DocClear;
  result=LineRep;
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,"B:/TOS/Wb2/LineRep.TXT.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
  official=LineRep("C:/*","-r")+
	LineRep("C:/Adam/*")+
	LineRep("C:/Compiler/*","-S+T")+
	LineRep("C:/Kernel/*");
  DocClear;
  CopyTree("B:/TOS","C:/Home");
  DelTree("B:/TOS");

  doc=DocRead("C:/Adam/ADefine.CPP.Z");
  DocTreeFind(doc,"LineRep",&tree_branch,&start_indent,&end_indent);
  DocCut(doc,start_indent->next,end_indent->last);
  doc->cur_entry=start_indent->next;
  doc->cur_col=0;

  DocPrint(doc,"DefinePrint(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",result);
  DocPrint(doc,"DefinePrint(\"DD_TEMPLEOS_LOC_OFFICIAL\",\"%,d\");\n",official);

  DefinePrint("DD_TEMPLEOS_LOC","%,d",result);
  DefinePrint("DD_TEMPLEOS_LOC_OFFICIAL","%,d",official);

  DocWrite(doc);
  DocDel(doc);

  "Total LOC:%12,d\n\n",result;
  return result;
}

U0 TOSDistroPrep()
{/*This is the TempleOS internal distro.  Only run this on a
virtual machine, not bare metal hardware because it's settings could cause
problems.  The hard drive must be partitioned.	Install Std Distro, then
install this on top.
>$LK,"TSInstall",A="FF:::/Demo/AcctExample/TOSDistro.CPP,TSInstall"$;
>$LK,"TSUpgrade",A="FF:::/Demo/AcctExample/TOSDistro.CPP,TSUpgrade"$;
See $LK,"MakeDistrosAndBackUp",A="FF:::/Demo/AcctExample/TOSDistro.CPP,MakeDistrosAndBackUp"$() to reproduce the ISO's in $FG,4$E:/Wb$FG$.  It makes this
website, too.
*/
  Drv('C');
  DistroPrep;
  Auto(TOS_CDROM_CFG);
  MakeOSDrvBootInstall('C','T');
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
}
U0 MakeTOSDistro()
{
  TOSDistroPrep;
  ISOFileCreate(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSTS_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - T.S. Company Internal Distro (%0.1f MB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}


U0 DbgDistroPrep()
{/*This is a debug distro.  The CDROM bootloader uses BIOS INT 0x13 0x42 to
load Kernel.BIN.C which has been modified to also hold a small RAM drive
(Drive A) that fits in the low 640k.  The bootloader starts in real mode
and the kernel changes to long, 64-bit mode.  It loads the 64-bit compiler
and assembler, compiles some, but not all, of the normal start-up scripts
and drops you off at the $LK,"Dbg",A="MN:Dbg"$ command-line where you can JIT compile,
use $LK,"EdLite",A="MN:EdLite"$ on RAM files, and use many of the other normal Kernel commands.
I could not fit all the normal start-up scripts into the small RAM drive,
so you do not have the window manager or normal document framework.  You
should have many Gig of RAM to play with with RAM Drive B, a high memory
RAM drive, but you have only those few files.  When I generate this distro,
I call $LK,"MakeBootDrvFile",A="FF:::/Demo/AcctExample/TOSDistro.CPP,MakeBootRescueDrvFile"$.
*/
  Drv('C');
  DistroPrep;
  MakeBootRescueDrvFile;
  Auto(TOS_CDROM_DBG_CFG);
  MakeOSDrvBootInstall('C','T');
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del("C:" CFG_BOOT_RESCUE_DRV_FILE);
}
U0 MakeDbgDistro()
{
  DbgDistroPrep;
  ISOFileCreate(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSDBG_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - Debug Distro (%0.1f MB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 StdDistroPrep()
{
  Drv('C');
  DistroPrep;
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSDrvBootInstall('C','T');
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  DelTree(TOS_DISTRO_DIR "/Home");
  MkDir(TOS_DISTRO_DIR "/Home");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR BOOT_DIR "/OldMBR.BIN.C");
  Del(TOS_DISTRO_DIR BOOT_DIR "/BootMHD2.BIN.C");
}
U0 MakeStdDistro()
{
  StdDistroPrep;
  ISOFileCreate(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  Drv('C');
}

U0 LiteDistroPrep()
{/*Missing: Dictionary, Bible and TOSZ Utility.Still 100% OpenSource.
Includes: x86_64 64BitMultiCoredKernel, 64BitCompiler, Assembler,
Unassembler, Debugger, Editor, VGAGraphics, Tools, BootLoaders,
Keyboard&Mouse, HardDrives, FAT32, Partitioner, CD/DVD, ISO9660FileCreator,
FileManager, WindowManager, PCSpeaker, Apps, Games, God-Oracle-App, Hymns,
Demos, Documentation.
*/
  StdDistroPrep;
  DelTree(TOS_DISTRO_DIR "/Linux");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACDefs.DAT*");
  Del(TOS_DISTRO_DIR "/Misc/PCIDevices.TXT.Z");
  Del(TOS_DISTRO_DIR "/Misc/Bible.TXT.Z");
}
U0 MakeLiteDistro()
{
  LiteDistroPrep;
  ISOFileCreate(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSLITE_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - Lite Distro (%0.1f MB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 UltraDistroPrep()
{/*
Missing: Apps, Demos, Dictionary, Bible and TOSZ Utility.  Still 100%OpenSource.
Includes: x86_64 64BitMultiCoredKernel, 64BitCompiler, Assembler,
Unassembler, Debugger, Editor, VGAGraphics, Tools, BootLoaders,
Keyboard&Mouse, HardDrives,  FAT32, Partitioner, CD/DVD, ISO9660FileCreator,
FileManager, WindowManager, PCSpeaker, Documentation.
*/
  LiteDistroPrep;
  DelTree(TOS_DISTRO_DIR "/Apps");
  MkDir(TOS_DISTRO_DIR "/Apps");
  DelTree(TOS_DISTRO_DIR "/Demo");
  DelTree(TOS_DISTRO_DIR "/Misc/Tour");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACWords.DAT*");
  Del(TOS_DISTRO_DIR "/Adam/God/Vocab.TXT.Z");
  Del(TOS_DISTRO_DIR "/Misc/OSTestSuite.CPP.Z");
}
U0 MakeUltraDistro()
{
  UltraDistroPrep;
  ISOFileCreate(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSULTRA_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - UltraLite Distro (%0.1f MB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 TOSWebFile(U8 *name)
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
	  "<meta name=\"keywords\" "
	  "content=\"Operating System,64-Bit,64 Bit,"
	  "Temple,OS,TempleOS,Free,Open Source,Public Domain,x86_64\">\n");
    Touch(name2,"",,de.datetime);
    Free(name2);
  }
}

U0 MakeWebSite()
{
  CDirEntry *tempde,*tempde1;
  DocClear;
  Cd("D:/Wb");
  DelTree("D:/Wb/Home/Archive");
  DelTree("D:/Wb/Home/Budget");
  DelTree("D:/Wb/Home/Psalmody");
  Del("D:/Wb/Home/*");
  Touch("D:/Wb/Home/Wb2/index.TXT.Z","",,Now);
  Touch("D:/Wb/Home/Wb2/Downloads.TXT.Z","",,Now);
  Touch("D:/Wb/Home/Wb2/TempleOS.TXT.Z","",,Now);
  tempde=tempde1=FilesFind("*",
	FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT|FUF_CLUSTER_ORDER);
  while (tempde) {
    TOSWebFile(tempde->full_name);
    tempde=tempde->next;
  }
  Copy(	"D:/Wb/Home/Wb2/Files/Bin/TempleBot",
	"D:/Wb/Home/Wb2/Files/Bin/TempleBot.TXT");
  ToDolDoc("D:/Wb/Home/Wb2/Files/Bin/TempleBot.TXT");
  ToHtml("D:/Wb/Home/Wb2/Files/Bin/TempleBot.TXT");
  Del("D:/Wb/Home/Wb2/Files/Bin/TempleBot.TXT");
  DirTreeDel(tempde1);
  Drv('C');
}

U0 FreshenWebDir(U8 *dirname)
{//No recurse
  CDirEntry *tempde,*tempde1;
  U8 *src,*dst;

  src=MStrPrint("D:/Wb%s/*",dirname);
  Del(src);
  Free(src);
  src=MStrPrint("C:%s/*",dirname);
  dst=MStrPrint("D:/Wb%s",dirname);
  Copy(src,dst);
  Free(src);
  Free(dst);

  src=MStrPrint("D:/Wb%s/*",dirname);
  tempde=tempde1=FilesFind(src,FUF_JUST_FILES|FUF_JUST_TXT|FUF_CLUSTER_ORDER);
  while (tempde) {
    TOSWebFile(tempde->full_name);
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}

U0 MakeWebSitePartial()
{
  FreshenWebDir("/Home/Wb2");
  Drv('C');
}

U0 UpdateISODocDefines()
{
  DefinePrint("DD_TEMPLEOSCD_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1f MB)",
	0.1*(10*Size("D:/Wb/TempleOSCD.ISO","+s")/1024/1024));
  DefinePrint("DD_TEMPLEOSCD_K_SIZE",
	"%d KB",Size("D:/Wb/TempleOSCD.ISO","+s")/1024);
  DefinePrint("DD_TEMPLEOSULTRA_SIZE",
	"Download $TX,"TempleOS V3.07",D="DD_OS_NAME_VERSION"$ - UltraSmall Distro (%0.1f MB)",
	0.1*(10*Size("D:/Wb/TempleOSUltra.ISO","+s")/1024/1024));
  DefinePrint("DD_TEMPLEOSULTRA_K_SIZE",
	"%d KB",Size("D:/Wb/TempleOSUltra.ISO","+s")/1024);
}

UpdateISODocDefines;

public U0 MakeDistrosAndBackUp()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  progress4=*progress4_desc=0;
  progress4_max=14;
  AutoComplete;
  WinMax;
  WinBorder;
  ChkAll; progress4++;

  MakeLineRepFile; progress4++;
  TOSCopyDrv('C','D'); progress4++;
  DelTree ("D:/Wb"); progress4++;
  CopyTree("C:/","D:/Wb"); progress4++;
  DocClear;

  MakeStdDistro; progress4++;
  DocClear;
  Copy(TOS_ISO_NAME,"D:/Wb/TempleOSCD.ISO"); progress4++;

  MakeUltraDistro; progress4++;
  DocClear;
  Copy(TOS_ISO_NAME,"D:/Wb/TempleOSUltra.ISO"); progress4++;

  UpdateISODocDefines;

  Cd("C:/");

  if (LongLines)
    throw;
  progress4++;

  if (LinkChk)
    throw;
  progress4++;

  MakeSymFiles; progress4++;
  MakeWebSite; progress4++;

  Drv('C');
  ChkAll; progress4++;
  progress4=progress4_max=0;
  SettingsPop;
}

public U0 PreBackUp()
{
  Copy("D:/Home/Wb2/Files/Bookmarks.html","C:/Home/Wb2/Files");
  Del("C:/Home/Wb2/Files/Bin/*");
  Copy("D:/Home/Wb2/Files/Bin/*","C:/Home/Wb2/Files/Bin");
  Copy("D:/Home/Wb2/Files/Bin/TempleBot","C:/Linux");
  Copy("D:/Home/Wb2/Files/Bin/God*","C:/Linux");
}
