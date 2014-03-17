#help_index "Graphics/Char;Char/Graphics"
public U0 PlotScreenStr(U8 *s,I64 attr,I64 x,I64 y)
{
  I64 ch;
  attr<<=8;
  while (ch=*s++) {
    PlotScreenU8(ch+attr,x,y);
    x++;
  }
}

public U0 PlotWinStr(CTask *task,U8 *s,I64 attr,I64 x,I64 y)
{
  I64 ch;
  attr<<=8;
  while (ch=*s++) {
    PlotWinU8(task,ch+attr,x,y);
    x++;
  }
}

public U0 PlotBorderStr(CTask *task,U8 *s,I64 attr,I64 x,I64 y)
{
  I64 ch;
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
      PlotWinU8(task,'É'+attr,l-1,t-1);
      PlotWinU8(task,'»'+attr,r+1,t-1);
      PlotWinU8(task,'È'+attr,l-1,b+1);
      PlotWinU8(task,'¼'+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotWinU8(task,'Í'+attr,i,t-1);
	PlotWinU8(task,'Í'+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotWinU8(task,'º'+attr,l-1,i);
	PlotWinU8(task,'º'+attr,r+1,i);
      }
    } else {
      PlotWinU8(task,'Ú'+attr,l-1,t-1);
      PlotWinU8(task,'¿'+attr,r+1,t-1);
      PlotWinU8(task,'À'+attr,l-1,b+1);
      PlotWinU8(task,'Ù'+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotWinU8(task,'Ä'+attr,i,t-1);
	PlotWinU8(task,'Ä'+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotWinU8(task,'³'+attr,l-1,i);
	PlotWinU8(task,'³'+attr,r+1,i);
      }
    }
  } else {
    if (solid) {
      PlotScreenU8('É'+attr,l-1,t-1);
      PlotScreenU8('»'+attr,r+1,t-1);
      PlotScreenU8('È'+attr,l-1,b+1);
      PlotScreenU8('¼'+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotScreenU8('Í'+attr,i,t-1);
	PlotScreenU8('Í'+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotScreenU8('º'+attr,l-1,i);
	PlotScreenU8('º'+attr,r+1,i);
      }
    } else {
      PlotScreenU8('Ú'+attr,l-1,t-1);
      PlotScreenU8('¿'+attr,r+1,t-1);
      PlotScreenU8('À'+attr,l-1,b+1);
      PlotScreenU8('Ù'+attr,r+1,b+1);
      for (i=l;i<=r;i++) {
	PlotScreenU8('Ä'+attr,i,t-1);
	PlotScreenU8('Ä'+attr,i,b+1);
      }
      for (i=t;i<=b;i++) {
	PlotScreenU8('³'+attr,l-1,i);
	PlotScreenU8('³'+attr,r+1,i);
      }
    }
  }
}

U0 DrawTaskBorder(CTask *task)
{
  PUSHFD
  CLI
  DrawBorder(task,task==sys_focus_task,
    task->win_left,task->win_right,
    task->win_top,task->win_bottom,
    task->border_attr,FALSE);
  POPFD
}
