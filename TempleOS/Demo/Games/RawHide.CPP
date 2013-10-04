//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.

AcctRegSetDftEntry("TempleOS/RawHide","F64 best_score=9999;\n");
AcctRegExeBranch("TempleOS/RawHide");

F64 t0,tf;

#define MAP_WIDTH	1500
#define MAP_HEIGHT	1500
#define MAP_BORDER	3
CDC *dc2;

$BG,10$
#define GE_COW		0


$SP,"<1>",1$
#define GE_BULL		1


$SP,"<2>",2$
#define GE_HORSE	2


$SP,"<3>",3$
#define GE_NUM		3
$BG$


class RHCSprite
{
  U8 *img;
  I64 min_x,max_x,min_y,max_y;
} rh_sprite[GE_NUM];



#define NUM_ANIMALS	128

class Animal
{
  I64 num,x,y,dx,dy;
  RHCSprite *ge;
  U32 buddy;
  U8 type;
  Bool dead,pad[2];
} *a;


// ************************************
#define COURSE_LEN	(MAP_HEIGHT*2)
I32 *rx,*ry,*rw;
I64 river_lo,river_hi;
U0 RiverNew()
{
  rx=MAlloc(COURSE_LEN*sizeof(I32));
  ry=MAlloc(COURSE_LEN*sizeof(I32));
  rw=MAlloc(COURSE_LEN*sizeof(I32));
  river_lo=river_hi=COURSE_LEN/2;
}

U0 RiverMake()
{
  I64 i,x,y,dx,dy,w,old_x,old_y;
  //Plan lower course of river
  old_x=-1;old_y=-1;
  x=MAP_WIDTH<<32/2; y=MAP_HEIGHT<<32/2;
  dx=0; dy=1<<32;
  w=20<<32;
  i=COURSE_LEN/2;
  while (y.i32[1]<MAP_HEIGHT-MAP_BORDER && i<COURSE_LEN) {
    if (old_x.i32[1]!=x.i32[1]||old_y.i32[1]!=y.i32[1]) {
      rx[i]=x.i32[1]; ry[i]=y.i32[1]; rw[i]=w.i32[1];
      river_hi=i++;
      old_x=x; old_y=y;
    }
    dx=LimitI64(dx+RandI32/64,-MAX_I32,MAX_I32);
    dy=LimitI64(dy+RandI32/64,0,MAX_I32);
    x+=dx; y+=dy;
    w=LimitI64(w+RandI32*2,10*MAX_U32,150*MAX_U32);
  }
  //Plan upper course of river
  old_x=-1;old_y=-1;
  x=MAP_WIDTH<<32/2; y=MAP_HEIGHT<<32/2;
  dx=0; dy=-1<<32;
  w=20<<32;
  i=COURSE_LEN/2-1;
  while (y.i32[1]>=MAP_BORDER && i>=0) {
    if (old_x.i32[1]!=x.i32[1]||old_y.i32[1]!=y.i32[1]) {
      rx[i]=x.i32[1]; ry[i]=y.i32[1]; rw[i]=w.i32[1];
      river_lo=i--;
      old_x=x; old_y=y;
    }
    dx=LimitI64(dx+RandI32/64,-MAX_I32,MAX_I32);
    dy=LimitI64(dy+RandI32/64,-MAX_I32,0);
    x+=dx; y+=dy;
    w=LimitI64(w+RandI32*2,10*MAX_U32,150*MAX_U32);
  }

  //Plot sand bar
  x=0; y=0;
  for (i=river_lo;i<=river_hi;i++) {
    dc2->color=YELLOW;
    dc2->pen_width=rw[i]+5;
    GrPlot3(dc2,rx[i]+x.i32[1],ry[i]+y.i32[1],0);

    x=LimitI64(x+RandI32,-6*MAX_U32,6*MAX_U32);
    y=LimitI64(y+RandI32,-6*MAX_U32,6*MAX_U32);
  }

  //Plot water
  for (i=river_lo;i<=river_hi;i++) {
    dc2->color=BLUE;
    dc2->pen_width=rw[i];
    GrPlot3(dc2,rx[i],ry[i],0);
  }
}

