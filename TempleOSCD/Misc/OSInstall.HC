U0 InstallDrv(U8 drv_let)
{
  U8 *st;
  ExePrint("CopyTree(\"::/\",\"%C:/\");",drv_let);
  ExePrint("DirMk(\"%C:/Tmp\");",drv_let);
  ExePrint("DirMk(\"%C:/Tmp/ScrnShots\");",drv_let);
  ExePrint("DirMk(\"%C:/Home\");",drv_let);

  st=MStrPrint("%C:/Home/DoDistro.HC.Z",drv_let);
  if (!FileFind(st))
    Copy("::/Misc/DoDistro.HC.Z",st);
  Free(st);

  st=MStrPrint("%C:/Home/MakeHome.HC.Z",drv_let);
  if (!FileFind(st))
    Copy("::/MakeHome.HC.Z",st);
  Free(st);
}

Bool VMPrtDsk(CTask *task,CATARep *ata_drv)
{
  if (ata_drv) {
    XTalkWait(task,"DskPrt(,0.5,0.5);\nC\np%d\nY",ata_drv->num);
    return TRUE;
  } else
    return FALSE;
}

U0 VMInstallDrv(CTask *task,U8 drv_let,
	CATARep *ata_drv,CATARep *atapi_drv)
{
  InstallDrv(drv_let);
  XTalkWait(task,"BootHDIns('%C');\n\nB\n0x20000\n",drv_let);
  if (ata_drv)
    XTalkWait(task,"C\np%d\n",ata_drv->num);
  if (atapi_drv)
    XTalkWait(task,"Tp%d\n",atapi_drv->num);
  XTalkWait(task,"\n\n\n"); //Exit Drives,Dsk Cache,Options
}

U0 VMInstallWiz()
{
  CATARep *head=NULL,*ata_drv=NULL,*atapi_drv=NULL;
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

  ATARep(FALSE,TRUE,&head);
  ATAIDDrvs(head,&ata_drv,&atapi_drv);
  if (VMPrtDsk(task,ata_drv)) {
    VMInstallDrv(task,'C',ata_drv,atapi_drv);
    VMInstallDrv(task,'D',ata_drv,atapi_drv);
    BootMHDIns('C');
  }
  LinkedLstDel(head);
  WinVert(task->win_top,Fs->win_bottom);
  Kill(task);
}

U0 RegularInstallWiz()
{
  I64 unit,drv_let;
  U8 *st,*base0,*base1;
  CATARep *head=NULL,*tmpha;
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
  XTalk(task,"Mount;\nC\np");

  num_hints=ATARep(FALSE,,&head);
  "\nInclude '$$PURPLE$$0x$$FG$$' for hexidecimal numbers.\n\n";
  while (TRUE) {
    base0=GetStr("Hard Drive I/O Port Base0  : ");
    if (0<Str2I64(base0)<=0xFFFF)
      break;
    Free(base0);
  }
  if (1<=Str2I64(base0)<=num_hints) {
    tmpha=ATARepFind(&head,Str2I64(base0));
    Free(base0);
    base0=MStrPrint("0x%X",tmpha->base0);
    base1=MStrPrint("0x%X",tmpha->base1);
    st	 =MStrPrint("0x%X",tmpha->unit);
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
  LinkedLstDel(head);
  XTalkWait(task,"%s\n%s\n%C\n",base0,base1,'0'+unit);
  DrvRep;
  do {
    st=GetStr("\nDestination Partition Letter: ");
    if (*st)
      drv_let=Let2Let(*st);
    else
      drv_let=0;
    Free(st);
  } while (!('A'<=drv_let<='Z'));
  '\n';

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
  InstallDrv(drv_let);
  XTalkWait(task,"BootHDIns('%C');\n\nB\n0x20000\n"
	"C\ns%s\n%s\n%C\n\n\n", //Exit Drives,Dsk Cache,Options
	drv_let,base0,base1,'0'+unit);
  "$$RED$$Install Master Boot loader?$$FG$$";
  if (YorN) {
    '\n';
    BootMHDIns(drv_let);
  }
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
      XTalk(task,"Ed(\"::/Doc/Install.DD.Z\");\n");
      break;
    }
    task=task->next_task;
  }
}

Bool DoInstall(Bool pmt_reboot)
{
  I64 res=FALSE,vm_install;
  "\n\n\n\n\nAre you installing inside VMware, QEMU, VirtualBox "
	"or a similar virtual machine? ";
  vm_install=YorN;
  DocBottom;

  if (vm_install) {
    VMInstallWiz();
    res=TRUE;
  } else {
    "\n\nThis wizard works if you have a partition ready.  "
	  "You can partition the drive or BootHDIns() "
	  "with more options if you do it by hand, not using this wizard.\n\n"
	  "Continue Install Wizard ";
    if (YorN) {
      RegularInstallWiz();
      res=TRUE;
    } else
      pmt_reboot=FALSE;
  }
  if (pmt_reboot) {
    "Reboot Now ";
    if (YorN)
      Reboot;
  }
  return res;
}

Bool OSInstall(Bool pmt_reboot=TRUE)
{
  DoInstructions;
  return DoInstall(pmt_reboot);
}

#if __CMD_LINE__
OSInstall(TRUE);
#endif
