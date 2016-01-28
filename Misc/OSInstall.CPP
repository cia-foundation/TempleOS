Bool VMPrtDsk(CTask *task,CATARep *ata_drv)
{
  if (ata_drv) {
    XTalkWait(task,"PrtDsk;\nYCp%d\nYYY",ata_drv->num);
    XTalkWait(task,"%d\nY\nY\n",(drv_dsk_dsk_size-DRV_HEADER)/2);
    return TRUE;
  } else
    return FALSE;
}

U0 VMInstallBoot(CTask *task,U8 dvd_drv,U8 drv_let,
	CATARep *ata_drv,CATARep *atapi_drv)
{
  U8 *st1,*st2;

  st1=MStrPrint("%C:/",dvd_drv);  st2=MStrPrint("%C:/",drv_let);
  CopyTree(st1,st2);
  Free(st1); Free(st2);

  st1=MStrPrint("%C:/Home",drv_let);
  MkDir(st1);
  Copy("::/Misc/DoDistro.CPP.Z",st1);
  Copy("::/MakeHome.CPP.Z",st1);
  Free(st1);

  XTalkWait(task,"MakeOSDrvBootInstall('%C');\n"
	"\nB\n0x20000\n",drv_let);
  if (ata_drv)
    XTalkWait(task,"Cp%d\n",ata_drv->num);
  if (atapi_drv)
    XTalkWait(task,"Tp%d\n",atapi_drv->num);
  XTalkWait(task,"\n\n\n"); //Exit Drives,Dsk Cache,Options
}

U0 VMInstallWiz()
{
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CTask *task;
  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=User;
  TaskWait(task);
  task->border_src=BDS_CONST;
  task->border_attr=LTGRAY<<4+DrvTextAttrGet(':')&15;
  task->text_attr  =LTGRAY<<4+BLUE;
  task->win_inhibit=WIG_TASK_DFT-WIF_SELF_BORDER;
  WinHorz(Fs->win_left,Fs->win_right,task);
  WinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  WinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);

  ATARep(FALSE,TRUE,&root);
  ATAIDDrvs(root,blkdev.boot_drv_let,&ata_drv,&atapi_drv);
  if (VMPrtDsk(task,ata_drv)) {
    VMInstallBoot(task,blkdev.boot_drv_let,'C',ata_drv,atapi_drv);
    VMInstallBoot(task,blkdev.boot_drv_let,'D',ata_drv,atapi_drv);
    MasterBootInstall('C');
  }
  LinkedLstDel(root);
  WinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 RegularInstallWiz()
{
  I64 unit,drv_let;
  U8 *st,*base0,*base1,*st1,*st2,*new_home_str;
  CATARep *root=NULL,*tempha;
  I64 ch,num_hints;
  CTask *task;

  "\nIt's normal for this to freeze for a moment or two.\n";
  PressAKey;

  task=User;
  TaskWait(task);
  task->border_src=BDS_CONST;
  task->border_attr=LTGRAY<<4+DrvTextAttrGet(':')&15;
  task->text_attr  =LTGRAY<<4+BLUE;
  task->win_inhibit=WIG_TASK_DFT-WIF_SELF_BORDER;
  WinHorz(Fs->win_left,Fs->win_right,task);
  WinVert(Fs->win_top,(Fs->win_top+Fs->win_bottom)>>2-1,task);
  WinVert(task->win_bottom+3,Fs->win_bottom);
  WinToTop(Fs);
  XTalk(task,"Mount;\nCp");

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
    st	 =MStrPrint("0x%X",tempha->unit);
    unit=Str2I64(st);
    Free(st);
  } else {
    while (TRUE) {
      base1=GetStr("Hard Drive I/O Port Base1  : ");
      if (0<Str2I64(base1)<=0xFFFF)
	break;
      Free(base1);
    }
    do {
      st =GetStr("Unit--$$PURPLE$$0$$FG$$=Master or $$PURPLE$$1$$FG$$=Slave: ","0");
      unit=Str2I64(st);
      Free(st);
    } while (!(0<=unit<=1));
  }
  LinkedLstDel(root);
  XTalkWait(task,"%s\n%s\n%C\n",base0,base1,'0'+unit);
  DrvRep;
  do {
    st=GetStr("\nDestination Partition Letter: ");
    drv_let=ToUpper(*st);
    Free(st);
  } while (!('A'<=drv_let<='Z'));
  '\n';
  st1=MStrPrint("%C:/",blkdev.boot_drv_let);
  st2=MStrPrint("%C:/",drv_let);

  "$$RED$$Format %C Partition?$$FG$$\n",drv_let;
  if (YorN) {
    '\n';
    do {
      "$$PURPLE$$1$$FG$$) Use FAT32\n"
	    "$$PURPLE$$2$$FG$$) Use RedSea\n"
	    "\nFile System Type: ";
      ch=GetChar;
      '\n';
    } while (!('1'<=ch<='2'));
    if (ch=='1')
      Fmt(drv_let,,FALSE,FSt_FAT32);
    else
      Fmt(drv_let,,FALSE,FSt_REDSEA);
  }
  CopyTree(st1,st2);
  st2=MStrPrint("%C:/Temp",drv_let);
  MkDir(st2);
  Free(st2);

  new_home_str=MStrPrint("%C:/Home",drv_let);
  MkDir(new_home_str);
  Copy("::/Misc/DoDistro.CPP.Z",new_home_str);
  Copy("::/MakeHome.CPP.Z",new_home_str);
  XTalkWait(task,"MakeOSDrvBootInstall('%C');\n"
	"\nB\n0x20000\n"
	"Cs%s\n%s\n%C"
	"\n\n\n", //Exit Drives,Dsk Cache,Options
	drv_let,base0,base1,'0'+unit);
  "$$RED$$Install Master Boot loader?$$FG$$";
  if (YorN) {
    '\n';
    MasterBootInstall(drv_let);
  }
  Free(new_home_str);
  Free(base0);
  Free(base1);

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
    VMInstallWiz();
    pmt_reboot=TRUE;
  } else {
    "\n\nThis wizard works if you have a partition ready.  "
	  "You can partition the drive or MakeOSDrvBootInstall() "
	  "with more options if you do it by hand, not using this wizard.\n\n"
	  "Continue Install Wizard ";
    if (pmt_reboot=YorN)
      RegularInstallWiz();
  }
  if (pmt_reboot) {
    "Reboot Now ";
    if (YorN)
      Reboot;
  }
}

DoInstructions;
DoInstall;