U0 RiverDel()
{
  Free(rx);
  Free(ry);
  Free(rw);
}


// ************************************
class RiverDrop
{
  RiverDrop *next,*last;
  I64 index,dx,dy;
} rd_root;
Bool rd_lock;

U0 RiverDropsDel()
{
  while (LBts(&rd_lock,0))
    Yield;
  QueDel(&rd_root,TRUE);
  QueInit(&rd_root);
  LBtr(&rd_lock,0);
}

U0 RiverDropsNext(CTask *mem_task)
{
  RiverDrop *tempr,*tempr1;
  while (LBts(&rd_lock,0))
    Yield;
  tempr=rd_root.next;
  while (tempr!=&rd_root) {
    tempr1=tempr->next;
    if (++tempr->index>river_hi) {
      QueRem(tempr);
      Free(tempr);
    } else {
      do {
	if (RandU16&1 &&
	    GrPeek(dc2,rx[tempr->index]+tempr->dx,ry[tempr->index]+tempr->dy)==BLUE)
	  break;
	tempr->dx=LimitI64(tempr->dx+RandU16%3-1,-rw[tempr->index]/2,rw[tempr->index]/2);
	tempr->dy=LimitI64(tempr->dy+RandU16%3-1,-rw[tempr->index]/2,rw[tempr->index]/2);
      } while (GrPeek(dc2,rx[tempr->index]+tempr->dx,ry[tempr->index]+tempr->dy)!=BLUE &&
	       GrPeek(dc2,rx[tempr->index]	      ,ry[tempr->index])	  ==BLUE); //Might be reinitializing
    }
    tempr=tempr1;
  }
  tempr=MAlloc(sizeof(RiverDrop),mem_task);
  tempr->index=river_lo;
  tempr->dx=0;
  tempr->dy=0;
  QueIns(tempr,rd_root.last);
  LBtr(&rd_lock,0);
}

U0 RiverDropsDraw(CDC *dc,I64 cx,I64 cy)
{
  RiverDrop *tempr;
  dc->color=LTBLUE;
  while (LBts(&rd_lock,0))
    Yield;
  tempr=rd_root.next;
  while (tempr!=&rd_root) {
    GrPlot(dc,rx[tempr->index]+tempr->dx-cx,ry[tempr->index]+tempr->dy-cy);
    tempr=tempr->next;
  }
  LBtr(&rd_lock,0);
}

