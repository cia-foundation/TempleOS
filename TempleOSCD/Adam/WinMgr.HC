#help_index "Windows;Task/Delay"

public U0 Refresh(I64 cnt=1,Bool force=FALSE)
{//Wait for 30fps WinMgr to start & finish scrn refresh.
  //0,FALSE Cnt Sync to WinMgr.
  //0,TRUE  Pump Msgs.
  //1 Cnt Wait and Pump Msgs.
  //2 Cnt Make Sure to do a Full Refresh
  //and Set Cur Pos.
  Bool	old_full_refresh,
	old_idle=LBts(&Fs->task_flags,TASKf_IDLE);
  CDoc *old_doc=DocPut;
  I64 update_cnt;
  if (!cnt&&force)
    LBts(&sys_semas[SEMA_JUST_PUMP_MSGS],0);
  while (Bt(&sys_semas[SEMA_REFRESH_IN_PROGRESS],0)) {
    if (force && sys_winmgr_task)
      sys_winmgr_task->wake_jiffy=cnts.jiffies;
    Yield;
  }
  if (cnt>1 && old_doc)
    old_full_refresh=LBts(&old_doc->flags,DOCf_DO_FULL_REFRESH);
  update_cnt=winmgr.updates+cnt;
  while (winmgr.updates<update_cnt) {
    if (force && sys_winmgr_task)
      sys_winmgr_task->wake_jiffy=cnts.jiffies;
    Sleep(1);
  }
  if (old_doc)
    LBEqu(&old_doc->flags,DOCf_DO_FULL_REFRESH,old_full_refresh);
  LBEqu(&Fs->task_flags,TASKf_IDLE,old_idle);
}

#help_index "Windows"

