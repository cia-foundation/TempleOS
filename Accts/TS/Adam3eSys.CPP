U0 UserStartUp()
{ //Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
  Dir;
  switch (ts_cid) {
    case CID_TAD8:
      "$$GREEN$$TAD8$$FG$$:";
      break;
    case CID_TAD8_VM:
      "$$RED$$TAD8_VM$$FG$$:";
      break;
    case CID_DFT:
      "$$PURPLE$$DFT$$FG$$:";
      break;
  }
  "$TX,"81,527",D="DD_TEMPLEOS_LOC_OFFICIAL"$LOC:";
  CPURep;
}

U0 SrvStartUp()
{ //Run each time a srv task is spawned.
  DocTermNew;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
}

CTask *user1,*user2;

U0 StartUpTasks()
{
  user1=SpawnUser;
  user2=SpawnUser;
  WinToTop(user1);
  WinTileVert;
  PopUp("#include \"::/Adam/Start\";");
  "Boot     Time:%7.3fs\n",tS;
  XTalk(user1,"Cd(\"HOME\");#include \"DoOnce\";\n");
  Silent(ON);
  WSInit("/*;~*Bible.TXT*");
  Silent(OFF);
  "WordStat Time:%7.3fs\n",tS;
  if (ts_has_win_drv)
    DrvEnable(TS_WIN_DRV_LET,OFF);
}

DelTree("::/Temp/ScreenShots");
MkDir("::/Temp/ScreenShots");

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
