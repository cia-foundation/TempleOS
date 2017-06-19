//Place this file in /Home and change
//anything you want.

U0 UserStartUp()
{//Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.DD");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Dir;
  "$TX,"82,150",D="DD_TEMPLEOS_LOC"$LOC:";
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
  XTalk(user1,"Cd;#include \"Once\";\n");
  Silent;
  ACInit("/*;!*/Bible.TXT*;!*.TXT"); //No Bible, no uncompressed DD.
  Silent(OFF);
  "AutoComplete Time:%7.3fs\n",tS;
}

if (DrvIsWritable(':')) {
  DelTree("::/Tmp/ScrnShots");
  DirMk("::/Tmp/ScrnShots");
}

StartUpTasks;

"\nTempleOS V%5.3f\t%D %T\n\n",sys_os_version,sys_compile_time,sys_compile_time;