I64 WinQueIPMsgs(Bool que)
{
  static CD3I64 single_ms={0,0,0};
  F64 time=tS;
  I64 msg_code=0,arg1,arg2,single_arg1,single_arg2;
  CTask *task_focus=sys_focus_task;

  if (task_focus && !winmgr.grab_scroll) {
    arg1=ms.pos.x-task_focus->pix_left-task_focus->scroll_x;
    arg2=ms.pos.y-task_focus->pix_top-task_focus->scroll_y;
    single_arg1=single_ms.x-task_focus->pix_left-task_focus->scroll_x;
    single_arg2=single_ms.y-task_focus->pix_top-task_focus->scroll_y;
    if (old_ms.presnap.x!=ms.presnap.x || old_ms.presnap.y!=ms.presnap.y) {
      if (que)
	TaskMsg(task_focus,0,
	      MSG_MS_MOVE,arg1,arg2,0);
      msg_code=MSG_MS_MOVE;
    }
//TODO que msg for ms.pos.z?
    if (ms.left_dbl_time) {
      if (time>ms.left_dbl_time) {
	if (ms.left_dbl) {
	  if (!ms.left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_L_D_DOWN,arg1,arg2,0);
	    ms.left_down_sent=TRUE;
	    msg_code=MSG_MS_L_D_DOWN;
	  }
	  if (!ms.lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_L_D_UP,arg1,arg2,0);
	    ms.left_dbl_time=0;
	    msg_code=MSG_MS_L_D_UP;
	  }
	} else {
	  if (!ms.left_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_L_DOWN,single_arg1,single_arg2,0);
	    ms.left_down_sent=TRUE;
	    msg_code=MSG_MS_L_DOWN;
	  }
	  if (!ms.lb) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_L_UP,arg1,arg2,0);
	    ms.left_dbl_time=0;
	    msg_code=MSG_MS_L_UP;
	  }
	}
      } else {
	if (ms.lb && !ms_last.lb) {
	  ms.left_dbl_time=time;
	  ms.left_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	    Bt(&task_focus->win_inhibit,WIf_FOCUS_TASK_MS_L_D)) {
	if (ms.lb  && !ms_last.lb) {
	  if (que)
	    TaskMsg(task_focus,0,
		  MSG_MS_L_DOWN,arg1,arg2,0);
	  msg_code=MSG_MS_L_DOWN;
	} else if (!ms.lb && ms_last.lb) {
	  if (que)
	    TaskMsg(task_focus,0,
		  MSG_MS_L_UP,arg1,arg2,0);
	  msg_code=MSG_MS_L_UP;
	}
      } else {
	if (ms.lb  && !ms_last.lb) {
	  ms.left_dbl=FALSE;
	  ms.left_down_sent=FALSE;
	  ms.left_dbl_time=time+ms.dbl_time;
	  single_ms.x=ms.pos.x;
	  single_ms.y=ms.pos.y;
	}
      }
    }

    if (ms.right_dbl_time) {
      if (time>ms.right_dbl_time) {
	if (ms.right_dbl) {
	  if (!ms.right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_R_D_DOWN,arg1,arg2,0);
	    ms.right_down_sent=TRUE;
	    msg_code=MSG_MS_R_D_DOWN;
	  }
	  if (!ms.rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_R_D_UP,arg1,arg2,0);
	    ms.right_dbl_time=0;
	    msg_code=MSG_MS_R_D_UP;
	  }
	} else {
	  if (!ms.right_down_sent) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_R_DOWN,single_arg1,single_arg2,0);
	    ms.right_down_sent=TRUE;
	    msg_code=MSG_MS_R_DOWN;
	  }
	  if (!ms.rb) {
	    if (que)
	      TaskMsg(task_focus,0,
		    MSG_MS_R_UP,arg1,arg2,0);
	    ms.right_dbl_time=0;
	    msg_code=MSG_MS_R_UP;
	  }
	}
      } else {
	if (ms.rb && !ms_last.rb) {
	  ms.right_dbl_time=time;
	  ms.right_dbl=TRUE;
	}
      }
    } else {
      if (TaskValidate(task_focus) &&
	    Bt(&task_focus->win_inhibit,WIf_FOCUS_TASK_MS_R_D)) {
	if (ms.rb  && !ms_last.rb) {
	  if (que)
	    TaskMsg(task_focus,0,
		  MSG_MS_R_DOWN,arg1,arg2,0);
	  msg_code=MSG_MS_R_DOWN;
	} else if (!ms.rb && ms_last.rb) {
	  if (que)
	    TaskMsg(task_focus,0,
		  MSG_MS_R_UP,arg1,arg2,0);
	  msg_code=MSG_MS_R_UP;
	}
      } else {
	if (ms.rb  && !ms_last.rb) {
	  ms.right_dbl=FALSE;
	  ms.right_down_sent=FALSE;
	  ms.right_dbl_time=time+ms.dbl_time;
	  single_ms.x=ms.pos.x;
	  single_ms.y=ms.pos.y;
	}
      }
    }

    MemCpy(&ms_last,&ms,sizeof(CMsStateGlbls));
    MemCpy(&old_ms,&ms,sizeof(CMsStateGlbls));
  }
  return msg_code;
}

U0 WinCalcIdles()
{
  F64 calc_idle_time;
  I64 i,k,total_jiffies,total_jiffies_delta,idle_pt_hits[MP_PROCESSORS_NUM];
  CCPU *c;
  CWinMgrTimingGlbls *t=winmgr.t;

  if ((t->calc_idle_delta_time=
	(calc_idle_time=tS)-t->last_calc_idle_time)>.25) {
    PUSHFD
    CLI
    total_jiffies=cpu_structs[0].total_jiffies;
    for (i=0;i<mp_cnt;i++)
      idle_pt_hits[i]=cpu_structs[i].idle_pt_hits;
    POPFD

    total_jiffies_delta=total_jiffies-t->last_total_jiffies;
    for (i=0;i<mp_cnt;i++) {
      c=&cpu_structs[i];
      if (total_jiffies_delta && (k=idle_pt_hits[i]-t->last_idle_pt_hits[i])>=0)
	c->idle_factor=Clamp(ToF64(k)/total_jiffies_delta,0.01,0.99);
      else
	c->idle_factor=0.01;
      t->last_idle_pt_hits[i]=idle_pt_hits[i];
    }
    t->last_total_jiffies=total_jiffies;
    t->last_calc_idle_time=calc_idle_time;
    t->calc_idle_cnt++;
  }
}

