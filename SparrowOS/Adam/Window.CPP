#help_index "Windows"

I64 old_ipx=-1000,old_ipy=-1000,old_ipz=0;
I64 old_ipx_presnap=-1000,old_ipy_presnap=-1000,old_ipz_presnap=0;
Bool old_ip_lb=FALSE,old_ip_rb=FALSE;
F64 left_ip_dbl_time=0;
Bool left_dbl=FALSE,left_down_sent=FALSE;
F64 right_ip_dbl_time=0;
Bool right_dbl=FALSE,right_down_sent=FALSE;
public F64 dbl_click_time=0.350;

public I64 win_updates=0;
Bool win_first_after_startup_complete=FALSE;
F64 refresh_ode_time=0,last_refresh_ode_time=0;

//These are not hardware rates.  They
//are the rate the video mem is updated.
public F64
   win_min_refresh=15,
   win_desired_refresh	  =win_min_refresh, //This is dynamically adjusted
   win_actual_refresh	  =win_min_refresh;

F64 WinMaxRefresh()
{
  if (sys_focus_task && TaskValidate(sys_focus_task))
    return sys_focus_task->win_max_refresh;
  else
    return WIN_DFT_MAX_REFRESH;
}

Bool WinInhibit(I64 flag)
{
  if (sys_focus_task && TaskValidate(sys_focus_task))
    return Bt(&sys_focus_task->win_inhibit,flag);
  else
    return FALSE;
}


#define PROGRESS_BAR_HEIGHT	20
#define PROGRESS_BAR_WIDTH	(3*GR_WIDTH/4)

U0 DrawProgressBars(CDC *dc)
{
  I64 i,j,k,m;
  for (i=0;i<NUM_PROGRESS_BARS;i++) {
    if (m=sys_progresses[i].max) {
      dc->color=BLACK;
      GrRect(dc,
	(GR_WIDTH-PROGRESS_BAR_WIDTH)/2,
	(GR_HEIGHT-(NUM_PROGRESS_BARS*2-1-i*4)*PROGRESS_BAR_HEIGHT)/2,
	PROGRESS_BAR_WIDTH,PROGRESS_BAR_HEIGHT);

      dc->color=LTGREEN;
      k=sys_progresses[i].val;
      if (k>m)
	k=m;
      GrRect(dc,
	(GR_WIDTH-PROGRESS_BAR_WIDTH)/2+2,
	(GR_HEIGHT-(NUM_PROGRESS_BARS*2-1-i*4)*PROGRESS_BAR_HEIGHT)/2+2,
	k*(PROGRESS_BAR_WIDTH-4)/m,
	PROGRESS_BAR_HEIGHT-4);

      if (m>1) {
	dc->color=BLACK;
	k=m-1;
	if (k>19) k=19;
	for (j=0;j<=k;j++)
	  GrLine(dc,
	  (GR_WIDTH-PROGRESS_BAR_WIDTH)/2+1+j*(PROGRESS_BAR_WIDTH-4)/ToF64(k+1),
	  (GR_HEIGHT-(NUM_PROGRESS_BARS*2-1-i*4)*PROGRESS_BAR_HEIGHT)/2+4,
	  (GR_WIDTH-PROGRESS_BAR_WIDTH)/2+1+j*(PROGRESS_BAR_WIDTH-4)/ToF64(k+1),
	  (GR_HEIGHT-(NUM_PROGRESS_BARS*2-3-i*4)*PROGRESS_BAR_HEIGHT)/2-4);
      }
      dc->color=WHITE;
      GrPutS(dc,
	(GR_WIDTH-FONT_WIDTH*StrLen(sys_progresses[i].desc))/2,
	(GR_HEIGHT-FONT_HEIGHT-(NUM_PROGRESS_BARS*2-2-i*4)*PROGRESS_BAR_HEIGHT)/2,
	sys_progresses[i].desc);
    }
  }
}

Bool win_show_grid=FALSE;
F64 win_grid_spacing=8.0,
    win_grid_x_offset=0,
    win_grid_y_offset=0;

U0 DrawWinGrid(CDC *dc)
{
  F64 d;
  dc->color=BLACK;
  dc->pen_width=1;
  for (d=win_grid_x_offset;d<GR_WIDTH;d+=win_grid_spacing)
    GrLine(dc,d,0,d,GR_HEIGHT-1);
  for (d=win_grid_y_offset;d<GR_HEIGHT;d+=win_grid_spacing)
    GrLine(dc,0,d,GR_WIDTH-1,d);
}

U0 WinGrid()
{
  win_grid_spacing =PmtF64("Spacing  (%12.5f):",win_grid_spacing,2.0);
  win_grid_x_offset=PmtF64("X-Offset (%12.5f):",win_grid_x_offset,0.0);
  win_grid_y_offset=PmtF64("Y-Offset (%12.5f):",win_grid_y_offset,0.0);
  win_show_grid=TRUE;
}

U0 CtrlAltG(I64 sc)
{
  if (sc&SCF_SHIFT)
    win_show_grid=FALSE;
  else
    PopUp("WinGrid;");
}

