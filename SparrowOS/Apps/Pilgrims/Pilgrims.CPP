/*
Ideas?

How about clouds and rain for crops?

How about achievement of Ben Franklin Almanac.

How about a shipping company office.

How about something like M.U.L.E.

How about turkey shoot.

Witch trials?  Maybe, have God talk?

Seven Cities of Gold or the game, Civilization,
is what's expected but that's dull.

Rivals Spanish and French?

*/


$SP,"<1>",1$



$BG,14$






$SP,"<2>",2$








$SP,"<3>",3$





$SP,"<4>",4$



$SP,"<5>",5$

$BG,9$



 
   $SP,"<6>",6$




$SP,"<7>",7$

$BG$
#define AFT_MAST	16
#define BOW_MAST	14
#define MAST_HEIGHT	20
#define AFT_LEN		36
#define BOW_LEN		43
#define BERTH		28

F64 screen_x,screen_y,d_screen_x,d_screen_y,
   wind_speed,

   //This is on a wave close to ship
   //in world coordinates.
   ship_wave_x,ship_wave_y,

   church_x,church_y,

   wave_speed,wave_é,d_wave_é,wave_period,
   wave_amplitude;

Bool wave_sound,ship_landed;
F64 song_time,game_over_time;

F64 view_scale,new_view_scale;
CDC *master_map;
U8 *vm;
F64 men;

#define NUM_DOLPHINS	5
#define NUM_PIGRAIMS	10
	
class Obj
{
  F64 x,y,radius,
    é,d_é,
    speed;
} ship,dolphins[NUM_DOLPHINS],pilgrims[NUM_PIGRAIMS];

#define MAX_BUBBLES	1024
class Bubble
{
  I32 x,y;
} bubbles[MAX_BUBBLES];

#define MAX_GARDENS	128
class Garden
{
  I32 x,y;
} gardens[MAX_GARDENS];
I64 num_gardens;


F64 WaterHeight(F64 x,F64 y)
{ //Returns wave height at world coordinate pt
  F64 dxn=Cos(wave_é),
      dyn=Sin(wave_é),é;
  é=2*ã*((x-ship_wave_x)*dxn+(y-ship_wave_y)*dyn)/wave_period;
  return wave_amplitude*Cos(é);
}

I64 *ShipMat()
{ //Returns rotation matrix for ship bobbing on waves
  F64 x1,y1,z1,x2,y2,z2,a;
  I64 *result;

  x1=ship.x-AFT_LEN/2*Cos(ship.é)/view_scale;
  y1=ship.y-AFT_LEN/2*Sin(ship.é)/view_scale;
  z1=WaterHeight(x1,y1);
  x2=ship.x+BOW_LEN/2*Cos(ship.é)/view_scale;
  y2=ship.y+BOW_LEN/2*Sin(ship.é)/view_scale;
  z2=WaterHeight(x2,y2);
  a=ASin((z2-z1)/((AFT_LEN/2+BOW_LEN/2)/view_scale));
  result=GrRotY(a);

  z1=WaterHeight(ship.x,ship.y);
  x2=ship.x-BERTH/2*Sin(ship.é)/view_scale;
  y2=ship.y+BERTH/2*Cos(ship.é)/view_scale;
  z2=WaterHeight(x2,y2);
  a=ASin((z2-z1)/(BERTH/view_scale));
  GrRotXEqu(result,a);

  GrRotZEqu(result,ship.é);
  return result;
}

U0 BubbleAdd(I64 x,I64 y)
{
  I64 i=RandU16&(MAX_BUBBLES-1);
  bubbles[i].x=x;
  bubbles[i].y=y;
}

