U0 SongPuppet(CTask *task,I64 passes)
{
  CDbgInfo *dbg_info;
  I64 i,start,end,rip,last_rip;
  CHashFun *tmpf=NULL;
  for (i=0;i<250 && TaskValidate(task);i++) {
    if (tmpf=HashFind("Song",task->hash_table,HTT_FUN))
      break;
    Sleep(1);
  }
  if (tmpf && (dbg_info=tmpf->dbg_info)) {
    start=dbg_info->body[0];
    end  =dbg_info->body[dbg_info->max_line+1-dbg_info->min_line];
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
  BirthWait(&juke_task->popup_task);
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

  if (found) {//Position cursor on song title during song
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

public U0 JukeSongsPuppet(U8 *dirname="~/Psalmody",I64 passes=2,
	I64 start_song=0,I64 end_song=I64_MAX)
{//Help make music video by puppeting JukeBox task.
  I64 i;
  CDirEntry *tmpde,*tmpde1;
  CTask *juke_task=User("JukeBox(0x%X);\n",dirname);
  F64 t0;
  Cd(dirname);
  tmpde1=FilesFind("*",FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES);
  for (tmpde=tmpde1,i=0;tmpde && i<start_song;i++)
    tmpde=tmpde->next;
  if (scrncast.record)
    t0=scrncast.t0_tS;
  else
    t0=tS;
  for (i=start_song;tmpde && i<end_song;i++) {
    "%12.6fs %s\n",tS-t0,tmpde->full_name;
    JukeSongPuppet(juke_task,passes,i,tmpde->name);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  Kill(juke_task);
}
