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
}

U0 SrvStartUp()
{//Run each time a srv task is spawned.
  DocTermNew;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
}

U0 StartUpTasks()
{
  CTask *user1,*user2;
  user1=User;
  user2=User;
  WinToTop(user1);
  WinTileVert;
  "Boot Time:%7.3fs\n",tS;
  XTalk(user1,"Cd;#include \"Once\";\n");
  Silent;	//no output to scrn
  ACInit("/*;!*/Bible.TXT*");
  Silent(OFF); //no output to scrn
}

StartUpTasks;

"\nTempleOS V%5.3f\t%D %T\n\n",sys_os_version,sys_compile_time,sys_compile_time;
