Bool VMPrtDsk(CTask *task,CATARep *ata_drv)
{
  if (ata_drv) {
    XTalkWait(task,"PrtDsk;\nYC2p%d\nYYY",ata_drv->num);
    XTalkWait(task,"%d\nY\nY\n",(drv_dsk_dsk_size-DRV_HEADER)/2);
    return TRUE;
  } else
    return FALSE;
}

U0 VMInstallBoot(CTask *task,U8 cd_drv,U8 drv,CATARep *ata_drv,CATARep *atapi_drv)
{
  U8 *st1,*st2;

  st1=MStrPrint("%C:/",cd_drv);
  st2=MStrPrint("%C:/",drv);
  CopyTree(st1,st2);
  Free(st1);
  Free(st2);

  st1=MStrPrint("%C:/Home",drv);
  MkDir(st1);
  Copy("::/Misc/MakeDistro.CPP.Z",st1);
  Free(st1);

  XTalkWait(task,
    "MakeOSDrvBootInstall('%C');\n"
    "\nB1\n0x20000\n",drv);

  if (ata_drv)
    XTalkWait(task,"C2p%d\n",ata_drv->num);
  if (atapi_drv)
    XTalkWait(task,"T3p%d\n",atapi_drv->num);

  XTalkWait(task,
    "\n"   //Exit Drives
    "\n"   //Dsk Cache
    "\n"); //Options
}

U0 VMInstallWizard()
{
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CTask *task;
  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=User;
  TaskWait(task);
  WinHorz(Fs->win_left,Fs->win_right,task);
  WinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  WinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);

  ATARep(FALSE,TRUE,&root);
  ATAIDDrvs(root,dsk.boot_drv_let,&ata_drv,&atapi_drv);
  if (VMPrtDsk(task,ata_drv)) {
    VMInstallBoot(task,dsk.boot_drv_let,'C',ata_drv,atapi_drv);
    VMInstallBoot(task,dsk.boot_drv_let,'D',ata_drv,atapi_drv);
    MasterBootIntall('C');
  }
  LinkedLstDel(root);

  WinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 RegularInstallWizard()
{
  I64 unit;
  U8 *base0,*base1,*ms,*dst,*st1,*st2,*new_home_str;
  CATARep *root=NULL,*tempha;
  I64 ch,num_hints;
  CTask *task;

  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=User;
  TaskWait(task);
  WinHorz(Fs->win_left,Fs->win_right,task);
  WinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  WinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);
  XTalk(task,"Mount;\nC2 ");

  num_hints=ATARep(FALSE,,&root);
  "\nInclude '$$PURPLE$$0x$$FG$$' for hexidecimal numbers.\n\n";
  while (TRUE) {
    base0=GetStr("Hard Drive I/O Port Base0  : ");
    if (0<Str2I64(base0)<=0xFFFF)
      break;
    Free(base0);
  }
  if (1<=Str2I64(base0)<=num_hints) {
    tempha=ATARepFind(&root,Str2I64(base0));
    Free(base0);
    base0=MStrPrint("0x%X",tempha->base0);
    base1=MStrPrint("0x%X",tempha->base1);
    ms	 =MStrPrint("0x%X",tempha->unit);
    unit=Str2I64(ms);
  } else {
    while (TRUE) {
      base1=GetStr("Hard Drive I/O Port Base1  : ");
      if (0<Str2I64(base1)<=0xFFFF)
	break;
      Free(base1);
    }
    while (TRUE) {
      ms =GetStr("Unit--$$PURPLE$$0$$FG$$=Master or $$PURPLE$$1$$FG$$=Slave: ","0");
      unit=Str2I64(ms);
      if (0<=unit<=1)
	break;
    }
  }
  LinkedLstDel(root);
  XTalkWait(task,"%s\n%s\n%C\n",base0,base1,'0'+unit);
  DrvRep;
  while (TRUE) {
    dst=GetStr("\nDestination Partition Letter: ");
    if ('A'<=ToUpper(*dst)<='Z')
      break;
    Free(dst);
  }

  '\n';
  st1=MStrPrint("%C:/",dsk.boot_drv_let);
  st2=MStrPrint("%C:/",*dst);

  "$$RED$$Format %C Partition?$$FG$$\n",*dst;
  if (YorN) {
    '\n';
    while (TRUE) {
    "$$PURPLE$$1$$FG$$) Use FAT32\n"
      "$$PURPLE$$2$$FG$$) Use RedSea\n"
      "\nFile System Type: ";
      ch=GetChar;
      '\n';
      if ('1'<=ch<='2')
	break;
    }
    if (ch=='1')
      Fmt(*dst,,FALSE,DVT_FAT32);
    else
      Fmt(*dst,,FALSE,DVT_REDSEA);
  }
  CopyTree(st1,st2);
  st2=MStrPrint("%C:/Temp",*dst);
  MkDir(st2);
  Free(st2);

  new_home_str=MStrPrint("%C:/Home",*dst);
  MkDir(new_home_str);
  Copy("::/Misc/MakeDistro.CPP.Z",new_home_str);
  XTalkWait(task,
  "MakeOSDrvBootInstall('%C');\n"
  "\nB1\n0x20000\n"
  "C"
  "2p"
  "%s\n"
  "%s\n"
  "%C"
  "\n"	 //Exit Drives
  "\n"	 //Dsk Cache
  "\n",	 //Options
  *dst,base0,base1,'0'+unit);
  "$$RED$$Install Master Boot loader?$$FG$$";
  if (YorN) {
    '\n';
    MasterBootIntall(*dst);
  }
  Free(new_home_str);
  Free(base0);
  Free(base1);
  Free(ms);
  Free(dst);

  WinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 DoInstructions()
{
  CTask *task;
  AutoComplete;
  task=Fs->next_task;
  while (task!=Fs) {
    if (task!=adam_task && task!=sys_winmgr_task && task!=ac.task) {
      XTalk(task,"Ed(\"::/Doc/Install.TXT.Z\");\n");
      break;
    }
    task=task->next_task;
  }
}

U0 DoInstall()
{
  I64 vm_install,pmt_reboot;
  "\n\n\n\n\nAre you installing inside VMware, QEMU, VirtualBox "
  "or a similar virtual machine? ";
  vm_install=YorN;
  DocBottom;

  if (vm_install) {
    VMInstallWizard();
    pmt_reboot=TRUE;
  } else {
    "\n\nThis wizard works if you have a partition ready.  "
    "You can partition the drive or MakeOSDrvBootInstall() "
    "with more options if you do it by hand, not using this wizard.\n\n"
    "Continue Install Wizard ";
    if (pmt_reboot=YorN)
      RegularInstallWizard();
  }
  if (pmt_reboot) {
    "Reboot Now ";
    if (YorN)
      Reboot;
  }
}

DoInstructions;
DoInstall;
