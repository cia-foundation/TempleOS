#help_index "Windows;Task/Delay"

public U0 WinMgrSync(I64 cnt=1,Bool force=FALSE)
{
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
      if (sys_winmgr_task) sys_winmgr_task->wake_jiffy=sys_jiffies;
    }
    Yield;
  }
  if (cnt>1 && old_doc)
    old_full_refresh=LBts(&old_doc->flags,DOCf_DO_FULL_REFRESH);
  update_cnt=win_updates+cnt;
  while (win_updates<update_cnt) {
    if (force) {
      LBts(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
      if (sys_winmgr_task) sys_winmgr_task->wake_jiffy=sys_jiffies;
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
  static I64 single_ipx=0,single_ipy=0;
  F64 time=tS;
  I64 msg_code=0,p1,p2,single_p1,single_p2;
  CTask *task_focus=sys_focus_task;

  if (task_focus && !win_grab_scroll) {
    p1=ipx-task_focus->pix_left-task_focus->scroll_x;
    p2=ipy-task_focus->pix_top-task_focus->scroll_y;
    single_p1=single_ipx-task_focus->pix_left-task_focus->scroll_x;
    single_p2=single_ipy-task_focus->pix_top-task_focus->scroll_y;
    if (old_ipx_presnap!=ipx_presnap || old_ipy_presnap!=ipy_presnap) {
      if (que)
	TaskMsg(task_focus,0,
	  MSG_IP_MOVE,p1,p2,0);
      msg_code=MSG_IP_MOVE;
    }
    //TODO que msg for ipz?
    if (left_ip_dbl_time) {
      if (time>left_ip_dbl_time) {
	if (left_dbl) {
	  if (!left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_D_DOWN,p1,p2,0);
	    left_down_sent=TRUE;
	    msg_code=MSG_IP_L_D_DOWN;
	  }
	  if (!ip_lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_D_UP,p1,p2,0);
	    left_ip_dbl_time=0;
	    msg_code=MSG_IP_L_D_UP;
	  }
	} else {
	  if (!left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_DOWN,single_p1,single_p2,0);
	    left_down_sent=TRUE;
	    msg_code=MSG_IP_L_DOWN;
	  }
	  if (!ip_lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_L_UP,p1,p2,0);
	    left_ip_dbl_time=0;
	    msg_code=MSG_IP_L_UP;
	  }
	}
      } else {
	if (ip_lb && !ip_lb_last) {
	  left_ip_dbl_time=time;
	  left_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	  Bt(&task_focus->win_inhibit,WIf_IP_L_D)) {
	if (ip_lb  && !ip_lb_last) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_L_DOWN,p1,p2,0);
	  msg_code=MSG_IP_L_DOWN;
	} else if (!ip_lb && ip_lb_last) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_L_UP,p1,p2,0);
	  msg_code=MSG_IP_L_UP;
	}
      } else {
	if (ip_lb  && !ip_lb_last) {
	  left_dbl=FALSE;
	  left_down_sent=FALSE;
	  left_ip_dbl_time=time+dbl_click_time;
	  single_ipx=ipx;
	  single_ipy=ipy;
	}
      }
    }

    if (right_ip_dbl_time) {
      if (time>right_ip_dbl_time) {
	if (right_dbl) {
	  if (!right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_D_DOWN,p1,p2,0);
	    right_down_sent=TRUE;
	    msg_code=MSG_IP_R_D_DOWN;
	  }
	  if (!ip_rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_D_UP,p1,p2,0);
	    right_ip_dbl_time=0;
	    msg_code=MSG_IP_R_D_UP;
	  }
	} else {
	  if (!right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_DOWN,single_p1,single_p2,0);
	    right_down_sent=TRUE;
	    msg_code=MSG_IP_R_DOWN;
	  }
	  if (!ip_rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		MSG_IP_R_UP,p1,p2,0);
	    right_ip_dbl_time=0;
	    msg_code=MSG_IP_R_UP;
	  }
	}
      } else {
	if (ip_rb && !ip_rb_last) {
	  right_ip_dbl_time=time;
	  right_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	  Bt(&task_focus->win_inhibit,WIf_IP_R_D)) {
	if (ip_rb  && !ip_rb_last) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_R_DOWN,p1,p2,0);
	  msg_code=MSG_IP_R_DOWN;
	} else if (!ip_rb && ip_rb_last) {
	  if (que)
	    TaskMsg(task_focus,0,
	      MSG_IP_R_UP,p1,p2,0);
	  msg_code=MSG_IP_R_UP;
	}
      } else {
	if (ip_rb  && !ip_rb_last) {
	  right_dbl=FALSE;
	  right_down_sent=FALSE;
	  right_ip_dbl_time=time+dbl_click_time;
	  single_ipx=ipx;
	  single_ipy=ipy;
	}
      }
    }

    ip_lb_last=ip_lb;
    ip_rb_last=ip_rb;
    old_ip_lb=ip_lb;
    old_ip_rb=ip_rb;
    old_ipx=ipx;
    old_ipy=ipy;
    old_ipz=ipz;
    old_ipx_presnap=ipx_presnap;
    old_ipy_presnap=ipy_presnap;
    old_ipz_presnap=ipz_presnap;
  }
  return msg_code;
}

