//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.

RegSetDftEntry("TempleOS/RawHide","F64 best_score=9999;\n");
RegExeBranch("TempleOS/RawHide");

F64 t0,tf;
I64 outside_cnt;

#define MAP_WIDTH	1000
#define MAP_HEIGHT	1000
#define FENCE_WIDTH	320
#define FENCE_HEIGHT	200
#define MAP_BORDER	3
CDC *map_dc;

$BG,10$#define GE_COW		0


$SP,"<1>",BI=1$
#define GE_BULL		1


$SP,"<2>",BI=2$
#define GE_HORSE	2


$SP,"<3>",BI=3$
#define GE_NUM		3


$SP,"<4>",BI=4$
$BG$
class RHCSprite
{
  U8 *img;
  I64 min_x,max_x,min_y,max_y;
} rh_sprite[GE_NUM];

#define NUM_ANIMALS	100

class Animal
{
  I64 num,x,y,dx,dy;
  RHCSprite *ge;
  U32 buddy;
  U8 type;
  Bool dead,pad[2];
} *a;

//************************************
#define COURSE_LEN	(MAP_HEIGHT*2)
I32 *r_x,*r_y,*r_width;
I64 river_lo,river_hi;
U0 RiverNew()
{
  r_x=MAlloc(COURSE_LEN*sizeof(I32));
  r_y=MAlloc(COURSE_LEN*sizeof(I32));
  r_width=MAlloc(COURSE_LEN*sizeof(I32));
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
      r_x[i]=x.i32[1]; r_y[i]=y.i32[1]; r_width[i]=w.i32[1];
      river_hi=i++;
      old_x=x; old_y=y;
    }
    dx=ClampI64(dx+RandI32/64,-MAX_I32,MAX_I32);
    dy=ClampI64(dy+RandI32/64,0,MAX_I32);
    x+=dx; y+=dy;
    w=ClampI64(w+RandI32*2,10*MAX_U32,150*MAX_U32);
  }
//Plan upper course of river
  old_x=-1;old_y=-1;
  x=MAP_WIDTH<<32/2; y=MAP_HEIGHT<<32/2;
  dx=0; dy=-1<<32;
  w=20<<32;
  i=COURSE_LEN/2-1;
  while (y.i32[1]>=MAP_BORDER && i>=0) {
    if (old_x.i32[1]!=x.i32[1]||old_y.i32[1]!=y.i32[1]) {
      r_x[i]=x.i32[1]; r_y[i]=y.i32[1]; r_width[i]=w.i32[1];
      river_lo=i--;
      old_x=x; old_y=y;
    }
    dx=ClampI64(dx+RandI32/64,-MAX_I32,MAX_I32);
    dy=ClampI64(dy+RandI32/64,-MAX_I32,0);
    x+=dx; y+=dy;
    w=ClampI64(w+RandI32*2,10*MAX_U32,150*MAX_U32);
  }

  //Plot sand bar
  x=0; y=0;
  for (i=river_lo;i<=river_hi;i++) {
    map_dc->color=YELLOW;
    map_dc->pen_width=r_width[i]+5;
    GrPlot3(map_dc,r_x[i]+x.i32[1],r_y[i]+y.i32[1],0);

    x=ClampI64(x+RandI32,-6*MAX_U32,6*MAX_U32);
    y=ClampI64(y+RandI32,-6*MAX_U32,6*MAX_U32);
  }

  //Plot water
  for (i=river_lo;i<=river_hi;i++) {
    map_dc->color=BLUE;
    map_dc->pen_width=r_width[i];
    GrPlot3(map_dc,r_x[i],r_y[i],0);
  }
}

U0 RiverDel()
{
  Free(r_x);
  Free(r_y);
  Free(r_width);
}

//************************************
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
	      GrPeek(map_dc,r_x[tempr->index]+tempr->dx,
	      r_y[tempr->index]+tempr->dy)==BLUE)
	  break;
	tempr->dx=ClampI64(tempr->dx+RandU16%3-1,-r_width[tempr->index]/2,
	      r_width[tempr->index]/2);
	tempr->dy=ClampI64(tempr->dy+RandU16%3-1,-r_width[tempr->index]/2,
	      r_width[tempr->index]/2);
      } while (GrPeek(map_dc,r_x[tempr->index]+tempr->dx,
	    r_y[tempr->index]+tempr->dy)!=BLUE &&
	    GrPeek(map_dc,r_x[tempr->index],
	    r_y[tempr->index])==BLUE); //Might be reinitializing
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
    GrPlot(dc,r_x[tempr->index]+tempr->dx-cx,r_y[tempr->index]+tempr->dy-cy);
    tempr=tempr->next;
  }
  LBtr(&rd_lock,0);
}

