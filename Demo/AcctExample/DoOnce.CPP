//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoOnce",A="FF:/Home/HomeSys.CPP,DoOnce"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.TXT,/Home Files"$.

//  Type("::/Doc/Customize.TXT");

U0 DoOnce()
{
  switch (sys_boot_src.u16[0]) {
    case BOOT_SRC_DVD:
      "\nIf you answer 'No' you can play with\n"
	    "the live CD without installing.\n\n"
	    "Install onto hard drive ";
      if (YorN) {
	DocBottom;
	ExeFile("::/Misc/OSInstall");
	if (ExePrint("OSInstall(FALSE);")) {
	  *sys_registry_doc->filename.name='C';
	  "\n\n1) TAD\n\n";
	  switch (GetI64("Staff Member Number:")) {
	    case 1:
	      RegWriteBranch("TempleOS/Staff",
		    "#define PERSONAL_INITIALS \"TAD\"\n"
		    "#define MACHINE_CFG %d\n",
		    GetI64("Machine Cfg Number:")
	      );
	      break;
	  }
	  Del("C:/Home/DoDistro.CPP.Z");
	  Del("D:/Home/DoDistro.CPP.Z");
	  "\n\n\nAfter Reboot be sure to run MakeDistrosAndBackUp.\n\n";
	  PressAKey;
	  Reboot; //Too dangerous for amateurs until reboot.
	}
      }
      break;
  }
  if (FileFind("D:/Temp/Logs/access.log")) {
    Auto("\n\n%C",CH_ESC);
    WebLogRep("D:/Temp/Logs/*.log","D:/Temp/LogRep.TXT");
    Del("D:/Temp/Logs/*.log");
  }
}

DoOnce;
