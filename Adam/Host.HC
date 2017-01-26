#help_index "Misc/Host"

public Bool HostChgDsk(U8 *iso_filename)
{//Help user change CD/DVD or ISO_file.
  Bool res=FALSE;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  try {
    AutoComplete;
    WinBorder;
    WinMax;
    DocClear;
    "$$PURPLE$$$$TX+CX,\"Set to %s\"$$$$FG$$\n\n\n\n",iso_filename;
    switch (ins_reg.host_vm) {
      case VM_VMWARE:
	if (IsDotC(iso_filename))
"
Click CDROM Icon in bottom-right border of VMware window.

Click \"Settings\".

Set ISO image to \"%s\"$FG$.
When browsing, change the file mask from \"*.iso\" to \"All Files\".

",iso_filename;
	  else
"
Click CDROM Icon in bottom-right border of VMware window.

Click \"Settings\".

Set ISO image to \"$FG,2$%s$FG$\"$FG$.

",iso_filename;
	break;
      case VM_QEMU:
"
Change to QEMU monitor using $FG,2$<CTRL-ALT-2>$FG$

Get the CDROM designation.

	>$FG,2$info block$FG$

Use the designation.

	>$FG,2$eject ide1-cd0$FG$

Use the designation.

	>$FG,2$change ide1-cd0 %s$FG$

Change back to QEMU using $FG,2$<CTRL-ALT-1>$FG$

",iso_filename;
	break;
      default:
"
Place the CDDVD %s in the drive.

",iso_filename;
    }
    if (PressAKey!=CH_SHIFT_ESC) {
      ">$FG,2$DskChg('T');$FG$\n\n";
      DskChg('T');
      res=TRUE;
    }
  } catch
    PutExcept;
  SettingsPop;
  return res;
}