U0 PosChurch(F64 x,F64 y)
{
  I64 i;
  CDC *dc=DCAlias(master_map,Fs);
  dc->color  =ROP_COLLISION;
  dc->bkcolor=BLACK;
  while (x>0) {
    dc->collision_cnt=0;
    CSpritePlot3(dc,x,y,0,$IB,"<2>",2$);
    if (dc->collision_cnt>2000)
      break;
    x--;
  }
  church_x=x; church_y=y;
  DCDel(dc);
  for (i=0;i<NUM_PIGRAIMS;i++) {
    pilgrims[i].x=church_x;
    pilgrims[i].y=church_y;
    pilgrims[i].é=2*ã*Rand;
    pilgrims[i].d_é=0;
    pilgrims[i].speed=10.0;
  }
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,
      w=task->win_pixel_width, cx=w>>1,
      h=task->win_pixel_height,cy=h>>1,
      c=1.05*(MaxI64(cx,cy)+wave_period),
      *ship_r,x,y,z;
  F64 d,xx,yy,dx,dy,dx2,dy2,wave_x,wave_y;
  Bool bow_down;

  task->text_attr=LTBLUE<<4;

  //Draw Bubbles
  dc->color=BLUE;
  for (i=0;i<MAX_BUBBLES;i++)
    GrPlot(dc,bubbles[i].x*view_scale-screen_x,bubbles[i].y*view_scale-screen_y);

  //Draw Waves
  wave_x=ship_wave_x*view_scale-screen_x;
  wave_y=ship_wave_y*view_scale-screen_y;

  dx=wave_period*Cos(wave_é)*view_scale;
  dy=wave_period*Sin(wave_é)*view_scale;
  dx2=-dy*c;
  dy2=dx*c;

  dc->color=BLUE;
  if (Abs(dx)>Abs(dy)) {
    xx=wave_x; yy=wave_y;
    while (xx>-c) {
      GrLine(dc,xx-dx2,yy-dy2,xx+dx2,yy+dy2);
      if (dx>0) { xx-=dx; yy-=dy;
      } else {	  xx+=dx; yy+=dy; }
    }
    xx=wave_x; yy=wave_y;
    while (xx<w+c) {
      GrLine(dc,xx-dx2,yy-dy2,xx+dx2,yy+dy2);
      if (dx<0) { xx-=dx; yy-=dy;
      } else {	  xx+=dx; yy+=dy; }
    }
  } else {
    xx=wave_x; yy=wave_y;
    while (yy>-c) {
      GrLine(dc,xx-dx2,yy-dy2,xx+dx2,yy+dy2);
      if (dy>0) { xx-=dx; yy-=dy;
      } else {	  xx+=dx; yy+=dy; }
    }
    xx=wave_x; yy=wave_y;
    while (yy<h+c) {
      GrLine(dc,xx-dx2,yy-dy2,xx+dx2,yy+dy2);
      if (dy<0) { xx-=dx; yy-=dy;
      } else {	  xx+=dx; yy+=dy; }
    }
  }

  //Draw Dolphins
  for (i=0;i<NUM_DOLPHINS;i++)
    CSpritePlotRotZ3b(dc,dolphins[i].x*view_scale-screen_x,dolphins[i].y*view_scale-screen_y,0,$IB,"<7>",7$,dolphins[i].é);

  //Draw Land
  dc->color=ROP_MONO+YELLOW;
  GrVectMapBlotMP(dc,-screen_x,-screen_y,vm,view_scale);

  //Draw Ship
  dc->color=ROP_EQU;
  ship_r=ShipMat;
  CSpritePlotRotMat3b(dc,ship.x*view_scale-screen_x,ship.y*view_scale-screen_y,0,$IB,"<6>",6$,ship_r);

  //Rudder
  x=-AFT_LEN; y=0; z=0;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  dc->color=BLACK;
  dc->pen_width=2;
  GrLine3(dc,x-screen_x,y-screen_y,0,x-8*Cos(ship.é-2.5*ship.d_é)-screen_x,y-8*Sin(ship.é-2.5*ship.d_é)-screen_y,0);
  dc->pen_width=1;

  //Wake
  x=(-(AFT_LEN+2)+RandU16%13-6)/view_scale; y=(RandU16%13-6)/view_scale; z=0;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x; y+=ship.y;
  BubbleAdd(x,y);
  for (i=0;i<NUM_DOLPHINS;i++) {
    if (!(RandU16&15)) {
      x=dolphins[i].x-(7*Cos(dolphins[i].é)+RandU16%3-1)/view_scale;
      y=dolphins[i].y-(7*Sin(dolphins[i].é)+RandU16%3-1)/view_scale;
      BubbleAdd(x,y);
    }
  }
 

  //Draw Sails
  d=Wrap(ship.é-Wrap(wave_é,ship.é-ã));
  if (Abs(d)<ã)
    d=(Abs(d)-ã)*Sign(-d);
  dx=-1.5*BERTH/2*Sin(ship.é+d/2);
  dy= 1.5*BERTH/2*Cos(ship.é+d/2);

  x=BOW_MAST; y=0; z=0;
  GrRot(ship_r,&x,&y,&z);
  if (z<0)
    bow_down=TRUE;
  else
    bow_down=FALSE;

  //Aft Sail
  x=-AFT_MAST; y=0; z=0;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  dc->color=BLACK;
  GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);

  xx=x-screen_x; yy=y-screen_y;

  if (!bow_down) {
    x=-AFT_MAST; y=0; z=MAST_HEIGHT;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    dc->color=DKGRAY;
    dc->pen_width=4;
    GrLine3(dc,x-screen_x,y-screen_y,0,xx,yy,0);
  }

  dc->pen_width=2;
  for (d=0;d<MAST_HEIGHT;d+=0.5) {
    x=-AFT_MAST; y=0; z=d;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    x-=1.5*Cos(wave_é); y-=1.5*Sin(wave_é);
    dc->color=WHITE;
    GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);
  }
  dc->pen_width=1;

  x=-AFT_MAST; y=0; z=MAST_HEIGHT;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  dc->color=BLACK;
  GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);

  if (bow_down) {
    x=-AFT_MAST; y=0; z=MAST_HEIGHT;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    dc->color=DKGRAY;
    dc->pen_width=4;
    GrLine3(dc,x-screen_x,y-screen_y,0,xx,yy,0);
  }

  dc->color=BLACK;
  dc->pen_width=1;
  GrCircle3(dc,x-screen_x,y-screen_y,0,2);

  //Bow Sail
  x=BOW_MAST; y=0; z=0;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  dc->color=BLACK;
  GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);

  xx=x-screen_x; yy=y-screen_y;

  if (!bow_down) {
    x=BOW_MAST; y=0; z=MAST_HEIGHT;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    dc->color=DKGRAY;
    dc->pen_width=4;
    GrLine3(dc,x-screen_x,y-screen_y,0,xx,yy,0);
  }

  dc->pen_width=2;
  for (d=0;d<MAST_HEIGHT;d+=0.5) {
    x=BOW_MAST; y=0; z=d;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    x-=1.5*Cos(wave_é); y-=1.5*Sin(wave_é);
    dc->color=WHITE;
    GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);
  }
  dc->pen_width=1;

  x=BOW_MAST; y=0; z=MAST_HEIGHT;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  dc->color=BLACK;
  GrLine3(dc,x-dx-screen_x,y-dy-screen_y,0,x+dx-screen_x,y+dy-screen_y,0);

  if (bow_down) {
    x=BOW_MAST; y=0; z=MAST_HEIGHT;
    GrRot(ship_r,&x,&y,&z);
    x+=ship.x*view_scale; y+=ship.y*view_scale;
    dc->color=DKGRAY;
    dc->pen_width=4;
    GrLine3(dc,x-screen_x,y-screen_y,0,xx,yy,0);
  }

  dc->color=BLACK;
  dc->pen_width=1;
  GrCircle3(dc,x-screen_x,y-screen_y,0,2);

  dc->color=BLACK;
  GrPrintF(dc,0,0,"Speed:%5.1f Knots",ship.speed);
  GrPrintF(dc,0,FONT_HEIGHT,"Men:%f",men);

  //Check for wreck
  x=BOW_LEN; y=0; z=0;
  GrRot(ship_r,&x,&y,&z);
  x+=ship.x*view_scale; y+=ship.y*view_scale;
  if (!ship_landed && GrPeek(dc,x-screen_x,y-screen_y)==YELLOW) {
    Noise(500,500,1000);
    song_time	  =tNP+0.5;
    game_over_time=tNP+1.0;
    wave_sound=FALSE;
    ship_landed=TRUE;
    PosChurch(x/view_scale,y/view_scale);
  }

  if (ship_landed) {
    //Draw Gardens
    for (i=0;i<MAX_GARDENS;i++)
      CSpritePlot3(dc,gardens[i].x*view_scale-screen_x,gardens[i].y*view_scale-screen_y,0,$IB,"<5>",5$);

    //Draw Church
    CSpritePlot3(dc,church_x*view_scale-screen_x,church_y*view_scale-screen_y,0,$IB,"<2>",2$);
    //Draw Pilgrims
    for (i=0;i<NUM_PIGRAIMS;i++)
      CSpritePlot3(dc,pilgrims[i].x*view_scale-screen_x,pilgrims[i].y*view_scale-screen_y,0,(i&1)?$IB,"<3>",3$:$IB,"<4>",4$);
    if (tNP>game_over_time && Blink(,tP(task))) {
      dc->color=LTRED;
      GrPrintF(dc,(task->win_pixel_width-14*FONT_WIDTH)/2,
	  (task->win_pixel_height-FONT_HEIGHT)/2,"Game Completed");
    }
  }
  Free(ship_r);
}


