//Change anything you want in this
//acct file or others.

U0 UserStartUp()
{ //Run each time a user a spawned
  UseTermDoc;
  DocCurAttr(WHITE<<4+BLUE);
  Type("::/SparrowOS/Doc/Start.TXT.Z");
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  WinZBufUpdate;
  if (!IsSysDeterministic)
    Preempt(ON);
}

U0 ServantStartUp()
{ //Run each time a servant task is spawned.
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
    PopUp("#include \"::/SparrowOS/Adam/Start.CPP.Z\";");
  AdamLog("Boot Time:%7.3fs\n",tS);
}

StartUpTasks;

"\nOS Compile Time:%D %T\n",sys_compile_time,sys_compile_time;
