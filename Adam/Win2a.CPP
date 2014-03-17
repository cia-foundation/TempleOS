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

#define PROGRESS_BAR_HEIGHT	20
#define PROGRESS_BAR_WIDTH	(3*GR_WIDTH/4)

U0 DrawProgressBars(CDC *dc)
{
  I64 i,j,k,n,m;
  U8 *st;
  for (i=0;i<NUM_PROGRESS_BARS;i++) {
    if (m=sys_progresses[i].max) {
      dc->color=BLACK;
      GrRect(dc,
	(GR_WIDTH-PROGRESS_BAR_WIDTH)/2,
	(GR_HEIGHT-(NUM_PROGRESS_BARS*2-1-i*4)*PROGRESS_BAR_HEIGHT)/2,
	PROGRESS_BAR_WIDTH,PROGRESS_BAR_HEIGHT);

      dc->color=LTGREEN;
      n=sys_progresses[i].val;
      if (n>m)
	n=m;
      GrRect(dc,
	(GR_WIDTH-PROGRESS_BAR_WIDTH)/2+2,
	(GR_HEIGHT-(NUM_PROGRESS_BARS*2-1-i*4)*PROGRESS_BAR_HEIGHT)/2+2,
	n*(PROGRESS_BAR_WIDTH-4)/m,
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
      if (*sys_progresses[i].desc)
	st=StrNew(sys_progresses[i].desc);
      else
	st=MPrint("%d/%d",n,m);
      GrPrint(dc,(GR_WIDTH-FONT_WIDTH*StrLen(st))/2,
	(GR_HEIGHT-FONT_HEIGHT-
	(NUM_PROGRESS_BARS*2-2-i*4)*PROGRESS_BAR_HEIGHT)/2,"%s",st);
      Free(st);
    }
  }
}

U0 DrawWinGrid(CDC *dc)
{
  F64 d;
  dc->color=BLACK;
  dc->pen_width=1;
  for (d=ip_grid.x_offset;d<GR_WIDTH; d+=ip_grid.x)
    GrLine(dc,d,0,d,GR_HEIGHT-1);
  for (d=ip_grid.y_offset;d<GR_HEIGHT;d+=ip_grid.y)
    GrLine(dc,0,d,GR_WIDTH-1,d);
}

U0 WinGrid(Bool val)
{
  CGrid last_grid;
  MemCpy(&last_grid,&ip_grid,sizeof(CGrid));
  if (!val || PopUpForm(&ip_grid)) {
    if (!val)
      GridInit;
    mxx_prescale*=last_grid.x_speed/ip_grid.x_speed;
    myy_prescale*=last_grid.y_speed/ip_grid.y_speed;
    mzz_prescale*=last_grid.z_speed/ip_grid.z_speed;
  } else
    MemCpy(&ip_grid,&last_grid,sizeof(CGrid));
}
U0 CtrlAltG(I64 sc)
{
  if (sc&SCF_SHIFT)
    PopUp("WinGrid(FALSE);");
  else
    PopUp("WinGrid(TRUE);");
}
SetCtrlAltLetCB('G',&CtrlAltG,"Sys/Grid On or Off");

CTask *ext_ASCII_task;
U0 ExtendedASCII()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"Select Char and Press <ESC>\n$$LTBLUE$$");
  for (i=0;i<256;i++) {
    if (i>=CH_SHIFT_SPACE && i!=127) {
      if (i==CH_SHIFT_SPACE)
	DocPrint(doc,"$$MU-UL,\"\\37\",LE=%d$$",i);
      else if (i=='$$')
	DocPrint(doc,"$$MU-UL,\"\\44\",LE=%d$$",i);
      else if (i=='\"'||i=='\\')
	DocPrint(doc,"$$MU-UL,\"\\%c\",LE=%d$$",i,i);
      else
	DocPrint(doc,"$$MU-UL,\"%c\",LE=%d$$",i,i);
    } else
      DocPrint(doc," ");
    if (i&15==15)
      DocPrint(doc,"\n");
  }
  i=PopUpMenu(doc);
  DocDel(doc);
  if (i>=0)
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
  s->x=task->scroll_x;
  s->y=task->scroll_y;
  s->z=task->scroll_z;
  task->scroll_x=0;
  task->scroll_y=0;
  task->scroll_z=0;
}

