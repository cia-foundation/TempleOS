#help_index "Misc/TOS"

public U0 TOSStaffIns()
{//Completes TOS install from TempleOSStaff.ISO
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  "$$PURPLE$$$$TX+CX,\"Complete TOS install from TempleOSStaff.ISO\"$$$$FG$$\n\n\n\n";
  InsRereg;
  if (HostChgDsk("/home/tad/TAD/Site1/Wb/TempleOSSup1.ISO.C"))
    CopyTree("T:/","~/Sup1");
  if (HostChgDsk("/home/tad/TAD/Site1Stale/files/TempleOSBooks1.ISO.C"))
    CopyTree("T:/","~/Books1");
  if (HostChgDsk("/home/tad/TAD/Site1Stale/files/TempleOSBooks2.ISO.C"))
    CopyTree("T:/","~/Books2");
  SettingsPop;
  Once("TOSBootHDIns;\"\n\nRun TOSRegen;\n\n\n\";");
  if (PressAKey!=CH_SHIFT_ESC)
    Reboot;
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
  if (FileFind("/Demo/AcctExample"))
    Copy("::/Demo/AcctExample/*","~");
  if (HostChgDsk("/home/tad/Downloads/TempleOSSup1.ISO.C"))
    CopyTree("T:/","~/Sup1");
  if (HostChgDsk("/home/tad/Downloads/TempleOSWeb.ISO.C"))
    CopyTree("T:/","~/Web");
  if (HostChgDsk("/home/tad/Downloads/TempleOSBooks1.ISO.C"))
    CopyTree("T:/","~/Books1");
  if (HostChgDsk("/home/tad/Downloads/TempleOSBooks2.ISO.C"))
    CopyTree("T:/","~/Books2");
  SettingsPop;
  Once("TOSBootHDIns;\"\n\nRun TOSRegen;\n\n\n\";");
  if (PressAKey!=CH_SHIFT_ESC)
    Reboot;
}