I64 WinMgrSleep(Bool flush_msgs=FALSE)
{
  I64 timeout_val,msg_code=0;
  CCtrl *c;
  Bool que;
  F64 t,t_delta;
  U8 *st;
  CDC *diff;
  CDate cdt;

  TimeCal;
  if ((t_delta=(t=tS)-winmgr.last_refresh_tS)>0.01)
    winmgr.fps=Max(1.0/t_delta,1);
  else
    winmgr.fps=99;
  winmgr.last_refresh_tS=t;
  WinCalcIdles;

  if (flush_msgs)
    FifoI64Flush(kbd.scan_code_fifo);
  else if (TaskValidate(sys_focus_task)) {
    KbdMsgsQue;

    que=TRUE;
    if (TaskValidate(sys_focus_task) &&
	  !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_CTRLS)) {
      c=sys_focus_task->next_ctrl;
      while (c!=&sys_focus_task->next_ctrl) {
	if (CtrlInside(c,ms.pos.x,ms.pos.y)) {
	  que=FALSE;
	  break;
	}
	c=c->next;
      }
    }
    msg_code=WinQueIPMsgs(que);
  } else {
    WinRefocus(sys_focus_task);
    if (!TaskValidate(sys_focus_task))
      FifoI64Flush(kbd.scan_code_fifo);
  }
  if (sys_focus_task)
    LBtr(&sys_focus_task->task_flags,TASKf_HAS_SONG);
  WinMsUpdate;

  if (!LBtr(&sys_semas[SEMA_JUST_PUMP_MSGS],0)) {
    t=tS+WINMGR_PERIOD/8;
    while (winmgr.ideal_refresh_tS<t)
      winmgr.ideal_refresh_tS+=WINMGR_PERIOD;
    timeout_val=cnts.jiffies+(winmgr.ideal_refresh_tS-tS)*JIFFY_FREQ;
    LBts(&sys_semas[SEMA_REFRESH_IN_PROGRESS],0);
    GrUpdateScrn;
    LBtr(&sys_semas[SEMA_REFRESH_IN_PROGRESS],0);
    if (scrncast.record  && !scrncast.just_audio) {
      cdt=scrncast.t0_now(I64)+ToI64(CDATE_FREQ*(tS-scrncast.t0_tS));
      if (!scrncast.dc) {
	scrncast.dc=DCCopy(scrncast.dc2_alias);
	scrncast.dc->cdt=cdt;
	st=MStrPrint(scrncast.print_fmt,cdt);
	GRWrite(st,scrncast.dc);
	Free(st);
      } else if (MemCmp(scrncast.dc->body,
	    scrncast.dc2_alias->body,MSize(scrncast.dc2_alias->body))) {
	diff=DCDiff(scrncast.dc,scrncast.dc2_alias);
	diff->cdt=cdt;
	st=MStrPrint(scrncast.print_fmt,cdt);
	GRWrite(st,diff);
	Free(st);
	DCDel(diff);
	Free(scrncast.dc->body);
	scrncast.dc->body=MAllocIdent(scrncast.dc2_alias->body);
      }
    } else if (scrncast.dc) {
      DCDel(scrncast.dc); //TODO: $LK,"MemRep",A="MN:MemRep"$ can crash.
      scrncast.dc=NULL;
    }
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
    if (ms_hard.install_attempts) //Don't call before boot mouse install attempt
      KbdMsHndlr(FALSE,TRUE);
    SleepUntil(timeout_val);
  }
  return msg_code;
}

