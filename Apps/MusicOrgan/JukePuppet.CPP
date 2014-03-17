U0 SongPuppet(CTask *task,I64 passes)
{
  CDbgInfo *dbg;
  I64 i,start,end,rip,last_rip;
  CHashFun *tempf=NULL;
  for (i=0;i<250 && TaskValidate(task);i++) {
    if (tempf=HashFind("Song",task->hash_table,HTT_FUN))
      break;
    Sleep(1);
  }
  if (tempf && (dbg=tempf->debug)) {
    start=dbg->body[0];
    end  =dbg->body[dbg->max_line+1-dbg->min_line];
    last_rip=0;
    while (TRUE) {
      i=0;
      while (TaskValidate(task) && (rip=TaskCaller(task,i++))) {
	if (start<=rip<end) {
	  if (rip<last_rip && --passes<=0)
	    return;
	  last_rip=rip;
	}
      }
      Sleep(1);
    }
  }
}

U0 JukeSongPuppet(CTask *juke_task,I64 passes,I64 song_num,U8 *name)
{
  Bool found;
  CTask *task;
  I64 i;

  if (FileExtDot(name))
    FileExtRem(name);
  while (!TaskValidate(juke_task->popup_task))
    Sleep(10);

  TaskWait(juke_task->popup_task);
  PostMsg(juke_task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP+SCF_CTRL);
  TaskWait(juke_task->popup_task);
  for (i=0;i<song_num;i++) {
    PostMsg(juke_task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    TaskWait(juke_task->popup_task);
  }

  PostMsg(juke_task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  TaskWait(juke_task->popup_task);
  Sleep(500);

  found=FALSE;
  task=Fs->next_task;
  while (task!=Fs) {
    if (!StrCmp(task->task_title,name)) {
      found=TRUE;
      break;
    }
    task=task->next_task;
  }

  if (found) {
    //Position cursor on song title during song
    TaskWait(juke_task->popup_task);
    PostMsg(juke_task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP+SCF_CTRL);
    TaskWait(juke_task->popup_task);
    for (i=0;i<song_num;i++) {
      PostMsg(juke_task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
      TaskWait(juke_task->popup_task);
    }
    SongPuppet(task,passes);
  }
  Kill(task);
}

public U0 JukeSongsPuppet(U8 *dir="HOME/MusicOrgan",I64 passes=2,
			I64 start_song=0,I64 end_song=MAX_I64)
{
  I64 i;
  CDirEntry *tempde,*tempde1;
  CTask *juke_task=SpawnUser("JukeBox(0x%X);\n",dir);
  Cd(dir);
  tempde1=FilesFind("*",FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES);
  for (tempde=tempde1,i=0;tempde && i<start_song;i++)
    tempde=tempde->next;
  for (i=start_song;tempde && i<end_song;i++) {
    JukeSongPuppet(juke_task,passes,i,tempde->name);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
  Kill(juke_task);
}
