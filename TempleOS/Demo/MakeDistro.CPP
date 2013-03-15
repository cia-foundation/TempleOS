/*$HL,0$
$FG,5$$TX+CX,"Roll Your Own Distro"$$FG$

It's unlikely you will get it running on bare hardware, so you'll probably
run it in a virtual machine such as VMware.  Download the free 64-bit
$FG,4$$TX+HTML,"VMware Player","https://my.vmware.com/web/vmware/free#desktop_end_user_computing/vmware_player/5_0"$$FG$ for Linux or Windows and point the CDROM at the $FG,4$$TX+HTML,"TempleOSCD.ISO","http://www.templeos.org/TempleOSCD.ISO"$$FG$.
(The $FG,4$$TX+HTML,"VMware","https://my.vmware.com/web/vmware/free#desktop_end_user_computing/vmware_player/5_0"$$FG$ web page takes a whole minute to load.  They hide the free version.)

Purchase $FG,4$$TX+HTML,"VMware WorkStation","http://www.vmware.com"$$FG$ or download $FG,4$$TX+HTML,"WinImage","http://www.winimage.com/winimage.htm"$$FG$ if you wish to transfer files
in and out.  The VMWare Workstation will stretch to full-screen and make drives
accessible from Windows.

$HL,1$*/

#define STD_DISTRO_CDROM_CFG	"b10x20000\nt3 \n\n\n\n\n"

U0 MakeMyISO(U8 *out_iso_filename)
{//Does everything with current drive.
  if (!IsDrvWritable) {
    "Drive must be writable.  Install on Hard drive, first.\n";
    return;
  }
  DelTree("/Distro");
  Auto(STD_DISTRO_CDROM_CFG);
  MakeOSInstallPrtBootLoader(,'T');
  CopyTree("/0000Boot","/Distro/0000Boot");
  CopyTree("/TempleOS","/Distro/TempleOS");
  //To save space, optionally delete dictionary.
  //Del("/Distro/TempleOS/Adam/WordStat/WSDefs.DAT");
  CopyTree("/Windows","/Distro/Windows"); //You can leave this out
  CopyTree("/Linux","/Distro/Linux");	  //You can leave this out.
  MkDir("/Distro/Temp");
  MkDir("/Distro/Temp/ScreenShots");
  CDFileCreate(out_iso_filename,"/Distro/*","",BOOTCD_BIN_Z);
  //CDImageWrite('T',out_iso_filename,MT_CD); //Uncomment to burn a CD-ROM or (MT_DVD)
  //DelTree("/Distro");
}

MakeMyISO("/MyDistro.ISO");

// Study my account examples $LK,"Cfg Strs","FL:::/TempleOS/Demo/AcctExample/TSAdam.CPP,1"$ $LK,"Update Funs","FL:::/TempleOS/Demo/AcctExample/TSDistro.CPP,1"$.





/*

* "Linux" is probably a trademark owned by Linus Torvalds.
* "VMware Player" and "VMware WorkStation" are trademarks of VMware, Inc.
* "Windows" is a trademark of MicroSoft Corp.
* "WinImage" is a trademark of Gilles Vollant.

*/
