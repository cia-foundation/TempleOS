//Place this file in /Home and change
//anything you want.

U0 UserStartUp()
{//Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Dir;
  "$TX,"80,142",D="DD_TEMPLEOS_LOC_OFFICIAL"$LOC:";
  CPURep;
}

U0 SrvStartUp()
{//Run each time a srv task is spawned.
  DocTermNew;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
}

CTask *user1,*user2;

U0 StartUpTasks()
{
  user1=User;
  user2=User;
  WinToTop(user1);
  WinTileVert;
  "Boot     Time:%7.3fs\n",tS;
  XTalk(user1,"Cd;#include \"DoOnce\";\n");
  Silent;
  ACInit("/*;!*/Bible.TXT*;!*.TXT*"); //No Bible, no uncompressed TXT.
  Silent(OFF);
  "AutoComplete Time:%7.3fs\n",tS;
}

if (DrvIsWritable(':')) {
  DelTree("::/Temp/ScreenShots");
  MkDir("::/Temp/ScreenShots");
}

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
