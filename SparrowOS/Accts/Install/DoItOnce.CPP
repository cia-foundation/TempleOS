U0 VMDrvs(CATARep *root,CATARep **_ata_drv,CATARep **_atapi_drv)
{
  CBlkDev *bd=Drv2BlkDev(dft_drv);
  CATARep *tempha=root,*ata_drv=NULL,*atapi_drv=NULL;
  Bool found_atapi=FALSE;
  while (tempha) {
    if (tempha->type==BDT_ATA) {
      if (!ata_drv || tempha->unit<ata_drv->unit ||
	tempha->unit==ata_drv->unit && tempha->num<ata_drv->num)
	ata_drv=tempha;
    } else if (tempha->type==BDT_ATAPI) {
      if (!found_atapi) {
	if (bd && tempha->base0==bd->base0 && tempha->unit==bd->unit) {
	  atapi_drv=tempha;
	  found_atapi=TRUE;
	} else if (!atapi_drv || tempha->unit<atapi_drv->unit ||
	  tempha->unit==atapi_drv->unit && tempha->num<atapi_drv->num)
	  atapi_drv=tempha;
      }
    }
    tempha=tempha->next;
  }
  if (_ata_drv)   *_ata_drv  =ata_drv;
  if (_atapi_drv) *_atapi_drv=atapi_drv;
}

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

  st1=MSPrintF("%C:/",cd_drv);
  st2=MSPrintF("%C:/",drv);
  CopyTree(st1,st2);
  Free(st1);
  Free(st2);

  st1=MSPrintF("%C:/SparrowOS/Accts/%s",drv,acct_name);
  MkDir(st1);
  Free(st1);

  XTalkWait(task,
    "InstallBoot('%C');\n"
    "B10x20000\n",drv);

  if (ata_drv)
    XTalkWait(task,"C2p%d\n",ata_drv->num);
  if (atapi_drv)
    XTalkWait(task,"T3p%d\n",atapi_drv->num);

  XTalkWait(task,
    "\n"   //Exit Drives
    "%s\n" //Acct
    "\n"   //Dsk Cache
    "\n",	//Options
    acct_name);
}

U0 VMInstallWizard(CTask *task)
{
  U8 *acct_name;
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  while (TRUE) {
    "Make-up a Log-In Name (account).\n";
    acct_name=PmtStr("User Acct Name (%s): ","User");
    if (FileNameChk(acct_name))
      break;
    Free(acct_name);
  }
  task=SpawnUser;
  task->win_top=Fs->win_top;
  task->win_bottom=(Fs->win_top+Fs->win_bottom)>>2-1;
  Fs-> win_top=task->win_bottom+3;
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  TaskDerivedValsUpdate(task);
  TaskDerivedValsUpdate;
  WinToTop(Fs);

  ATARep(FALSE,&root);
  VMDrvs(root,&ata_drv,&atapi_drv);
  if (VMPrtDsk(task,ata_drv)) {
    VMInstallBoot(task,dft_drv,'C',acct_name,ata_drv,atapi_drv);
    VMInstallBoot(task,dft_drv,'D',acct_name,ata_drv,atapi_drv);
    InstallMasterBoot('C');
  }
  LinkedListDel(root);
  Free(acct_name);
}