I64 last_total_jiffies,
   last_idle_pt_hits[MP_MAX_PROCESSORS],
   last_swap_cnter[MP_MAX_PROCESSORS];
F64 last_refresh_time=tS,
   last_refresh_start_time=0,
   last_refresh_end_time=0,
   refresh_time,refresh_delta_time;
I64 win_calc_refresh_cnt=0,
   win_calc_refresh_skips=0;

I64 WinCalcRefreshTimeOut(Bool was_forced)
{
  F64 d,d1;
  I64 i,k,total,
      total_jiffies,idle_pt_hits[MP_MAX_PROCESSORS];
  CCPU *c;

  win_calc_refresh_skips++;
  if ((refresh_time=tS)-last_refresh_time>.25) {
    PUSHFD
    CLI
    total_jiffies=cpu_structs[0].total_jiffies;
    for (i=0;i<mp_cnt;i++)
      idle_pt_hits[i]=cpu_structs[i].idle_pt_hits;
    POPFD

    refresh_delta_time=refresh_time-last_refresh_time;
    total=total_jiffies-last_total_jiffies;
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      d1=0.01;
      if (refresh_delta_time>0) {
	if (total) {
	  if ((k=idle_pt_hits[i]-last_idle_pt_hits[i])>=0) {
	    d1=ToF64(k)/total;
	    d=ToF64(k*time_stamp_freq)
	      /(JIFFY_FREQ*refresh_delta_time);
	    if (d<d1) d1=d;
	  }
	} else {
	  if ((k=idle_pt_hits[i]-last_idle_pt_hits[i])>=0)
	    d1=ToF64(k*time_stamp_freq)
	    /(JIFFY_FREQ*refresh_delta_time);
	}
      }
      last_idle_pt_hits[i]=idle_pt_hits[i];
      c->idle_factor=Clamp(d1,0.01,0.99);
    }
    last_total_jiffies=total_jiffies;
    last_refresh_time=refresh_time;
    if (!was_forced) {
      if (win_first_after_startup_complete) {
	d=Gs->idle_factor;
	d1=(last_refresh_end_time-last_refresh_start_time)/
	    (refresh_delta_time/win_calc_refresh_skips);
	if (d1<0.2)
	  win_desired_refresh*=1.25;
	else if (d<=0.0)
	  win_desired_refresh=win_min_refresh;
	else if (d<0.02) {
	  d=win_min_refresh/win_actual_refresh;
	  win_desired_refresh=d*win_min_refresh+0.5*(1.0-d)*win_desired_refresh;
	} else if (d<0.99) {
	  if (d=(1.0-d)`0.25)
	    win_desired_refresh*= 0.8`0.25/d;
	} else
	  win_desired_refresh=WinMaxRefresh;
	win_calc_refresh_cnt++;
	win_calc_refresh_skips=0;
      } else {
	win_desired_refresh=WinMaxRefresh;
	win_first_after_startup_complete=TRUE;
      }
    }
  }
  win_desired_refresh=Clamp(win_desired_refresh,win_min_refresh,WinMaxRefresh);
  return sys_jiffies+JIFFY_FREQ/win_desired_refresh;
}