SetCtrlAltLetCB('G',&CtrlAltG,"Sys/Grid On or Off");

CTask *ext_ASCII_task;
U0 ExtendedASCII()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPutS(doc,"Select Char and Press <ESC>\n");
  for (i=0;i<256;i++) {
    if (i>=CH_SHIFT_SPACE && i!=127) {
      if (i==CH_SHIFT_SPACE)
	DocPrintF(doc,"$$MU-X-UL,\"\\37\",%d$$",i);
      else if (i=='$$')
	DocPrintF(doc,"$$MU-X-UL,\"\\44\",%d$$",i);
      else if (i=='\"'||i=='\\')
	DocPrintF(doc,"$$MU-X-UL,\"\\%c\",%d$$",i,i);
      else
	DocPrintF(doc,"$$MU-X-UL,\"%c\",%d$$",i,i);
    } else
      DocPutS(doc," ");
    if (i&15==15)
      DocPutS(doc,"\n");
  }
  i=PopUpMenu(doc);
  DocDel(doc);
  PostMsg(ext_ASCII_task,MSG_KEY_DOWN_UP,i,A2ScanCode(i));
}

U0 CtrlAltA(I64)
{
  if (ext_ASCII_task=sys_focus_task)
    Spawn(&ExtendedASCII);
}
SetCtrlAltLetCB('A',&CtrlAltA,"Sys/Extended ASCII");


Bool win_grab_scroll=FALSE,win_grab_scroll_closed=FALSE;
public U0 WinScrollNull(CTask *task,CD3I64 *s)
{
  s->x=task->win_scroll_x;
  s->y=task->win_scroll_y;
  s->z=task->win_scroll_z;
  task->win_scroll_x=0;
  task->win_scroll_y=0;
  task->win_scroll_z=0;
}

public U0 WinScrollRestore(CTask *task,CD3I64 *s)
{
  task->win_scroll_x=s->x;
  task->win_scroll_y=s->y;
  task->win_scroll_z=s->z;
}

U0 DrawInputPtr(CDC *dc)
{
  I64 x,y;
  PushFD;
  Cli;
  x=ipx;
  y=ipy;
  PopFD;
  if (ip_on && ip_dev!=IP_NULL) {
    if (text_mode)
      gr_text_base[iptx+ipty*TEXT_COLS]^=0x7F00;
    else {
      if (fp_draw_input_ptr) {
	if (ip_lb)
	  dc->color=ROP_XOR+LTPURPLE^COLOR_TRANSPARENT;
	else if (ip_rb)
	  dc->color=ROP_XOR+LTCYAN^COLOR_TRANSPARENT;
	else
	  dc->color=ROP_XOR+BLACK^COLOR_TRANSPARENT;
	if (win_grab_scroll && fp_draw_grab_input_ptr)
	  (*fp_draw_grab_input_ptr)(dc,x,y,win_grab_scroll_closed);
	else
	  (*fp_draw_input_ptr)(dc,x,y);
      }
    }
  }
}

Bool win_show_menu=FALSE;
U0 WinFinalUpdate(CDC *dc)
{
  if (win_show_grid)
    DrawWinGrid(dc);
  DrawProgressBars(dc);
  if (win_show_menu)
    DrawMenu(dc);
  else
    sys_cur_submenu_entry=NULL;
  DrawInputPtr(dc);
}

fp_final_screen_update=&WinFinalUpdate;

U0 WinIPUpdate()
{
  I64 dd;
  F64 temp,temp2;
  Bool set=FALSE,old_preempt=Preempt(OFF);
 
  if (ip_dev==IP_MOUSE) {
    ip_has_wheel=mouse_has_wheel;
    if (mouse_evt) {
      ip_throttle+=ipz_scale*(mzz-ipz)*0x10000000;
      IPVarsUpdate(mxx,myy,mzz,mouse_buttons[0],ip_cb,mouse_buttons[1]);
      mouse_evt=FALSE;
      set=TRUE;
    }
  } else if (ip_dev==IP_NULL && mouse_installed)
    ip_dev=IP_MOUSE;
 
  if (set) {
    if (ip_dev==IP_MOUSE) {
      ip_speed=mouse_speed;
      ip_last_move_time=mouse_evt_time;
    } else {
      if (dd=SqrI64(old_ipx_presnap-ipx_presnap)+
	  SqrI64(old_ipy_presnap-ipy_presnap)) {
	temp2=tS;
	temp=temp2-ip_last_move_time;
	ip_last_move_time=temp2;
	if (!temp) temp=1;
	ip_speed=Sqrt(dd)/temp;
      } else
	ip_speed=0.0;
    }
  } else
    ip_speed*=0.95;
  if (gr_screen_zoom!=1) {
    if (gr_continuous_scroll)
      GrScaleZoom(1.0);
    else {
      dd=(ipx-grsx)*gr_screen_zoom;
      if (dd<8 || dd>=GR_WIDTH-8)
	GrScaleZoom(1.0);
      else {
	dd=(ipy-grsy)*gr_screen_zoom;
	if (dd<8 || dd>=GR_HEIGHT-8)
	  GrScaleZoom(1.0);
      }
    }
  }
  Preempt(old_preempt);
}

