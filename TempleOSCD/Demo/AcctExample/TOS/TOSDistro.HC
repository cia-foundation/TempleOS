//The CFG defines are $LK,"~/TOS/TOSCfg.HC",A="FI:::/Demo/AcctExample/TOS/TOSCfg.HC"$.

#help_index "Misc/TOS/Distro"

#define MAKE_STD	1
#define MAKE_STD_RS	0
#define MAKE_ULTRA	1
#define MAKE_ULTRA_RS	0
#define MAKE_SUP1	1
#define MAKE_WEB	1
#define MAKE_BOOKS1	0
#define MAKE_BOOKS2	0
#define MAKE_DBG	0
#define MAKE_STAFF	1

public U8 TOSGetDrv()
{//Pmt for drv let.
  I64 res;
  "Drive (%s):",TOS_HDS;
  res=Let2Let(GetChar);
  '\n';
  return res;
}

public U0 TOSBootHDIns(U8 drv_let=0)
{//Make Compiler and Kernel. Reinstall Kernel.
  drv_let=Let2Let(drv_let);
  In(TOS_CFG);
  BootHDIns(drv_let);
  if (StrOcc(TOS_MASTER_BOOT_DRVS,drv_let))
    BootMHDIns(drv_let);
}

public U0 TOSCopyDrv(U8 src,U8 dst)
{//Fmt dst and copy entire drv.
  U8 buf_s[STR_LEN],buf_d[STR_LEN];
  src=Let2Let(src);
  dst=Let2Let(dst);

  Fmt(dst,,FALSE,FSt_FAT32);

  StrPrint(buf_s,"%c:/",src);
  StrPrint(buf_d,"%c:/",dst);
  CopyTree(buf_s,buf_d);

  DocClear;
  Drv(dst);
  TOSBootHDIns(dst);
}

public U0 TOSPmtAndCopyDrv()
{//Pmt for drv lets. Then, Fmt dst and copy entire drv.
   I64 src,dst;
  "$$RED$$\nCopy Src Drive:\n$$FG$$";
  src=TOSGetDrv;
  "$$RED$$\nCopy Dst Drive:\n$$FG$$";
  dst=TOSGetDrv;
  TOSCopyDrv(src,dst);
}

