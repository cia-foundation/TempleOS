#help_index "Task/Settings"

public CTaskSettings *SettingsPush(CTask *task=NULL,I64 flags=0)
{//Typically, called at start of an application.
//It saves many settings so they can be restored
  //at the end of the application with $LK,"SettingsPop",A="MN:SettingsPop"$().

  CTaskSettings *tmpse;
  CDoc *doc;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return NULL;
  tmpse=CAlloc(sizeof(CTaskSettings),task);
  tmpse->cur_dir=DirCur(task,task);
  tmpse->draw_it=task->draw_it;
  GrPaletteGet(tmpse->palette);
  tmpse->task_end_cb=task->task_end_cb;

  if (!(flags&TSF_SAME_SONG)) {
    if (tmpse->song_task=task->song_task) {
      Suspend(task->song_task);
      Snd;
    }
    task->song_task=NULL;
  }

  if (tmpse->animate_task=task->animate_task)
    Suspend(task->animate_task);
  task->animate_task=NULL;

  if (doc=DocPut(task)) {
    tmpse->hide_cursor=!Bt(&doc->flags,DOCf_HIDE_CURSOR);
    tmpse->highlight_cursor=!Bt(&doc->flags,DOCf_DONT_HIGHLIGHT_CURSOR);
    tmpse->scroll=!Bt(&doc->flags,DOCf_NO_SCROLL_BARS);
  }

  tmpse->left=task->win_left;
  tmpse->right=task->win_right;
  tmpse->top=task->win_top;
  tmpse->bottom=task->win_bottom;

  tmpse->scroll_x=task->scroll_x;
  tmpse->scroll_y=task->scroll_y;
  tmpse->scroll_z=task->scroll_z;

  tmpse->win_inhibit=task->win_inhibit;
  tmpse->text_attr=task->text_attr;
  StrCpy(tmpse->task_title,task->task_title);
  tmpse->title_src  =task->title_src;
  tmpse->border_attr=task->border_attr;
  tmpse->border_src =task->border_src;
  tmpse->border=!Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  if (TaskValidate(ac.task))
    tmpse->autocomplete=TRUE;
  else
    tmpse->autocomplete=FALSE;

  tmpse->next=task->next_settings;
  task->next_settings=tmpse;
  return tmpse;
}

U0 SettingsPop2(CTask *task,CTaskSettings *tmpse)
{
  CDoc *doc;

  if (doc=DocPut(task)) {
    LBEqu(&doc->flags,DOCf_HIDE_CURSOR,!tmpse->hide_cursor);
    LBEqu(&doc->flags,DOCf_DONT_HIGHLIGHT_CURSOR,!tmpse->highlight_cursor);
    LBEqu(&doc->flags,DOCf_NO_SCROLL_BARS,!tmpse->scroll);
  }

  WinBorder(tmpse->border,task);
  WinHorz(tmpse->left,tmpse->right,task);
  WinVert(tmpse->top,tmpse->bottom,task);
  task->scroll_x=tmpse->scroll_x;
  task->scroll_y=tmpse->scroll_y;
  task->scroll_z=tmpse->scroll_z;
  task->win_inhibit=tmpse->win_inhibit;
  task->text_attr=tmpse->text_attr;
  task->border_attr=tmpse->border_attr;
  task->border_src =tmpse->border_src;
  task->title_src  =tmpse->title_src;
  StrCpy(task->task_title,tmpse->task_title);
  AutoComplete(tmpse->autocomplete);
  GrPaletteSet(tmpse->palette);
  Snd;
}

public U0 SettingsPop(CTask *task=NULL,I64 flags=0)
{//Typically, called at end of an application.
  CTaskSettings *tmpse;
  if (!task) task=Fs;
  if (!TaskValidate(task))
    return;
  if (tmpse=task->next_settings) {
    task->next_settings=tmpse->next;
    Cd(tmpse->cur_dir);
    Free(tmpse->cur_dir);
    task->draw_it=tmpse->draw_it;
    task->task_end_cb=tmpse->task_end_cb;

    if (task->animate_task)
      Kill(task->animate_task);
    if (task->animate_task=tmpse->animate_task)
      Suspend(task->animate_task,FALSE);

    if (!(flags&TSF_SAME_SONG)) {
      if (task->song_task)
	Kill(task->song_task);
      if (task->song_task=tmpse->song_task)
	Suspend(task->song_task,FALSE);
    }

    SettingsPop2(task,tmpse); //Do it to get ress fast
    Refresh(,TRUE);
    SettingsPop2(task,tmpse); //Redo in case was lost by old update

    Free(tmpse);
  }
}
