U0 UserStartUp()
{ //Run each time a user a spawned
  UseTermDoc;
  Type("::/TempleOS/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
  Dir;
  switch (ts_cid) {
    case CID_TAD8:
      "$$FG,GREEN$$TAD8$$FG$$: ";
      break;
    case CID_TAD8_VM:
      "$$FG,RED$$TAD8_VM$$FG$$: ";
      break;
    case CID_DFT:
      "$$FG,PURPLE$$DFT$$FG$$: ";
      break;
  }
  CPURep;
}

U0 SrvStartUp()
{ //Run each time a srv task is spawned.
  UseTermDoc;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
}

CTask *user1,*user2;

U0 StartUpTasks()
{
  user1=SpawnUser;
  user2=SpawnUser;
  WinToTop(user1);
  WinTileVert;
  if (!IsSysDeterministic)
    PopUp("#include \"::/TempleOS/Adam/Start\";");
  AdamLog("Boot     Time:%7.3fs\n",tS);
  XTalk(user1,"#include \"DoItOnce\";\n");
  Silent(ON);
  if (!IsSysDeterministic)
    WSInit("/TempleOS/" FILENAME_MASK_TXT ";~*Bible.TXT*");
  Silent(OFF);
  AdamLog("WordStat Time:%7.3fs\n",tS);
  if (ts_has_win_drv)
    DrvEnable(TS_WIN_DRV_LET,OFF);
}

DelTree("::/Temp/ScreenShots");
MkDir("::/Temp/ScreenShots");

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
