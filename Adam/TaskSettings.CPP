#help_index "Task/Settings"

public CTaskSettings *SettingsPush(CTask *task=NULL,I64 flags=0)
{//Typically, called at start of an application.
//It saves many settings so they can be restored
//at the end of the application with $LK,"SettingsPop",A="MN:SettingsPop"$().

  CTaskSettings *tempse;
  CDoc *doc;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return NULL;
  tempse=CAlloc(sizeof(CTaskSettings),task);
  tempse->cur_dir=CurDir(task,task);
  tempse->draw_it=task->draw_it;
  GetVGAPalette4(tempse->palette4);
  tempse->task_end_cb=task->task_end_cb;

  if (!(flags&TSF_SAME_SONG)) {
    if (tempse->song_task=task->song_task) {
      Suspend(task->song_task);
      Snd(0);
    }
    task->song_task=NULL;
  }

  if (tempse->animate_task=task->animate_task)
    Suspend(task->animate_task);
  task->animate_task=NULL;

  if (doc=DocPut(task)) {
    tempse->cursor=!Bt(&doc->flags,DOCf_HIDE_CURSOR);
    tempse->scroll=!Bt(&doc->flags,DOCf_NO_SCROLL_BARS);
  }

  tempse->left=task->win_left;
  tempse->right=task->win_right;
  tempse->top=task->win_top;
  tempse->bottom=task->win_bottom;

  tempse->scroll_x=task->scroll_x;
  tempse->scroll_y=task->scroll_y;
  tempse->scroll_z=task->scroll_z;

  tempse->win_inhibit=task->win_inhibit;
  tempse->text_attr=task->text_attr;
  StrCpy(tempse->task_title,task->task_title);
  tempse->title_src  =task->title_src;
  tempse->border_attr=task->border_attr;
  tempse->border_src =task->border_src;
  tempse->border=!Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  tempse->preempt=Bt(&task->task_flags,TASKf_PREEMPT);
  if (TaskValidate(ac.task))
    tempse->autocomplete=TRUE;
  else
    tempse->autocomplete=FALSE;

  tempse->next=task->next_settings;
  task->next_settings=tempse;
  return tempse;
}

U0 SettingsPop2(CTask *task,CTaskSettings *tempse)
{
  CDoc *doc;

  if (doc=DocPut(task)) {
    LBEqu(&doc->flags,DOCf_HIDE_CURSOR,!tempse->cursor);
    LBEqu(&doc->flags,DOCf_NO_SCROLL_BARS,!tempse->scroll);
  }

  WinBorder(tempse->border,task);
  WinHorz(tempse->left,tempse->right,task);
  WinVert(tempse->top,tempse->bottom,task);
  task->scroll_x=tempse->scroll_x;
  task->scroll_y=tempse->scroll_y;
  task->scroll_z=tempse->scroll_z;
  task->win_inhibit=tempse->win_inhibit;
  task->text_attr=tempse->text_attr;
  task->border_attr=tempse->border_attr;
  task->border_src =tempse->border_src;
  task->title_src  =tempse->title_src;
  StrCpy(task->task_title,tempse->task_title);
  AutoComplete(tempse->autocomplete);
  SetVGAPalette4(tempse->palette4);
  Snd(0);
}

public U0 SettingsPop(CTask *task=NULL,I64 flags=0)
{//Typically, called at end of an application.
  CTaskSettings *tempse;
  Bool old_preempt=Preempt(OFF);
  if (!task) task=Fs;
  if (!TaskValidate(task)) {
    Preempt(old_preempt);
    return;
  }
  if (tempse=task->next_settings) {
    task->next_settings=tempse->next;
    Cd(tempse->cur_dir);
    Free(tempse->cur_dir);
    task->draw_it=tempse->draw_it;
    task->task_end_cb=tempse->task_end_cb;

    if (task->animate_task)
      Kill(task->animate_task);
    if (task->animate_task=tempse->animate_task)
      Suspend(task->animate_task,FALSE);

    if (!(flags&TSF_SAME_SONG)) {
      if (task->song_task)
	Kill(task->song_task);
      if (task->song_task=tempse->song_task)
	Suspend(task->song_task,FALSE);
    }

    SettingsPop2(task,tempse); //Do it to get results fast
    WinMgrSync(,TRUE);
    SettingsPop2(task,tempse); //Redo in case was lost by old update

    Preempt(old_preempt);
    LBEqu(&task->task_flags,TASKf_PREEMPT,tempse->preempt);
    Free(tempse);
  } else
    Preempt(old_preempt);
}
