Bool VMPrtDsk(CTask *task,CATARep *ata_drv)
{
  if (ata_drv) {
    XTalkWait(task,"PrtDsk;\nYC2p%d\nYYY",ata_drv->num);
    XTalkWait(task,"%d\nY\nY\n",(prt_dsk_dsk_size-PRT_HEADER)/2);
    return TRUE;
  } else
    return FALSE;
}

U0 VMInstallBoot(CTask *task,U8 cd_drv,U8 drv,U8 *acct_name,
	CATARep *ata_drv,CATARep *atapi_drv)
{
  U8 *st1,*st2;

  st1=MPrint("%C:/",cd_drv);
  st2=MPrint("%C:/",drv);
  CopyTree(st1,st2);
  Free(st1);
  Free(st2);

  st1=MPrint("%C:/Accts/%s",drv,acct_name);
  MkDir(st1);
  Copy("::/Misc/MakeDistro.CPP.Z",st1);
  Free(st1);

  XTalkWait(task,
    "MakeOSPrtBootInstall('%C');\n"
    "\nB1\n0x20000\n",drv);

  if (ata_drv)
    XTalkWait(task,"C2p%d\n",ata_drv->num);
  if (atapi_drv)
    XTalkWait(task,"T3p%d\n",atapi_drv->num);

  XTalkWait(task,
    "\n"   //Exit Drives
    "%s\n" //Acct
    "\n"   //Dsk Cache
    "\n",  //Options
    acct_name);
}

U0 VMInstallWizard()
{
  U8 *acct_name;
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CTask *task;
  while (TRUE) {
    "Make-up a Log-In Name (account).\n";
    acct_name=PmtStr("User Acct Name: ");
    if (FileNameChk(acct_name))
      break;
    Free(acct_name);
  }

  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=SpawnUser;
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  SetWinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);

  ATARep(FALSE,TRUE,&root);
  ATAIDDrvs(root,*sys_acct,&ata_drv,&atapi_drv);
  if (VMPrtDsk(task,ata_drv)) {
    VMInstallBoot(task,*sys_acct,'C',acct_name,ata_drv,atapi_drv);
    VMInstallBoot(task,*sys_acct,'D',acct_name,ata_drv,atapi_drv);
    MasterBootIntall('C');
  }
  LinkedLstDel(root);
  Free(acct_name);

  SetWinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 RegularInstallWizard()
{
  I64 unit;
  U8 *base0,*base1,*ms,*dst,*acct_name,*st1,*st2,*new_home_str;
  CATARep *root=NULL,*tempha;
  I64 ch,num_hints;
  CTask *task;

  while (TRUE) {
    "Make-up a Log-In Name (account).\n";
    acct_name=PmtStr("User Acct Name: ");
    if (FileNameChk(acct_name))
      break;
    Free(acct_name);
  }

  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=SpawnUser;
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  SetWinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);
  XTalk(task,"Mount;\nC2 ");

  num_hints=ATARep(FALSE,,&root);
  "\nInclude '$$PURPLE$$0x$$FG$$' for hexidecimal numbers.\n\n";
  while (TRUE) {
    base0=PmtStr("Hard Drive I/O Port Base0  : ");
    if (0<A2I64(base0)<=0xFFFF)
      break;
    Free(base0);
  }
  if (1<=A2I64(base0)<=num_hints) {
    tempha=ATARepFind(&root,A2I64(base0));
    Free(base0);
    base0=MPrint("0x%X",tempha->base0);
    base1=MPrint("0x%X",tempha->base1);
    ms	 =MPrint("0x%X",tempha->unit);
    unit=A2I64(ms);
  } else {
    while (TRUE) {
      base1=PmtStr("Hard Drive I/O Port Base1  : ");
      if (0<A2I64(base1)<=0xFFFF)
	break;
      Free(base1);
    }
    while (TRUE) {
      ms =PmtStr("Unit--$$PURPLE$$0$$FG$$=Master or $$PURPLE$$1$$FG$$=Slave: ","0");
      unit=A2I64(ms);
      if (0<=unit<=1)
	break;
    }
  }
  LinkedLstDel(root);
  XTalkWait(task,"%s\n%s\n%C\n",base0,base1,'0'+unit);
  PrtRep;
  while (TRUE) {
    dst=PmtStr("\nDestination Partition Letter: ");
    if ('A'<=ToUpper(*dst)<='Z')
      break;
    Free(dst);
  }

  NewLine;
  st1=MPrint("%C:/",*sys_acct);
  st2=MPrint("%C:/",*dst);

  "$$RED$$Format %C Partition?$$FG$$\n",*dst;
  if (YorN) {
    NewLine;
    while (TRUE) {
    "$$PURPLE$$1$$FG$$) Use FAT32\n"
      "$$PURPLE$$2$$FG$$) Use RedSea\n"
      "\nFile System Type: ";
      ch=GetChar;
      NewLine;
      if ('1'<=ch<='2')
	break;
    }
    if (ch=='1')
      Fmt(*dst,,FALSE,PT_FAT32);
    else
      Fmt(*dst,,FALSE,PT_REDSEA);
  }
  CopyTree(st1,st2);
  st2=MPrint("%C:/Temp",*dst);
  MkDir(st2);
  Free(st2);

  new_home_str=MPrint("%C:/Accts/%s",*dst,acct_name);
  MkDir(new_home_str);
  Copy("::/Misc/MakeDistro.CPP.Z",new_home_str);
  XTalkWait(task,
  "MakeOSPrtBootInstall('%C');\n"
  "\nB1\n0x20000\n"
  "C"
  "2p"
  "%s\n"
  "%s\n"
  "%C"
  "\n"	 //Exit Drives
  "%s\n" //Acct
  "\n"	 //Dsk Cache
  "\n",	//Options
  *dst,base0,base1,'0'+unit,acct_name);
  "$$RED$$Install Master Boot loader?$$FG$$";
  if (YorN) {
    NewLine;
    MasterBootIntall(*dst);
  }
  Free(new_home_str);
  Free(acct_name);
  Free(base0);
  Free(base1);
  Free(ms);
  Free(dst);

  SetWinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 DoInstall()
{
  I64 pmt_reboot;
  NewLine(5);
  "Install on virtual machine ";
  if (pmt_reboot=YorN)
    VMInstallWizard();
  else {
    "\n\nThis wizard works if you have a partition ready.  "
    "You can partition the drive or MakeOSPrtBootInstall() "
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

DoInstall;
