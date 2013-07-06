public CCtrl *CtrlFindUnique(CTask *task,I64 type)
{
  CCtrl *c;
  Bool old_preempt=Preempt(OFF);
  c=task->next_ctrl;
  while (c!=&task->next_ctrl) {
    if (c->type==type) {
      Preempt(old_preempt);
      return c;
    }
    c=c->next;
  }
  Preempt(old_preempt);
  return NULL;
}

U0 CtrlsUpdate(CTask *task)
{
  CCtrl *c;
  Bool old_preempt=Preempt(OFF);
  c=task->next_ctrl;
  while (c!=&task->next_ctrl) {
    if (c->update_derived_vals)
      (*c->update_derived_vals)(c);
    if (c->flags&CTRLF_BORDER) {
      c->screen_left  =task->win_pixel_left+c->left-FONT_WIDTH;
      c->screen_right =task->win_pixel_left+c->right-FONT_WIDTH;
      c->screen_top   =task->win_pixel_top+c->top-FONT_HEIGHT;
      c->screen_bottom=task->win_pixel_top+c->bottom-FONT_HEIGHT;
    } else {
      c->screen_left  =task->win_pixel_left+c->left;
      c->screen_right =task->win_pixel_left+c->right;
      c->screen_top   =task->win_pixel_top+c->top;
      c->screen_bottom=task->win_pixel_top+c->bottom;
    }
    c=c->next;
  }
  Preempt(old_preempt);
}

fp_update_ctrls=&CtrlsUpdate;


Bool CtrlInsideRect(CCtrl *c,I64 x,I64 y)
{//screen coordinates
  if (c->screen_left<=x<=c->screen_right &&
      c->screen_top<=y<=c->screen_bottom)
    return TRUE;
  else
    return FALSE;
}

public Bool CtrlInside(CCtrl *c,I64 x,I64 y)
{
  if (c->flags&CTRLF_SHOW) {
    if (c->inside_ctrl)
      return (*c->inside_ctrl)(c,x,y);
    else
      return CtrlInsideRect(c,x,y);
  } else
    return FALSE;
}

U0 DrawCtrls(CTask *task)
{
  CCtrl *c;
  CDC *dc=DCAlias(gr_dc2,task);
  Bool old_preempt=Preempt(OFF);
  c=task->next_ctrl;
  while (c!=&task->next_ctrl) {
    if (c->flags&CTRLF_SHOW) {
      if (c->flags&CTRLF_BORDER) {
	if (!Bt(&task->display_flags,DISPLAYf_NO_BORDER)) {
	  PushFD;
	  Cli;
	  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
	    Pause;

	  task->win_left--; //Allow drawing on border
	  task->win_right++;
	  task->win_top--;
	  task->win_bottom++;
	  WinDerivedValsUpdate(task);

	  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
	  PopFD;

	  if (c->draw_it)
	    (*c->draw_it)(dc,c);

	  PushFD;
	  Cli;
	  while (LBts(&task->locked_flags,TASKLf_TASK_LOCK))
	    Pause;

	  task->win_left++;
	  task->win_right--;
	  task->win_top++;
	  task->win_bottom--;
	  WinDerivedValsUpdate(task);

	  LBtr(&task->locked_flags,TASKLf_TASK_LOCK);
	  PopFD;
	}
      } else
	if (c->draw_it)
	  (*c->draw_it)(dc,c);
    }
    c=c->next;
  }
  Preempt(old_preempt);
  DCDel(dc);
}


#define WIN_SCROLL_SIZE		8
#define WIN_SCROLL_BORDER_BONUS	4
U0 DrawWinScroll(CDC *dc,CCtrl *c)
{
  CWinScroll *s=c->state;
  Bool old_preempt=Preempt(OFF);

  if (c->flags&CTRLF_CLICKED)
    dc->color=s->color>>4;
  else
    dc->color=s->color&0xF;
  GrRect(dc,c->left,c->top,c->right-c->left+1,c->bottom-c->top+1);

  if (c->flags&CTRLF_CLICKED)
    dc->color=s->color&0xF;
  else
    dc->color=s->color>>4;
  GrRect(dc,c->left+2,c->top+2,c->right-c->left+1-4,c->bottom-c->top+1-4);
  Preempt(old_preempt);
}

