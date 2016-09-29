//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"DoOnce",A="FF:/Home/HomeSys.HC,DoOnce"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.DD,/Home Files"$.

//  Type("::/Doc/Customize.DD");

U0 DoOnce()
{
  switch (sys_boot_src.u16[0]) {
    case BOOT_SRC_DVD:
      "\nIf you answer 'No' you can play with\n"
	    "the live CD without installing.\n\n"
	    "Install onto hard drive ";
      if (YorN) {
	DocBottom;
	if (RunFile("::/Misc/OSInstall",,FALSE)) {
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
	  Del("C:/Home/DoDistro.HC.Z");
	  Del("D:/Home/DoDistro.HC.Z");
	  "\n\n\nAfter Reboot be sure to run MakeDistrosAndBackUp.\n\n";
	  PressAKey;
	  Reboot; //Too dangerous for amateurs until reboot.
	}
      }
      break;
  }
  if (FileFind("D:/Tmp/Logs/access.log")) {
    In("\n\n%C",CH_ESC);
    RunFile("::/Demo/WebLogDemo/WebLogRep",,
	  "D:/Tmp/Logs/*.log","D:/Home/WebLogRep.DD");
    Del("D:/Tmp/Logs/*.log");
  }
}

DoOnce;
