//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoOnce",A="FF:/Home/AdamSys.CPP,DoOnce"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.TXT,/Home Files"$.

//Delete the rest from this file.

U0 DoOnce()
{
  if (sys_boot_src.u16[0]==BOOT_SRC_CDROM) {
    "\nIf you answer 'No' you can play with\n"
    "the live CD without installing.\n\n"
    "Install onto hard drive ";
    if (YorN) {
      DocBottom;
      ExeFile("::/Misc/OSInstall");
    }
  }
  Type("::/Doc/Customize.TXT");
  "\nTake Tour";
  if (YorN) {
    Cd("::/Misc/Tour");
    AutoFile("Tour");
  }
}

DoOnce;