CColorROPU32 pray_colors[1]={RED};
CTask *pray_task;

U0 LeftClickPrayButton(CCtrl *,I64,I64,Bool down)
{ //$LK,"::/SparrowOS/Apps/AfterEgypt/AENotes.TXT.Z","FI:::/SparrowOS/Apps/AfterEgypt/AENotes.TXT.Z"$
  I64 i,size,l=512;
  U8 *bible_filename,*buf;
  if (down && !TaskValidate(pray_task)) {
    bible_filename=Define("ST_BIBLE_FILE");
    if (buf=FileRead(bible_filename,&size)) {
      i=GetTimeStamp>>4%(size-l);
      buf[i+l]=0;
      pray_task=PopUpViewStr(buf+i);
      Free(buf);
    }
  }
}

U0 Init()
{
  I64 i;
  Fs->draw_it=NULL;
  Kill(Fs->song_task);
  Fs->song_task=NULL;
  WinMgrSync;

  men=102.0;
  pray_task=NULL;

  screen_x=400;   d_screen_x=0;
  screen_y=500;   d_screen_y=0;
  view_scale=1.0; new_view_scale=1.0;
  MemSetU32(&bubbles,MAX_I32,sizeof(bubbles)/sizeof(U32));

  num_gardens=0;
  MemSetU32(&gardens,MAX_I32,sizeof(gardens)/sizeof(U32));

  MemSet(&dolphins,0,sizeof(dolphins));
  for (i=0;i<NUM_DOLPHINS;i++) {
    dolphins[i].radius =5;
    dolphins[i].x	   =screen_x+Rand*(GR_WIDTH-2*dolphins[i].radius)+dolphins[i].radius;
    dolphins[i].y	   =screen_y+Rand*(GR_WIDTH-2*dolphins[i].radius)+dolphins[i].radius;
    dolphins[i].é  =Rand*2*ã;
    dolphins[i].d_é=0;
    dolphins[i].speed  =15;
  }

  MemSet(&ship,0,sizeof(ship));
  ship.radius =MaxI64(AFT_LEN,BOW_LEN);
  ship.x	    =screen_x+GR_WIDTH/2-ship.radius;
  ship.y	    =screen_y+GR_HEIGHT/2;
  ship.é  =ã;
  ship.d_é=0;
  ship.speed  =1.0;
  ship_wave_x=0; ship_wave_y=0;

  wave_é=Rand*ã*2;
  d_wave_é=0;
  wave_amplitude=10;
  wave_period=75;
  wave_speed=20*2*ã/wave_period;
  wind_speed=25;
  wave_sound=TRUE;
  ship_landed=FALSE;
  song_time=0;
  game_over_time=0;
  Fs->draw_it=&DrawIt;
}