U0 DistroSongsPrep(U8 threshold='7')
{
  CDoc *doc;
  CDocEntry *doc_e;
  CDirEntry *tmpde=FilesFind("/Home/Sup1/Sup1Hymns/*.HC*",
	FUF_CLUS_ORDER),*tmpde1=tmpde;
  DelTree("/Apps/Psalmody/Examples");
  DirMk("/Apps/Psalmody/Examples");
  while (tmpde) {
    doc=DocRead(tmpde->full_name);
    doc_e=doc->head.next;
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[2]>=threshold)
      Copy(tmpde->full_name,"/Apps/Psalmody/Examples");
    DocDel(doc);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

U0 DistroPrep()
{
  AOnceFlush; //Don't want in Registry
  OnceFlush;

  Del("/Home/Demo*");
  DelTree("/Home/*Tmp.DD.Z");
  if (FileFind("/Home/Sup1/Sup1Hymns"))
    DistroSongsPrep;

  DelTree("/Tmp");
  DirMk("/Tmp");
  DirMk("/Tmp/ScrnShots");

  Touch("/PersonalMenu.DD.Z","+T");
  Touch("/Home/PersonalMenu.DD.Z","+T");

  DelTree("/Demo/AcctExample");
  CopyTree("/Home","/Demo/AcctExample");
  DelTree("/Demo/AcctExample/Budget");
  DelTree("/Demo/AcctExample/Private");
  DelTree("/Demo/AcctExample/Sup1");
  DelTree("/Demo/AcctExample/Books1");
  DelTree("/Demo/AcctExample/Books2");
  DelTree("/Demo/AcctExample/Web");
  Del("/Demo/AcctExample/Test*");
  Copy("/Home/Web/index.DD.Z",
	"/Demo/ToHtmlToTXTDemo/DemoInPage.DD.Z");
  if (FileFind("~/Sup1/Sup1Utils/SortHeaders.HC.Z"))
    ExeFile("~/Sup1/Sup1Utils/SortHeaders.HC.Z");

  if (FileFind("~/Sup1/Sup1Blog/YouTube.DD.Z"))
    Sort("~/Sup1/Sup1Blog/YouTube.DD.Z",,2);

  //Once in a while, do ~/Sup1/Sup1Utils/DblSpaceScan.HC.

  CursorRem("/*");
  DelTree("/Demo/*.BI*");
  S2T("/*","+r+S");
  DocOpt("/*","+R");
  Move(ACD_DEF_FILENAME,ACD_DEF_FILENAME_Z);
}

U0 DbgDistroFilePrep()
{
  CBlkDev *bd;
  if (!Let2Drv('A',FALSE)) {
    In(CFG_DBG_DISTRO "\n");
    Mount;
  }
  bd=Let2BlkDev('A');
  Fmt('A',,FALSE,FSt_REDSEA);

  DirMk("A:/Compiler");
  Copy("C:/Compiler/Compiler.BIN.Z",	"A:/Compiler");
  Copy("C:/Compiler/OpCodes.DD.Z",	"A:/Compiler");
  Copy("C:/Compiler/CompilerA.HH.Z",	"A:/Compiler");
  Copy("C:/Compiler/CompilerB.HH.Z",	"A:/Compiler");

  DirMk("A:/Kernel");
  Copy("C:/Kernel/*.HH*",		"A:/Kernel");
  CopyTree("C:/Kernel/BlkDev",		"A:/Kernel/BlkDev");

  Copy("C:/Home/Sup1/Sup1Distro/DbgStartOS.HC.Z","A:/StartOS.HC.Z");

  DirMk("A:/Adam");
  Copy("C:/Home/Sup1/Sup1Distro/DbgMakeAdam.HC.Z","A:/Adam/MakeAdam.HC.Z");
  Copy("C:/Home/Sup1/Sup1Distro/DbgMount.HC.Z","A:/Adam");
  Copy("C:/Adam/AExts.HC.Z",	"A:/Adam");
  Copy("C:/Adam/AMath.HC.Z",	"A:/Adam");
  Copy("C:/Adam/Training.HC.Z","A:/Adam");
  Copy("C:/Adam/AMem.HC.Z",	"A:/Adam");
  Copy("C:/Adam/TaskRep.HC.Z",	"A:/Adam");

  FileWrite("C:" CFG_DBG_DISTRO_FILE,
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
    StrCpy(doc->filename.name,"D:/Wb/Home/Web/SymsAlpha.DD.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
  DocMax;
  Who("+m");
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,"D:/Wb/Home/Web/SymsAddress.DD.Z");
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
  I64 res,official;

  DocClear;
  Drv('C');
  DistroPrep;

  Cd("C:/");
  DelTree("B:/TOS/TOS");
  CopyTree("C:/Home","B:/TOS/TOS");
  DelTree("C:/Home");

  DocMax;
  DocClear;
  res=LineRep;
  if (doc=DocPut) {
    st=StrNew(doc->filename.name);
    StrCpy(doc->filename.name,"B:/TOS/TOS/Web/LineRep.DD.Z");
    DocWrite(doc);
    StrCpy(doc->filename.name,st);
    Free(st);
  }
  DocClear;
  official=LineRep("C:/*","-r")+
	LineRep("C:/Adam/*")+
	LineRep("C:/Compiler/*","-S+$$")+
	LineRep("C:/Kernel/*");
  DocClear;
  CopyTree("B:/TOS/TOS","C:/Home");
  DelTree("B:/TOS/TOS");

  DocTreeFWrite("C:/Adam/ADefine.HC.Z","LineRep",
	"DefinePrint(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",res);
  DocTreeFAppend("C:/Adam/ADefine.HC.Z","LineRep",
	"DefinePrint(\"DD_TEMPLEOS_LOC_OFFICIAL\",\"%,d\");\n",official);

  DefinePrint("DD_TEMPLEOS_LOC","%,d",res);
  DefinePrint("DD_TEMPLEOS_LOC_OFFICIAL","%,d",official);

  "Total LOC:%12,d\n\n",res;
  return res;
}

U0 StaffDistroPrep()
{
  Drv('C');
  DistroPrep;
  In(TOS_DVD_CFG);
  BootDVDIns('C');
  Fmt('B',,FALSE,FSt_REDSEA);
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  DelTree(TOS_DISTRO_DIR "/Home/Sup1");
  DelTree(TOS_DISTRO_DIR "/Home/Books1");
  DelTree(TOS_DISTRO_DIR "/Home/Books2");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
}
U0 MakeStaffDistro()
{
  StaffDistroPrep;
  ISO9660ISO(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_STAFF_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - T.S. Company Internal Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 DbgDistroPrep()
{
  Drv('C');
  DistroPrep;
  DbgDistroFilePrep;
  In(TOS_DVD_DBG_CFG);
  BootDVDIns('C');
  Fmt('B',,FALSE,FSt_REDSEA);
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  DelTree(TOS_DISTRO_DIR "/Home");
  DirMk(TOS_DISTRO_DIR "/Home");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR BOOT_DIR "/OldMBR.BIN.C");
  Del(TOS_DISTRO_DIR BOOT_DIR "/BootMHD2.BIN.C");
}
U0 MakeDbgDistro()
{
  DbgDistroPrep;
  ISO9660ISO(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_DBG_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - Debug Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 StdDistroPrep()
{
  Drv('C');
  DistroPrep;
  In(STD_DISTRO_DVD_CFG);
  BootDVDIns('C');
  Fmt('B',,FALSE,FSt_REDSEA);
  DelTree(TOS_DISTRO_DIR);
  CopyTree("C:/",TOS_DISTRO_DIR "/");
  DelTree(TOS_DISTRO_DIR "/Home");
  DirMk(TOS_DISTRO_DIR "/Home");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR BOOT_DIR "/OldMBR.BIN.C");
  Del(TOS_DISTRO_DIR BOOT_DIR "/BootMHD2.BIN.C");
}
U0 MakeStdDistro()
{
  StdDistroPrep;
  ISO9660ISO(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSCD_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}
U0 MakeStdRedSeaDistro()
{
  StdDistroPrep;
  RedSeaISO(TOS_ISO_C_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSCD_RS_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - Standard RedSea Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_C_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 UltraDistroPrep()
{
  StdDistroPrep;
  DelTree(TOS_DISTRO_DIR "/Linux");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACDefs.DATA*");
  Del(TOS_DISTRO_DIR "/Misc/PCIDevices.DD.Z");
  Del(TOS_DISTRO_DIR "/Misc/Bible.TXT.Z");

  DelTree(TOS_DISTRO_DIR "/Apps");
  DirMk(TOS_DISTRO_DIR "/Apps");
  DelTree(TOS_DISTRO_DIR "/Demo");
  DelTree(TOS_DISTRO_DIR "/Misc/Tour");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACWords.DATA*");
  Del(TOS_DISTRO_DIR "/Adam/God/Vocab.DD.Z");
  Del(TOS_DISTRO_DIR "/Misc/OSTestSuite.HC.Z");
  Copy("C:/Home/Sup1/Sup1Distro/UltraMenu.DD.Z",
	TOS_DISTRO_DIR "/PersonalMenu.DD.Z");
  Copy("C:/Demo/Games/Talons.HC.Z",
	TOS_DISTRO_DIR "/Misc");
}
U0 MakeUltraDistro()
{
  UltraDistroPrep;
  ISO9660ISO(TOS_ISO_NAME,TOS_DISTRO_DIR "/*",,
	TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_ULTRA_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - UltraSmall Distro (%0.2fMB)",
	0.01*(100*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}
U0 MakeUltraRedSeaDistro()
{
  UltraDistroPrep;
  RedSeaISO(TOS_ISO_C_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_ULTRA_RS_SIZE",
	"Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - UltraSmall RedSea Distro (%0.2fMB)",
	0.01*(100*Size(TOS_ISO_C_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 UpdateISODocDefines()
{
  try {
#if MAKE_STD
    DefinePrint("DD_TEMPLEOSCD_SIZE",
	  "Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1fMB)",
	  0.1*(10*Size("D:/Wb/TempleOSCD.ISO","+s")/1024/1024));
    DefinePrint("DD_TEMPLEOSCD_K_SIZE",
	  "%dKB",Size("D:/Wb/TempleOSCD.ISO","+s")/1024);
#endif
#if MAKE_STD_RS
    DefinePrint("DD_TEMPLEOSCD_RS_SIZE",
	  "Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - Standard RedSea Distro (%0.1fMB)",
	  0.1*(10*Size("D:/Wb/TempleOSCDRS.ISO","+s")/1024/1024));
    DefinePrint("DD_TEMPLEOSCD_RS_K_SIZE",
	  "%dKB",Size("D:/Wb/TempleOSCDRS.ISO","+s")/1024);
#endif
#if MAKE_ULTRA
    DefinePrint("DD_TEMPLEOS_ULTRA_SIZE",
	  "Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - UltraSmall Distro (%0.2fMB)",
	  0.01*(100*Size("D:/Wb/TempleOSUltra.ISO","+s")/1024/1024));
    DefinePrint("DD_TEMPLEOS_ULTRA_K_SIZE",
	  "%dKB",Size("D:/Wb/TempleOSUltra.ISO","+s")/1024);
#endif
#if MAKE_ULTRA_RS
    DefinePrint("DD_TEMPLEOS_ULTRA_RS_SIZE",
	  "Download $TX,"TempleOS V5.02",D="DD_OS_NAME_VERSION"$ - UltraSmall RedSea Distro (%0.2fMB)",
	  0.01*(100*Size("D:/Wb/TempleOSUltraRS.ISO","+s")/1024/1024));
    DefinePrint("DD_TEMPLEOS_ULTRA_RS_K_SIZE",
	  "%dKB",Size("D:/Wb/TempleOSUltraRS.ISO","+s")/1024);
#endif
  } catch
    Fs->catch_except=TRUE;
}
UpdateISODocDefines;


I64 tos_progress;
F64 tos_progress_t0;

U0 TOSProgress(U8 *st)
{
  U8 buf[STR_LEN];
  progress4=tos_progress;
  progress3_max=1;
  *progress4_desc=0;
  progress4_max=13+MAKE_STD+MAKE_STD_RS+MAKE_ULTRA+MAKE_ULTRA_RS+MAKE_SUP1+
	MAKE_WEB+MAKE_BOOKS1+MAKE_BOOKS2+MAKE_DBG+MAKE_STAFF;
  progress4_t0=tos_progress_t0;
  StrPrint(buf,"%d. %s",++progress4,st);
  "$$PURPLE$$$$TX+CX,\"%s\"$$$$FG$$\n",buf;
  StrCpy(progress3_desc,buf);
  tos_progress=progress4;
}

I64 TOSProfanity()
{
  I64 res;
  if (res=Profanity(,"D:/Wb/*;"
	"!*/Misc/PCIDevices.DD*;!*/Misc/Bible.TXT*;"
	"!*/Books?/*;!*/Sup1/Sup1Words/*;!*/Sup1/Sup1Blog/Profanity.DD*;"
	"!*/Sup1/Sup1Blog/NumBible.TXT*;!*/God/Vocab.DD*")) {
    while (PressAKey!=CH_SHIFT_ESC) {
      ProgressBarsRst;
      if (Profanity(,"C:/Home/*;"
	    "!*/Misc/PCIDevices.DD*;!*/Misc/Bible.TXT*;"
	    "!*/Books?/*;!*/Sup1/Sup1Words/*;!*/Sup1/Sup1Blog/Profanity.DD*;"
	    "!*/Sup1/Sup1Blog/NumBible.TXT*;!*/God/Vocab.DD*")) {
	"\n\nEdit profanity and press $FG,2$ESC$FG$.\n";
	if (!View)
	  break;
	DocBottom;
      } else
	break;
    }
  }
  return res;
}

U0 TOSRegen3()
{
  I64 slash_home=0;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  tos_progress=-1;
  tos_progress_t0=tS;
  RegExe("TempleOS/TOSRegen");

  TOSProgress("DskChk All");
  AutoComplete;
  WinBorder;
  WinMax;
  DskChkAll;

  TOSProgress("Make LineRep");
  MakeLineRepFile;

  TOSProgress("Copy C to D");
  TOSCopyDrv('C','D');

  TOSProgress("Del D:/Wb");
  DelTree ("D:/Wb");

  TOSProgress("Copy C to D:/Wb");
  CopyTree("C:/","D:/Wb");
  DocClear;

#if MAKE_STD
  TOSProgress("Make Standard Distro ISO");
  MakeStdDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Wb/TempleOSCD.ISO");
#endif
#if MAKE_STD_RS
  TOSProgress("Make Standard RedSea Distro ISO");
  MakeStdRedSeaDistro;
  DocClear;
  Move(TOS_ISO_C_NAME,"D:/Wb/TempleOSCDRS.ISO");
#endif
#if MAKE_ULTRA
  TOSProgress("Make Ultra Distro ISO");
  MakeUltraDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Wb/TempleOSUltra.ISO");
#endif
#if MAKE_ULTRA_RS
  TOSProgress("Make Ultra RedSea Distro ISO");
  MakeUltraRedSeaDistro;
  DocClear;
  Move(TOS_ISO_C_NAME,"D:/Wb/TempleOSUltraRS.ISO");
#endif
#if MAKE_DBG
  TOSProgress("Make Dbg Distro ISO");
  MakeDbgDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Wb/TempleOSDbg.ISO");
#endif
#if MAKE_STAFF
  TOSProgress("Make Staff Distro ISO");
  MakeStaffDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Wb/TempleOSStaff.ISO");
#endif

#if MAKE_SUP1
  TOSProgress("Make Supplemental #1 ISO");
  RedSeaISO("D:/Wb/TempleOSSup1","C:/Home/Sup1");
#endif
#if MAKE_WEB
  TOSProgress("Make www.templeos.org ISO");
  RedSeaISO("D:/Wb/TempleOSWeb","C:/Home/Web");
#endif
#if MAKE_BOOKS1
  TOSProgress("Make Books #1 ISO");
  RedSeaISO("D:/Wb/TempleOSBooks1","C:/Home/Books1");
#endif
#if MAKE_BOOKS2
  TOSProgress("Make Books #2 ISO");
  RedSeaISO("D:/Wb/TempleOSBooks2","C:/Home/Books2");
#endif

  UpdateISODocDefines;
  Cd("C:/");
  DocClear;

  TOSProgress("Check for Long Lines");
  if (LongLines)
    throw;

  DocClear;
  TOSProgress("Check for Broken DolDoc Links");
  if (LinkChk)
    throw;

  DocClear;
  TOSProgress("Make Symbol Files");
  MakeSymFiles;

  DocClear;
  TOSProgress("Freshen WebSite");
  FreshenSite;

  DocClear;
  TOSProgress("Check WebSite for Profanity");
  if (TOSProfanity)
    throw;

  TOSProgress("Find /Home");
  slash_home=F2("/Home","-i+la");

  TOSProgress("DskChk All");
  Drv('C');
  DskChkAll;

  TOSProgress("Done");
  SettingsPop;
  "F2(\"/Home\") Cnt\t:%d\n",slash_home;
  "Elapsed Time\t:%5.3fs\n",tS-progress4_t0;
  ProgressBarsRst("TempleOS/TOSRegen");
}

public U0 TOSPreRegen()
{//Copy bookmarks and bins from D:/Home/Sup1 to C:/Home/Sup1
  Copy("D:/Home/Sup1/Sup1Blog/Bookmarks.html","C:/Home/Sup1/Sup1Blog");
  Del("C:/Home/Sup1/Sup1Bin/*");
  Copy("D:/Home/Sup1/Sup1Bin/*","C:/Home/Sup1/Sup1Bin");
  Copy("D:/Home/Sup1/Sup1CodeScraps/Comm/TOSSocket*",
	"C:/Home/Sup1/Sup1CodeScraps/Comm");
  Copy("D:/Home/Sup1/Sup1Bin/God*","C:/Linux");
}

public U0 TOSHomeIn()
{//Copy D:/Home to C:/Home
  DelTree("C:/Home");
  CopyTree("D:/Home","C:/Home");
}

public U0 TOSHomeOut()
{//Copy C:/Home to D:/Home
  DelTree("D:/Home");
  CopyTree("C:/Home","D:/Home");
}

public U0 TOSSongsIn()
{//Copy songs from D:/Home/Sup1 to C:/Home/Sup1
  Del("C:/Home/Sup1/Sup1Hymns/*");
  Del("C:/Home/Sup1/Sup1Songs/*");
  Copy("D:/Home/Sup1/Sup1Hymns/*","C:/Home/Sup1/Sup1Hymns");
  Copy("D:/Home/Sup1/Sup1Songs/*","C:/Home/Sup1/Sup1Songs");
}

public U0 TOSSongsOut()
{//Copy songs from C:/Home/Sup1 to D:/Home/Sup1
  Del("D:/Home/Sup1/Sup1Hymns/*");
  Del("D:/Home/Sup1/Sup1Songs/*");
  Copy("C:/Home/Sup1/Sup1Hymns/*","D:/Home/Sup1/Sup1Hymns");
  Copy("C:/Home/Sup1/Sup1Songs/*","D:/Home/Sup1/Sup1Songs");
}

U0 TOSRegen2()
{
  TOSChgLog; //$LK,"::/Doc/ChgLog.DD"$ datetime matches Kernel $LK,"sys_compile_time",A="MN:sys_compile_time"$.
  TOSBootHDIns('C');
  Once("TOSRegen3;");
  BootRAM("C:/Kernel/" KERNEL_BIN_C); //Boot new Kernel and Compiler.
}

public U0 TOSRegen()
{//Generate distro ISO's and website image, D:/Wb.
  TOSBootHDIns('C');
  Once("TOSRegen2;");
  BootRAM("C:/Kernel/" KERNEL_BIN_C); //Boot to load $LK,"TOS_CFG",A="PF:::/Demo/AcctExample/TOS/TOSCfg.HC,TOS_CFG"$.
}
