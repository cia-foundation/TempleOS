//Make Your own Distro by #include-ing this file.

#define STD_DISTRO_DVD_CFG	"Tb1\nScale2Mem(2048,0x40000)\nt3 \n\n\n\n"

U0 MakeMyISO(U8 *out_iso_filename)
{//Does everything with current drive.
  if (!DrvIsWritable) {
    "Drive must be writable.  Install on Hard drive, first.\n";
    return;
  }
  DelTree("/Distro");
  Del(out_iso_filename);

  MkDir("/Distro");
  Auto(STD_DISTRO_DVD_CFG);
  MakeOSDrvBootInstall(,'T');

  Copy("/*","/Distro");
  Del("/Distro/" KERNEL_BIN_C);

  CopyTree(BOOT_DIR,	"/Distro" BOOT_DIR);
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
  ISOFileCreate(out_iso_filename,"/Distro/*",,"/Distro" BOOT_DIR_KERNEL_BIN_C);

  //If CD-ROM use MT_CD instead of MT_DVD.
  //DVDImageWrite('T',out_iso_filename,MT_DVD); //Uncomment to burn.

  //DelTree("/Distro");
}

MakeMyISO("/MyDistro.ISO");

// Study my account examples $LK,"Cfg Strs",A="FL:::/Demo/AcctExample/TOSCfg.CPP,1"$, $LK,"Update Funs",A="FL:::/Demo/AcctExample/TOSDistro.CPP,1"$.
