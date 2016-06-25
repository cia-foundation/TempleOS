//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoOnce",A="FF:/Home/HomeSys.CPP,DoOnce"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.TXT,/Home Files"$.

//Delete the rest from this file.

U0 DoOnce()
{
  switch (sys_boot_src.u16[0]) {
    case BOOT_SRC_ROM:
      "Continue booting hard drive ";
      if (YorN) {
	DocBottom;
	ExeFile("C:/Home/DoOnce");
      }
      break;
    case BOOT_SRC_DVD:
      "\nIf you answer 'No' you can play with\n"
	    "the live CD without installing.\n\n"
	    "Install onto hard drive ";
      if (YorN) {
	DocBottom;
	ExeFile("::/Misc/OSInstall");
	ExePrint("OSInstall(TRUE);");
      }
      if (FileFind("::/Misc/Tour")) {
	"\nTake Tour";
	if (YorN) {
	  DocBottom;
	  Cd("::/Misc/Tour");
	  AutoFile("Tour");
	}
      }
      break;
    case BOOT_SRC_RAM:
    case BOOT_SRC_HARDDRV:
      "$$PURPLE$$$$TX+CX,\"Tip of the Day\"$$$$FG$$\n";
      TipOfDay;
      Type("::/Doc/Customize.TXT");
      if (FileFind("::/Misc/Tour")) {
	"\nTake Tour";
	if (YorN) {
	  DocBottom;
	  Cd("::/Misc/Tour");
	  AutoFile("Tour");
	}
      }
      break;
  }
}

DoOnce;
