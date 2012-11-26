U0 UserStartUp()
{ //Run each time a user a spawned
  UseTermDoc;
  DocCurAttr(WHITE<<4+BLUE);
  Type("::/SparrowOS/Doc/Start.TXT.Z");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
  Dir;
  CPURep;
}

U0 ServantStartUp()
{ //Run each time a servant task is spawned.
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
    PopUp("#include \"::/SparrowOS/Adam/Start.CPP.Z\";");
  AdamLog("Boot     Time:%7.3fs\n",tS);
  XTalk(user1,"#include \"DoItOnce.CPP.Z\";\n");
  Silent(ON);
  if (!IsSysDeterministic)
    WSInit("/SparrowOS/" TEXT_FILE_MASK ";~*Bible.TXT.Z");
  Silent(OFF);
  AdamLog("WordStat Time:%7.3fs\n",tS);
  if (ts_has_win_drv)
    DrvEnable(TS_WIN_DRV_LET,OFF);
}

DelTree("::/Temp/ScreenShots");
MkDir("::/Temp/ScreenShots");

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
