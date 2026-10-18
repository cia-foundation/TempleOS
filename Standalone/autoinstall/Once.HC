//Unattended install, used only by Standalone/mkdisk.py.
//mkiso.py --auto-install puts this in T:/Home/, where it replaces
//::/Once.HC as the first terminal's start-up file.

//Answers for ::/Misc/OSInstall.HC prompts: inside a VM? Y.
//PressAKey: space.  Reboot now? N.
In("Y N");
RunFile("::/Misc/OSInstall",,FALSE);

//InstallDrv() copied this file to the new drives; don't install again
//when they boot.
Del("C:/Home/Once.HC*");
Del("D:/Home/Once.HC*");

"Unattended install done.\n";
Sleep(2000);
//Quit QEMU through its isa-debug-exit device (iobase 0xF4).
OutU8(0xF4,0x10);
