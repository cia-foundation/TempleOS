#help_index "Misc/TOS"

U0 TOSStaffIns()
{//Completes TOS install from TOS_Staff.ISO
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  "$$PURPLE$$$$TX+CX,\"Complete TOS install from TOS_Staff.ISO\"$$$$FG$$\n\n\n\n";
  InsRereg;

  "\nDownload TOS_Supplemental1.ISO.C\tfrom http://www.templeos.org\n"
  "Download TOS_Supplemental2.ISO.C\tfrom http://www.templeos.org\n"
  "Download TOS_Supplemental3.ISO.C\tfrom http://www.templeos.org\n";

  if (PressAKey!=CH_SHIFT_ESC) {
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental1.ISO.C"))
      CopyTree("T:/","~/Sup1");
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental2.ISO.C"))
      CopyTree("T:/","~/Sup2");
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental3.ISO.C"))
      CopyTree("T:/","~/Sup3");

    Once("TOSBootHDIns;");
    "\nDo you wish to run >$FG,2$TOSRegen;$FG$ to generate\n the "
	  "distro ISO's ";
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
{//Completes TOS install from TOS_Distro.ISO
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  "$$PURPLE$$$$TX+CX,\"Complete TOS install from TOS_Distro.ISO\"$$$$FG$$\n\n\n\n";
  InsRereg;

  "\nDownload TOS_Supplemental1.ISO.C\tfrom http://www.templeos.org\n"
  "Download TOS_Supplemental2.ISO.C\tfrom http://www.templeos.org\n"
  "Download TOS_Supplemental3.ISO.C\tfrom http://www.templeos.org\n";

  if (PressAKey!=CH_SHIFT_ESC) {
    if (FileFind("/Demo/AcctExample"))
      CopyTree("::/Demo/AcctExample","~");
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental1.ISO.C"))
      CopyTree("T:/","~/Sup1");
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental2.ISO.C"))
      CopyTree("T:/","~/Sup2");
    if (HostChgDsk("/home/tad/Downloads/TOS_Supplemental3.ISO.C"))
      CopyTree("T:/","~/Sup3");

    Once("TOSBootHDIns;");
    "\nDo you wish to run >$FG,2$TOSRegen;$FG$ to generate\n the distro ISO's ";
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