public U0 WinScrollRestore(CTask *task,CD3I64 *s)
{
  task->scroll_x=s->x;
  task->scroll_y=s->y;
  task->scroll_z=s->z;
}

U0 DrawInputPtr(CDC *dc)
{
  I64 x,y;
  PUSHFD
  CLI
  x=ipx;
  y=ipy;
  POPFD
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
  if (ip_grid.show)
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
  Bool set=FALSE;
  if (ip_dev==IP_MOUSE) {
    ip_has_wheel=mouse_has_wheel;
    if (mouse_evt) {
      ip_throttle+=ip_throttle_step*SignI64(mzz-ipz);
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
    }
  } else
    ip_speed*=0.95;
  if (gr_screen_zoom!=1) {
    if (gr_continuous_scroll)
      GrScaleZoom(1.0);
    else {
      dd=(ipx-grsx)*gr_screen_zoom;
      if (!(8<=dd<GR_WIDTH-8))
	GrScaleZoom(1.0);
      else {
	dd=(ipy-grsy)*gr_screen_zoom;
	if (!(8<=dd<GR_HEIGHT-8))
	  GrScaleZoom(1.0);
      }
    }
  }
}

CTask *WinRefocus()
{
  CTask *task;
  CLI
  PUSHFD
  task=sys_focus_task;
  if (!task) {
    task=sys_winmgr_task->last_task;
    while (TaskValidate(task) && task!=sys_winmgr_task) {
      if (!Bt(&task->win_inhibit,WIf_FOCUS)) {
	sys_focus_task=task;
	break;
      }
      task=task->last_task;
    }
  }
  POPFD
  return sys_focus_task;
}

