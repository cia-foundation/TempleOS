//Place this file in /Home and change
//anything you want.

//This file is executed by the
//first terminal window upon start-up.
//See $LK,"Once",A="FF:/Home/HomeSys.HC,Once"$ and $LK,"Home Files",A="FF:::/Doc/GuideLines.DD,/Home Files"$.

//  Type("::/Doc/Customize.DD");

U0 Tmp()
{
  OnceExe;
  switch (sys_boot_src.u16[0]) {
    case BOOT_SRC_DVD:
      "\nIf you answer 'No' you can play with\n"
	    "the live CD without installing.\n\n"
	    "Install onto hard drive ";
      if (YorN) {
	DocBottom;
	if (RunFile("::/Misc/OSInstall",,FALSE)) {
	  Del("C:/Home/DoDistro.HC.Z");
	  Del("D:/Home/DoDistro.HC.Z");
	  OnceDrv('C',"\"\n\nRun TOSStaffIns;\n\n\n\";");
	  OnceDrv('D',""); //This command uses cached Registry file.
	  if (PressAKey!=CH_SHIFT_ESC)
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

Tmp;