U0 WinDerivedScrollValsUpdate(CCtrl *c)
{
  CWinScroll *s=c->state;
  I64 range;
  if (s->max<s->min) s->max=s->min;
  if (s->pos<s->min) s->pos=s->min;
  if (s->pos>s->max) s->pos=s->max;
  s->color=c->win_task->border_attr&0xF^0xF+
	  (c->win_task->border_attr&0xF)<<4;
  range=s->max-s->min;
  if (!range) range=1;
  switch (c->type) {
    case CTRLT_WIN_HSCROLL:
      c->left=FONT_WIDTH-WIN_SCROLL_BORDER_BONUS+(s->pos-s->min)*(c->win_task->win_pixel_width+2*WIN_SCROLL_BORDER_BONUS-WIN_SCROLL_SIZE)/range;
      c->right=c->left+WIN_SCROLL_SIZE-1;
      c->top=FONT_HEIGHT+(FONT_WIDTH-WIN_SCROLL_SIZE)/2+c->win_task->win_pixel_height;
      c->bottom=c->top+WIN_SCROLL_SIZE-1;
      break;
    case CTRLT_WIN_VSCROLL:
      c->left=FONT_WIDTH+(FONT_WIDTH-WIN_SCROLL_SIZE)/2+c->win_task->win_pixel_width;
      c->right=c->left+WIN_SCROLL_SIZE-1;
      c->top=FONT_HEIGHT-WIN_SCROLL_BORDER_BONUS+(s->pos-s->min)*(c->win_task->win_pixel_height+2*WIN_SCROLL_BORDER_BONUS-WIN_SCROLL_SIZE)/range;
      c->bottom=c->top+WIN_SCROLL_SIZE-1;
      break;
  }
}