I64 WinOnTopWindows()
{
  CTask *task,*task1,*first_moved_fwd=NULL;
  I64 result=0;
  PUSHFD
  CLI //TODO Multiprocessor safe
  task=sys_winmgr_task->next_task;
  while (task!=sys_winmgr_task && task!=first_moved_fwd) {
    task1=task->next_task;
    if (!TaskValidate(task)) {
      POPFD
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
  POPFD
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
  PUSHFD
  CLI
  if (!TaskValidate(task)) {
    POPFD
    return 0;
  }
  if (task!=sys_winmgr_task->last_task) {
    TaskQueRem(task);
    TaskQueIns(task,sys_winmgr_task);
    result++;
  }
  if (!Bt(&task->win_inhibit,WIf_FOCUS))
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
  POPFD
  result+=WinOnTopWindows;
  if (result && update_z_buf)
    WinZBufUpdate;
  return result;
}
ext[EXT_WIN_TO_TOP]=&WinToTop;

public U0 WinFocus(CTask *task=NULL)
{
  CLI
  PUSHFD
  if (!TaskValidate(task)||Bt(&task->win_inhibit,WIf_FOCUS))
    task=WinRefocus;
  WinToTop(sys_focus_task=task);
  POPFD
}
ext[EXT_WIN_FOCUS]=&WinFocus;

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
  PUSHFD
  CLI //TODO Multiprocessor safe
  if (task->win_left!=left || task->win_right!=right) {
    task->win_left=left;
    task->win_right=right;
    TaskDerivedValsUpdate(task);
    POPFD
    return TRUE;
  } else {
    POPFD
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
  PUSHFD
  CLI //TODO Multiprocessor safe
  if (task->win_top!=top || task->win_bottom!=bottom) {
    task->win_top=top;
    task->win_bottom=bottom;
    TaskDerivedValsUpdate(task);
    POPFD
    return TRUE;
  } else {
    POPFD
    return FALSE;
  }
}

public U0 WinTileHorz()
{
  CTask *task,*last_task=Fs;
  I64 cnt,c,i,vert_size,no_border;

  PUSHFD
  CLI //TODO Multiprocessor safe
  task=sys_winmgr_task;
  cnt=0;
  do {
    if (!Bt(&task->win_inhibit,WIf_FOCUS))
      cnt++;
    task=task->last_task;
  } while (task!=sys_winmgr_task);


  task=sys_winmgr_task;
  i=0;
  do {
    if (!Bt(&task->win_inhibit,WIf_FOCUS)) {
      no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
      c=cnt- i&~3;
      if (!c)
	c=1;
      else if (c>4)
	c=4;
      vert_size=(TEXT_ROWS-1)/c;

      SetWinHorz(1-no_border,TEXT_COLS-2+no_border,task);
      SetWinVert((i&3)*vert_size+2-no_border,(i&3+1)*vert_size+no_border,task);
      last_task=task;
      if (i&3==3)
	SetWinVert(task->win_top,TEXT_ROWS-2,task);
      i++;
    }
    task=task->last_task;
  } while (task!=sys_winmgr_task);
  SetWinVert(last_task->win_top,TEXT_ROWS-2,last_task);
  POPFD
}

public U0 WinTileVert()
{
  CTask *task,*last_task=Fs;
  I64 cnt,c,i,horz_size,no_border;
  PUSHFD
  CLI //TODO Multiprocessor safe
  task=sys_winmgr_task;
  cnt=0;
  do {
    if (!Bt(&task->win_inhibit,WIf_FOCUS))
      cnt++;
    task=task->last_task;
  } while (task!=sys_winmgr_task);

  task=sys_winmgr_task;
  i=0;
  do {
    if (!Bt(&task->win_inhibit,WIf_FOCUS)) {
      no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
      c=cnt- i&~3;
      if (!c)
	c=1;
      else if (c>4)
	c=4;
      horz_size=TEXT_COLS/c;
      SetWinHorz((i&3)*horz_size+1-no_border,(i&3+1)*horz_size-1+no_border,task);
      SetWinVert(2-no_border,TEXT_ROWS-2+no_border,task);
      last_task=task;
      if (i&3==3)
	SetWinHorz(task->win_left,TEXT_COLS-2,task);
      i++;
    }
    task=task->last_task;
  } while (task!=sys_winmgr_task);
  SetWinHorz(last_task->win_left,TEXT_COLS-2,last_task);
  POPFD
}


public U0 WinMax(CTask *task=NULL)
{
  I64 no_border;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  PUSHFD
  CLI //TODO Multiprocessor safe
  no_border=Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  SetWinHorz(1-no_border,TEXT_COLS-2+no_border,task);
  SetWinVert(2-no_border,TEXT_ROWS-2+no_border,task);
  WinToTop(task);
  POPFD
}

public Bool WinBorder(Bool val=OFF,CTask *task=NULL)
{
  Bool old_has_border;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return FALSE;
  PUSHFD
  CLI //TODO Multiprocessor safe
  old_has_border=!Bt(&task->display_flags,DISPLAYf_NO_BORDER);
  if (val) {
    if (!old_has_border) {
      LBtr(&task->display_flags,DISPLAYf_NO_BORDER);
      task->win_left++; task->win_right--;
      task->win_top++;	task->win_bottom--;
      TaskDerivedValsUpdate(task,FALSE);
    }
  } else {
    if (old_has_border) {
      LBts(&task->display_flags,DISPLAYf_NO_BORDER);
      task->win_left--; task->win_right++;
      task->win_top--;	task->win_bottom++;
      TaskDerivedValsUpdate(task,FALSE);
    }
  }
  POPFD
  return old_has_border;
}

#help_index ""