U0 RegularInstallWizard(CTask *task)
{
  I64 unit;
  U8 ch,*base0,*base1,*ms,*dst,*acct_name;
  U8 *st1,*st2,*new_home_str;
  CATARep *root=NULL,*tempha;
  I64 num_hints;

  while (TRUE) {
    "Make-up a Log-In Name (account).\n";
    acct_name=PmtStr("User Acct Name (%s): ","User");
    if (FileNameChk(acct_name))
      break;
    Free(acct_name);
  }
  task=SpawnUser;
  task->win_top=Fs->win_top;
  task->win_bottom=(Fs->win_top+Fs->win_bottom)>>2-1;
  Fs-> win_top=task->win_bottom+3;
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  TaskDerivedValsUpdate(task);
  TaskDerivedValsUpdate;
  WinToTop(Fs);
  XTalk(task,"Mount;\n");
  XTalk(task,"C");
  XTalk(task,"2 ");

  num_hints=ATARep(FALSE,&root);
  "\nInclude '$$FG,PURPLE$$0x$$FG$$' for hexidecimal numbers.\n\n";
  while (TRUE) {
    base0=PmtStr("Hard Drive I/O Port Base0  : ");
    if (0<A2I(base0)<=0xFFFF)
      break;
    Free(base0);
  }
  if (1<=A2I(base0)<=num_hints) {
    tempha=ATARepFind(&root,A2I(base0));
    Free(base0);
    base0=MSPrintF("0x%X",tempha->base0);
    base1=MSPrintF("0x%X",tempha->base1);
    ms	 =MSPrintF("0x%X",tempha->unit);
    unit=A2I(ms);
    Free(ms);
  } else {
    while (TRUE) {
      base1=PmtStr("Hard Drive I/O Port Base1  : ");
      if (0<A2I(base1)<=0xFFFF)
	break;
      Free(base1);
    }
    while (TRUE) {
      ms =PmtStr("Unit--$$FG,PURPLE$$0$$FG$$=Master or $$FG,PURPLE$$1$$FG$$=Slave: ","0");
      unit=A2I(ms);
      Free(ms);
      if (0<=unit<=1)
	break;
    }
  }
  LinkedListDel(root);
  XTalk(task,"%s\n",base0);
  XTalk(task,"%s\n",base1);
  XTalk(task,"%C",'0'+unit);
  XTalkWait(task,"\n");
  PrtRep;
  while (TRUE) {
    dst=PmtStr("\nDestination Partition Letter: ");
    if ('A'<=ToUpper(*dst)<='Z')
      break;
    Free(dst);
  }

  NewLine;
  st1=MSPrintF("%C:/SparrowOS",dft_drv);
  st2=MSPrintF("%C:/SparrowOS",*dst);

  "$$FG,RED$$Format %C Partition?$$FG$$\n",*dst;
  if (YorN) {
    NewLine;
    while (TRUE) {
    "$$FG,PURPLE$$1$$FG$$) Use FAT32\n"
      "$$FG,PURPLE$$2$$FG$$) Use SparrowOS Native\n"
      "\nFile System Type: ";
      ch=GetChar;
      NewLine;
      if ('1'<=ch<='2')
	break;
    }
    if (ch=='1')
      Fmt(*dst,TRUE,FALSE,PT_FAT32);
    else
      Fmt(*dst,TRUE,FALSE,PT_NATIVE);
  } else {
  "\n$$BK,1$$WARNING$$BK,0$$ if you answer yes to the following "
    "you will lose all SparrowOS files in your /SparrowOS directory tree.\n"
    "Delete %s Directory ",st2;
    if (YorN)
      DelTree(st2);
    NewLine;
  }
  CopyTree(st1,st2);
  st2=MSPrintF("%C:/0000Boot",*dst);
  MkDir(st2);
  Free(st2);
  st2=MSPrintF("%C:/Temp",*dst);
  MkDir(st2);
  Free(st2);

  new_home_str=MSPrintF("%C:/SparrowOS/Accts/%s",*dst,acct_name);
  MkDir(new_home_str);
  XTalkWait(task,
  "InstallBoot('%C');\n"
  "B10x20000\n"
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
  "$$FG,RED$$Install Master Boot loader?$$FG$$";
  if (YorN) {
    NewLine;
    InstallMasterBoot(*dst);
  }
  Free(new_home_str);
  Free(acct_name);
  Free(base0);
  Free(base1);
  Free(ms);
  Free(dst);
}

U0 DoInstall()
{
  CTask *task=Fs->next_task;
  while (task->user_num!=2 && task!=Fs)
    task=task->next_task;
  XTalk(task,"Ed(\"::/SparrowOS/Doc/Install.TXT.Z\");\n");

  NewLine(5);
  "Install on virtual machine ";
  if (YorN)
    VMInstallWizard(task);
  else {
  "\n\nThis wizard works if you have a partition ready.  "
    "You can partition the drive or compile the kernel "
    "with more options if you do it by hand, not using this wizard.\n\n"
    "Continue Install Wizard ";
    if (YorN)
      RegularInstallWizard(task);
  }
  Fs->win_top=task->win_top;
  Kill(task);

  "Reboot Now ";
  if (YorN)
    Reboot;
}

DoInstall;
