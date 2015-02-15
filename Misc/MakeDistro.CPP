//Make Your own Distro by #include-ing this file.

#define STD_DISTRO_CDROM_CFG	"Tb1\nScale2Mem(2048,0x40000)\nt3 \n\n\n\n"

U0 MakeMyISO(U8 *out_iso_filename)
{//Does everything with current drive.
  if (!IsDrvWritable) {
    "Drive must be writable.  Install on Hard drive, first.\n";
    return;
  }
  DelTree("/Distro");
  MkDir("/Distro");
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSDrvBootInstall(,'T');

  Copy("/*","/Distro");
  Del("/Distro/Kernel.BIN.C");

  CopyTree("/0000Boot",	"/Distro/0000Boot");
  CopyTree("/Home",	"/Distro/Home");
  CopyTree("/Adam",	"/Distro/Adam");
  CopyTree("/Apps",	"/Distro/Apps");
  CopyTree("/Compiler",	"/Distro/Compiler");
  CopyTree("/Demo",	"/Distro/Demo");
  CopyTree("/Doc",	"/Distro/Doc");
  CopyTree("/Kernel",	"/Distro/Kernel");
  CopyTree("/Misc",	"/Distro/Misc");

  //To save space, optionally delete dictionary.
  //Del("/Distro/Adam/AutoComplete/ACDefs.DAT");
  CopyTree("/Linux","/Distro/Linux");	  //You can leave this out.
  MkDir("/Distro/Temp");
  MkDir("/Distro/Temp/ScreenShots");
  CDFileCreate(out_iso_filename,"/Distro/*",,"/Distro" BOOT_DIR_KERNEL_BIN_C);
  //CDImageWrite('T',out_iso_filename,MT_CD); //Uncomment to burn a CD-ROM or (MT_DVD)
  //DelTree("/Distro");
}

MakeMyISO("/MyDistro.ISO");

// Study my account examples $LK,"Cfg Strs",A="FL:::/Demo/AcctExample/TOSCfg.CPP,1"$, $LK,"Update Funs",A="FL:::/Demo/AcctExample/TOSDistro.CPP,1"$.
