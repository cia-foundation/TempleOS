//8 has graphics
class CBubble
{
  CBubble *next,*last;
  F64 x,y,dx,dy,radius,t0;
} root;

F64 t0,x,y;
U64 flags;

F64 Windé()
{
  F64 t=ã*(Beat-t0);
  return Arg(Sin(t/8),1.0+Sin(t/4));
}

U0 DrawIt(CTask *,CDC *dc)
{
  CBubble *tempb;
  F64 é;
  I64 r,x1,y1,z1,x2,y2,z2;

  if (LBts(&flags,0))
    return;

  dc->flags|=DCF_TRANSFORMATION;
  GrRotXEqu(dc->r,ã/4);

  é=Wrap(ã-Windé);
  GrRotZEqu(dc->r,-ã/2+é/10);

  dc->color=BLUE;
  dc->x=x;
  dc->y=y;
  dc->pen_width=2;
  GrCircle3(dc,0,0,0,10);
  GrLine3(dc,-10,0,0,-30,0,0);

  tempb=root.last;
  if (tempb!=&root) {
    dc->color=LTBLUE;
    r=tempb->radius*Min(1.0,0.25+2*(Beat-tempb->t0));
    for (é=0;é<2*ã;é+=ã/2) {
      x1=10*Cos(é);y1=10*Sin(é);z1=0;
      GrTransform(dc,&x1,&y1,&z1);

      x2=r*Cos(é);y2=r*Sin(é);z2=0;
      GrTransform(dc,&x2,&y2,&z2);

      GrLine(dc,x1,y1,tempb->x-x+x2,tempb->y-y+y2);
    }
    GrCircle(dc,tempb->x,tempb->y,r);
    tempb=tempb->last;
  }
  while (tempb!=&root) {
    dc->color=LTBLUE;
    GrCircle(dc,tempb->x,tempb->y,tempb->radius);
    dc->color=LTGRAY;
    GrCircle(dc,tempb->x+0.65*tempb->radius*Cos(5*ã/8),
		tempb->y-0.65*tempb->radius*Sin(5*ã/8),1.0+0.2*tempb->radius);
    tempb=tempb->last;
  }
  LBtr(&flags,0);
}

U0 AnimateTask(I64)
{//Preemption is off after Spawn
  CTask *task=Fs->parent_task;
  CBubble *tempb,*tempb1;
  F64 t_last=Beat,dt,tSpawn=0,é;

  while (TRUE) {
    dt=Beat-t_last;
    t_last=Beat;

    x+=2*Sin(Beat/3)*dt;
    y+=2*Sin(Beat/2)*dt;

    while (LBts(&flags,0))
      Yield;

    tSpawn+=dt;
    while (tSpawn>1.0) {
      tempb=MAlloc(sizeof(CBubble));
      tempb->x=x;
      tempb->y=y;
      tempb->dx=0;
      tempb->dy=0;
      tempb->radius=Rand*6+4;
      tempb->t0=Beat;
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

      if (!(0<=tempb->x<task->pix_width) ||
	  !(0<=tempb->y<task->pix_height) ) {
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
  t0=Beat;
  x=360; y=52;
}

U0 CleanUp()
{
  QueDel(&root);
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Preempt(OFF);
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Romans,8:28",A="BF:Romans,8:28"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3qEEBeBDsBBBBqDsBFBFD2G3D2G",
	    "Free \0will... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0fail.\n\0 \0 \0 \0");
      Play("3qEEBeBDsBBBBqDsBFBFD2G3D2G",
	    "God \0still... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0hail.\n\0 \0 \0 \0");
      Play("3qD2G3eA2G3qCetECAqBsACACqA",
	    "All \0things \0work \0for \0good... \0to \0the \0Lord.\n\0 \0 \0 \0 \0 \0 \0");
      Play("D2G3eA2G3qCetECAqBsACACqA",
	    "It's \0all \0good... \0 \0 \0as\0 \0sured.\n\0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
}

Song;