// ************************************
U0 DrawIt(CTask *task,CDC *dc)
{
  static I64 last_ipx=0;
  static Bool left=TRUE;
  F64 t;
  I64 i,
     cx=(MAP_WIDTH -task->win_pixel_width)/2,
     cy=(MAP_HEIGHT-task->win_pixel_height)/2;
  if (task->win_scroll_x+cx<0)
    task->win_scroll_x=-cx;
  if (task->win_scroll_x+cx>MAP_WIDTH-task->win_pixel_width)
    task->win_scroll_x=MAP_WIDTH-task->win_pixel_width-cx;
  if (task->win_scroll_y+cy<0)
    task->win_scroll_y=-cy;
  if (task->win_scroll_y+cy>MAP_HEIGHT-task->win_pixel_height)
    task->win_scroll_y=MAP_HEIGHT-task->win_pixel_height-cy;

  dc2->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,-cx,-cy,dc2);

  RiverDropsDraw(dc,cx,cy);

  for (i=0;i<NUM_ANIMALS;i++)
    if (!a[i].dead) {
      if (a[i].dx<0) {
	dc->flags|=DCF_JUST_MIRROR|DCF_SYMMETRY;
	GrSetSymmetry(dc,a[i].x.i32[1]-cx,0,a[i].x.i32[1]-cx,1);
      }
      SpritePlot3(dc,a[i].x.i32[1]-cx,a[i].y.i32[1]-cy,0,a[i].ge->img);
      dc->flags&=~(DCF_JUST_MIRROR|DCF_SYMMETRY);
    }


  if (ipx-last_ipx>0)
    left=FALSE;
  else if (ipx-last_ipx<0)
    left=TRUE;
  if (left) {
    dc->flags|=DCF_JUST_MIRROR|DCF_SYMMETRY;
    GrSetSymmetry(dc,ipx-task->win_pixel_left-task->win_scroll_x,0,ipx-task->win_pixel_left-task->win_scroll_x,1);
  }
  SpritePlot3(dc,
    ipx-task->win_pixel_left-task->win_scroll_x,
    ipy-task->win_pixel_top -task->win_scroll_y,0,rh_sprite[GE_HORSE].img);
  dc->flags&=~(DCF_JUST_MIRROR|DCF_SYMMETRY);
  last_ipx=ipx;

  if (tf) {
    dc->color=RED;
    t=tf-t0;
    if (Blink)
      GrPrint(dc,(task->win_pixel_width-FONT_WIDTH*14)>>1-task->win_scroll_x,
		  (task->win_pixel_height-FONT_HEIGHT)>>1-task->win_scroll_y,
	"Game Completed");
  } else {
    dc->color=BLACK;
    t=tT-t0;
  }
  GrPrint(dc,-task->win_scroll_x,-task->win_scroll_y,"Time:%7.2fs  Best:%7.2fs",t,best_score);
}


U0 BuddySelect(I64 i)
{
  I64 b,best_b=i,score,best_score=MAX_I64;
  for (b=0;b<NUM_ANIMALS;b++) {
    if (b!=i && !a[b].dead) {
      score=RandU32%(512*512)+
	SqrI64(a[b].x.i32[1]-a[i].x.i32[1])+
	SqrI64(a[b].y.i32[1]-a[i].y.i32[1]);
      if (score<best_score) {
	best_score=score;
	best_b=b;
      }
    }
  }
  a[i].buddy=best_b;
}

