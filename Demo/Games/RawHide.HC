//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.

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

#define GATE_WIDTH	22
#define GATE_HEIGHT	7
F64 gate_é,gate_t;

$BG,10$#define GE_COW		0


$SP,"<1>",BI=1$	$SP,"<2>",BI=2$	$SP,"<3>",BI=3$
#define GE_BULL		1


$SP,"<4>",BI=4$	$SP,"<5>",BI=5$	$SP,"<6>",BI=6$
#define GE_HORSE	2


$SP,"<7>",BI=7$	$SP,"<8>",BI=8$
#define GE_NUM		3


$SP,"<9>",BI=9$

	$SP,"<10>",BI=10$







$BG$
#define ANIMAL_WIDTH	20
#define ANIMAL_HEIGHT	16

U8 *cow_imgs[4]  ={$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<2>",BI=2$},
   *bull_imgs[4] ={$IB,"<4>",BI=4$,$IB,"<5>",BI=5$,$IB,"<6>",BI=6$,$IB,"<5>",BI=5$},
   *horse_imgs[4]={$IB,"<7>",BI=7$,$IB,"<8>",BI=8$,$IB,"<7>",BI=7$,$IB,"<8>",BI=8$};

#define NUM_ANIMALS	100

class Animal
{
  I64	num,x,y,dx,dy;
  U8	**imgs;
  U32	buddy;
  U8	type,frame0;
  Bool	dead,pad;
} *a;

//************************************
#define WATERFALL_HEIGHT	(MAP_HEIGHT/20)
#define WATERFALL_DROPS		512
#define WATERFALL_ACCELERATION	10
I32 *r_x,*r_width,*wfd_x;
F64 *wfd_t0,waterfall_tf;
I64 waterfall_x,waterfall_y,waterfall_width;

U0 RiverNew()
{
  r_x=MAlloc(MAP_HEIGHT*sizeof(I32));
  r_width=MAlloc(MAP_HEIGHT*sizeof(I32));
  wfd_x=MAlloc(WATERFALL_DROPS*sizeof(I32));
  wfd_t0=MAlloc(WATERFALL_DROPS*sizeof(F64));
  waterfall_tf=Sqrt(2*WATERFALL_HEIGHT/WATERFALL_ACCELERATION);
}

U0 RiverMake()
{
  I64 i,x=2*MAP_WIDTH<<32/3,y,dx=0,w=15<<32;
  waterfall_y=(MAP_HEIGHT-WATERFALL_HEIGHT)/2*Rand+
    (MAP_HEIGHT-WATERFALL_HEIGHT)/4;
  for (y=MAP_BORDER;y<MAP_HEIGHT-MAP_BORDER;y++) {
    r_x[y]=x.i32[1]; r_width[y]=w.i32[1];
    if (waterfall_y-5<y<waterfall_y+WATERFALL_HEIGHT+5) {
      waterfall_width=r_width[y];
      waterfall_x=r_x[y]-waterfall_width/2;
    } else {
      dx=ClampI64(dx+RandI32/64,-MAX_I32,MAX_I32)-MAX_I32/256;
      w=ClampI64(w+RandI32*2,10*MAX_U32,150*MAX_U32);
      x+=dx;
    }
  }
  for (i=0;i<WATERFALL_DROPS;i++) {
    wfd_x[i]=Rand*waterfall_width+waterfall_x;
    wfd_t0[i]=tS-Rand*waterfall_tf;
  }

  //Plot waterfall cliff
  Sprite3B(map_dc,waterfall_x,waterfall_y,0,$IB,"<10>",BI=10$);

  //Plot sand bar
  x=0;
  for (y=MAP_BORDER;y<MAP_HEIGHT-MAP_BORDER;y++) {
    if (!(waterfall_y-9<y<waterfall_y+WATERFALL_HEIGHT+9)) {
      map_dc->color=YELLOW;
      map_dc->pen_width=r_width[y]+10;
      GrPlot3(map_dc,r_x[y]+x.i32[1],y,0);
    }
    x=ClampI64(x+RandI32,-6*MAX_U32,6*MAX_U32);
  }

  //Plot water
  for (y=MAP_BORDER;y<MAP_HEIGHT-MAP_BORDER;y++) {
    map_dc->color=BLUE;
    map_dc->pen_width=r_width[y];
    GrPlot3(map_dc,r_x[y],y,0);
  }
}

U0 RiverDel()
{
  Free(r_x);
  Free(r_width);
  Free(wfd_x);
  Free(wfd_t0);
}

//************************************
class RiverDrop
{
  RiverDrop *next,*last;
  I64 y,dx,dy;
} rd_head;
Bool rd_lock;

U0 RiverDropsDel()
{
  while (LBts(&rd_lock,0))
    Yield;
  QueDel(&rd_head,TRUE);
  QueInit(&rd_head);
  LBtr(&rd_lock,0);
}

U0 RiverDropsNext(CTask *mem_task)
{
  RiverDrop *tempr,*tempr1;
  while (LBts(&rd_lock,0))
    Yield;
  tempr=rd_head.next;
  while (tempr!=&rd_head) {
    tempr1=tempr->next;
    if (++tempr->y>=MAP_HEIGHT-MAP_BORDER) {
      QueRem(tempr);
      Free(tempr);
    } else {
      do {
	if (RandU16&1 && GrPeek(map_dc,r_x[tempr->y]+tempr->dx,
	      tempr->y+tempr->dy)==BLUE)
	  break;
	tempr->dx=ClampI64(tempr->dx+RandU16%3-1,-r_width[tempr->y]/2,
	      r_width[tempr->y]/2);
	tempr->dy=ClampI64(tempr->dy+RandU16%3-1,-r_width[tempr->y]/2,
	      r_width[tempr->y]/2);
      } while (GrPeek(map_dc,r_x[tempr->y]+tempr->dx,
	    tempr->y+tempr->dy)!=BLUE &&
	    GrPeek(map_dc,r_x[tempr->y],tempr->y)==BLUE);//Might be reiniting
    }
    tempr=tempr1;
  }
  tempr=MAlloc(sizeof(RiverDrop),mem_task);
  tempr->y=MAP_BORDER;
  tempr->dx=0;
  tempr->dy=0;
  QueIns(tempr,rd_head.last);
  LBtr(&rd_lock,0);
}

U0 RiverDropsDraw(CDC *dc,I64 cx,I64 cy)
{
  I64 i;
  F64 t=tS;
  RiverDrop *tempr;
  while (LBts(&rd_lock,0))
    Yield;
  tempr=rd_head.next;
  dc->color=LTBLUE;
  while (tempr!=&rd_head) {
    GrPlot(dc,r_x[tempr->y]+tempr->dx-cx,tempr->y+tempr->dy-cy);
    tempr=tempr->next;
  }
  LBtr(&rd_lock,0);

  dc->color=WHITE;
  for (i=0;i<WATERFALL_DROPS;i++)
    GrPlot(dc,wfd_x[i]-cx,waterfall_y+0.5*WATERFALL_ACCELERATION*
	  Sqr(waterfall_tf*Saw(t-wfd_t0[i],waterfall_tf))-cy);
}

//************************************
U0 DrawIt(CTask *task,CDC *dc)
{
  static I64 last_pos_x=0;
  static Bool left=TRUE;
  F64 t;
  I64 i,frame=4*tS,
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
      Sprite3(dc,a[i].x.i32[1]-cx,a[i].y.i32[1]-cy,0,
	    a[i].imgs[(frame+a[i].frame0)&3]);
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
  Sprite3(dc,ip.pos.x-task->pix_left-task->scroll_x,
	     ip.pos.y-task->pix_top -task->scroll_y,0,horse_imgs[frame&3]);
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

U0 BuddySel(I64 i)
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


U0 RedrawGate()
{
  F64 tt=tS-gate_t;
  I64 x1=FENCE_WIDTH-63,y1=FENCE_HEIGHT-1,dx,dy;

  if (tt<0.5)
    gate_é=Clamp(gate_é+0.02,0,ã/2);
  else if (tt>5.0)
    gate_é=Clamp(gate_é-0.02,0,ã/2);

  dx=GATE_WIDTH*Cos(gate_é); dy=-0.8*GATE_WIDTH*Sin(gate_é);

  map_dc->color=LTGREEN;
  GrRect(map_dc,x1,y1-0.8*GATE_WIDTH-GATE_HEIGHT,
	46,0.8*GATE_WIDTH+GATE_HEIGHT+3);

  map_dc->color=BLACK;

  GrLine(map_dc,x1,y1,x1+dx,y1+dy);
  GrLine(map_dc,x1,y1,x1,y1-GATE_HEIGHT);
  GrLine(map_dc,x1+dx,y1+dy,x1+dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1,y1-GATE_HEIGHT,x1+dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1,y1,x1+dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1,y1-GATE_HEIGHT,x1+dx,y1+dy);

  GrLine(map_dc,x1+45,y1,x1+45-dx,y1+dy);
  GrLine(map_dc,x1+45,y1,x1+45,y1-GATE_HEIGHT);
  GrLine(map_dc,x1+45-dx,y1+dy,x1+45-dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1+45,y1-GATE_HEIGHT,x1+45-dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1+45,y1,x1+45-dx,y1+dy-GATE_HEIGHT);
  GrLine(map_dc,x1+45,y1-GATE_HEIGHT,x1+45-dx,y1+dy);
}

Bool CheckMap(I64 x,I64 y)
{
  I64 i,j,c;
  if (SqrI64(x-(waterfall_x+waterfall_width/2))>>1+
	SqrI64(y-(waterfall_y+WATERFALL_HEIGHT/2))<2500)
    return FALSE;
  for (j=-4;j<=2;j++)
    for (i=-4;i<=4;i++) {
      c=GrPeek(map_dc,x+i,y+j);
      if (c==LTGRAY || c==BLACK)
	return FALSE;
    }
  return TRUE;
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
    if (cursor_x<FENCE_WIDTH && cursor_y<FENCE_HEIGHT)
      gate_t=tS;
    RedrawGate;
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

	//Resel buddy about every NUM_ANIMALS*10ms=5.12 seconds
	tempa1=&a[tempa->buddy];
	if (tempa1->dead || i==updates%NUM_ANIMALS) {
	  BuddySel(i);
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

	if (CheckMap((tempa->x+tempa->dx)>>32,(tempa->y+tempa->dy)>>32)) {
	  tempa->x+=tempa->dx;
	  tempa->y+=tempa->dy;
	}

	//Keep on map
	if (!(MAP_BORDER+ANIMAL_WIDTH/2
	      <=tempa->x.i32[1]<MAP_WIDTH-MAP_BORDER-ANIMAL_WIDTH/2)) {
	  tempa->x -=tempa->dx;
	  tempa->dx=-tempa->dx;
	}
	if (!(MAP_BORDER+ANIMAL_HEIGHT
	      <=tempa->y.i32[1]<MAP_HEIGHT-MAP_BORDER)) {
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
    Sprite3(map_dc,i,FENCE_HEIGHT,0,$IB,"<9>",BI=9$);
  map_dc->pen_width=1;
  map_dc->color=BROWN;
  for (i=0;i<FENCE_HEIGHT-16;i+=16)
    GrLine(map_dc,FENCE_WIDTH-1,i,FENCE_WIDTH-1,i+7);
  map_dc->color=LTGRAY;
  GrLine(map_dc,FENCE_WIDTH,0,FENCE_WIDTH,FENCE_HEIGHT-6);
  RedrawGate;

  map_dc->pen_width=MAP_BORDER;
  map_dc->color=RED;
  GrBorder(map_dc,MAP_BORDER/2,MAP_BORDER/2,
	MAP_WIDTH-(MAP_BORDER+1)/2,MAP_HEIGHT-(MAP_BORDER+1)/2);

  for (i=MAP_BORDER;i<=MAP_HEIGHT-MAP_BORDER;i++)
    RiverDropsNext(Fs);

  MemSet(a,0,NUM_ANIMALS*sizeof(Animal));
  for (i=0;i<NUM_ANIMALS;i++) {
    a[i].num=i;
    do {
      a[i].x=(64+RandU32%(MAP_WIDTH-128))<<32;
      a[i].y=(64+RandU32%(MAP_WIDTH-128))<<32;
    } while (!CheckMap(a[i].x>>32,a[i].y>>32));
    if (i&1)
      a[i].imgs=cow_imgs;
    else
      a[i].imgs=bull_imgs;
    a[i].frame0=RandU16&3;
    BuddySel(i);
  }
  outside_cnt=NUM_ANIMALS;
  gate_t=0;
  gate_é=0;
  t0=tS;
  tf=0;
}

U0 Init()
{
  RiverNew;
  rd_lock=0;
  QueInit(&rd_head);
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

  PopUpOk(
	"Coral the cattle.  The coral is in the\n"
	"upper-left corner if you scroll.\n\n"
	"Keep holding the $$GREEN$$<CTRL>$$FG$$ key and\n"
	"scroll with $$GREEN$${CTRL-Left Grab}$$FG$$.");

  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL-WIF_FOCUS_TASK_MENU;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,Fs,"Animate",,Fs);
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
             öÿÿÿïÿÿÿ      ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿ    ÿÿÿÿÿÿ ÿÿÿÿÿÿ     ÿÿÿÿÿ       ÿÿ    ÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ   ÿÿ     ÿÿÿÿ     ÿÿÿÿÿ ÿÿ   ÿ    ÿÿÿ     ÿÿÿÿ        ÿÿ     ÿÿÿ        ÿ     ÿÿ     ÿÿ    ÿÿ     ÿ      ÿÿÿÿÿÿÿÿ     ÿÿ ÿ    ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿ         ÿÿÿÿÿÿÿÿ            ÿÿ     ÿÿÿÿÿÿÿ         ÿÿÿ     ÿÿÿÿÿÿÿ            ÿÿÿÿÿÿ    ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             ’     öÿÿÿóÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ ÿÿÿ             ÿÿÿÿ ÿÿ    ÿÿÿ     ÿÿ      ÿÿ         ÿÿÿ ÿÿ       ÿÿÿÿÿÿ ÿÿ  ÿ     ÿ  ÿÿÿÿÿÿ ÿ  ÿ     ÿ  ÿÿÿÿÿ ÿ   ÿ  ÿ  ÿÿ  ÿÿÿÿÿ ÿ  ÿÿ   ÿ  ÿÿ  ÿÿÿÿÿ ÿ   ÿÿÿÿÿÿ  ÿÿ   ÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ         Ú     öÿÿÿïÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿ              ÿÿÿ  ÿÿ    ÿÿ      ÿ       ÿ          ÿÿ  ÿÿ      ÿÿÿÿÿ  ÿÿ  ÿ     ÿ   ÿÿÿÿ  ÿ  ÿ     ÿ  ÿÿÿÿ  ÿ   ÿ  ÿ  ÿÿ  ÿÿÿÿ  ÿ  ÿÿ   ÿ  ÿÿ  ÿÿÿÿ  ÿ   ÿÿÿÿÿÿ  ÿÿ   ÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   	       d         
            ùÿÿÿ   
    úÿÿÿðÿÿÿúÿÿÿ
    ÿÿÿÿðÿÿÿÿÿÿÿ
ðÿÿÿÿÿÿÿÿÿÿÿúÿÿÿ
ÿÿÿÿÿÿÿÿðÿÿÿúÿÿÿ        z     öÿÿÿóÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿ             ÿÿÿ  ÿÿ    ÿÿ      ÿ       ÿ          ÿÿ  ÿÿ       ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿÿ  ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿÿ  ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          z     öÿÿÿóÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿ             ÿÿÿ  ÿÿ    ÿÿ      ÿ       ÿ          ÿÿ  ÿÿ       ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ       ÿÿÿÿÿ  ÿÿ  ÿ        ÿÿÿÿÿ  ÿÿ  ÿÿ   ÿ    ÿÿÿÿÿ  ÿÿ   ÿÿÿÿÿÿ     ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          Ú     öÿÿÿïÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿ              ÿÿÿ  ÿÿ    ÿÿ      ÿ       ÿ          ÿÿ  ÿÿ      ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿÿÿÿÿ  ÿÿ  ÿ   ÿ  ÿ  ÿÿÿÿÿ  ÿÿ   ÿÿÿÿÿ  ÿ   ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          Ú     öÿÿÿïÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿ ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿ              ÿÿÿ  ÿÿ    ÿÿ      ÿ       ÿ          ÿÿ  ÿÿ      ÿÿÿÿÿ  ÿÿ  ÿ     ÿ  ÿÿÿÿÿ  ÿÿ  ÿ       ÿÿÿÿÿ  ÿÿ  ÿÿ       ÿÿÿÿÿ  ÿÿ  ÿÿ   ÿ    ÿÿÿÿÿ  ÿÿ   ÿÿÿÿÿÿ     ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ               ñÿÿÿêÿÿÿ"       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿ   ÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ  ÿ ÿ     ÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿ      ÿÿÿÿÿ    ÿ      ÿÿÿÿÿÿÿ      ÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ         ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿ         ÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ            ÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿ        ÿ     ÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ   ÿÿÿ      ÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿ    ÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿ    ÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ     ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       
       ò      Êÿÿÿõÿÿÿx   D   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   
ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        
 
  

ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ 
 
     
  
       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ 

    

   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      


      

    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                       
    
  
 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                          
        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ
                      
 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ

  
               

ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ         

 
         ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  






  
    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          

ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ


           
    
 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ

 
          
 

      

 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ
  
 
       
      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ
         



               
ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ         
                      ÿÿÿÿÿÿÿÿÿÿÿÿÿ

 
                      


ÿÿÿÿÿÿÿÿÿÿÿ

  
                       
     ÿÿÿÿÿÿÿÿÿÿ 

 
            
   
            ÿÿÿÿÿÿÿÿÿ
 

                          
   ÿÿÿÿÿÿÿÿ  


 


    
  


        ÿÿÿÿÿÿÿ
   


      
 




   


  ÿÿÿÿÿÿ 
    
   
            
  
        ÿÿÿÿÿ
    

  
   

             

    ÿÿÿ
 


 



                 
  ÿÿÿ  
    
  
     

 
  


  ÿÿ
     
         


             ÿ   
                 
          
 
 
                                   

   
                
         

     
 
                

        

   
  
                      

                
     
                          


                                    
                 
    
      	         
                   
                                  
                                                 
  
                          
            
 
      
   
                  
 
      
                                                                                       
 
           	         
        

 


                                        
                   	           

                                                                                          
         
           
                         
  
           
                                                                                                                                                       	                                                                                                                                                                                                                                                                                                                                                 
           	                                                                                                                                                     
                   	                                                                                       	                                                ÿ                                                                            ÿÿÿÿ                               	                                       ÿÿÿÿÿÿ                              	                                          ÿÿÿÿÿÿÿÿÿ                                                                      ÿÿÿÿÿÿÿÿÿÿÿÿÿ                                  	                                   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                     	                                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                                                 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                                                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                                        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                             ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ 