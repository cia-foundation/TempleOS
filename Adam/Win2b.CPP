#help_index "Windows;Task/Delay"

public U0 WinMgrSync(I64 cnt=1,Bool force=FALSE)
{//Wait for win manager to start and finish updating screen.
//0 Cnt Sync to WinMgr. (not used)
//1 Cnt Wait and Pump Msgs.
//2 Cnt Make Sure to do a Full Refresh
//	and Set Cur Pos.
  Bool old_preempt=Preempt(OFF),
  old_full_refresh,
  old_idle=LBts(&Fs->task_flags,TASKf_IDLE);
  CDoc *old_doc=DocPut;
  I64 update_cnt;
  while (Bt(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0)) {
    if (force) {
      LBts(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
      if (sys_winmgr_task) sys_winmgr_task->wake_jiffy=cnts.jiffies;
    }
    Yield;
  }
  if (cnt>1 && old_doc)
    old_full_refresh=LBts(&old_doc->flags,DOCf_DO_FULL_REFRESH);
  update_cnt=winmgr.updates+cnt;
  while (winmgr.updates<update_cnt) {
    if (force) {
      LBts(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
      if (sys_winmgr_task) sys_winmgr_task->wake_jiffy=cnts.jiffies;
    }
    Yield;
  }
  if (old_doc)
    LBEqu(&old_doc->flags,DOCf_DO_FULL_REFRESH,old_full_refresh);
  LBEqu(&Fs->task_flags,TASKf_IDLE,old_idle);
  Preempt(old_preempt);
}

#help_index "Windows"

I64 WinQueIPMsgs(Bool que)
{
  static CD3I64 single_ip={0,0,0};
  F64 time=tS;
  I64 msg_code=0,a1,a2,single_a1,single_a2;
  CTask *task_focus=sys_focus_task;

  if (task_focus && !winmgr.grab_scroll) {
    a1=ip.pos.x-task_focus->pix_left-task_focus->scroll_x;
    a2=ip.pos.y-task_focus->pix_top-task_focus->scroll_y;
    single_a1=single_ip.x-task_focus->pix_left-task_focus->scroll_x;
    single_a2=single_ip.y-task_focus->pix_top-task_focus->scroll_y;
    if (old_ip.presnap.x!=ip.presnap.x || old_ip.presnap.y!=ip.presnap.y) {
      if (que)
	TaskMsg(task_focus,0,
	  MSG_IP_MOVE,a1,a2,0);
      msg_code=MSG_IP_MOVE;
    }
    //TODO que msg for ip.pos.z?
    if (ip.left_dbl_time) {
      if (time>ip.left_dbl_time) {
	if (ip.left_dbl) {
	  if (!ip.left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_D_DOWN,a1,a2,0);
	    ip.left_down_sent=TRUE;
	    msg_code=MSG_IP_L_D_DOWN;
	  }
	  if (!ip.lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_D_UP,a1,a2,0);
	    ip.left_dbl_time=0;
	    msg_code=MSG_IP_L_D_UP;
	  }
	} else {
	  if (!ip.left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_DOWN,single_a1,single_a2,0);
	    ip.left_down_sent=TRUE;
	    msg_code=MSG_IP_L_DOWN;
	  }
	  if (!ip.lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_UP,a1,a2,0);
	    ip.left_dbl_time=0;
	    msg_code=MSG_IP_L_UP;
	  }
	}
      } else {
	if (ip.lb && !ip_last.lb) {
	  ip.left_dbl_time=time;
	  ip.left_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	  Bt(&task_focus->win_inhibit,WIf_FOCUS_TASK_IP_L_D)) {
	if (ip.lb  && !ip_last.lb) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_L_DOWN,a1,a2,0);
	  msg_code=MSG_IP_L_DOWN;
	} else if (!ip.lb && ip_last.lb) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_L_UP,a1,a2,0);
	  msg_code=MSG_IP_L_UP;
	}
      } else {
	if (ip.lb  && !ip_last.lb) {
	  ip.left_dbl=FALSE;
	  ip.left_down_sent=FALSE;
	  ip.left_dbl_time=time+ip.dbl_time;
	  single_ip.x=ip.pos.x;
	  single_ip.y=ip.pos.y;
	}
      }
    }

    if (ip.right_dbl_time) {
      if (time>ip.right_dbl_time) {
	if (ip.right_dbl) {
	  if (!ip.right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_D_DOWN,a1,a2,0);
	    ip.right_down_sent=TRUE;
	    msg_code=MSG_IP_R_D_DOWN;
	  }
	  if (!ip.rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_D_UP,a1,a2,0);
	    ip.right_dbl_time=0;
	    msg_code=MSG_IP_R_D_UP;
	  }
	} else {
	  if (!ip.right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_DOWN,single_a1,single_a2,0);
	    ip.right_down_sent=TRUE;
	    msg_code=MSG_IP_R_DOWN;
	  }
	  if (!ip.rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_UP,a1,a2,0);
	    ip.right_dbl_time=0;
	    msg_code=MSG_IP_R_UP;
	  }
	}
      } else {
	if (ip.rb && !ip_last.rb) {
	  ip.right_dbl_time=time;
	  ip.right_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	  Bt(&task_focus->win_inhibit,WIf_FOCUS_TASK_IP_R_D)) {
	if (ip.rb  && !ip_last.rb) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_R_DOWN,a1,a2,0);
	  msg_code=MSG_IP_R_DOWN;
	} else if (!ip.rb && ip_last.rb) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_R_UP,a1,a2,0);
	  msg_code=MSG_IP_R_UP;
	}
      } else {
	if (ip.rb  && !ip_last.rb) {
	  ip.right_dbl=FALSE;
	  ip.right_down_sent=FALSE;
	  ip.right_dbl_time=time+ip.dbl_time;
	  single_ip.x=ip.pos.x;
	  single_ip.y=ip.pos.y;
	}
      }
    }

    MemCpy(&ip_last,&ip,sizeof(CIPStateGlbls));
    MemCpy(&old_ip,&ip,sizeof(CIPStateGlbls));
  }
  return msg_code;
}

I64 WinCalcRefreshTimeOut(Bool was_forced)
{
  F64 d,d1;
  I64 i,k,total,
      total_jiffies,idle_pt_hits[MP_MAX_PROCESSORS];
  CCPU *c;
  CWinMgrTimingGlbls *t=winmgr.t;

  t->refresh_skips++;
  if ((t->refresh_time=tS)-t->last_refresh_time>.25) {
    PUSHFD
    CLI
    total_jiffies=cpu_structs[0].total_jiffies;
    for (i=0;i<mp_cnt;i++)
      idle_pt_hits[i]=cpu_structs[i].idle_pt_hits;
    POPFD

    t->refresh_delta_time=t->refresh_time-t->last_refresh_time;
    total=total_jiffies-t->last_total_jiffies;
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      d1=0.01;
      if (t->refresh_delta_time>0) {
	if (total) {
	  if ((k=idle_pt_hits[i]-t->last_idle_pt_hits[i])>=0) {
	    d1=ToF64(k)/total;
	    d=ToF64(k*cnts.time_stamp_freq)
	      /(JIFFY_FREQ*t->refresh_delta_time);
	    if (d<d1) d1=d;
	  }
	} else {
	  if ((k=idle_pt_hits[i]-t->last_idle_pt_hits[i])>=0)
	    d1=ToF64(k*cnts.time_stamp_freq)
	    /(JIFFY_FREQ*t->refresh_delta_time);
	}
      }
      t->last_idle_pt_hits[i]=idle_pt_hits[i];
      c->idle_factor=Clamp(d1,0.01,0.99);
    }
    t->last_total_jiffies=total_jiffies;
    t->last_refresh_time=t->refresh_time;
    if (!was_forced) {
      if (winmgr.first) {
	d=Gs->idle_factor;
	d1=(t->last_refresh_end_time-t->last_refresh_start_time)/
	    (t->refresh_delta_time/t->refresh_skips);
	if (d1<0.2)
	  winmgr.desired_fps*=1.25;
	else if (d<=0.0)
	  winmgr.desired_fps=15.0;
	else if (d<0.02) {
	  d=15.0/winmgr.fps;
	  winmgr.desired_fps=d*15.0+0.5*(1.0-d)*winmgr.desired_fps;
	} else if (d<0.99) {
	  if (d=(1.0-d)`0.25)
	    winmgr.desired_fps*= 0.8`0.25/d;
	} else
	  winmgr.desired_fps=30.0;
	t->refresh_cnt++;
	t->refresh_skips=0;
      } else {
	winmgr.desired_fps=30.0;
	winmgr.first=TRUE;
      }
    }
  }
  winmgr.desired_fps=Clamp(winmgr.desired_fps,15.0,30.0);
  return cnts.jiffies+JIFFY_FREQ/winmgr.desired_fps;
}

I64 WinMgrSleep(Bool flush_msgs=FALSE)
{
  static Bool was_forced=FALSE;
  I64 timeout_val,msg_code=0;
  CCtrl *c;
  Bool que;

  TimeStampFreqCal;

  if (!flush_msgs && winmgr.old_focus_task!=sys_focus_task) {
    TaskMsg(winmgr.old_focus_task,0,MSG_FOCUS,FALSE,0,0);
    TaskMsg(sys_focus_task,0,MSG_FOCUS,TRUE,0,0);
    winmgr.old_focus_task=sys_focus_task;
  }

  if (flush_msgs)
    FifoI64Flush(kbd.scan_code_fifo);
  else if (TaskValidate(sys_focus_task)) {
    KbdMsgsQue;

    que=TRUE;
    if (TaskValidate(sys_focus_task) &&
      !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_CTRLS)) {
      c=sys_focus_task->next_ctrl;
      while (c!=&sys_focus_task->next_ctrl) {
	if (CtrlInside(c,ip.pos.x,ip.pos.y)) {
	  que=FALSE;
	  break;
	}
	c=c->next;
      }
    }
    msg_code=WinQueIPMsgs(que);
  } else {
    WinRefocus;
    if (!TaskValidate(sys_focus_task))
      FifoI64Flush(kbd.scan_code_fifo);
  }

  if (sys_focus_task)
    LBtr(&sys_focus_task->task_flags,TASKf_HAS_SONG);

  timeout_val=WinCalcRefreshTimeOut(was_forced);
  WinIPUpdate;
  LBts(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);
  winmgr.t->last_refresh_start_time=tS;
  GrUpdateScreen;
  winmgr.t->last_refresh_end_time=tS;
  LBtr(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);

  if (sys_focus_task && !Bt(&sys_focus_task->task_flags,TASKf_HAS_SONG)) {
    Free(music.cur_song);
    music.cur_song=NULL;
  }
  if (music.cur_song) {
    if (!music.cur_song_task)
      music.cur_song_task=Spawn(&CurSongTask,NULL,"Song");
  } else if (music.cur_song_task) {
    Kill(music.cur_song_task);
    music.cur_song_task=NULL;
  }
  winmgr.updates++;
  if (!mouse.install_attempts)
    SleepUntil(timeout_val);
  else
    do {
      KbdMouseHandler(FALSE,TRUE);
      SleepUntil(MinI64(timeout_val,cnts.jiffies+JIFFY_FREQ/500));
    } while (!Bt(&sys_semas[SYS_SEMA_FORCE_WINMGR],0)&&cnts.jiffies<timeout_val);
  was_forced=LBtr(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
  return msg_code;
}

CDoc *SetCursorPos(CTask *task,I64 ip_text_x,I64 ip_text_y,Bool set_cursor=TRUE)
{
  CDoc *result=NULL;
  Bool unlock;
  I64 x0,y0;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  if (task->win_top <=ip_text_y<=task->win_bottom &&
      task->win_left<=ip_text_x<=task->win_right) {
    if ((result=DocDisplay(task)) && result->flags&DOCF_DONT_SHOW)
      result=NULL;
    else if (set_cursor) {
      unlock=DocLock(result);
      if (result->doc_signature!=DOC_SIGNATURE_VAL)
	result=NULL;
      else {
	x0=result->line_start_col;
	y0=result->cur_top_line_num;
	DocRecalc(result,TRUE);
	result->x=ip_text_x-task->win_left-task->scroll_x/FONT_WIDTH +x0;
	result->y=ip_text_y-task->win_top -task->scroll_y/FONT_HEIGHT+y0;
	DocRecalc(result,FALSE,DOC_RECALC_FIND_CURSOR);
	task->scroll_x=0;
	task->scroll_y=0;
	task->scroll_z=0;
	if (unlock)
	  DocUnlock(result);
      }
    }
    WinToTop(task);
  }
  return result;
}

U0 WinChkMoveSize(CTask *task)
{
  if (task->win_left!=task->win_old_left ||
      task->win_top!=task->win_old_top)
    TaskMsg(task,0,
	MSG_MOVE,task->win_left,task->win_top,0);
  if (task->win_right-task->win_left!=task->win_old_right-task->win_old_left ||
      task->win_bottom-task->win_top!=task->win_old_bottom-task->win_old_top)
    TaskMsg(task,0,
	MSG_RESIZE,task->win_right-task->win_left+1,task->win_bottom-task->win_top+1,0);
  task->win_old_left  =task->win_left;
  task->win_old_right =task->win_right;
  task->win_old_top   =task->win_top;
  task->win_old_bottom=task->win_bottom;
}

Bool WinKeyNavMenu()
{
  I64 i,old_key_cnt;
  CD3I64 old_pos,new_pos;
  CMenu *m;
  CMenuEntry *tempme;
  CTask *focus=MenuTask;
  if (Bt(kbd.down_bitmap,SC_GUI) &&
      focus && (m=focus->cur_menu)) {
    winmgr.show_menu=TRUE;
    sys_cur_submenu_entry=NULL;
    old_pos.x=ip.pos.x; old_pos.y=ip.pos.y;
    ip.pos.x=new_pos.x=ip.pos.y=new_pos.y=0;
    while (m->mu_signature==MENU_SIGNATURE_VAL && Bt(kbd.down_bitmap,SC_GUI)) {
      old_key_cnt=kbd.cnt;
      if (Bt(kbd.down_bitmap,SC_CURSOR_LEFT)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_LEFT) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	if (new_pos.x) {
	  i=0;
	  tempme=m->sub;
	  while (tempme) {
	    if (i+MenuEntryWidth(tempme)*FONT_WIDTH==new_pos.x) {
	      new_pos.x=i;
	      break;
	    }
	    i+=MenuEntryWidth(tempme)*FONT_WIDTH;
	    tempme=tempme->next;
	  }
	}
	new_pos.y=0;
      } else if (Bt(kbd.down_bitmap,SC_CURSOR_RIGHT)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_RIGHT) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	i=0;
	tempme=m->sub;
	while (tempme) {
	  if (i==new_pos.x) {
	    if (tempme->next)
	      new_pos.x=i+MenuEntryWidth(tempme)*FONT_WIDTH;
	    break;
	  }
	  i+=MenuEntryWidth(tempme)*FONT_WIDTH;
	  tempme=tempme->next;
	}
	new_pos.y=0;
      } else if (Bt(kbd.down_bitmap,SC_CURSOR_UP)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_UP) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_pos.y-=FONT_HEIGHT;
      } else if (Bt(kbd.down_bitmap,SC_CURSOR_DOWN)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_DOWN) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_pos.y+=FONT_HEIGHT;
      }
      new_pos.x=ClampI64(new_pos.x,0,GR_WIDTH-1);
      new_pos.y=ClampI64(new_pos.y,0,GR_HEIGHT-1);
      ip.pos.x=new_pos.x; ip.pos.y=new_pos.y;
      WinMgrSleep(TRUE);
      if (!sys_cur_submenu_entry)
	ip.pos.y=new_pos.y=0;
    }
    if (sys_cur_submenu_entry) {
      TaskMsg(sys_focus_task,0,sys_cur_submenu_entry->msg_code,
	  sys_cur_submenu_entry->a1,sys_cur_submenu_entry->a2,0);
      Noise(30,6000,8000);
    }
    winmgr.show_menu=FALSE;
    ip.pos.x=old_pos.x; ip.pos.y=old_pos.y;
    return TRUE;
  }
  return FALSE;
}

U0 WinMgrTask(I64)
{
  CTask *task=Fs;
  CDoc *doc;
  CDocEntry *doc_e;
  I64 x,y,z,msg_code,
	my_ip_z=0,left,top,
	old_flags=GetRFlags;
  Bool has_border;
  CCtrl *c;
  SetWinHorz(0,TEXT_COLS-1);
  SetWinVert(0,TEXT_ROWS-1);
  LBts(&Fs->display_flags,DISPLAYf_NO_BORDER);
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  gr.dc->win_task=Fs;
  Fs->win_inhibit&=~WIF_SELF_CTRLS;
  GrSetUpTables;
  WinZBufUpdate;
  LBts(&sys_run_level,RLf_WINMGR);
  while (TRUE) {
    try {
wmt_start:
      WinMgrSleep;
      //We delay freeing in case lingering
      //references are made to task's.
      TaskFinishOffDying;

      task=Fs->last_task;
      while (TRUE) {
	CLI
	if (!TaskValidate(task)) {
	  SetRFlags(old_flags);
	  goto wmt_start;
	}
	WinChkMoveSize(task);
	TaskDerivedValsUpdate(task,FALSE);
	task=task->last_task;
	SetRFlags(old_flags);
	if (task==Fs)
	  break;
      }
      TaskDerivedValsUpdate(Fs,FALSE);

      task=Fs->last_task;
      while (TRUE) {
	CLI
	if (!TaskValidate(task)) {
	  SetRFlags(old_flags);
	  goto wmt_start;
	}
	if (Bt(&task->display_flags,DISPLAYf_SHOW) &&
	      task->border_pix_left<=ip.pos.x<=task->border_pix_right &&
	      task->border_pix_top<=ip.pos.y<=task->border_pix_bottom) {
	  SetRFlags(old_flags);
	  break;
	}
	if (task==Fs) {	//Shouldn't happen
	  SetRFlags(old_flags);
	  goto wmt_start;
	}
	task=task->last_task;
	SetRFlags(old_flags);
      }

      if (Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	has_border=FALSE;
      else
	has_border=TRUE;

      winmgr.show_menu=FALSE;
      sys_cur_submenu_entry=NULL;
      if (TaskValidate(sys_focus_task) &&
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_MENU)) {
	if (WinKeyNavMenu)
	  goto wmt_start;
	if (task==Fs && 0<=ip.pos.y<FONT_HEIGHT && ip.dev!=IP_NULL) {
	  winmgr.show_menu=TRUE;
	  if (ip.lb && !old_ip.lb) {
	    winmgr.show_menu=TRUE;
	    while (ip.lb)
	      WinMgrSleep(TRUE);
	    if (sys_cur_submenu_entry) {
	      TaskMsg(sys_focus_task,0,
		    sys_cur_submenu_entry->msg_code,
		    sys_cur_submenu_entry->a1,
		    sys_cur_submenu_entry->a2,0);
	      Noise(30,6000,8000);
	    }
	    winmgr.show_menu=FALSE;
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      //grab scroll
      if (!Bt(&task->win_inhibit,WIf_SELF_GRAB_SCROLL) &&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_GRAB_SCROLL)) &&
	    kbd.scan_code&SCF_CTRL && TaskValidate(task)) {
	winmgr.grab_scroll_closed=FALSE;
	winmgr.grab_scroll=TRUE;
	while (kbd.scan_code&SCF_CTRL && TaskValidate(task)) {
	  if (ip.lb) {
	    winmgr.grab_scroll_closed=TRUE;
	    x=ip.pos.x-task->scroll_x;
	    y=ip.pos.y-task->scroll_y;
	    z=ip.pos.z-task->scroll_z;
	    while (ip.lb && kbd.scan_code&SCF_CTRL && TaskValidate(task)) {
	      task->scroll_x=(ip.pos.x-x)&~7;
	      task->scroll_y=(ip.pos.y-y)&~7;
	      task->scroll_z=ip.pos.z-z;
	      WinMgrSleep(TRUE);
	    }
	    winmgr.grab_scroll_closed=FALSE;
	  } else if (ip.rb)  {
	    task->scroll_x=0;
	    task->scroll_y=0;
	    task->scroll_z=0;
	    WinMgrSleep(TRUE);
	  } else
	    WinMgrSleep;
	}
	goto wmt_start;
      } else
	winmgr.grab_scroll=FALSE;

      if (!Bt(&task->win_inhibit,WIf_SELF_CTRLS) &&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_CTRLS))) {
	if (ip.lb && !old_ip.lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ip.pos.x,ip.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_LEFT_IP) {
		while (ip.lb && TaskValidate(task)) {
		  if (c->left_click)
		    (*c->left_click)(c,ip.pos.x-left,ip.pos.y-top,TRUE);
		  WinMgrSleep;
		}
		if (c->left_click)
		  (*c->left_click)(c,ip.pos.x-left,ip.pos.y-top,FALSE);
		old_ip.lb=FALSE;
		goto wmt_start;
	      } else {
		if (c->left_click)
		  (*c->left_click)(c,ip.pos.x-left,ip.pos.y-top,TRUE);
		old_ip.lb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ip.lb && !ip.lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ip.pos.x,ip.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->left_click)
		(*c->left_click)(c,ip.pos.x-left,ip.pos.y-top,FALSE);
	      old_ip.lb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ip.rb && !old_ip.rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ip.pos.x,ip.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_RIGHT_IP) {
		while (ip.rb && TaskValidate(task)) {
		  if (c->right_click)
		    (*c->right_click)(c,ip.pos.x-left,ip.pos.y-top,TRUE);
		  WinMgrSleep;
		}
		if (c->right_click)
		  (*c->right_click)(c,ip.pos.x-left,ip.pos.y-top,FALSE);
		old_ip.rb=FALSE;
		goto wmt_start;
	      } else {
		if (c->right_click)
		  (*c->right_click)(c,ip.pos.x-left,ip.pos.y-top,TRUE);
		old_ip.rb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ip.rb && !ip.rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ip.pos.x,ip.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->right_click)
		(*c->right_click)(c,ip.pos.x-left,ip.pos.y-top,FALSE);
	      old_ip.rb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ip.has_wheel && my_ip_z!=ip.pos.z) {
	  if (task==sys_focus_task) {
	    c=task->next_ctrl;
	    while (c!=&task->next_ctrl) {
	      if (c->wheel_chg) {
		(*c->wheel_chg)(c,ip.pos.z-my_ip_z);
		my_ip_z=ip.pos.z;
		goto wmt_start;
	      }
	      c=c->next;
	    }
	    my_ip_z=ip.pos.z;
	  } else if (!sys_focus_task)
	    my_ip_z=ip.pos.z;
	}
      }

      if (task==Fs)
	goto wmt_start;

      if (!Bt(&task->win_inhibit,WIf_SELF_IP_L)&&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_IP_L))) {
	if (!old_ip.lb && ip.lb) {
	  if (doc=SetCursorPos(task,ip.pos_text.x,ip.pos_text.y)) {
	    DocLock(doc);
	    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
	      doc_e=doc->cur_entry;
	      if (doc_e!=doc) {
		if (doc_e->de_flags & DOCEF_HAS_BORDER)
		  doc_e->de_flags|=DOCEF_SOLID_BORDER;
	      }
	    }
	    DocUnlock(doc);
	    old_ip.lb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_IP_R)&&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_IP_R))) {
	if (!old_ip.rb && ip.rb) {
	  if (SetCursorPos(task,ip.pos_text.x,ip.pos_text.y)) {
	    old_ip.rb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_BORDER) && has_border &&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_BORDER))) {
	if (old_ip.lb && !ip.lb) {
	  if (ip.pos_text.y==task->win_top-1) {
	    if (task->win_left<=ip.pos_text.x<task->win_left+4) {
	      TaskMsg(task,0,MSG_KEY_DOWN,CH_CTRLM,0x43200000432,0);
	      old_ip.lb=FALSE;
	      goto wmt_start;
	    } else if (task->win_right-2<=ip.pos_text.x<=task->win_right) {
	      if (DocPut(task))
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	      else
		Kill(task,FALSE);
	      old_ip.lb=FALSE;
	      goto wmt_start;
	    }
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_IP_L)&&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_IP_L))) {
	if (old_ip.lb && !ip.lb) {
	  if (doc=SetCursorPos(task,ip.pos_text.x,ip.pos_text.y,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) &&
		  (ip.lb || ip.left_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_IP_L_UP) {
		if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		  DocLock(doc);
		  if (TaskValidate(task)) {
		    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		      doc_e=doc->cur_entry;
		      if (doc_e!=doc) {
			if (doc_e->de_flags & DOCEF_HAS_BORDER)
			  doc_e->de_flags&=~DOCEF_SOLID_BORDER;
			if (doc_e->de_flags & (DOCEF_TREE|DOCEF_LST|DOCEF_LINK|DOCEF_CHECK_COLLAPSABLE|
			      DOCEF_LEFT_CB|DOCEF_LEFT_MACRO|DOCEF_LEFT_EXP))
			  TaskMsg(task,0,MSG_KEY_DOWN,CH_SPACE,0,0);
		      }
		    }
		    DocUnlock(doc);
		  }
		}
	      } else if (msg_code==MSG_IP_L_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_ESC,0,0);
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_SELF_IP_R)&&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_IP_R))) {
	if (old_ip.rb && !ip.rb) {
	  if (doc=SetCursorPos(task,ip.pos_text.x,ip.pos_text.y,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) &&
		  (ip.rb || ip.right_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_IP_R_UP) {
		if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		  DocLock(doc);
		  if (TaskValidate(task)) {
		    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		      doc_e=doc->cur_entry;
		      if (doc_e!=doc) {
			if (doc_e->de_flags&(DOCEF_LINK|
			      DOCEF_RIGHT_CB|DOCEF_RIGHT_MACRO|DOCEF_RIGHT_EXP))
			  TaskMsg(task,0,MSG_KEY_DOWN,'\n',0,0);
		      }
		    }
		    DocUnlock(doc);
		  }
		}
	      } else if (msg_code==MSG_IP_R_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	    }
	    old_ip.rb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_SELF_BORDER) && has_border &&
	(!TaskValidate(sys_focus_task)||!Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_BORDER))) {
	if (ip.lb && !old_ip.lb) {
	  if (task->win_top==ip.pos_text.y+1 &&
		task->win_left-1<=ip.pos_text.x<=task->win_right+1) {
	    if (task->win_left<=ip.pos_text.x<task->win_left+4) {
	      old_ip.lb=TRUE;
	      goto wmt_start;
	    }
	    if (task->win_right-2<=ip.pos_text.x<=task->win_right) {
	      old_ip.lb=TRUE;
	      goto wmt_start;
	    }
	    x=ip.pos_text.x-task->win_left;
	    if (ip.lb) {
	      WinToTop(task);
	      while (ip.lb && TaskValidate(task)) {
		SetWinHorz(ip.pos_text.x-x,task->win_width-1+ip.pos_text.x-x,task);
		SetWinVert(ip.pos_text.y+1,task->win_height+ip.pos_text.y,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_left==ip.pos_text.x+1 &&
		task->win_top-1<=ip.pos_text.y<=task->win_bottom+1) {
	    y=ip.pos_text.y-task->win_top;
	    if (ip.lb) {
	      WinToTop(task);
	      while (ip.lb && TaskValidate(task)) {
		SetWinHorz(ip.pos_text.x+1,task->win_width+ip.pos_text.x,task);
		SetWinVert(ip.pos_text.y-y,task->win_height-1+ip.pos_text.y-y,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right+1==ip.pos_text.x && task->win_bottom+1==ip.pos_text.y) {
	    if (ip.lb) {
	      WinToTop(task);
	      while (ip.lb && TaskValidate(task)) {
		SetWinHorz(task->win_left,ip.pos_text.x-1,task);
		SetWinVert(task->win_top,ip.pos_text.y-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_bottom==ip.pos_text.y-1 &&
		task->win_left<=ip.pos_text.x<=task->win_right) {
	    if (ip.lb) {
	      WinToTop(task);
	      while (ip.lb && TaskValidate(task)) {
		SetWinVert(task->win_top,ip.pos_text.y-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right==ip.pos_text.x-1 &&
		task->win_top<=ip.pos_text.y<=task->win_bottom) {
	    if (ip.lb) {
	      WinToTop(task);
	      while (ip.lb && TaskValidate(task)) {
		SetWinHorz(task->win_left,ip.pos_text.x-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip.lb=FALSE;
	    goto wmt_start;
	  }
	}
      }
    } catch {
      Beep;
      Fs->catch_except=TRUE;
      task=Fs;
    }
  }
}