CDoc *WinCursorPosSet(CTask *task,I64 msx,I64 msy,Bool set_cursor=TRUE)
{
  CDoc *res=NULL;
  Bool unlock;
  I64 x0,y0;
  if (!task) task=Fs;
  if (WinInside(msx,msy,task)) {
    if ((res=DocDisplay(task)) && res->flags&DOCF_DONT_SHOW)
      res=NULL;
    else if (set_cursor) {
      unlock=DocLock(res);
      if (res->doc_signature!=DOC_SIGNATURE_VAL)
	res=NULL;
      else {
	x0=res->line_start_col;
	y0=res->top_line_num;
	DocRecalc(res,RECALCF_HAS_CURSOR);
	res->x=(msx-task->pix_left-task->scroll_x)/FONT_WIDTH +x0;
	res->y=(msy-task->pix_top -task->scroll_y)/FONT_HEIGHT+y0;
	DocRecalc(res,RECALCt_FIND_CURSOR);
	task->scroll_x=0;
	task->scroll_y=0;
	task->scroll_z=0;
	if (unlock)
	  DocUnlock(res);
      }
    }
    WinToTop(task);
  }
  return res;
}

Bool WinKeyNavMenu()
{
  I64 i,old_key_cnt;
  CD3I64 old_pos,new_pos;
  CMenu *m;
  CMenuEntry *tmpme;
  CTask *focus=MenuTask;
  if (Bt(kbd.down_bitmap,SC_GUI) && focus && (m=focus->cur_menu)) {
    winmgr.show_menu=TRUE;
    sys_cur_submenu_entry=NULL;
    old_pos.x=ms.pos.x; old_pos.y=ms.pos.y;
    ms.pos.x=new_pos.x=ms.pos.y=new_pos.y=0;
    while (Bt(kbd.down_bitmap,SC_GUI)) {
      old_key_cnt=kbd.cnt;
      if (Bt(kbd.down_bitmap,SC_CURSOR_LEFT)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_LEFT) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	if (new_pos.x) {
	  i=0;
	  tmpme=m->sub;
	  while (tmpme) {
	    if (i+MenuEntryWidth(tmpme)*FONT_WIDTH==new_pos.x) {
	      new_pos.x=i;
	      break;
	    }
	    i+=MenuEntryWidth(tmpme)*FONT_WIDTH;
	    tmpme=tmpme->next;
	  }
	}
	new_pos.y=0;
      } else if (Bt(kbd.down_bitmap,SC_CURSOR_RIGHT)) {
	while (Bt(kbd.down_bitmap,SC_CURSOR_RIGHT) && kbd.cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	i=0;
	tmpme=m->sub;
	while (tmpme) {
	  if (i==new_pos.x) {
	    if (tmpme->next)
	      new_pos.x=i+MenuEntryWidth(tmpme)*FONT_WIDTH;
	    break;
	  }
	  i+=MenuEntryWidth(tmpme)*FONT_WIDTH;
	  tmpme=tmpme->next;
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
      ms.pos.x=new_pos.x; ms.pos.y=new_pos.y;
      WinMgrSleep(TRUE);
      if (!sys_cur_submenu_entry)
	ms.pos.y=new_pos.y=0;
    }
    if (sys_cur_submenu_entry)
      TaskMsg(sys_focus_task,0,sys_cur_submenu_entry->msg_code,
	    sys_cur_submenu_entry->arg1,sys_cur_submenu_entry->arg2,0);
    winmgr.show_menu=FALSE;
    ms.pos.x=old_pos.x; ms.pos.y=old_pos.y;
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
	my_ms_z=0,left,top,
	old_flags=GetRFlags;
  Bool has_border;
  CCtrl *c;
  WinHorz(0,TEXT_COLS-1);
  WinVert(0,TEXT_ROWS-1);
  LBts(&Fs->display_flags,DISPLAYf_NO_BORDER);
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  gr.dc->win_task=Fs;
  Fs->win_inhibit&=~WIF_SELF_CTRLS;
  GrSetUpTables;
  scrncast.dc2_alias=DCAlias(gr.dc2);
  WinZBufUpdate;
  LBts(&sys_run_level,RLf_WINMGR);
  while (TRUE) {
    try {
wmt_start:
      if (Bt(&sys_run_level,RLf_ADAM_SERVER))
	TaskKillDying;
      WinMgrSleep;

      task=Fs->last_task;
      while (TRUE) {
	CLI
	if (!TaskValidate(task)) {
	  SetRFlags(old_flags);
	  goto wmt_start;
	}
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
	if (WinInside(ms.pos.x,ms.pos.y,task,FONT_WIDTH)) {
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
	if (task==Fs && 0<=ms.pos.y<FONT_HEIGHT && ms_hard.installed) {
	  winmgr.show_menu=TRUE;
	  if (ms.lb && !old_ms.lb) {
	    winmgr.show_menu=TRUE;
	    while (ms.lb)
	      WinMgrSleep(TRUE);
	    if (sys_cur_submenu_entry)
	      TaskMsg(sys_focus_task,0,
		    sys_cur_submenu_entry->msg_code,
		    sys_cur_submenu_entry->arg1,
		    sys_cur_submenu_entry->arg2,0);
	    winmgr.show_menu=FALSE;
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      //grab scroll
      if (!Bt(&task->win_inhibit,WIf_SELF_GRAB_SCROLL) &&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_GRAB_SCROLL)) &&
	    kbd.scan_code&SCF_CTRL && TaskValidate(task)) {
	winmgr.grab_scroll_closed=FALSE;
	winmgr.grab_scroll=TRUE;
	while (kbd.scan_code&SCF_CTRL && TaskValidate(task) && (!ac.task ||
	      !WinInside(ms.pos.x,ms.pos.y,ac.task,FONT_WIDTH))) {
	  if (ms.lb) {
	    winmgr.grab_scroll_closed=TRUE;
	    x=ms.pos.x-task->scroll_x;
	    y=ms.pos.y-task->scroll_y;
	    z=ms.pos.z-task->scroll_z;
	    while (ms.lb && kbd.scan_code&SCF_CTRL && TaskValidate(task)) {
	      task->scroll_x=(ms.pos.x-x)&~7;
	      task->scroll_y=(ms.pos.y-y)&~7;
	      task->scroll_z=ms.pos.z-z;
	      WinMgrSleep(TRUE);
	    }
	    winmgr.grab_scroll_closed=FALSE;
	  } else if (ms.rb)  {
	    task->scroll_x=0;
	    task->scroll_y=0;
	    task->scroll_z=0;
	    WinMgrSleep(TRUE);
	  } else
	    WinMgrSleep;
	}
	winmgr.grab_scroll=FALSE;
	goto wmt_start;
      } else
	winmgr.grab_scroll=FALSE;

      if (!Bt(&task->win_inhibit,WIf_SELF_CTRLS) &&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_CTRLS))) {
	if (ms.lb && !old_ms.lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ms.pos.x,ms.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_LEFT_MS) {
		while (ms.lb && TaskValidate(task)) {
		  if (c->left_click)
		    (*c->left_click)(c,ms.pos.x-left,ms.pos.y-top,TRUE);
		  WinMgrSleep;
		}
		if (c->left_click)
		  (*c->left_click)(c,ms.pos.x-left,ms.pos.y-top,FALSE);
		old_ms.lb=FALSE;
		goto wmt_start;
	      } else {
		if (c->left_click)
		  (*c->left_click)(c,ms.pos.x-left,ms.pos.y-top,TRUE);
		old_ms.lb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ms.lb && !ms.lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ms.pos.x,ms.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->left_click)
		(*c->left_click)(c,ms.pos.x-left,ms.pos.y-top,FALSE);
	      old_ms.lb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ms.rb && !old_ms.rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ms.pos.x,ms.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->flags&CTRLF_CAPTURE_RIGHT_MS) {
		while (ms.rb && TaskValidate(task)) {
		  if (c->right_click)
		    (*c->right_click)(c,ms.pos.x-left,ms.pos.y-top,TRUE);
		  WinMgrSleep;
		}
		if (c->right_click)
		  (*c->right_click)(c,ms.pos.x-left,ms.pos.y-top,FALSE);
		old_ms.rb=FALSE;
		goto wmt_start;
	      } else {
		if (c->right_click)
		  (*c->right_click)(c,ms.pos.x-left,ms.pos.y-top,TRUE);
		old_ms.rb=TRUE;
		goto wmt_start;
	      }
	    }
	    c=c->next;
	  }
	}
	if (old_ms.rb && !ms.rb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ms.pos.x,ms.pos.y)) {
	      left=task->pix_left;
	      top =task->pix_top;
	      if (c->flags&CTRLF_BORDER) {
		left-=FONT_WIDTH;
		top -=FONT_HEIGHT;
	      }
	      if (c->right_click)
		(*c->right_click)(c,ms.pos.x-left,ms.pos.y-top,FALSE);
	      old_ms.rb=FALSE;
	      goto wmt_start;
	    }
	    c=c->next;
	  }
	}
	if (ms.has_wheel && my_ms_z!=ms.pos.z) {
	  if (task==sys_focus_task) {
	    c=task->next_ctrl;
	    while (c!=&task->next_ctrl) {
	      if (c->wheel_chg) {
		(*c->wheel_chg)(c,ms.pos.z-my_ms_z);
		my_ms_z=ms.pos.z;
		goto wmt_start;
	      }
	      c=c->next;
	    }
	    my_ms_z=ms.pos.z;
	  } else if (!sys_focus_task)
	    my_ms_z=ms.pos.z;
	}
      }

      if (task==Fs)
	goto wmt_start;

      if (!Bt(&task->win_inhibit,WIf_SELF_MS_L)&&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_MS_L))) {
	if (!old_ms.lb && ms.lb) {
	  if (doc=WinCursorPosSet(task,ms.pos.x,ms.pos.y)) {
	    DocLock(doc);
	    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
	      doc_e=doc->cur_entry;
	      if (doc_e!=doc) {
		if (doc_e->de_flags & DOCEF_HAS_BORDER)
		  doc_e->de_flags|=DOCEF_SOLID_BORDER;
	      }
	    }
	    DocUnlock(doc);
	    old_ms.lb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_MS_R)&&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_MS_R))) {
	if (!old_ms.rb && ms.rb) {
	  if (WinCursorPosSet(task,ms.pos.x,ms.pos.y)) {
	    old_ms.rb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_BORDER) && has_border &&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_BORDER))) {
	if (old_ms.lb && !ms.lb) {
	  if (ms.pos_text.y==task->win_top-1) {
	    if (task->win_left<=ms.pos_text.x<task->win_left+4) {
	      TaskMsg(task,0,MSG_KEY_DOWN,CH_CTRLM,0x43200000432,0);
	      old_ms.lb=FALSE;
	      goto wmt_start;
	    } else if (task->win_right-2<=ms.pos_text.x<=task->win_right) {
	      if (DocPut(task))
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	      else
		Kill(task,FALSE);
	      old_ms.lb=FALSE;
	      goto wmt_start;
	    }
	  }
	}
      }
      if (!Bt(&task->win_inhibit,WIf_SELF_MS_L)&&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_MS_L))) {
	if (old_ms.lb && !ms.lb) {
	  if (doc=WinCursorPosSet(task,ms.pos.x,ms.pos.y,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) && (ms.lb || ms.left_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_MS_L_UP) {
		if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		  DocLock(doc);
		  if (TaskValidate(task)) {
		    if (doc->doc_signature==DOC_SIGNATURE_VAL) {
		      doc_e=doc->cur_entry;
		      if (doc_e!=doc) {
			if (doc_e->de_flags & DOCEF_HAS_BORDER)
			  doc_e->de_flags&=~DOCEF_SOLID_BORDER;
			if (doc_e->de_flags & (DOCEF_TREE|DOCEF_LST|
			      DOCEF_LINK|DOCEF_CHECK_COLLAPSABLE|
			      DOCEF_LEFT_CB|DOCEF_LEFT_MACRO|DOCEF_LEFT_EXP))
			  TaskMsg(task,0,MSG_KEY_DOWN,CH_SPACE,0,0);
		      }
		    }
		    DocUnlock(doc);
		  }
		}
	      } else if (msg_code==MSG_MS_L_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_ESC,0,0);
	    }
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_SELF_MS_R)&&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_MS_R))) {
	if (old_ms.rb && !ms.rb) {
	  if (doc=WinCursorPosSet(task,ms.pos.x,ms.pos.y,FALSE)) {
	    do msg_code=WinMgrSleep;
	    while (TaskValidate(task) && (ms.rb || ms.right_dbl_time));
	    if (TaskValidate(task)) {
	      if (msg_code==MSG_MS_R_UP) {
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
	      } else if (msg_code==MSG_MS_R_D_UP)
		TaskMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
	    }
	    old_ms.rb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!Bt(&task->win_inhibit,WIf_SELF_BORDER) && has_border &&
	    (!TaskValidate(sys_focus_task)||
	    !Bt(&sys_focus_task->win_inhibit,WIf_FOCUS_TASK_BORDER))) {
	if (ms.lb && !old_ms.lb) {
	  if (task->win_top==ms.pos_text.y+1 &&
		task->win_left-1<=ms.pos_text.x<=task->win_right+1) {
	    if (task->win_left<=ms.pos_text.x<task->win_left+4) {
	      old_ms.lb=TRUE;
	      goto wmt_start;
	    }
	    if (task->win_right-2<=ms.pos_text.x<=task->win_right) {
	      old_ms.lb=TRUE;
	      goto wmt_start;
	    }
	    x=ms.pos_text.x-task->win_left;
	    if (ms.lb) {
	      WinToTop(task);
	      while (ms.lb && TaskValidate(task)) {
		WinHorz(ms.pos_text.x-x,task->win_width-1+ms.pos_text.x-x,task);
		WinVert(ms.pos_text.y+1,task->win_height+ms.pos_text.y,task);
		WinMgrSleep;
	      }
	    }
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_left==ms.pos_text.x+1 &&
		task->win_top-1<=ms.pos_text.y<=task->win_bottom+1) {
	    y=ms.pos_text.y-task->win_top;
	    if (ms.lb) {
	      WinToTop(task);
	      while (ms.lb && TaskValidate(task)) {
		WinHorz(ms.pos_text.x+1,task->win_width+ms.pos_text.x,task);
		WinVert(ms.pos_text.y-y,
		      task->win_height-1+ms.pos_text.y-y,task);
		WinMgrSleep;
	      }
	    }
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right+1==ms.pos_text.x &&
		task->win_bottom+1==ms.pos_text.y) {
	    if (ms.lb) {
	      WinToTop(task);
	      while (ms.lb && TaskValidate(task)) {
		WinHorz(task->win_left,ms.pos_text.x-1,task);
		WinVert(task->win_top,ms.pos_text.y-1,task);
		WinMgrSleep;
	      }
	    }
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_bottom==ms.pos_text.y-1 &&
		task->win_left<=ms.pos_text.x<=task->win_right) {
	    if (ms.lb) {
	      WinToTop(task);
	      while (ms.lb && TaskValidate(task)) {
		WinVert(task->win_top,ms.pos_text.y-1,task);
		WinMgrSleep;
	      }
	    }
	    old_ms.lb=FALSE;
	    goto wmt_start;
	  }
	  if (task->win_right==ms.pos_text.x-1 &&
		task->win_top<=ms.pos_text.y<=task->win_bottom) {
	    if (ms.lb) {
	      WinToTop(task);
	      while (ms.lb && TaskValidate(task)) {
		WinHorz(task->win_left,ms.pos_text.x-1,task);
		WinMgrSleep;
	      }
	    }
	    old_ms.lb=FALSE;
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