U0 AnimateObj(Obj *tempo,F64 dt)
{
  tempo->x+=tempo->speed*dt*Cos(tempo->é);
  tempo->y+=tempo->speed*dt*Sin(tempo->é);
  tempo->é=Wrap(tempo->é+tempo->d_é*dt);
  tempo->d_é=LowPass1(0.5,tempo->d_é,0,dt);
}

U0 SongTask(I64)
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  Play("M3/4");
  Play("hRW13qDhG4eB3G");
  Play("4hBqA3hGqE");
  Play("hDqDhG4eB3G");
  Play("4hBqAh.DhD");
  Fs->parent_task->song_task=NULL;
}

#define SCREEN_BUMP	100

U0 AnimateTask(I64)
{
  I64 i,j,
     w=Fs->parent_task->win_pixel_width,  cx=w/2,
     h=Fs->parent_task->win_pixel_height, cy=h/2;
  F64 die_timer=0,d,dx,dy,dx2,dy2,dxn,dyn,t0=tP(Fs->parent_task),dt,old_view_scale;

  while (TRUE) {
    if (TaskValidate(pray_task)) {
      while (TaskValidate(pray_task))
	WinMgrSync;
      pray_task=NULL;
      t0=tP(Fs->parent_task);
    }

    dt=tP(Fs->parent_task)-t0;
    t0=tP(Fs->parent_task);

    for (die_timer+=dt;die_timer>0.1;die_timer-=0.1)
      if (Rand<0.05)
	men*=Exp(-0.05*(Rand+0.05)`20);

    old_view_scale=view_scale;
    view_scale=Exp(0.5*dt*(Ln(new_view_scale)-Ln(view_scale))+Ln(view_scale));
    screen_x=(screen_x+cx)*view_scale/old_view_scale-cx;
    screen_y=(screen_y+cy)*view_scale/old_view_scale-cy;

    for (i=0;i<NUM_DOLPHINS;i++) {
      dolphins[i].d_é+=10.0*(Rand-0.5)*dt;
      AnimateObj(&dolphins[i],dt);
      if (dolphins[i].x-screen_x<dolphins[i].radius)
	dolphins[i].x+=(w-2*dolphins[i].radius)/view_scale;
      if (dolphins[i].x-screen_x>w-1-dolphins[i].radius)
	dolphins[i].x-=(w-2*dolphins[i].radius)/view_scale;
      if (dolphins[i].y-screen_y<dolphins[i].radius)
	dolphins[i].y+=(h-2*dolphins[i].radius)/view_scale;
      if (dolphins[i].y-screen_y>h-1-dolphins[i].radius)
	dolphins[i].y-=(h-2*dolphins[i].radius)/view_scale;
    }

    if (ship_landed) {
      for (i=0;i<NUM_PIGRAIMS;i++) {
	pilgrims[i].d_é+=10.0*(Rand-0.5)*dt;
	AnimateObj(&pilgrims[i],dt);
	if (GrPeek(master_map,pilgrims[i].x,pilgrims[i].y)!=1) {
	  pilgrims[i].speed=-pilgrims[i].speed;
	  AnimateObj(&pilgrims[i],dt);
	} else if (!(ToI64(pilgrims[i].x)&15) && !(ToI64(pilgrims[i].y)&15)) {
	  for (j=0;j<MAX_GARDENS;j++)
	    if (gardens[j].x==ToI64(pilgrims[i].x)&&
		gardens[j].y==ToI64(pilgrims[i].y))
	      break;
	  if (j==MAX_GARDENS) {
	    gardens[num_gardens  &(MAX_GARDENS-1)].x=pilgrims[i].x;
	    gardens[num_gardens++&(MAX_GARDENS-1)].y=pilgrims[i].y;
	  }
	}
      }
      if (church_x*view_scale-screen_x-0.9*d_screen_x<w/4)
	d_screen_x-=SCREEN_BUMP;
      if (church_x*view_scale-screen_x-0.9*d_screen_x>w-1-w/4)
	d_screen_x+=SCREEN_BUMP;
      if (church_y*view_scale-screen_y-0.9*d_screen_y<h/4)
	d_screen_y-=SCREEN_BUMP;
      if (church_y*view_scale-screen_y-0.9*d_screen_y>h-1-h/4)
	d_screen_y+=SCREEN_BUMP;
    } else {
      d=Wrap(ship.é-Wrap(wave_é,ship.é-ã));
      if (Abs(d)<ã)
	d=(Abs(d)-ã)*Sign(-d);
      d=wind_speed*Cos(d/2);
      ship.speed=LowPass1(0.25,ship.speed,d,dt);
      AnimateObj(&ship,dt);
      if (ship.x*view_scale-screen_x-0.9*d_screen_x<w/4)
	d_screen_x-=SCREEN_BUMP;
      if (ship.x*view_scale-screen_x-0.9*d_screen_x>w-1-w/4)
	d_screen_x+=SCREEN_BUMP;
      if (ship.y*view_scale-screen_y-0.9*d_screen_y<h/4)
	d_screen_y-=SCREEN_BUMP;
      if (ship.y*view_scale-screen_y-0.9*d_screen_y>h-1-h/4)
	d_screen_y+=SCREEN_BUMP;
      if (ship.x<0) ship.x=0;
      if (ship.y<0) ship.y=0;
    }

    if (screen_x<0) {
      screen_x=0;
      d_screen_x=0;
    }
    if (screen_y<0) {
      screen_y=0;
      d_screen_x=0;
    }

    wave_é=Wrap(wave_é+d_wave_é*dt);
    d_wave_é=Limit(LowPass1(1.0,d_wave_é,
	d_wave_é+1.0*(Rand-0.5),dt),-0.05,0.05);

    screen_x+=0.9*d_screen_x*dt;
    d_screen_x-=0.9*d_screen_x*dt;
    screen_y+=0.9*d_screen_y*dt;
    d_screen_y-=0.9*d_screen_y*dt;

    dxn=Cos(wave_é);
    dyn=Sin(wave_é);
    dx=wave_period*dxn;
    dy=wave_period*dyn;
    ship_wave_x-=wave_speed*dt/(2*ã)*dx;
    ship_wave_y-=wave_speed*dt/(2*ã)*dy;
    if (Abs(dx)>Abs(dy)) {
      i=(ship_wave_x-ship.x)/dx;
      ship_wave_x-=i*dx;
      ship_wave_y-=i*dy;
      dx2=ship_wave_x-ship.x;
      dy2=ship_wave_y-ship.y;
      d=dyn*dx2-dxn*dy2;
      ship_wave_x-=dyn*d;
      ship_wave_y+=dxn*d;
    } else {
      i=(ship_wave_y-ship.y)/dy;
      ship_wave_x-=i*dx;
      ship_wave_y-=i*dy;
      dx2=ship_wave_x-ship.x;
      dy2=ship_wave_y-ship.y;
      d=dyn*dx2-dxn*dy2;
      ship_wave_x-=dyn*d;
      ship_wave_y+=dxn*d;
    }

    if (wave_sound)
      Snd(200+8*WaterHeight(ship.x,ship.y));
    else if (song_time && tNP>song_time) {
      Kill(Fs->parent_task->song_task);
      Fs->parent_task->song_task=Spawn(&SongTask,NULL,"Song",Fs->parent_task);
      song_time=0;
    }
    WinMgrSync;
  }
}

U0 Pilgrims()
{
  I64 msg_code,ch,sc;
  CCtrl *bt_pray;

  "Initializing Maps...\n";
  master_map=GRARead("NewEngland"); //See $LK,"Acknowledgements","FF:::/SparrowOS/Doc/Acknowledgements.TXT.Z,US Geological"$ (7).

  vm=DC2VectMap(master_map);
  PopUpOk("This is about the pilgrims who fled\n"
	"religious persecution in England.\n"
	"They sailed in 1620 on the Mayflower\n"
	"and landed at Plymouth, Massachusetts.\n"
	"\n"
	"This game is comic, mostly.  Be sure\n"
	"to try 'z' or 'Z' for zooming.\n");
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$

  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Restart(,CH_NEW_LINE);"
  "  Pray(,'p');"
  "  Port(,,SC_CURSOR_LEFT);"
  "  Starboard(,,SC_CURSOR_RIGHT);"
  "}"
  "View {"
  "  ZoomIn(,'z');"
  "  ZoomOut(,'Z');"
      "}"
      );

  WordStat;
  Preempt(OFF);

  bt_pray=CtrlButtonNew(0,2.5*FONT_HEIGHT,,,,"Pray\0",pray_colors),
  bt_pray->left_click=&LeftClickPrayButton;

  WinMax;
  DocCursor;
  DocClear;
  Fs->song_task=NULL;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",Fs);
  try {
    while (TRUE) {
      switch (msg_code=GetMsg(&ch,&sc,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_KEY_DOWN:
	  switch (ch) {
	    case 0:
	      switch (sc.u8[0]) {
		case SC_CURSOR_LEFT:
		  ship.d_é=Limit(ship.d_é-5*ã/180,-.5,.5);
		  break;
		case SC_CURSOR_RIGHT:
		  ship.d_é=Limit(ship.d_é+5*ã/180,-.5,.5);
		  break;
	      }
	      break;
	    case 'p':
	      LeftClickPrayButton(NULL,0,0,TRUE);
	      break;
	    case 'z':
	      if (new_view_scale<10.0)
		new_view_scale*=1.5;
	      break;
	    case 'Z':
	      if (new_view_scale>0.1)
		new_view_scale/=1.5;
	      break;
	    case CH_NEW_LINE:
	      Init;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto pilgrim_done;
	  }
	  break;
      }
    }
pilgrim_done:
  } catch
    CatchAll;
  MenuPop;
  SettingsPop;
  CtrlButtonDel(bt_pray);
  GrVectMapDel(vm);
  DCDel(master_map);
}
                X   #   Õÿÿÿðÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                                    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿ          ÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿ              ÿ ÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                   
   øÿÿÿúÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿ      ÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿ      ÿÿÿ         ÿÿ      ÿÿ    ÿÿ      ÿÿ       ÿ      ÿÿ   ÿÿÿÿÿ     ÿÿ      ÿÿ ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                 B     /   !      êÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ         ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ              ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿ         ÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ         ÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ         ÿÿÿÿ     ÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿ                           ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ                            ÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ                             ÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ                             ÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ                               ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ                               ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ                ÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ                    ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ         ÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿ ÿÿÿÿ   ÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ          ÿÿÿ ÿ   ÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ               ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ             J     1   A   õÿÿÿ½ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿ         ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿ  ÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿ    ÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿ  ÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ       ÿÿÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿÿÿ     ÿ       ÿÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿ    ÿ       ÿÿÿÿÿÿ     ÿ       ÿÿÿÿÿ    ÿ       ÿÿÿÿ    ÿ       ÿÿÿ    ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ   ÿ       ÿÿÿ            ÿ       ÿÿÿ            ÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ            ÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ                ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ                    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ                  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ                         ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿ                 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                  "        !   øÿÿÿâÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ     ÿÿÿÿ ÿÿÿÿÿ    ÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿ ÿÿÿÿ     ÿÿÿÿÿ ÿÿ           ÿÿ ÿÿÿÿ  ÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿÿ ÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿ        ÿÿÿÿ ÿÿ         ÿÿÿ  ÿÿ  ÿ     ÿÿ  ÿÿ  ÿ     ÿÿ  ÿÿ  ÿ      ÿ  ÿÿ  ÿ      ÿ  ÿ   ÿ         ÿ  ÿÿ    ÿ    ÿ  ÿÿ    ÿ    ÿ  ÿÿ    ÿÿ   ÿ  ÿÿ      ÿÿÿ  ÿ  ÿÿ      ÿÿÿ  ÿÿÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿÿ  ÿÿ   ÿÿ  ÿÿÿÿÿ  ÿÿÿ  ÿÿ  ÿÿÿÿÿ  ÿÿÿ  ÿÿ  ÿÿÿÿÿ  ÿÿÿ  ÿÿ  ÿÿÿÿÿ  ÿÿÿ  ÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿ                         øÿÿÿâÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ   ÿÿÿÿÿ   ÿÿÿÿÿ   ÿÿÿ   ÿÿÿÿ ÿ    ÿÿÿ   ÿÿÿÿ   ÿÿÿÿ   ÿÿÿÿ  ÿÿÿÿ   ÿÿÿÿÿ  ÿÿÿÿ   ÿÿÿÿÿ  ÿÿÿÿ   ÿÿÿ      ÿÿ   ÿÿ     ÿ   ÿ      ÿ   ÿ      ÿ   ÿ     ÿ    ÿ   ÿ    ÿ  ÿ ÿ    ÿÿ  ÿ ÿ    ÿÿ  ÿ ÿ    ÿÿ  ÿ ÿ    ÿÿ  ÿ      ÿÿ  ÿÿÿ    ÿÿ  ÿÿÿ     ÿ   ÿÿÿ   ÿÿÿ   ÿÿ   ÿÿÿ   ÿÿ   ÿÿÿ   ÿÿ   ÿÿ   ÿ   ÿÿ   ÿ   ÿÿ   ÿ   ÿÿ              ÿÿÿÿÿ ÿÿÿÿ ÿÿ   ÿÿÿÿ   ÿÿ   ÿ              ²           ôÿÿÿúÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ                    ÿÿÿÿÿÿÿÿÿÿÿ  




 ÿ       ÿÿÿÿÿÿÿÿÿÿ  




 ÿÿ       ÿÿÿÿÿÿÿÿÿ  




 ÿÿÿ       ÿÿÿÿÿÿÿÿ  




 ÿÿÿÿ       ÿÿÿÿÿÿÿ  



 ÿÿÿÿÿ       ÿÿÿÿÿÿ           ÿÿÿÿÿÿ       ÿÿÿÿÿ   ÿÿÿÿÿÿÿ       ÿÿÿÿ  ÿÿÿÿÿÿÿÿ       ÿÿÿ   ÿÿÿÿÿÿÿÿÿ       ÿÿ  ÿÿÿÿÿÿÿÿÿÿ       ÿ   ÿÿÿÿÿÿÿÿÿÿÿ                    ÿÿÿÿÿÿÿÿÿÿÿÿ        