U0 AnimateTask(CTask *parent)
{
  I64 i,cx,cy,cursor_x,cursor_y,dd,ddx,ddy,cnt,max_speed=MAX_I64,
     min_x,max_x,min_y,max_y,updates=0;
  F64 f,d,dx,dy,s,stress;
  Animal *tempa,*tempa1;
  while (TRUE) {
    max_speed=LimitU64(max_speed,MAX_U32/3,200*MAX_U32);
    cx=(MAP_WIDTH -parent->win_pixel_width)/2,
    cy=(MAP_HEIGHT-parent->win_pixel_height)/2;
    cursor_x=ipx+cx-parent->win_pixel_left-parent->win_scroll_x;
    cursor_y=ipy+cy-parent->win_pixel_top -parent->win_scroll_y;
    min_x=max_x=min_y=max_y=cnt=0;
    stress=0;
    for (i=0;i<NUM_ANIMALS;i++) {
      tempa=&a[i];
      if (!tempa->dead) {
//Move away from horse
	ddx=tempa->x.i32[1]-cursor_x;
	ddy=tempa->y.i32[1]-cursor_y;
	if (dd=SqrI64(ddx)+SqrI64(ddy)) {
	  d=Sqrt(dd);
	  dx=ddx/d;
	  dy=ddy/d;
	  f=2.0e2*MAX_U32/dd;
	  tempa->dx+=f*dx;
	  tempa->dy+=f*dy;
	}

//Reselect buddy about every NUM_ANIMALS*10ms=5.12 seconds
	tempa1=&a[tempa->buddy];
	if (tempa1->dead || i==updates%NUM_ANIMALS) {
	  BuddySelect(i);
	  tempa1=&a[tempa->buddy];
	}

//Move toward buddy
	ddx=tempa->x.i32[1]-tempa1->x.i32[1];
	ddy=tempa->y.i32[1]-tempa1->y.i32[1];
	if (dd=SqrI64(ddx)+SqrI64(ddy)) {
	  d=Sqrt(dd);
	  s=d`1.25-80;
	  stress+=Abs(s);
	  dx=ddx/d;
	  dy=ddy/d;
	  f=-0.001*s*MAX_U32;
	  tempa->dx+=f*dx;
	  tempa->dy+=f*dy;
	}

//Make velocity similar to buddy
	tempa->dx+=0.1*(tempa1->dx-tempa->dx);
	tempa->dy+=0.1*(tempa1->dy-tempa->dy);


//Add random movement, limit speed and dampen speed
	tempa->dx=0.995*LimitI64(tempa->dx+RandI32/32,-max_speed,max_speed);
	tempa->dy=0.995*LimitI64(tempa->dy+RandI32/32,-max_speed,max_speed);

//Slow in river
	if (GrPeek(dc2,tempa->x.i32[1],tempa->y.i32[1])!=LTGREEN) {
	  tempa->dx/=4;
	  tempa->dy/=4;
	}

	tempa->x+=tempa->dx;
	tempa->y+=tempa->dy;

//Keep on map
	if (!(MAP_BORDER-tempa->ge->min_x<=tempa->x.i32[1]<MAP_WIDTH-MAP_BORDER-tempa->ge->max_x)) {
	  tempa->x -=tempa->dx;
	  tempa->dx=-tempa->dx;
	}
	if (!(MAP_BORDER-tempa->ge->min_y<=tempa->y.i32[1]<MAP_HEIGHT-MAP_BORDER-tempa->ge->max_y)) {
	  tempa->y -=tempa->dy;
	  tempa->dy=-tempa->dy;
	}

	if (cnt) {
	  min_x=MinI64(min_x,tempa->x);
	  max_x=MaxI64(max_x,tempa->x);
	  min_y=MinI64(min_y,tempa->y);
	  max_y=MaxI64(max_y,tempa->y);
	} else {
	  min_x=max_x=tempa->x;
	  min_y=max_y=tempa->y;
	}

	cnt++;
      }
    }
    if (!(updates&15))
      RiverDropsNext(parent);

    if (!tf && (max_x-min_x)>>32<GR_WIDTH && (max_y-min_y)>>32<GR_HEIGHT) {
      tf=tT;
      music_mute=TRUE;
      Snd(2000);Sleep(200);Snd(0);Sleep(100);
      if (tf-t0<best_score) {
	best_score=tf-t0;
	Snd(2000);Sleep(200);Snd(0);Sleep(100);
      }
      music_mute=FALSE;
    }

    updates++;

    if (cnt)
      stress/=cnt;
    else
      stress=0;
    if (stress>100.0) {
      Yield;
      max_speed=stress/5.0*MAX_U32; //Converge faster at start-up
    } else {
      Sleep(10);
      max_speed=0; //Will be set to normal max speed
    }
  }
}

U0 SongTask(I64)
{ //Randomly generated (by God :-)
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
	Play("3qC2etG3DCBDCECFqFCsADADqB");
	Play("C2etG3DCBDCECFqFCsADADqB");
	Play("2sG3A2G3AqG2etG3GDeBBqBFeBAqE");
	Play("2sG3A2G3AqG2etG3GDeBBqBFeBAqE");
  }
}


U0 ReInit()
{
  I64 i;

  RiverDropsDel;
  dc2->color=LTGREEN;
  GrRect(dc2,2,2,MAP_WIDTH-4,MAP_HEIGHT-4);

  RiverMake;

  dc2->pen_width=MAP_BORDER;
  dc2->color=RED;
  GrLineRect4(dc2,MAP_BORDER/2,MAP_BORDER/2,MAP_WIDTH-(MAP_BORDER+1)/2,MAP_HEIGHT-(MAP_BORDER+1)/2);

  for (i=river_lo;i<=river_hi;i++)
    RiverDropsNext(Fs);

  MemSet(a,0,NUM_ANIMALS*sizeof(Animal));
  for (i=0;i<NUM_ANIMALS;i++) {
    a[i].num=i;
    a[i].x=(64+RandU32%(MAP_WIDTH-128))<<32;
    a[i].y=(64+RandU32%(MAP_WIDTH-128))<<32;
    if (i&1)
      a[i].ge=&rh_sprite[GE_COW];
    else
      a[i].ge=&rh_sprite[GE_BULL];
    BuddySelect(i);
  }
  t0=tT;
  tf=0;
}

U0 Init()
{
  I64 i;
  RHCSprite *ge;

  rh_sprite[0].img=$IB,"<1>",1$;
  rh_sprite[1].img=$IB,"<2>",2$;
  rh_sprite[2].img=$IB,"<3>",3$;
  for (i=0;i<GE_NUM;i++) {
    ge=&rh_sprite[i];
    SpriteExtents(ge->img,&ge->min_x,&ge->max_x,&ge->min_y,&ge->max_y);
  }

  RiverNew;
  rd_lock=0;
  QueInit(&rd_root);
  dc2=DCNew(DCT_COLOR8,MAP_WIDTH,MAP_HEIGHT);
  a=MAlloc(NUM_ANIMALS*sizeof(Animal));
  ReInit;
}

U0 CleanUp()
{
  DCDel(dc2);
  Free(a);
  RiverDropsDel;
  RiverDel;
}

U0 RawHide()
{
  I64 msg_code,p1,p2;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  PopUpOk(
	"Coral the cattle into an area\n"
	"the size of a screen.\n\n"
	"Scroll with {CTRL-Left Grab}.\n"
	"Center with {CTRL-Right}.");
  MenuPush(
"File {"
"  Abort(,CH_SHIFT_ESC);"
"  Exit(,CH_ESC);"
"}"
"Play {"
"  Restart(,'\n');"
"}"
);
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_GRAB_SCROLL-WIF_MENU;
  WinMax;
  WordStat;
  Preempt(OFF);
  Init;
  Fs->animate_task=Spawn(&AnimateTask,Fs,"Animate",,Fs);
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+1<<MSG_IP_R_DOWN);
      switch (msg_code) {
	case MSG_IP_L_DOWN:  //Doesn't do anything, yet.
	  break;
	case MSG_IP_R_DOWN:  //Doesn't do anything, yet.
	  break;
	case MSG_KEY_DOWN:
	  switch (p1) {
	    case '\n':
	      ReInit;
	      break;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      goto rh_done;
	  }
	  break;
      }
    }
rh_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    CatchAll;
  MenuPop;
  SettingsPop;
  CleanUp;
  AcctRegWriteBranch("TempleOS/RawHide","F64 best_score=%5.4f;\n",best_score);
}

RawHide;
            ò           öÿÿÿðÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿ    ÿÿÿÿÿÿ ÿÿÿÿÿ      ÿÿÿÿÿ       ÿÿ    ÿÿÿÿ      ÿÿÿÿÿÿÿÿ   ÿÿ     ÿÿÿ      ÿÿÿÿÿ ÿÿ   ÿ    ÿÿ      ÿÿÿÿ        ÿ      ÿÿÿ              ÿÿ     ÿÿ    ÿ      ÿ      ÿÿÿÿÿÿÿ      ÿÿ ÿ    ÿÿÿÿÿÿÿÿ      ÿÿÿÿ     ÿÿÿÿÿÿÿÿ      ÿÿÿÿ         ÿÿÿÿÿÿÿ             ÿÿ     ÿÿÿÿÿÿ          ÿÿÿ     ÿÿÿÿÿÿ             ÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ                 b           öÿÿÿôÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ   ÿÿÿ             ÿÿ   ÿÿ    ÿ                          ÿ   ÿÿ       ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿ   ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿ   ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ              Â           öÿÿÿðÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ   ÿÿÿ              ÿÿ   ÿÿ    ÿ                          ÿ   ÿÿ      ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿ   ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿ   ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ    