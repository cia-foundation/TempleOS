#help_index "Misc/TOS"

U0 TOSStaffIns()
{//Completes TOS install from TempleOSStaff.ISO
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  "$$PURPLE$$$$TX+CX,\"Complete TOS install from TempleOSStaff.ISO\"$$$$FG$$\n\n\n\n";
  InsRereg;

  "\nDownload TempleOSSup1.ISO.C\tfrom http://www.templeos.org AppStore.\n"
  "Download TempleOSBooks1.ISO.C\tfrom http://www.templeos.org AppStore.\n"
  "Download TempleOSBooks2.ISO.C\tfrom http://www.templeos.org AppStore.\n\n";

  if (PressAKey!=CH_SHIFT_ESC) {
    if (HostChgDsk("/home/tad/TempleOSSup1.ISO.C"))
      CopyTree("T:/","~/Sup1");
    if (HostChgDsk("/home/tad/TempleOSBooks1.ISO.C"))
      CopyTree("T:/","~/Books1");
    if (HostChgDsk("/home/tad/TempleOSBooks2.ISO.C"))
      CopyTree("T:/","~/Books2");

    Once("TOSBootHDIns;");
    "\nDo you wish to run >$FG,2$TOSRegen;$FG$ to generate\n the "
    "distro ISO's and the website image, $FG,2$D:/Wb$FG$ ";
    if (YorN) {
      Once("TOSRegen;");
      BootRAM("C:/" KERNEL_BIN_C); //Boot so TOSRegen cmd is available.
    } else {
      "\nReboot ";
      if (YorN)
	Reboot;
    }
  }
  SettingsPop;
}

public U0 TOSStdIns()
{//Completes TOS install from TempleOSCD.ISO
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  "$$PURPLE$$$$TX+CX,\"Complete TOS install from TempleOSCD.ISO\"$$$$FG$$\n\n\n\n";
  InsRereg;

  "\nDownload TempleOSSup1.ISO.C\tfrom http://www.templeos.org AppStore.\n"
  "Download TempleOSWeb.ISO.C\tfrom http://www.templeos.org AppStore.\n"
  "Download TempleOSBooks1.ISO.C\tfrom http://www.templeos.org AppStore.\n"
  "Download TempleOSBooks2.ISO.C\tfrom http://www.templeos.org AppStore.\n\n";

  if (PressAKey!=CH_SHIFT_ESC) {
    if (FileFind("/Demo/AcctExample"))
      CopyTree("::/Demo/AcctExample","~");
    if (HostChgDsk("/home/tad/Downloads/TempleOSSup1.ISO.C"))
      CopyTree("T:/","~/Sup1");
    if (HostChgDsk("/home/tad/Downloads/TempleOSWeb.ISO.C"))
      CopyTree("T:/","~/Web");
    if (HostChgDsk("/home/tad/Downloads/TempleOSBooks1.ISO.C"))
      CopyTree("T:/","~/Books1");
    if (HostChgDsk("/home/tad/Downloads/TempleOSBooks2.ISO.C"))
      CopyTree("T:/","~/Books2");

    Once("TOSBootHDIns;");
    "\nDo you wish to run >$FG,2$TOSRegen;$FG$ to generate\n the "
    "distro ISO's and the website image, $FG,2$D:/Wb$FG$ ";
    if (YorN) {
      Once("TOSRegen;");
      BootRAM("C:/" KERNEL_BIN_C); //Boot so TOSRegen cmd is available.
    } else {
      "\nReboot ";
      if (YorN)
	Reboot;
    }
  }
  SettingsPop;
}