//************************************
U0 DrawIt(CTask *task,CDC *dc)
{
  static I64 last_pos_x=0;
  static Bool left=TRUE;
  F64 t;
  I64 i,
	cx=(MAP_WIDTH -task->pix_width)/2,
	cy=(MAP_HEIGHT-task->pix_height)/2;
  if (task->scroll_x+cx<0)
    task->scroll_x=-cx;
  if (task->scroll_x+cx>MAP_WIDTH-task->pix_width)
    task->scroll_x=MAP_WIDTH-task->pix_width-cx;
  if (task->scroll_y+cy<0)
    task->scroll_y=-cy;
  if (task->scroll_y+cy>MAP_HEIGHT-task->pix_height)
    task->scroll_y=MAP_HEIGHT-task->pix_height-cy;

  map_dc->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,-cx,-cy,map_dc);

  RiverDropsDraw(dc,cx,cy);

  for (i=0;i<NUM_ANIMALS;i++)
    if (!a[i].dead) {
      if (a[i].dx<0) {
	dc->flags|=DCF_JUST_MIRROR|DCF_SYMMETRY;
	DCSymmetrySet(dc,a[i].x.i32[1]-cx,0,a[i].x.i32[1]-cx,1);
      }
      Sprite3(dc,a[i].x.i32[1]-cx,a[i].y.i32[1]-cy,0,a[i].ge->img);
      dc->flags&=~(DCF_JUST_MIRROR|DCF_SYMMETRY);
    }

  if (ip.pos.x-last_pos_x>0)
    left=FALSE;
  else if (ip.pos.x-last_pos_x<0)
    left=TRUE;
  if (left) {
    dc->flags|=DCF_JUST_MIRROR|DCF_SYMMETRY;
    DCSymmetrySet(dc,ip.pos.x-task->pix_left-task->scroll_x,0,
	  ip.pos.x-task->pix_left-task->scroll_x,1);
  }
  Sprite3(dc,
	ip.pos.x-task->pix_left-task->scroll_x,
	ip.pos.y-task->pix_top -task->scroll_y,0,rh_sprite[GE_HORSE].img);
  dc->flags&=~(DCF_JUST_MIRROR|DCF_SYMMETRY);
  last_pos_x=ip.pos.x;

  if (tf) {
    dc->color=RED;
    t=tf-t0;
    if (Blink)
      GrPrint(dc,(task->pix_width-FONT_WIDTH*14)>>1-task->scroll_x,
	    (task->pix_height-FONT_HEIGHT)>>1-task->scroll_y,
	    "Game Completed");
  } else {
    dc->color=BLACK;
    t=tS-t0;
  }
  GrPrint(dc,-task->scroll_x,-task->scroll_y,
	"Outside:%03d Time:%7.2fs Best:%7.2fs",
	outside_cnt,t,best_score);
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
  I64 i,cx,cy,cursor_x,cursor_y,dd,ddx,ddy,cnt,max_speed=MAX_I64,updates=0,
	my_outside_cnt;
  F64 f,d,dx,dy,s,stress;
  Animal *tempa,*tempa1;
  while (TRUE) {
    max_speed=ClampU64(max_speed,MAX_U32/3,200*MAX_U32);
    cx=(MAP_WIDTH -parent->pix_width)/2,
	  cy=(MAP_HEIGHT-parent->pix_height)/2;
    cursor_x=ip.pos.x+cx-parent->pix_left-parent->scroll_x;
    cursor_y=ip.pos.y+cy-parent->pix_top -parent->scroll_y;
    cnt=0;stress=0;
    my_outside_cnt=0;
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
	  f=5.0e2*MAX_U32/dd;
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
	tempa->dx=0.995*ClampI64(tempa->dx+RandI32/32,-max_speed,max_speed);
	tempa->dy=0.995*ClampI64(tempa->dy+RandI32/32,-max_speed,max_speed);

	//Slow in river
	if (GrPeek(map_dc,tempa->x.i32[1],tempa->y.i32[1])!=LTGREEN) {
	  tempa->dx/=2;
	  tempa->dy/=2;
	}

	if (GrPeek(map_dc,(tempa->x+tempa->dx)>>32,(tempa->y+tempa->dy)>>32)
	      !=LTGRAY) {
	  tempa->x+=tempa->dx;
	  tempa->y+=tempa->dy;
	}

	//Keep on map
	if (!(MAP_BORDER-tempa->ge->min_x
	      <=tempa->x.i32[1]<MAP_WIDTH-MAP_BORDER-tempa->ge->max_x)) {
	  tempa->x -=tempa->dx;
	  tempa->dx=-tempa->dx;
	}
	if (!(MAP_BORDER-tempa->ge->min_y
	      <=tempa->y.i32[1]<MAP_HEIGHT-MAP_BORDER-tempa->ge->max_y)) {
	  tempa->y -=tempa->dy;
	  tempa->dy=-tempa->dy;
	}
	cnt++;
	if (tempa->x>>32>=FENCE_WIDTH || tempa->y>>32>=FENCE_HEIGHT)
	  my_outside_cnt++;
      }
    }
    outside_cnt=my_outside_cnt;

    if (!(updates&15))
      RiverDropsNext(parent);

    if (!tf && !outside_cnt) {
      tf=tS;
      music.mute=TRUE;
      Snd(2000);Sleep(200);Snd(0);Sleep(100);
      if (tf-t0<best_score) {
	best_score=tf-t0;
	Snd(2000);Sleep(200);Snd(0);Sleep(100);
      }
      music.mute=FALSE;
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
{//Randomly generated (by God :-)
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
  map_dc->color=LTGREEN;
  GrRect(map_dc,2,2,MAP_WIDTH-4,MAP_HEIGHT-4);

  RiverMake;

  //Plot fence
  for (i=FENCE_WIDTH;i>0;i-=16)
    Sprite3(map_dc,i,FENCE_HEIGHT,0,$IB,"<4>",BI=4$);
  map_dc->pen_width=1;
  map_dc->color=BROWN;
  for (i=0;i<FENCE_HEIGHT-16;i+=16)
    GrLine(map_dc,FENCE_WIDTH-1,i,FENCE_WIDTH-1,i+7);
  map_dc->color=LTGRAY;
  GrLine(map_dc,FENCE_WIDTH,0,FENCE_WIDTH,FENCE_HEIGHT-6);

  map_dc->color=LTGREEN;
  GrRect(map_dc,FENCE_WIDTH-63,FENCE_HEIGHT-8,46,9);

  map_dc->pen_width=MAP_BORDER;
  map_dc->color=RED;
  GrBorder(map_dc,MAP_BORDER/2,MAP_BORDER/2,
	MAP_WIDTH-(MAP_BORDER+1)/2,MAP_HEIGHT-(MAP_BORDER+1)/2);

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
  outside_cnt=NUM_ANIMALS;
  t0=tS;
  tf=0;
}

U0 Init()
{
  I64 i;
  RHCSprite *ge;

  rh_sprite[0].img=$IB,"<1>",BI=1$;
  rh_sprite[1].img=$IB,"<2>",BI=2$;
  rh_sprite[2].img=$IB,"<3>",BI=3$;
  for (i=0;i<GE_NUM;i++) {
    ge=&rh_sprite[i];
    SpriteExtents(ge->img,&ge->min_x,&ge->max_x,&ge->min_y,&ge->max_y);
  }

  RiverNew;
  rd_lock=0;
  QueInit(&rd_root);
  map_dc=DCNew(MAP_WIDTH,MAP_HEIGHT);
  a=MAlloc(NUM_ANIMALS*sizeof(Animal));
  ReInit;
}

U0 CleanUp()
{
  DCDel(map_dc);
  Free(a);
  RiverDropsDel;
  RiverDel;
}

U0 RawHide()
{
  I64 msg_code,a1,a2;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  PopUpOk(
	"Coral the cattle.  The coral is in the\n"
	"upper-left corner if you scroll.\n\n"
	"Scroll with $$GREEN$${CTRL-Left Grab}$$FG$$.\n");
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
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL-WIF_FOCUS_TASK_MENU;
  WinMax;
  AutoComplete;
  Preempt;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,Fs,"Animate",,Fs);
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      msg_code=GetMsg(&a1,&a2,
	    1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+1<<MSG_IP_R_DOWN);
      switch (msg_code) {
	case MSG_IP_L_DOWN:  //Doesn't do anything, yet.
	  break;
	case MSG_IP_R_DOWN:  //Doesn't do anything, yet.
	  break;
	case MSG_KEY_DOWN:
	  switch (a1) {
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
    PutExcept;
  MenuPop;
  SettingsPop;
  CleanUp;
  RegWriteBranch("TempleOS/RawHide","F64 best_score=%5.4f;\n",best_score);
}

RawHide;
        ò     öÿÿÿðÿÿÿ      ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿ    ÿÿÿÿÿÿ ÿÿÿÿÿ      ÿÿÿÿÿ       ÿÿ    ÿÿÿÿ      ÿÿÿÿÿÿÿÿ   ÿÿ     ÿÿÿ      ÿÿÿÿÿ ÿÿ   ÿ    ÿÿ      ÿÿÿÿ        ÿ      ÿÿÿ              ÿÿ     ÿÿ    ÿ      ÿ      ÿÿÿÿÿÿÿ      ÿÿ ÿ    ÿÿÿÿÿÿÿÿ      ÿÿÿÿ     ÿÿÿÿÿÿÿÿ      ÿÿÿÿ         ÿÿÿÿÿÿÿ             ÿÿ     ÿÿÿÿÿÿ          ÿÿÿ     ÿÿÿÿÿÿ             ÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ              b     öÿÿÿôÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ   ÿÿÿ             ÿÿ   ÿÿ    ÿ                          ÿ   ÿÿ       ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿ   ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿ   ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ           Â     öÿÿÿðÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ   ÿÿÿ              ÿÿ   ÿÿ    ÿ                          ÿ   ÿÿ      ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ     ÿ  ÿÿÿÿ   ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿ   ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿ   ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ           d         
            ùÿÿÿ   
    úÿÿÿðÿÿÿúÿÿÿ
    ÿÿÿÿðÿÿÿÿÿÿÿ
ðÿÿÿÿÿÿÿÿÿÿÿúÿÿÿ
ÿÿÿÿÿÿÿÿðÿÿÿúÿÿÿ 