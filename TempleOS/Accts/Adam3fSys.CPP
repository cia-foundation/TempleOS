//Change anything you want in this
//acct file or others.

U0 UserStartUp()
{ //Run each time a user a spawned
  UseTermDoc;
  Type("::/TempleOS/Doc/Start.TXT");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
  Dir;
}

U0 SrvStartUp()
{ //Run each time a srv task is spawned.
  UseTermDoc;
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
}

U0 StartUpTasks()
{
  CTask *task1,*task2;
  task1=SpawnUser;
  task2=SpawnUser;
  WinToTop(task1);
  WinTileVert;
  if (!IsSysDeterministic)
    PopUp("#include \"::/TempleOS/Adam/Start\";");
  AdamLog("Boot Time:%7.3fs\n",tS);
  XTalk(task1,"#include \"DoItOnce\";\n");
  Silent(ON);	//no output to screen
  WSInit("/TempleOS/" FILENAME_MASK_TXT ";~*Bible.TXT*");
  Silent(OFF); //no output to screen
}

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
