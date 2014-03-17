//Change anything you want in this
//acct file or others.

U0 UserStartUp()
{ //Run each time a user a spawned
  DocTermNew;
  Type("::/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
}

U0 SrvStartUp()
{ //Run each time a srv task is spawned.
  DocTermNew;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinToTop;
  WinZBufUpdate;
  Preempt(ON);
}

U0 StartUpTasks()
{
  CTask *user1,*user2;
  user1=SpawnUser;
  user2=SpawnUser("Ed(\"::/Doc/Install.TXT.Z\");\n");
  WinToTop(user1);
  WinTileVert;
  "Boot Time:%7.3fs\n",tS;
  XTalk(user1,"Cd(\"HOME\");#include \"DoOnce\";\n");
  Silent(ON);	//no output to screen
  WSInit;
  Silent(OFF); //no output to screen
}

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