I64 last_total_jiffies,
   last_idle_pt_hits[MP_MAX_PROCESSORS],
   last_swap_cnter[MP_MAX_PROCESSORS];
F64 last_refresh_time=tNP,
   last_refresh_start_time=0,
   last_refresh_end_time=0,
   refresh_time,refresh_delta_time;
I64 win_calc_refresh_cnt=0,
   win_calc_refresh_skips=0;

I64 WinCalcRefreshTimeout(Bool was_forced)
{
  F64 d,d1;
  I64 i,k,total,
      total_jiffies,idle_pt_hits[MP_MAX_PROCESSORS];
  CCPU *c;

  win_calc_refresh_skips++;
  if ((refresh_time=tNP)-last_refresh_time>.25) {
    PushFD;
    Cli;
    total_jiffies=cpu_structs[0].total_jiffies;
    for (i=0;i<mp_cnt;i++)
      idle_pt_hits[i]=cpu_structs[i].idle_pt_hits;
    PopFD;

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
      c->idle_factor=Limit(d1,0.01,0.99);
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
  win_desired_refresh=Limit(win_desired_refresh,win_min_refresh,WinMaxRefresh);
  return sys_jiffies+JIFFY_FREQ/win_desired_refresh;
}

I64 WinQueIPMsgs(Bool que)
{
  static I64 single_ipx=0,single_ipy=0;
  F64 time=tNP;
  I64 msg_code=0,p1,p2,single_p1,single_p2;
  CTask *task_focus=sys_focus_task;

  if (task_focus && !win_grab_scroll) {
    p1=ipx-task_focus->win_pixel_left-task_focus->win_scroll_x;
    p2=ipy-task_focus->win_pixel_top-task_focus->win_scroll_y;
    single_p1=single_ipx-task_focus->win_pixel_left-task_focus->win_scroll_x;
    single_p2=single_ipy-task_focus->win_pixel_top-task_focus->win_scroll_y;
    if (old_ipx_presnap!=ipx_presnap || old_ipy_presnap!=ipy_presnap) {
      if (que)
	QueMsg(task_focus,0,
	  MSG_IP_MOVE,p1,p2,0);
      msg_code=MSG_IP_MOVE;
    }
    //TODO que msg for ipz?
    if (left_ip_dbl_time) {
      if (time>left_ip_dbl_time) {
	if (left_dbl) {
	  if (!left_down_sent) {
	    if (que)
	      QueMsg(task_focus,0,
		MSG_IP_L_D_DOWN,p1,p2,0);
	    left_down_sent=TRUE;
	    msg_code=MSG_IP_L_D_DOWN;
	  }
	  if (!ip_lb) {
	    if (que)
	      QueMsg(task_focus,0,
		MSG_IP_L_D_UP,p1,p2,0);
	    left_ip_dbl_time=0;
	    msg_code=MSG_IP_L_D_UP;
	  }
	} else {
	  if (!left_down_sent) {
	    if (que)
	      QueMsg(task_focus,0,
		MSG_IP_L_DOWN,single_p1,single_p2,0);
	    left_down_sent=TRUE;
	    msg_code=MSG_IP_L_DOWN;
	  }
	  if (!ip_lb) {
	    if (que)
	      QueMsg(task_focus,0,
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
      if (WinInhibit(WIf_DBL_CLICK)) {
	if (ip_lb  && !ip_lb_last) {
	  if (que)
	    QueMsg(task_focus,0,
	      MSG_IP_L_DOWN,p1,p2,0);
	  msg_code=MSG_IP_L_DOWN;
	} else if (!ip_lb && ip_lb_last) {
	  if (que)
	    QueMsg(task_focus,0,
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
	      QueMsg(task_focus,0,
		MSG_IP_R_D_DOWN,p1,p2,0);
	    right_down_sent=TRUE;
	    msg_code=MSG_IP_R_D_DOWN;
	  }
	  if (!ip_rb) {
	    if (que)
	      QueMsg(task_focus,0,
		MSG_IP_R_D_UP,p1,p2,0);
	    right_ip_dbl_time=0;
	    msg_code=MSG_IP_R_D_UP;
	  }
	} else {
	  if (!right_down_sent) {
	    if (que)
	      QueMsg(task_focus,0,
		MSG_IP_R_DOWN,single_p1,single_p2,0);
	    right_down_sent=TRUE;
	    msg_code=MSG_IP_R_DOWN;
	  }
	  if (!ip_rb) {
	    if (que)
	      QueMsg(task_focus,0,
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
      if (WinInhibit(WIf_DBL_CLICK)) {
	if (ip_rb  && !ip_rb_last) {
	  if (que)
	    QueMsg(task_focus,0,
	      MSG_IP_R_DOWN,p1,p2,0);
	  msg_code=MSG_IP_R_DOWN;
	} else if (!ip_rb && ip_rb_last) {
	  if (que)
	    QueMsg(task_focus,0,
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

CTask *WinRefocus()
{
  CTask *task;
  Cli;
  PushFD;
  task=sys_focus_task;
  if (!task) {
    task=sys_winmgr_task->last_task;
    while (TaskValidate(task) && task!=sys_winmgr_task) {
      if (Bt(&task->task_flags,TASKf_USER)) {
	sys_focus_task=task;
	break;
      }
      task=task->last_task;
    }
  }
  PopFD;
  return sys_focus_task;
}

CTask *old_sys_focus_task=NULL;

I64 WinMgrSleep(Bool flush_msgs=FALSE)
{
  static Bool was_forced=FALSE;
  I64 timeout_val,timeout_val2,msg_code=0;
  CCtrl *c;
  Bool que;

  TimeStampFreqCal;

  if (!flush_msgs && old_sys_focus_task!=sys_focus_task) {
    QueMsg(old_sys_focus_task,0,
	  MSG_FOCUS,FALSE,0,0);
    QueMsg(sys_focus_task,0,
	  MSG_FOCUS,TRUE,0,0);
    old_sys_focus_task=sys_focus_task;
  }

  if (flush_msgs)
    FifoI64Flush(scan_code_fifo);
  else if (TaskValidate(sys_focus_task)) {
    KbdQueMsgs;

    que=TRUE;
    if (!WinInhibit(WIf_CTRLS)) {
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

  timeout_val=WinCalcRefreshTimeout(was_forced);
  WinIPUpdate;
  LBts(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);
  last_refresh_start_time=tNP;
  GrUpdateScreen;
  last_refresh_end_time=tNP;
  LBtr(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0);

  if (sys_focus_task) {
    if (!Bt(&sys_focus_task->task_flags,TASKf_HAS_SONG)) {
      Free(cur_song);
      cur_song=NULL;
    }
  }
  if (cur_song) {
    if (!cur_song_task)
      cur_song_task=Spawn(&CurSongTask,NULL,"Song");
  } else {
    if (cur_song_task) {
      Kill(cur_song_task);
      cur_song_task=NULL;
    }
  }
  win_updates++;

  if (!mouse_install_attempts)
    SleepUntil(timeout_val);
  else {
    do {
      KbdMouseHandler(FALSE,TRUE);
      timeout_val2=sys_jiffies+JIFFY_FREQ/500;
      if (timeout_val2>timeout_val)
	timeout_val2=timeout_val;
      SleepUntil(timeout_val2);
    } while (!Bt(&sys_semas[SYS_SEMA_FORCE_WINMGR],0)&&sys_jiffies<timeout_val);
  }
  was_forced=LBtr(&sys_semas[SYS_SEMA_FORCE_WINMGR],0);
  return msg_code;
}

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
  CDoc *old_doc=DocCur;
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
I64 WinOnTopWindows()
{
  CTask *task,*task1,*first_moved_fwd=NULL;
  I64 result=0;
  PushFD;
  Cli; //TODO Multiprocessor safe
  task=sys_winmgr_task->next_task;
  while (task!=sys_winmgr_task && task!=first_moved_fwd) {
    task1=task->next_task;
    if (!TaskValidate(task)) {
      PopFD;
      return result;
    }
    if (Bt(&task->display_flags,DISPLAYf_WIN_ON_TOP) &&
	task!=sys_winmgr_task->last_task) {
      TaskQueRem(task);
      TaskQueIns(task,sys_winmgr_task);
      result++;
      if (!first_moved_fwd)
	first_moved_fwd=task;
    }
    task=task1;
  }
  PopFD;
  return result;
}

public I64 WinToTop(CTask *task=NULL,Bool update_z_buf=TRUE)
{
  CTask *task1;
  I64 result=0;
  if (!task) task=Fs;
  if (!TaskValidate(task) || task->gs->num)
    return 0;
  TaskDerivedValsUpdate(task,FALSE);
  if (!sys_winmgr_task || task==sys_winmgr_task)
    return 0;
  PushFD;
  Cli;
  if (!TaskValidate(task)) {
    PopFD;
    return 0;
  }
  if (task!=sys_winmgr_task->last_task) {
    TaskQueRem(task);
    TaskQueIns(task,sys_winmgr_task);
    result++;
  }
  if (Bt(&task->task_flags,TASKf_USER))
    sys_focus_task=task;
  if (result && !Bt(&task->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP)) {
    task1=task->next_child_task;
    while (task1!=&task->next_child_task) {
      if (!TaskValidate(task1))
	break;
      result+=WinToTop(task1,FALSE);
      task1=task1->next_sibling_task;
    }
    if (task->popup_task &&
      task->popup_task->parent_task==task)
      result+=WinToTop(task->popup_task,FALSE);
  }
  PopFD;
  result+=WinOnTopWindows;
  if (result && update_z_buf)
    WinZBufUpdate;
  return result;
}
ext[EXT_WIN_TO_TOP]=&WinToTop;

public U0 WinFocus(CTask *task=NULL)
{
  Cli;
  PushFD;
  if (!TaskValidate(task)||!Bt(&task->task_flags,TASKf_USER))
    task=WinRefocus;
  WinToTop(sys_focus_task=task);
  PopFD;
}

public Bool SetWinHorz(I64 left,I64 right,CTask *task=NULL)
{
  I64 d=right-left;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  if (d<0) d=0;
  if (left>=TEXT_COLS) {
    left=TEXT_COLS-1;
    right=left+d;
  }
  if (right<0) {
    right=0;
    left=right-d;
  }
  if (left>right) {
    if (left>0)
      right=left;
    else
      left=right;
  }
  PushFD;
  Cli; //TODO Multiprocessor safe
  if (task->win_left!=left || task->win_right!=right) {
    LBts(&task->display_flags,DISPLAYf_HAS_BEEN_RESIZED);
    LBts(&task->display_flags,DISPLAYf_USER_HAS_BEEN_RESIZED);
    task->win_left=left;
    task->win_right=right;
    TaskDerivedValsUpdate(task);
    PopFD;
    return TRUE;
  } else {
    PopFD;
    return FALSE;
  }
}

public Bool SetWinVert(I64 top,I64 bottom,CTask *task=NULL)
{
  I64 d=bottom-top;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  if (d<0) d=0;
  if (top>=TEXT_ROWS) {
    top=TEXT_ROWS-1;
    bottom=top+d;
  }
  if (bottom<=0) {
    bottom=1;
    top=bottom-d;
  }
  if (top>bottom) {
    if (top>=0)
      bottom=top;
    else
      top=bottom;
  }
  PushFD;
  Cli; //TODO Multiprocessor safe
  if (task->win_top!=top || task->win_bottom!=bottom) {
    LBts(&task->display_flags,DISPLAYf_HAS_BEEN_RESIZED);
    LBts(&task->display_flags,DISPLAYf_USER_HAS_BEEN_RESIZED);
    task->win_top=top;
    task->win_bottom=bottom;
    TaskDerivedValsUpdate(task);
    PopFD;
    return TRUE;
  } else {
    PopFD;
    return FALSE;
  }
}

Bool SetCursorPos(CTask *task,I64 iptx,I64 ipty,Bool set_cursor=TRUE)
{
  CDoc *doc;
  Bool unlock;
  I64 x0,y0;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  if (task->win_top<=ipty<=task->win_bottom &&
    task->win_left <=iptx<=task->win_right) {
    doc=task->next_doc;
    while (doc!=(&task->next_doc)(U8 *)-offset(CDoc.next_doc) &&
	   doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
      if (set_cursor) {
	unlock=DocLock(doc);
	if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
	  if (doc->flags&DOCF_AUX_DOC) {
	    doc->x=iptx-task->win_left;
	    doc->y=ipty-task->win_top;
	    DocRecalc(doc,FALSE,DOC_RECALC_FIND_CURSOR);
	  } else {
	    x0=doc->line_start_col;
	    y0=doc->cur_top_line_num;
	    DocRecalc(doc,TRUE);

	    doc->x=iptx-task->win_left-task->win_scroll_x/FONT_WIDTH +x0;
	    doc->y=ipty-task->win_top -task->win_scroll_y/FONT_HEIGHT+y0;
	    DocRecalc(doc,FALSE,DOC_RECALC_FIND_CURSOR);
	    task->win_scroll_x=0;
	    task->win_scroll_y=0;
	    task->win_scroll_z=0;
	  }
	  if (!(doc->flags&DOCF_DONT_SHOW) &&
	    (!doc->best_d || !(doc->flags&DOCF_AUX_DOC))) {
	    if (unlock)
	      DocUnlock(doc);
	    break;
	  }
	  if (unlock)
	    DocUnlock(doc);
	} else {
	  if (unlock)
	    DocUnlock(doc);
	  break;
	}
      }
      doc=doc->next_doc;
    }
    WinToTop(task);
    return TRUE;
  }
  return FALSE;
}

public U0 WinTileHorz()
{
  CTask *task,*last_task=Fs;
  I64 cnt,c,i,vert_size,no_border;

  PushFD;
  Cli; //TODO Multiprocessor safe
  task=sys_winmgr_task;
  cnt=0;
  do {
    if (Bt(&task->task_flags,TASKf_USER))
      cnt++;
    task=task->last_task;
  } while (task!=sys_winmgr_task);


  task=sys_winmgr_task;
  i=0;
  do {
    if (Bt(&task->task_flags,TASKf_USER)) {
      no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
      c=cnt- i&~3;
      if (!c)
	c=1;
      else if (c>4)
	c=4;
      vert_size=(TEXT_ROWS-1)/c;

      task->win_right=TEXT_COLS-2+no_border;
      task->win_left=1-no_border;
      task->win_top=(i&3)*vert_size+2-no_border;
      task->win_bottom=(i&3+1)*vert_size+no_border;
      last_task=task;
      if (i&3==3)
	task->win_bottom=TEXT_ROWS-2;
      i++;
      TaskDerivedValsUpdate(task);
    }
    task=task->last_task;
  } while (task!=sys_winmgr_task);
  last_task->win_bottom=TEXT_ROWS-2;
  PopFD;
}

public U0 WinTileVert()
{
  CTask *task,*last_task=Fs;
  I64 cnt,c,i,horz_size,no_border;
  PushFD;
  Cli; //TODO Multiprocessor safe
  task=sys_winmgr_task;
  cnt=0;
  do {
    if (Bt(&task->task_flags,TASKf_USER))
      cnt++;
    task=task->last_task;
  } while (task!=sys_winmgr_task);

  task=sys_winmgr_task;
  i=0;
  do {
    if (Bt(&task->task_flags,TASKf_USER)) {
      no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
      c=cnt- i&~3;
      if (!c)
	c=1;
      else if (c>4)
	c=4;
      horz_size=TEXT_COLS/c;

      task->win_top=2-no_border;
      task->win_bottom=TEXT_ROWS-2+no_border;
      task->win_left=(i&3)*horz_size+1-no_border;
      task->win_right=(i&3+1)*horz_size-1+no_border;
      last_task=task;
      if (i&3==3)
	task->win_right=TEXT_COLS-2;
      i++;
      TaskDerivedValsUpdate(task);
    }
    task=task->last_task;
  } while (task!=sys_winmgr_task);
  last_task->win_right=TEXT_COLS-2;
  PopFD;
}


public U0 WinMax(CTask *task=NULL)
{
  I64 no_border;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  PushFD;
  Cli; //TODO Multiprocessor safe
  no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  task->win_right=TEXT_COLS-2+no_border;
  task->win_left=1-no_border;
  task->win_top=2-no_border;
  task->win_bottom=TEXT_ROWS-2+no_border;
  TaskDerivedValsUpdate(task);
  WinToTop(task);
  PopFD;
}

public Bool WinBorder(Bool val=OFF,CTask *task=NULL)
{
  Bool old_has_border;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  PushFD;
  Cli; //TODO Multiprocessor safe
  old_has_border=!Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  if (val) {
    if (!old_has_border) {
      LBtr(&task->display_flags,DISPLAYf_NO_BORDER);
      task->win_right--;
      task->win_left++;
      task->win_top++;
      task->win_bottom--;
      TaskDerivedValsUpdate(task,FALSE);
    }
  } else {
    if (old_has_border) {
      LBts(&task->display_flags,DISPLAYf_NO_BORDER);
      task->win_right++;
      task->win_left--;
      task->win_top--;
      task->win_bottom++;
      TaskDerivedValsUpdate(task,FALSE);
    }
  }
  PopFD;
  return old_has_border;
}

U0 WinChkMoveSize(CTask *task)
{
  if (task->win_left!=task->win_old_left ||
      task->win_top!=task->win_old_top)
    QueMsg(task,0,
	MSG_MOVE,task->win_left,task->win_top,0);
  if (task->win_right-task->win_left!=task->win_old_right-task->win_old_left ||
      task->win_bottom-task->win_top!=task->win_old_bottom-task->win_old_top)
    QueMsg(task,0,
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
  if (Bt(key_down_bitmap,SC_GUI) &&
      sys_focus_task &&
      (m=sys_focus_task->cur_menu)) {
    win_show_menu=TRUE;
    sys_cur_submenu_entry=NULL;
    old_ipx=ipx; old_ipy=ipy;
    ipx=new_ipx=ipy=new_ipy=0;
    while (m->mu_signature==MENU_SIGNATURE_VAL &&
	Bt(key_down_bitmap,SC_GUI)) {
      old_key_cnt=sys_key_cnt;
      if (Bt(key_down_bitmap,SC_CURSOR_LEFT)) {
	while (Bt(key_down_bitmap,SC_CURSOR_LEFT)
	  && sys_key_cnt==old_key_cnt)
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
	while (Bt(key_down_bitmap,SC_CURSOR_RIGHT)
	  && sys_key_cnt==old_key_cnt)
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
	while (Bt(key_down_bitmap,SC_CURSOR_UP)
	  && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_ipy-=FONT_HEIGHT;
      } else if (Bt(key_down_bitmap,SC_CURSOR_DOWN)) {
	while (Bt(key_down_bitmap,SC_CURSOR_DOWN)
	  && sys_key_cnt==old_key_cnt)
	  WinMgrSleep(TRUE);
	new_ipy+=FONT_HEIGHT;
      }
      new_ipx=LimitI64(new_ipx,0,GR_WIDTH-1);
      new_ipy=LimitI64(new_ipy,0,GR_HEIGHT-1);
      ipx=new_ipx; ipy=new_ipy;
      WinMgrSleep(TRUE);
      if (!sys_cur_submenu_entry)
	ipy=new_ipy=0;
    }
    if (sys_cur_submenu_entry) {
      QueMsg(sys_focus_task,0,
	  sys_cur_submenu_entry->msg_code,
	  sys_cur_submenu_entry->p1,
	  sys_cur_submenu_entry->p2,0);
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
      old_flags=GetFlags;
  Bool has_border;
  CCtrl *c;
  TaskInitExt;
  last_total_jiffies=0;
  MemSet(last_idle_pt_hits,0,sizeof(I64)*MP_MAX_PROCESSORS);
  MemSet(last_swap_cnter,0,sizeof(I64)*MP_MAX_PROCESSORS);

  Fs->win_left=0;
  Fs->win_right=TEXT_COLS-1;
  Fs->win_top=0;
  Fs->win_bottom=TEXT_ROWS-1;
  TaskDerivedValsUpdate;
  LBts(&Fs->display_flags,DISPLAYf_NO_BORDER);
  LBts(&Fs->display_flags,DISPLAYf_SHOW);
  GrInit;
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
	Cli;
	if (!TaskValidate(task)) {
	  SetFlags(old_flags);
	  goto wmt_start;
	}
	WinChkMoveSize(task);
	TaskDerivedValsUpdate(task,FALSE);
	task=task->last_task;
	SetFlags(old_flags);
	if (task==Fs)
	  break;
      }

      task=Fs->last_task;
      while (TRUE) {
	Cli;
	if (!TaskValidate(task)) {
	  SetFlags(old_flags);
	  goto wmt_start;
	}
	if (Bt(&task->display_flags,DISPLAYf_SHOW) &&
	    task->win_border_pixel_left<=ipx<=task->win_border_pixel_right &&
	    task->win_border_pixel_top<=ipy<=task->win_border_pixel_bottom) {
	  SetFlags(old_flags);
	  break;
	}
	if (task==Fs) {	//Shouldn't happen
	  SetFlags(old_flags);
	  goto wmt_start;
	}
	task=task->last_task;
	SetFlags(old_flags);
      }

      if (Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	has_border=FALSE;
      else
	has_border=TRUE;

      win_show_menu=FALSE;
      sys_cur_submenu_entry=NULL;
      if (!WinInhibit(WIf_MENU)) {
	if (WinKeyNavMenu)
	  goto wmt_start;
	if (task==Fs && 0<=ipy<FONT_HEIGHT && ip_dev!=IP_NULL) {
	  win_show_menu=TRUE;
	  if (ip_lb && !old_ip_lb) {
	    win_show_menu=TRUE;
	    while (ip_lb)
	      WinMgrSleep(TRUE);
	    if (sys_cur_submenu_entry) {
	      QueMsg(sys_focus_task,0,
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
      if (!WinInhibit(WIf_GRAB_SCROLL) &&
	sys_cur_scan_code&SCF_CTRL && TaskValidate(sys_focus_task)) {
	win_grab_scroll_closed=FALSE;
	win_grab_scroll=TRUE;
	while (sys_cur_scan_code&SCF_CTRL && TaskValidate(sys_focus_task)) {
	  if (ip_lb) {
	    win_grab_scroll_closed=TRUE;
	    x=ipx-sys_focus_task->win_scroll_x;
	    y=ipy-sys_focus_task->win_scroll_y;
	    z=ipz-sys_focus_task->win_scroll_z;
	    while (ip_lb && sys_cur_scan_code&SCF_CTRL && TaskValidate(sys_focus_task)) {
	      sys_focus_task->win_scroll_x=(ipx-x)&~7;
	      sys_focus_task->win_scroll_y=(ipy-y)&~7;
	      sys_focus_task->win_scroll_z=ipz-z;
	      WinMgrSleep(TRUE);
	    }
	    win_grab_scroll_closed=FALSE;
	    goto wmt_start;
	  } else if (ip_rb)  {
	    sys_focus_task->win_scroll_x=0;
	    sys_focus_task->win_scroll_y=0;
	    sys_focus_task->win_scroll_z=0;
	    WinMgrSleep(TRUE);
	  } else
	    WinMgrSleep;
	}
	goto wmt_start;
      } else
	win_grab_scroll=FALSE;

      if (!WinInhibit(WIf_CTRLS)) {
	if (ip_lb && !old_ip_lb) {
	  c=task->next_ctrl;
	  while (c!=&task->next_ctrl) {
	    if (CtrlInside(c,ipx,ipy)) {
	      left=task->win_pixel_left;
	      top =task->win_pixel_top;
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
	      left=task->win_pixel_left;
	      top =task->win_pixel_top;
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
	      left=task->win_pixel_left;
	      top =task->win_pixel_top;
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
	      left=task->win_pixel_left;
	      top =task->win_pixel_top;
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

      if (!WinInhibit(WIf_IP_L)) {
	if (!old_ip_lb && ip_lb) {
	  if (SetCursorPos(task,iptx,ipty)) {
	    doc=task->next_doc;
	    while (doc!=(&task->next_doc)(U8 *)-offset(CDoc.next_doc) &&
		doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
	      DocLock(doc);
	      if (!(doc->flags&DOCF_DONT_SHOW) &&
		  (!doc->best_d || !(doc->flags&DOCF_AUX_DOC))) {
		if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
		  doc_e=doc->cur_entry;
		  if (doc_e!=doc) {
		    if (doc_e->flags1 & DOCEF1_HAS_BORDER)
		      doc_e->flags1|=DOCEF1_SOLID_BORDER;
		  }
		}
		DocUnlock(doc);
		break;
	      }
	      DocUnlock(doc);
	      doc=doc->next_doc;
	    }
	    old_ip_lb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!WinInhibit(WIf_IP_R)) {
	if (!old_ip_rb && ip_rb) {
	  if (SetCursorPos(task,iptx,ipty)) {
	    old_ip_rb=TRUE;
	    goto wmt_start;
	  }
	}
      }
      if (!WinInhibit(WIf_BORDER) && has_border) {
	if (old_ip_lb && !ip_lb) {
	  if (ipty==task->win_top-1) {
	    if (task->win_left<=iptx<task->win_left+4) {
	      QueMsg(task,0,MSG_KEY_DOWN,CH_CTRLM,0x43200000432,0);
	      old_ip_lb=FALSE;
	      goto wmt_start;
	    } else if (task->win_right-2<=iptx<=task->win_right) {
	      if (Bt(&task->display_flags,DISPLAYf_HAS_CLOSE_WIN)) {
		if (DocCur(task))
		  QueMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
		else
		  Kill(task,FALSE);
		old_ip_lb=FALSE;
		goto wmt_start;
	      }
	    }
	  }
	}
      }
      if (!WinInhibit(WIf_IP_L)) {
	if (old_ip_lb && !ip_lb) {
	  if (SetCursorPos(task,iptx,ipty,FALSE)) {
	    doc=task->next_doc;
	    while (doc!=(&task->next_doc)(U8 *)-offset(CDoc.next_doc) &&
		doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
	      if (!(doc->flags&DOCF_DONT_SHOW) &&
		  (!doc->best_d || !(doc->flags&DOCF_AUX_DOC))) {
		do msg_code=WinMgrSleep;
		while (TaskValidate(task) &&
		    (ip_lb || left_ip_dbl_time));
		if (TaskValidate(task)) {
		  if (msg_code==MSG_IP_L_UP) {
		    if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
		      DocLock(doc);
		      if (TaskValidate(task)) {
			if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
			  doc_e=doc->cur_entry;
			  if (doc_e!=doc) {
			    if (doc_e->flags1 & DOCEF1_HAS_BORDER)
			      doc_e->flags1&=~DOCEF1_SOLID_BORDER;
			    if (doc_e->flags1 & (DOCEF1_TREE|DOCEF1_LINK|DOCEF1_CHECK_COLLAPSABLE|
			    DOCEF1_LEFT_CB|DOCEF1_LEFT_MACRO|DOCEF1_LEFT_EXP))
			      QueMsg(task,0,MSG_KEY_DOWN,CH_SPACE,0,0);
			  }
			}
			DocUnlock(doc);
		      }
		    }
		  } else if (msg_code==MSG_IP_L_D_UP)
		    QueMsg(task,0,MSG_KEY_DOWN,CH_ESC,0,0);
		}
		break;
	      }
	      doc=doc->next_doc;
	    }
	    old_ip_lb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!WinInhibit(WIf_IP_R)) {
	if (old_ip_rb && !ip_rb) {
	  if (SetCursorPos(task,iptx,ipty,FALSE)) {
	    doc=task->next_doc;
	    while (doc!=(&task->next_doc)(U8 *)-offset(CDoc.next_doc) &&
		doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
	      if (!(doc->flags&DOCF_DONT_SHOW) &&
		  (!doc->best_d || !(doc->flags&DOCF_AUX_DOC))) {
		do msg_code=WinMgrSleep;
		while (TaskValidate(task) &&
		    (ip_rb || right_ip_dbl_time));
		if (TaskValidate(task)) {
		  if (msg_code==MSG_IP_R_UP) {
		    if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
		      DocLock(doc);
		      if (TaskValidate(task)) {
			if (doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
			  doc_e=doc->cur_entry;
			  if (doc_e!=doc) {
			    if (doc_e->flags1&(DOCEF1_LINK|
			    DOCEF1_RIGHT_CB|DOCEF1_RIGHT_MACRO|DOCEF1_RIGHT_EXP))
			      QueMsg(task,0,MSG_KEY_DOWN,CH_NEW_LINE,0,0);
			  }
			}
			DocUnlock(doc);
		      }
		    }
		  } else if (msg_code==MSG_IP_R_D_UP)
		    QueMsg(task,0,MSG_KEY_DOWN,CH_SHIFT_ESC,0,0);
		}
		break;
	      }
	      doc=doc->next_doc;
	    }
	    old_ip_rb=FALSE;
	    goto wmt_start;
	  }
	}
      }

      if (!WinInhibit(WIf_BORDER) && has_border) {
	if (ip_lb && !old_ip_lb) {
	  if (task->win_top==ipty+1 &&
	      task->win_left-1<=iptx<=task->win_right+1) {
	    if (task->win_left<=iptx<task->win_left+4) {
	      old_ip_lb=TRUE;
	      goto wmt_start;
	    }
	    if (task->win_right-2<=iptx<=task->win_right &&
		Bt(&task->display_flags,DISPLAYf_HAS_CLOSE_WIN)) {
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
