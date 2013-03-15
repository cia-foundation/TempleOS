
/*
You must "Adam Include" this because the Adam task's memory is never freed,
so the code for this file will not be yanked from underneath the task.

 $WW,1$This is a really bad alternative window mgr.  It augments the existing
one found in $LK-A,"::/TempleOS/Adam/Window.CPP"$.  It uses the real-time differential equation solver to reposition windows as though they were repulsively charged particles.  I find this window mgr to be a novelty, not desirable for actual use.  You're welcome to add it, or some variation to your start-up scripts if you like it.  It's not fully debugged.

I had to cheat to make this work.  I added $LK,"DISPLAYf_USER_HAS_BEEN_RESIZED","MN:DISPLAYf_USER_HAS_BEEN_RESIZED"$ and modified $LK-A,"::/TempleOS/Adam/Window.CPP"$.

The differential equation stuff was overkill and made it overly complicated.  Oh, well.
$WW,0$*/

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *state,COrder2D3 *DstateDt)
{
  I64 i,j,task_cnt=ode->n/6;
  F64 f,d,dd,dx,dy;
  COrder2D3 *tempo;
  if (!task_cnt) return;

  MemSet(DstateDt,0,sizeof(COrder2D3)*task_cnt);

  tempo=DstateDt;
  for (i=0;i<task_cnt;i++,tempo++) {
    tempo->x=state[i].DxDt;
    tempo->y=state[i].DyDt;
    for (j=0;j<task_cnt;j++) {
      if (i!=j) {
	dx=state[i].x-state[j].x;
	dy=state[i].y-state[j].y;
	dd=Sqr(dx)+Sqr(dy)+1.0;
	d=Sqrt(dd);

	dx/=d;
	dy/=d;

	f=1000000.0/dd;

	tempo->DxDt+=f*dx; //Repulsive force like electro-static
	tempo->DyDt+=f*dy;
      }
    }
  }

  tempo=DstateDt; //a=-v  (Damp-out motion.)
  for (i=0;i<task_cnt;i++,tempo++) {
    tempo->DxDt-=state[i].DxDt;
    tempo->DyDt-=state[i].DyDt;
  }
}

U0 WinMngTask(I64)
{ //Preempt() is off after Spawn()
  Bool same=FALSE;
  I64 i,j,w,h,x,y,cnt,task_cnt=0;
  COrder2D3 *tempo;
  CTask *task,**task_map=NULL;
  CMathODE *ode=NULL;

  while (TRUE) {
restart:
    WinMgrSync;
    while (Bt(&sys_semas[SYS_SEMA_REFRESH_IN_PROGRESS],0))
      Yield;
    cnt=0;
    task=Fs->next_task;
    while (task!=Fs) {
      if (Bt(&task->display_flags,DISPLAYf_SHOW))
	cnt++;
      task=task->next_task;
    }
    if (!cnt) {
      same=FALSE;
      goto restart;
    }

    if (same && cnt==task_cnt) {
      i=0;
      task=Fs->next_task;
      while (task!=Fs) {
	if (Bt(&task->display_flags,DISPLAYf_SHOW))
	  if (task_map[i++]!=task)
	    same=FALSE;
	task=task->next_task;
      }
    } else
      same=FALSE;
    if (!same) {
      task_cnt=cnt;
      if (task_map) {
	Free(task_map);
	QueRem(ode);
	ODEDel(ode);
      }
      task_map=MAlloc(sizeof(U8 *)*task_cnt);
      ode=ODENew(task_cnt*6,0.1);
      ode->derivative=&MyDerivative;
      QueIns(ode,Fs->last_ode);

      i=0;
      task=Fs->next_task;
      while (task!=Fs) {
	if (Bt(&task->display_flags,DISPLAYf_SHOW)) {
	  tempo=ode->state(U8 *)+sizeof(COrder2D3)*i;
	  task_map[i++]=task;
	  w=task->win_width-1;
	  h=task->win_height-1;
	  tempo->x=task->win_left+w>>1;
	  tempo->y=task->win_top+h>>1;
	  tempo->DxDt=0;
	  tempo->DyDt=0;
	}
	task=task->next_task;
      }
      same=TRUE;
    } else {
      tempo=ode->state;
      for (i=0;i<task_cnt;i++,tempo++) {
	task=task_map[i];
	w=task->win_width-1;
	h=task->win_height-1;
	if (LBtr(&task->display_flags,DISPLAYf_USER_HAS_BEEN_RESIZED)) {
	  tempo->x=task->win_left+w>>1;
	  tempo->y=task->win_top+h>>1;
	  tempo->DxDt=0;
	  tempo->DyDt=0;
	} else {
	  x=task->win_left+w>>1;
	  y=task->win_top+h>>1;
	  tempo->x=Limit(tempo->x,x-2,x+2); //Only move up to 2 chars at a time
	  tempo->y=Limit(tempo->y,y-2,y+2);

	  j=Round(tempo->x-w>>1);
	  if (Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	    j=LimitI64(j,0,TEXT_COLS-1-w);
	  else
	    j=LimitI64(j,1,TEXT_COLS-2-w);
	  SetWinHorz(j,j+w,task);

	  j=Round(tempo->y-h>>1);
	  if (Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	    j=LimitI64(j,0,TEXT_ROWS-1-h);
	  else
	    j=LimitI64(j,1,TEXT_ROWS-2-h);
	  SetWinVert(j,j+h,task);

	  x=task->win_left+(task->win_width-1)>>1;
	  y=task->win_top+(task->win_height-1)>>1;
	  if (LBtr(&task->display_flags,DISPLAYf_USER_HAS_BEEN_RESIZED)) {
	    tempo->x=x;
	    tempo->y=y;
	  }
	  tempo->DxDt=Limit(tempo->DxDt,-8.0,8.0); //+/- 8 chars/sec
	  tempo->DyDt=Limit(tempo->DyDt,-8.0,8.0);
	}
      }
    }
  }
}

Spawn(&WinMngTask,NULL,"Window Mgr 2");
