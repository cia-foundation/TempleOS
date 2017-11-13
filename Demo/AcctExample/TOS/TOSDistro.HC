//The CFG defines are $LK,"~/TOS/TOSCfg.HC",A="FI:::/Demo/AcctExample/TOS/TOSCfg.HC"$.

#help_index "Misc/TOS/Distro"

#define MAKE_LITE	1
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

  if (dst=='D')
    Fmt(dst,,FALSE,FSt_FAT32);
  else
    Fmt(dst,,FALSE,FSt_REDSEA);

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

U0 DistroPrep()
{
  AOnceFlush; //Don't want in Registry
  OnceFlush;

  Del("/Home/Demo*");
  DelTree("/Home/*Tmp.DD.Z");

  DelTree("/Tmp");
  DirMk("/Tmp");
  DirMk("/Tmp/ScrnShots");

  Touch("/PersonalMenu.DD.Z","+T");
  Touch("/Home/PersonalMenu.DD.Z","+T");

  DelTree("/Demo/AcctExample");
  CopyTree("/Home","/Demo/AcctExample");
  DelTree("/Demo/AcctExample/TAD");
  DelTree("/Demo/AcctExample/Sup1");
  DelTree("/Demo/AcctExample/Sup2");
  DelTree("/Demo/AcctExample/Sup3");
  Del("/Demo/AcctExample/Test*");
  if (FileFind("~/Sup1/Sup1Utils/SortHeaders.HC.Z"))
    ExeFile("~/Sup1/Sup1Utils/SortHeaders.HC.Z");

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
  RedSeaISO(TOS_ISO_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSCD_SIZE",
	"Download $TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

U0 LiteDistroPrep()
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
  DelTree(TOS_DISTRO_DIR "/Apps");
  DelTree(TOS_DISTRO_DIR "/Demo");
  Copy(TOS_DISTRO_DIR "/Demo/Games/Talons.HC.Z",TOS_DISTRO_DIR "/Home");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
  Del(TOS_DISTRO_DIR BOOT_DIR "/OldMBR.BIN.C");
  Del(TOS_DISTRO_DIR BOOT_DIR "/BootMHD2.BIN.C");
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACDefs.DATA.Z");
  Del(TOS_DISTRO_DIR "/Adam/AutoComplete/ACWords.DATA.Z");
  Del(TOS_DISTRO_DIR "/Misc/Bible.TXT.Z");
}
U0 MakeLiteDistro()
{
  LiteDistroPrep;
  RedSeaISO(TOS_ISO_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOSCD_SIZE",
	"Download $TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1fMB)",
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
  RedSeaISO(TOS_ISO_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_DBG_SIZE",
	"Download $TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$ - Debug Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
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
  DelTree(TOS_DISTRO_DIR "/Home/Sup2");
  DelTree(TOS_DISTRO_DIR "/Home/Sup3");
  Del(TOS_DISTRO_DIR "/" KERNEL_BIN_C);
}
U0 MakeStaffDistro()
{
  StaffDistroPrep;
  RedSeaISO(TOS_ISO_NAME,TOS_DISTRO_DIR,TOS_DISTRO_DIR BOOT_DIR_KERNEL_BIN_C);
  DefinePrint("DD_TEMPLEOS_STAFF_SIZE",
	"Download $TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$ - T.S. Company Internal Distro (%0.1fMB)",
	0.1*(10*Size(TOS_ISO_NAME,"+s")/1024/1024));
  Drv('C');
}

I64 UpdateLineCnts()
{
  I64 res;

  DocClear;
  Drv('C');
  DistroPrep;

  Cd("C:/");
  DelTree("B:/TOS/TOS");
  CopyTree("C:/Home","B:/TOS/TOS");
  DelTree("C:/Home");

  DocMax;
  DocClear;
  res=LineRep("C:/*","-r")+LineRep("C:/Adam/*")+
	LineRep("C:/Compiler/*","-S+$$")+LineRep("C:/Kernel/*");
  CopyTree("B:/TOS/TOS","C:/Home");
  DelTree("B:/TOS/TOS");

  DocTreeFWrite("C:/Adam/ADefine.HC.Z","LineRep",
	"DefinePrint(\"DD_TEMPLEOS_LOC\",\"%,d\");\n",res);
  DefinePrint("DD_TEMPLEOS_LOC","%,d",res);

  "Total LOC:%12,d\n\n",res;
  return res;
}

U0 UpdateISODocDefines()
{
  try {
    DefinePrint("DD_TEMPLEOSCD_SIZE",
	  "Download $TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$ - Standard Distro (%0.1fMB)",
	  0.1*(10*Size("D:/Downloads/TOS_Distro.ISO","+s")/1024/1024));
    DefinePrint("DD_TEMPLEOSCD_K_SIZE",
	  "%dKB",Size("D:/Downloads/TOS_Distro.ISO","+s")/1024);
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
  progress4_max=9+MAKE_LITE+MAKE_DBG+MAKE_STAFF;
  progress4_t0=tos_progress_t0;
  StrPrint(buf,"%d. %s",++progress4,st);
  "$$PURPLE$$$$TX+CX,\"%s\"$$$$FG$$\n",buf;
  StrCpy(progress3_desc,buf);
  tos_progress=progress4;
}

U0 TOSRegen2()
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

  TOSProgress("Update Line Cnts");
  UpdateLineCnts;

  TOSProgress("Copy C to D");
  TOSCopyDrv('C','D');

  TOSProgress("Make Standard Distro ISO");
  MakeStdDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Downloads/TOS_Distro.ISO");

  TOSProgress("Make Supplemental1 ISO");
  RedSeaISO("D:/Downloads/TOS_Supplemental1","C:/Home/Sup1");

  TOSProgress("Make Supplemental2 ISO");
  RedSeaISO("D:/Downloads/TOS_Supplemental2","C:/Home/Sup2");

  TOSProgress("Make Supplemental3 ISO");
  RedSeaISO("D:/Downloads/TOS_Supplemental3","C:/Home/Sup3");

#if MAKE_LITE
  TOSProgress("Make Lite Distro ISO");
  MakeLiteDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Downloads/TOS_Lite.ISO");
#endif
#if MAKE_DBG
  TOSProgress("Make Dbg Distro ISO");
  MakeDbgDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Downloads/TOS_Dbg.ISO");
#endif
#if MAKE_STAFF
  TOSProgress("Make Staff Distro ISO");
  MakeStaffDistro;
  DocClear;
  Move(TOS_ISO_NAME,"D:/Downloads/TOS_Staff.ISO");
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
{//Copy bins from D:/Home/Sup1 to C:/Home/Sup1
  Copy("D:/Home/" INS_REG_PERSONAL_INITIALS "/*",
	"C:/Home/" INS_REG_PERSONAL_INITIALS);
  DelTree("C:/Home/Sup1/Sup1Bin");
  CopyTree("D:/Home/Sup1/Sup1Bin","C:/Home/Sup1/Sup1Bin");
  Copy("D:/Home/Sup1/Sup1CodeScraps/Comm/TOSSocket*",
	"C:/Home/Sup1/Sup1CodeScraps/Comm");
  DelTree("C:/Downloads/Linux");
  CopyTree("D:/Downloads/Linux","C:/Downloads/Linux");
}

public U0 TOSRegen()
{//Generate distro ISO's
  TOSBootHDIns('C');
  Once("TOSRegen2;");
  BootRAM("C:/Kernel/" KERNEL_BIN_C); //Boot to load $LK,"TOS_CFG",A="PF:::/Demo/AcctExample/TOS/TOSCfg.HC,TOS_CFG"$.
}
