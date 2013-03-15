//8 has graphics

class CBubble
{
  CBubble *next,*last;
  F64 x,y,dx,dy,radius;
} root;

F64 t0;
U64 flags;

#define X	360
#define Y	52

F64 Windé()
{
  F64 t=ã*(tMBeat-t0);
  return Arg(Sin(t/8),1.0+Sin(t/4));
}

U0 DrawIt(CTask *,CDC *dc)
{
  CBubble *tempb;
  F64 é;

  if (LBts(&flags,0))
    return;

  dc->flags|=DCF_TRANSFORMATION;
  GrRotXEqu(dc->r,ã/4);

  é=Wrap(ã-Windé);
  GrRotZEqu(dc->r,-ã/2+é/4);

  dc->color=BLUE;
  dc->x=X;
  dc->y=Y;
  dc->pen_width=2;
  GrCircle3(dc,0,0,0,10);
  GrLine3(dc,-10,0,0,-30,0,0);

  tempb=root.next;
  while (tempb!=&root) {
    dc->color=LTBLUE;
    GrCircle(dc,tempb->x,tempb->y,tempb->radius);
    dc->color=LTGRAY;
    GrCircle(dc,
	      tempb->x+0.65*tempb->radius*Cos(5*ã/8),
	      tempb->y-0.65*tempb->radius*Sin(5*ã/8),
	      1.0+0.2*tempb->radius);
    tempb=tempb->next;
  }
  LBtr(&flags,0);
}

U0 AnimateTask(I64)
{ //Preemption is off after Spawn
  CTask *task=Fs->parent_task;
  CBubble *tempb,*tempb1;
  F64 t_last=tMBeat,dt,tSpawn=0,é;

  while (TRUE) {
    dt=tMBeat-t_last;
    t_last=tMBeat;

    while (LBts(&flags,0))
      Yield;

    tSpawn+=dt;
    while (tSpawn>1.0) {
      tempb=MAlloc(sizeof(CBubble));
      tempb->x=X;
      tempb->y=Y;
      tempb->dx=0;
      tempb->dy=0;
      tempb->radius=Rand*6+4;
      QueIns(tempb,root.last);
      tSpawn-=Rand;
    }

    é=Windé;
    tempb=root.next;
    while (tempb!=&root) {
      tempb1=tempb->next;

      tempb->dx+=25*dt*(Rand-0.5);
      tempb->dy+=25*dt*(Rand-0.5);
      tempb->x += dt*(20*Cos(é)+tempb->dx);
      tempb->y +=-dt*(20*Sin(é)+tempb->dy);

      if (!(0<=tempb->x<task->win_pixel_width) ||
	  !(0<=tempb->y<task->win_pixel_height) ) {
	QueRem(tempb);
	Free(tempb);
      }

      tempb=tempb1;
    }

    LBtr(&flags,0);
    Sleep(20);
  }
}

U0 Init()
{
  flags=0;
  QueInit(&root);
  t0=tMBeat;
}

U0 CleanUp()
{
  QueDel(&root);
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Preempt(OFF);
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",Fs);
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  task=PopUpViewStr(
    "$LK,"Romans,8:28","BF:Romans,8:28"$\n"
    );
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
  while (!ScanKey) {
	Play("W13qEEBeBDsBBB!BqDsBFB!!FD2G3D!2G",
		"Free \0will... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0fail.\n\0 \0 \0 \0");
	Play("3qEEBeBDsBBB!BqDsBFB!!FD2G3D!2G",
		"God \0still... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0hail.\n\0 \0 \0 \0");
	Play("3qD2G3eA2G3qCetEC!AqBsA!!C!ACqA",
		"All \0things \0work \0for \0good... \0to \0the \0Lord.\n\0 \0 \0 \0 \0 \0 \0");
	Play("D2G3eA2G3qCetEC!AqBsA!!C!ACqA",
		"It's \0all \0good... \0 \0 \0as\0 \0sured.\n\0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  SettingsPop;
  CleanUp;
}

Song;