U0 LeftClickHWinScroll(CCtrl *c,I64 x,I64,Bool down)
{
  CTask *task=c->win_task;
  CWinScroll *s=c->state;
  I64 range=task->win_pixel_width+2*WIN_SCROLL_BORDER_BONUS-WIN_SCROLL_SIZE;
  LBts(&s->flags,WSSf_SET_TO_POS);
  s->pos=((x-(FONT_WIDTH-WIN_SCROLL_BORDER_BONUS))*(s->max-s->min+1)+range/2)/range+s->min;
  if (down)
    c->flags|=CTRLF_CLICKED;
  else
    c->flags&=~CTRLF_CLICKED;
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

U0 LeftClickVWinScroll(CCtrl *c,I64,I64 y,Bool down)
{
  CTask *task=c->win_task;
  CWinScroll *s=c->state;
  I64 range=task->win_pixel_height+2*WIN_SCROLL_BORDER_BONUS-WIN_SCROLL_SIZE;
  LBts(&s->flags,WSSf_SET_TO_POS);
  s->pos=((y-(FONT_HEIGHT-WIN_SCROLL_BORDER_BONUS))*(s->max-s->min+1)+range/2)/range+s->min;
  if (down)
    c->flags|=CTRLF_CLICKED;
  else
    c->flags&=~CTRLF_CLICKED;
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

U0 WheelChangeWinScroll(CCtrl *c,I64 delta)
{
  CWinScroll *s=c->state;
  LBts(&s->flags,WSSf_SET_TO_POS);
  s->pos+=delta;
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

U0 WinScrollsInit(CTask *task)
{
  Bool old_preempt=Preempt(OFF);
  CCtrl *c;

  if (!CtrlFindUnique(task,CTRLT_WIN_HSCROLL)) {
    c=CAlloc(sizeof(CCtrl));
    c->win_task=task;
    c->flags=CTRLF_SHOW|CTRLF_BORDER|CTRLF_CAPTURE_LEFT_IP;
    c->type=CTRLT_WIN_HSCROLL;
    c->state=&task->horz_scroll;
    c->update_derived_vals=&WinDerivedScrollValsUpdate;
    c->draw_it=&DrawWinScroll;
    c->left_click=&LeftClickHWinScroll;
    QueIns(c,task->last_ctrl);
  }

  if (!CtrlFindUnique(task,CTRLT_WIN_VSCROLL)) {
    c=CAlloc(sizeof(CCtrl));
    c->win_task=task;
    c->flags=CTRLF_SHOW|CTRLF_BORDER|CTRLF_CAPTURE_LEFT_IP;
    c->type=CTRLT_WIN_VSCROLL;
    c->state=&task->vert_scroll;
    c->update_derived_vals=&WinDerivedScrollValsUpdate;
    c->draw_it=&DrawWinScroll;
    c->left_click=&LeftClickVWinScroll;
    c->wheel_chg=&WheelChangeWinScroll;
    QueIns(c,task->last_ctrl);
  }
  TaskDerivedValsUpdate(task);
  Preempt(old_preempt);
}
#define VIEWANGLES_SPACING	22
#define VIEWANGLES_RANGE	48
#define VIEWANGLES_BORDER	2
#define VIEWANGLES_SNAP		2

U0 DrawViewAnglesCtrl(CDC *dc,CCtrl *c)
{
  I64 i,j;
  CViewAngles *s=c->state;

  dc->color=s->cbd;
  GrRect(dc, c->left,c->top,VIEWANGLES_SPACING*4+3,VIEWANGLES_SPACING*2+VIEWANGLES_RANGE);
  dc->color=s->cbg;
  GrRect(dc, c->left+VIEWANGLES_BORDER,c->top+VIEWANGLES_BORDER,
    VIEWANGLES_SPACING*4+3-2*VIEWANGLES_BORDER,VIEWANGLES_SPACING*2+VIEWANGLES_RANGE-2*VIEWANGLES_BORDER);
  dc->color=s->cfg;
  GrLine(dc,c->left+VIEWANGLES_SPACING,c->top+VIEWANGLES_SPACING,
	      c->left+VIEWANGLES_SPACING,c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1);
  GrLine(dc,c->left+2*VIEWANGLES_SPACING+1,c->top+VIEWANGLES_SPACING,
	      c->left+2*VIEWANGLES_SPACING+1,c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1);
  GrLine(dc,c->left+3*VIEWANGLES_SPACING+2,c->top+VIEWANGLES_SPACING,
	      c->left+3*VIEWANGLES_SPACING+2,c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1);
  for (i=1;i<VIEWANGLES_RANGE+1;i+=2*VIEWANGLES_SNAP) {
    j=2-i/3&1;
    GrLine(dc,c->left+VIEWANGLES_SPACING-j,c->bottom-VIEWANGLES_SPACING-i,
		c->left+VIEWANGLES_SPACING+j,c->bottom-VIEWANGLES_SPACING-i);
    GrLine(dc,c->left+2*VIEWANGLES_SPACING+1-j,c->bottom-VIEWANGLES_SPACING-i,
		c->left+2*VIEWANGLES_SPACING+1+j,c->bottom-VIEWANGLES_SPACING-i);
    GrLine(dc,c->left+3*VIEWANGLES_SPACING+2-j,c->bottom-VIEWANGLES_SPACING-i,
		c->left+3*VIEWANGLES_SPACING+2+j,c->bottom-VIEWANGLES_SPACING-i);
  }

  dc->color=s->cx;
  GrPrintF(dc,c->left+VIEWANGLES_SPACING-FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING-(1+FONT_HEIGHT),"X");
  GrPrintF(dc,c->left+VIEWANGLES_SPACING-3*FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE+3,
    "%3d",s->sx*360/VIEWANGLES_RANGE);
  i=c->left+VIEWANGLES_SPACING;
  if (s->sx>VIEWANGLES_RANGE/2)
    j=-VIEWANGLES_RANGE/2+s->sx;
  else
    j=s->sx+VIEWANGLES_RANGE/2;
  j=c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1-j;
  GrRect(dc,i-3,j-2,7,5);
  dc->color=s->cx^8;
  GrRect(dc,i-2,j-1,5,3);


  dc->color=s->cy;
  GrPrintF(dc,c->left+2*VIEWANGLES_SPACING+1-FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING-(1+FONT_HEIGHT),"Y");
  GrPrintF(dc,c->left+2*VIEWANGLES_SPACING+1-3*FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE+3,
    "%3d",s->sy*360/VIEWANGLES_RANGE);
  i=c->left+2*VIEWANGLES_SPACING+1;
  if (s->sy>VIEWANGLES_RANGE/2)
    j=-VIEWANGLES_RANGE/2+s->sy;
  else
    j=s->sy+VIEWANGLES_RANGE/2;
  j=c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1-j;
  GrRect(dc,i-3,j-2,7,5);
  dc->color=s->cy^8;
  GrRect(dc,i-2,j-1,5,3);


  dc->color=s->cz;
  GrPrintF(dc,c->left+3*VIEWANGLES_SPACING+2-FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING-(1+FONT_HEIGHT),"Z");
  GrPrintF(dc,c->left+3*VIEWANGLES_SPACING+2-3*FONT_WIDTH/2,
	      c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE+3,
    "%3d",s->sz*360/VIEWANGLES_RANGE);
  i=c->left+3*VIEWANGLES_SPACING+2;
  if (s->sz>VIEWANGLES_RANGE/2)
    j=-VIEWANGLES_RANGE/2+s->sz;
  else
    j=s->sz+VIEWANGLES_RANGE/2;
  j=c->top+VIEWANGLES_SPACING+VIEWANGLES_RANGE-1-j;
  GrRect(dc,i-3,j-2,7,5);
  dc->color=s->cz^8;
  GrRect(dc,i-2,j-1,5,3);
}

U0 UpdateDerivedViewAnglesCtrl(CCtrl *c)
{
  CViewAngles *s=c->state;
  c->left=c->win_task->win_pixel_width-(VIEWANGLES_SPACING*4+3);
  c->right=c->left+VIEWANGLES_SPACING*4+3;
  c->top=c->win_task->win_pixel_height-(VIEWANGLES_SPACING*2+VIEWANGLES_RANGE);
  c->bottom=c->top+VIEWANGLES_SPACING*2+VIEWANGLES_RANGE;
  s->sx=LimitI64(RoundI64(s->sx,VIEWANGLES_SNAP),0,VIEWANGLES_RANGE-1);
  s->sy=LimitI64(RoundI64(s->sy,VIEWANGLES_SNAP),0,VIEWANGLES_RANGE-1);
  s->sz=LimitI64(RoundI64(s->sz,VIEWANGLES_SNAP),0,VIEWANGLES_RANGE-1);
  s->ax=2*ã*s->sx/VIEWANGLES_RANGE;
  s->ay=2*ã*s->sy/VIEWANGLES_RANGE;
  s->az=2*ã*s->sz/VIEWANGLES_RANGE;
}

U0 LeftClickViewAngles(CCtrl *c,I64 x,I64 y,Bool)
{
  CViewAngles *s=c->state;
  I64 i;
  i=VIEWANGLES_RANGE-1-(y-(c->top+VIEWANGLES_SPACING));
  if (i>=VIEWANGLES_RANGE/2)
    i-=VIEWANGLES_RANGE/2;
  else
    i+=VIEWANGLES_RANGE/2;
  if (x<c->left+(c->right-c->left)/3)
    s->sx=i;
  else if (x<c->left+2*(c->right-c->left)/3)
    s->sy=i;
  else
    s->sz=i;
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

public CCtrl *ViewAnglesNew(CTask *task=NULL)
{
  Bool old_preempt;
  CCtrl *c;
  CViewAngles *s;
  if (!task) task=Fs;
  if (!(c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES))) {
    s=CAlloc(sizeof(CViewAngles),task);
    c=CAlloc(sizeof(CCtrl));
    s->cbd=BLUE;
    s->cbg=LTBLUE;
    s->cfg=BLACK;
    s->cx=LTGREEN;
    s->cy=GREEN;
    s->cz=LTGREEN;
    c->win_task=task;
    c->flags=CTRLF_SHOW|CTRLF_CAPTURE_LEFT_IP;
    c->type=CTRLT_VIEWING_ANGLES;
    c->state=s;
    c->draw_it=&DrawViewAnglesCtrl;
    c->left_click=&LeftClickViewAngles;
    c->update_derived_vals=&UpdateDerivedViewAnglesCtrl;
    old_preempt=Preempt(OFF);
    QueIns(c,task->last_ctrl);
    TaskDerivedValsUpdate(task);
    Preempt(old_preempt);
  }
  return c;
}

public U0 ViewAnglesDel(CTask *task=NULL)
{
  CCtrl *c;
  Bool old_preempt=Preempt(OFF);
  if (!task) task=Fs;
  if (c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES)) {
    QueRem(c);
    Free(c->state);
    Free(c);
  }
  Preempt(old_preempt);
}
