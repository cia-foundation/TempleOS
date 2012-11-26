#help_index "Graphics/Char;Char/Graphics"
public U0 PlotScreenStr(U8 *s,I64 attr,I64 x,I64 y)
{
  U8 ch;
  attr<<=8;
  while (ch=*s++) {
    PlotScreenU8(ch+attr,x,y);
    x++;
  }
}

public U0 PlotWinStr(CTask *task,U8 *s,I64 attr,I64 x,I64 y)
{
  U8 ch;
  attr<<=8;
  while (ch=*s++) {
    PlotWinU8(task,ch+attr,x,y);
    x++;
  }
}

public U0 PlotBorderStr(CTask *task,U8 *s,I64 attr,I64 x,I64 y)
{
  U8 ch;
  attr<<=8;
  while (ch=*s++) {
    PlotBorderU8(task,ch+attr,x,y);
    x++;
  }
}

public U0 DrawBorder(CTask *task,Bool solid,I64 l,I64 r,I64 t,I64 b,I64 attr,Bool clip_to_win)
{
  I64 i;
  attr<<=8;
  if (clip_to_win) {
    if (solid) {
      PlotWinU8(task,CH_DBL_TL+attr,l-1,t-1);
      PlotWinU8(task,CH_DBL_TR+attr,r+1,t-1);
      PlotWinU8(task,CH_DBL_BL+attr,l-1,b+1);
      PlotWinU8(task,CH_DBL_BR+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotWinU8(task,CH_DBL_HORZ+attr,i,t-1);
	PlotWinU8(task,CH_DBL_HORZ+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotWinU8(task,CH_DBL_VERT+attr,l-1,i);
	PlotWinU8(task,CH_DBL_VERT+attr,r+1,i);
      }
    } else {
      PlotWinU8(task,CH_SINGLE_TL+attr,l-1,t-1);
      PlotWinU8(task,CH_SINGLE_TR+attr,r+1,t-1);
      PlotWinU8(task,CH_SINGLE_BL+attr,l-1,b+1);
      PlotWinU8(task,CH_SINGLE_BR+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotWinU8(task,CH_SINGLE_HORZ+attr,i,t-1);
	PlotWinU8(task,CH_SINGLE_HORZ+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotWinU8(task,CH_SINGLE_VERT+attr,l-1,i);
	PlotWinU8(task,CH_SINGLE_VERT+attr,r+1,i);
      }
    }
  } else {
    if (solid) {
      PlotScreenU8(CH_DBL_TL+attr,l-1,t-1);
      PlotScreenU8(CH_DBL_TR+attr,r+1,t-1);
      PlotScreenU8(CH_DBL_BL+attr,l-1,b+1);
      PlotScreenU8(CH_DBL_BR+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotScreenU8(CH_DBL_HORZ+attr,i,t-1);
	PlotScreenU8(CH_DBL_HORZ+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotScreenU8(CH_DBL_VERT+attr,l-1,i);
	PlotScreenU8(CH_DBL_VERT+attr,r+1,i);
      }
    } else {
      PlotScreenU8(CH_SINGLE_TL+attr,l-1,t-1);
      PlotScreenU8(CH_SINGLE_TR+attr,r+1,t-1);
      PlotScreenU8(CH_SINGLE_BL+attr,l-1,b+1);
      PlotScreenU8(CH_SINGLE_BR+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotScreenU8(CH_SINGLE_HORZ+attr,i,t-1);
	PlotScreenU8(CH_SINGLE_HORZ+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotScreenU8(CH_SINGLE_VERT+attr,l-1,i);
	PlotScreenU8(CH_SINGLE_VERT+attr,r+1,i);
      }
    }
  }
}

U0 DrawTaskBorder(CTask *task)
{
  PushFD;
  Cli;
  DrawBorder(task,task==sys_focus_task,
    task->win_left,task->win_right,
    task->win_top,task->win_bottom,
    task->border_attr,FALSE);
  PopFD;
}

#help_index "Graphics"