CTask *old_sys_focus_task=NULL;

I64 WinMgrSleep(Bool flush_msgs=FALSE)
{
  static Bool was_forced=FALSE;
  I64 timeout_val,msg_code=0;
  CCtrl *c;
  Bool que;

  TimeStampFreqCal;

  if (!flush_msgs && old_sys_focus_task!=sys_focus_task) {
    TaskMsg(old_sys_focus_task,0,MSG_FOCUS,FALSE,0,0);
    TaskMsg(sys_focus_task,0,MSG_FOCUS,TRUE,0,0);
    old_sys_focus_task=sys_focus_task;
  }

  if (flush_msgs)
    FifoI64Flush(scan_code_fifo);
  else if (TaskValidate(sys_focus_task)) {
    KbdMsgsQue;

    que=TRUE;
    if (TaskValidate(sys_focus_task) &&
      !Bt(&sys_focus_task->win_inhibit,WIf_CTRLS)) {
      c=sys_focus_task->next_ctrl;
      while (c!=&sys_focus_task->next_ctrl) {
	if (CtrlInside(c,ipx,ipy)) {
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
      FifoI64Flush(scan_code_fifo);
  }

  if (sys_focus_task)
    LBtr(&sys_focus_task->task_flags,TASKf_HAS_SONG);

  timeout_val=WinCalcRefreshTimeOut(was_forced);
  WinIPUpdate;
  LBts(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);
  last_refresh_start_time=tS;
  GrUpdateScreen;
  last_refresh_end_time=tS;
  LBtr(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);

  if (sys_focus_task && !Bt(&sys_focus_task->task_flags,TASKf_HAS_SONG)) {
    Free(cur_song);
    cur_song=NULL;
  }
  if (cur_song) {
    if (!cur_song_task)
      cur_song_task=Spawn(&CurSongTask,NULL,"Song");
  } else if (cur_song_task) {
    Kill(cur_song_task);
    cur_song_task=NULL;
  }
  win_updates++;
  if (!mouse_install_attempts)
    SleepUntil(timeout_val);
  else
    do {
      KbdMouseHandler(FALSE,TRUE);
      SleepUntil(MinI64(timeout_val,sys_jiffies+JIFFY_FREQ/500));
    } while (!Bt(&sys_semas[SYS_SEMA_FORCE_WINMGR],0)&&sys_jiffies<timeout_val);
  was_forced=LBtr(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
  return msg_code;
}

CDoc *SetCursorPos(CTask *task,I64 iptx,I64 ipty,Bool set_cursor=TRUE)
{
  CDoc *result=NULL;
  Bool unlock;
  I64 x0,y0;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  if (task->win_top <=ipty<=task->win_bottom &&
      task->win_left<=iptx<=task->win_right) {
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
	result->x=iptx-task->win_left-task->scroll_x/FONT_WIDTH +x0;
	result->y=ipty-task->win_top -task->scroll_y/FONT_HEIGHT+y0;
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
  I64 i,old_key_cnt,
    old_ipx,old_ipy,
    new_ipx,new_ipy;
  CMenu *m;
  CMenuEntry *tempme;
  CTask *focus=MenuTask;
  if (Bt(key_down_bitmap,SC_GUI) &&
      focus && (m=focus->cur_menu)) {
    win_show_menu=TRUE;
    sys_cur_submenu_entry=NULL;
    old_ipx=ipx; old_ipy=ipy;
    ipx=new_ipx=ipy=new_ipy=0;
    while (m->mu_signature==MENU_SIGNATURE_VAL && Bt(key_down_bitmap,SC_GUI)) {
      old_key_cnt=sys_key_cnt;
      if (Bt(key_down_bitmap,SC_CURSOR_LEFT)) {
	while (Bt(key_down_bitmap,SC_CURSOR_LEFT) && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	if (new_ipx) {
	  i=0;
	  tempme=m->sub;
	  while (tempme) {
	    if (i+MenuEntryWidth(tempme)*FONT_WIDTH==new_ipx) {
	      new_ipx=i;
	      break;
	    }
	    i+=MenuEntryWidth(tempme)*FONT_WIDTH;
	    tempme=tempme->next;
	  }
	}
	new_ipy=0;
      } else if (Bt(key_down_bitmap,SC_CURSOR_RIGHT)) {
	while (Bt(key_down_bitmap,SC_CURSOR_RIGHT) && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	i=0;
	tempme=m->sub;
	while (tempme) {
	  if (i==new_ipx) {
	    if (tempme->next)
	      new_ipx=i+MenuEntryWidth(tempme)*FONT_WIDTH;
	    break;
	  }
	  i+=MenuEntryWidth(tempme)*FONT_WIDTH;
	  tempme=tempme->next;
	}
	new_ipy=0;
      } else if (Bt(key_down_bitmap,SC_CURSOR_UP)) {
	while (Bt(key_down_bitmap,SC_CURSOR_UP) && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_ipy-=FONT_HEIGHT;
      } else if (Bt(key_down_bitmap,SC_CURSOR_DOWN)) {
	while (Bt(key_down_bitmap,SC_CURSOR_DOWN) && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_ipy+=FONT_HEIGHT;
      }
      new_ipx=ClampI64(new_ipx,0,GR_WIDTH-1);
      new_ipy=ClampI64(new_ipy,0,GR_HEIGHT-1);
      ipx=new_ipx; ipy=new_ipy;
      WinMgrSleep(TRUE);
      if (!sys_cur_submenu_entry)
	ipy=new_ipy=0;
    }
    if (sys_cur_submenu_entry) {
      TaskMsg(sys_focus_task,0,sys_cur_submenu_entry->msg_code,
	  sys_cur_submenu_entry->p1,sys_cur_submenu_entry->p2,0);
      Noise(30,6000,8000);
    }
    win_show_menu=FALSE;
    ipx=old_ipx; ipy=old_ipy;
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
	my_ipz=0,left,top,
	old_flags=GetRFlags;
  Bool has_border;
  CCtrl *c;
  last_total_jiffies=0;
  MemSet(last_idle_pt_hits,0,sizeof(I64)*MP_MAX_PROCESSORS);
  MemSet(last_swap_cnter,0,sizeof(I64)*MP_MAX_PROCESSORS);
  SetWinHorz(0,TEXT_COLS-1);
  SetWinVert(0,TEXT_ROWS-1);
  LBts(&Fs->display_flags,DISPLAYf_NO_BORDER);
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  gr_dc->win_task=Fs;
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
	      task->border_pix_left<=ipx<=task->border_pix_right &&
	      task->border_pix_top<=ipy<=task->border_pix_bottom) {
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

      win_show_menu=FALSE;
      sys_cur_submenu_entry=NULL;
      if (TaskValidate(sys_focus_task) &&
	    !Bt(&sys_focus_task->win_inhibit,WIf_MENU)) {
	if (WinKeyNavMenu)
	  goto wmt_start;
	if (task==Fs && 0<=ipy<FONT_HEIGHT && ip_dev!=IP_NULL) {
	  win_show_menu=TRUE;
	  if (ip_lb && !old_ip_lb) {
	    win_show_menu=TRUE;
	    while (ip_lb)
	      WinMgrSleep(TRUE);
	    if (sys_cur_submenu_entry) {
	      TaskMsg(sys_focus_task,0,
		    sys_cur_submenu_entry->msg_code,
		    sys_cur_submenu_entry->p1,
		    sys_cur_submenu_entry->p2,0);
	      Noise(30,6000,8000);
	    }
	    win_show_menu=FALSE;
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      //grab scroll
      if (!Bt(&task->win_inhibit,WIf_GRAB_SCROLL) &&
	    sys_cur_scan_code&SCF_CTRL && TaskValidate(task)) {
	win_grab_scroll_closed=FALSE;
	win_grab_scroll=TRUE;
	while (sys_cur_scan_code&SCF_CTRL && TaskValidate(task)) {
	  if (ip_lb) {
	    win_grab_scroll_closed=TRUE;
	    x=ipx-task->scroll_x;
	    y=ipy-task->scroll_y;
	    z=ipz-task->scroll_z;
	    while (ip_lb && sys_cur_scan_code&SCF_CTRL && TaskValidate(task)) {
	      task->scroll_x=(ipx-x)&~7;
	      task->scroll_y=(ipy-y)&~7;
	      task->scroll_z=ipz-z;
	      WinMgrSleep(TRUE);
	    }
	    win_grab_scroll_closed=FALSE;
	  } else if (ip_rb)  {
	    task->scroll_x=0;
	    task->scroll_y=0;
	    task->scroll_z=0;
	    WinMgrSleep(TRUE);
	  } else
	    WinMgrSleep;
	}
	goto wmt_start;
      } else
	win_grab_scroll=FALSE;

      if (!Bt(&task->win_inhibit,WIf_CTRLS)) {
	if (ip_lb && !old_ip_lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ipx,ipy)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_LEFT_IP) {
		while (ip_lb && TaskValidate(task)) {
		  if (c->left_click)
		    (*c->left_click)(c,ipx-left,ipy-top,TRUE);
		  WinMgrSleep;
		}
		if (c->left_click)
		  (*c->left_click)(c,ipx-left,ipy-top,FALSE);
		old_ip_lb=FALSE;
		goto wmt_start;
	      } else {
		if (c->left_click)
		  (*c->left_click)(c,ipx-left,ipy-top,TRUE);
		old_ip_lb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ip_lb && !ip_lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ipx,ipy)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->left_click)
		(*c->left_click)(c,ipx-left,ipy-top,FALSE);
	      old_ip_lb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ip_rb && !old_ip_rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ipx,ipy)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_RIGHT_IP) {
		while (ip_rb && TaskValidate(task)) {
		  if (c->right_click)
		    (*c->right_click)(c,ipx-left,ipy-top,TRUE);
		  WinMgrSleep;
		}
		if (c->right_click)
		  (*c->right_click)(c,ipx-left,ipy-top,FALSE);
		old_ip_rb=FALSE;
		goto wmt_start;
	      } else {
		if (c->right_click)
		  (*c->right_click)(c,ipx-left,ipy-top,TRUE);
		old_ip_rb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ip_rb && !ip_rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ipx,ipy)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->right_click)
		(*c->right_click)(c,ipx-left,ipy-top,FALSE);
	      old_ip_rb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ip_has_wheel && my_ipz!=ipz) {
	  if (task==sys_focus_task) {
	    c=task->next_ctrl;
	    while (c!=&task->next_ctrl) {
	      if (c->wheel_chg) {
		(*c->wheel_chg)(c,ipz-my_ipz);
		my_ipz=ipz;
		goto wmt_start;
	      }
	      c=c->next;
	    }
	    my_ipz=ipz;
	  } else if (!sys_focus_task)
	    my_ipz=ipz;
	}
      }

      if (task==Fs)
	goto wmt_start;

      if (!Bt(&task->win_inhibit,WIf_IP_L)) {
	if (!old_ip_lb && ip_lb) {
	  if (doc=SetCursorPos(task,iptx,ipty)) {
	    DocLock(doc);
	    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
	      doc_e=doc->cur_entry;
	      if (doc_e!=doc) {
		if (doc_e->flags1 & DOCEF1_HAS_BORDER)
		  doc_e->flags1|=DOCEF1_SOLID_BORDER;
	      }
	    }
	    DocUnlock(doc);
	    old_ip_lb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_IP_R)) {
	if (!old_ip_rb && ip_rb) {
	  if (SetCursorPos(task,iptx,ipty)) {
	    old_ip_rb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_BORDER) && has_border) {
	if (old_ip_lb && !ip_lb) {
	  if (ipty==task->win_top-1) {
	    if (task->win_left<=iptx<task->win_left+4) {
	      TaskMsg(task,0,MSG_KEY_DOWN,CH_CTRLM,0x43200000432,0);
	      old_ip_lb=FALSE;
	      goto wmt_start;
	    } else if (task->win_right-2<=iptx<=task->win_right) {
	      if (DocPut(task))
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	      else
		Kill(task,FALSE);
	      old_ip_lb=FALSE;
	      goto wmt_start;
	    }
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_IP_L)) {
	if (old_ip_lb && !ip_lb) {
	  if (doc=SetCursorPos(task,iptx,ipty,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) &&
		  (ip_lb || left_ip_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_IP_L_UP) {
		if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		  DocLock(doc);
		  if (TaskValidate(task)) {
		    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		      doc_e=doc->cur_entry;
		      if (doc_e!=doc) {
			if (doc_e->flags1 & DOCEF1_HAS_BORDER)
			  doc_e->flags1&=~DOCEF1_SOLID_BORDER;
			if (doc_e->flags1 & (DOCEF1_TREE|DOCEF1_LINK|DOCEF1_CHECK_COLLAPSABLE|
			      DOCEF1_LEFT_CB|DOCEF1_LEFT_MACRO|DOCEF1_LEFT_EXP))
			  TaskMsg(task,0,MSG_KEY_DOWN,CH_SPACE,0,0);
		      }
		    }
		    DocUnlock(doc);
		  }
		}
	      } else if (msg_code==MSG_IP_L_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_ESC,0,0);
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_IP_R)) {
	if (old_ip_rb && !ip_rb) {
	  if (doc=SetCursorPos(task,iptx,ipty,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) &&
		  (ip_rb || right_ip_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_IP_R_UP) {
		if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		  DocLock(doc);
		  if (TaskValidate(task)) {
		    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		      doc_e=doc->cur_entry;
		      if (doc_e!=doc) {
			if (doc_e->flags1&(DOCEF1_LINK|
			      DOCEF1_RIGHT_CB|DOCEF1_RIGHT_MACRO|DOCEF1_RIGHT_EXP))
			  TaskMsg(task,0,MSG_KEY_DOWN,'\n',0,0);
		      }
		    }
		    DocUnlock(doc);
		  }
		}
	      } else if (msg_code==MSG_IP_R_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	    }
	    old_ip_rb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_BORDER) && has_border) {
	if (ip_lb && !old_ip_lb) {
	  if (task->win_top==ipty+1 &&
		task->win_left-1<=iptx<=task->win_right+1) {
	    if (task->win_left<=iptx<task->win_left+4) {
	      old_ip_lb=TRUE;
	      goto wmt_start;
	    }
	    if (task->win_right-2<=iptx<=task->win_right) {
	      old_ip_lb=TRUE;
	      goto wmt_start;
	    }
	    x=iptx-task->win_left;
	    if (ip_lb) {
	      WinToTop(task);
	      while (ip_lb && TaskValidate(task)) {
		SetWinHorz(iptx-x,task->win_width-1+iptx-x,task);
		SetWinVert(ipty+1,task->win_height+ipty,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_left==iptx+1 &&
		task->win_top-1<=ipty<=task->win_bottom+1) {
	    y=ipty-task->win_top;
	    if (ip_lb) {
	      WinToTop(task);
	      while (ip_lb && TaskValidate(task)) {
		SetWinHorz(iptx+1,task->win_width+iptx,task);
		SetWinVert(ipty-y,task->win_height-1+ipty-y,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right+1==iptx && task->win_bottom+1==ipty) {
	    if (ip_lb) {
	      WinToTop(task);
	      while (ip_lb && TaskValidate(task)) {
		SetWinHorz(task->win_left,iptx-1,task);
		SetWinVert(task->win_top,ipty-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_bottom==ipty-1 &&
		task->win_left<=iptx<=task->win_right) {
	    if (ip_lb) {
	      WinToTop(task);
	      while (ip_lb && TaskValidate(task)) {
		SetWinVert(task->win_top,ipty-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right==iptx-1 &&
		task->win_top<=ipty<=task->win_bottom) {
	    if (ip_lb) {
	      WinToTop(task);
	      while (ip_lb && TaskValidate(task)) {
		SetWinHorz(task->win_left,iptx-1,task);
		WinChkMoveSize(task);
		WinMgrSleep;
	      }
	    }
	    old_ip_lb=FALSE;
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

#help_index ""
