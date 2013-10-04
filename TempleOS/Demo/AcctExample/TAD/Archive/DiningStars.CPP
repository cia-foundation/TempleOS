//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.


AcctRegSetDftEntry("TempleOS/DiningStars","F64 best_score=9999;\n");
AcctRegExeBranch("TempleOS/DiningStars");

#define PERSON_SCALE	6.0










		$SP,"<1>",1$









		$SP,"<2>",2$








		$SP,"<3>",3$






#define PLANT_SCALE	6.0



		     $SP,"<4>",4$














		      $SP,"<5>",5$







class Wall
{
  Wall *next,*last;
  CD3I32 p1,p2;
  I64 cpu_num;
} walls_root;




#define SCREEN_SCALE		1024
#define PLOT_GRID_WIDTH		24
#define PLOT_GRID_HEIGHT	24
#define MAN_HEIGHT		300
#define WALL_HEIGHT		1.0
#define BLUEPRINT_SCALE		4

CDC *blueprint_dc;
I64 blueprint_width,blueprint_height;
U8 *blueprint=NULL,
   *panels_processed=NULL;

I64 man_xx,man_yy;
F64 man_й;

F64 t0,tf;

#define NUM_PERSONS	16
class Person
{
  I64 x,y;
} persons[NUM_PERSONS];

U0 DSTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
{
  I64 zz;
  GrRot(dc->r,x,y,z);
  zz=SCREEN_SCALE/3+*z;
  if (zz<1) zz=1;
  *x=SCREEN_SCALE/2* *x/zz;
  *y=SCREEN_SCALE/2* (*y+MAN_HEIGHT)/zz;
  *x+=dc->x;
  *y+=dc->y;
  *z+=dc->z;
}

public U0 DSLighting(CDC *dc,CD3I32 *p1,CD3I32 *p2,CD3I32 *p3,CColorROPU32 color)
{
  CD3I32 v1,v2;
  I64 i,vn_x,vn_y,vn_z;
  F64 d;

  v1.x=p1->x-p2->x;
  v1.y=p1->y-p2->y;
  v1.z=p1->z-p2->z;

  v2.x=p3->x-p2->x;
  v2.y=p3->y-p2->y;
  v2.z=p3->z-p2->z;

//V1 and V2 are vects along two sides
//of the tri joined at p2.

  vn_x=v1.y*v2.z-v1.z*v2.y;
  vn_y=v1.z*v2.x-v1.x*v2.z;
  vn_z=v1.x*v2.y-v1.y*v2.x;
  if (d=Sqrt(SqrI64(vn_x)+SqrI64(vn_y)+SqrI64(vn_z)))
    d=1<<16/d;
  vn_x*=d;
  vn_y*=d;
  vn_z*=d;
//Vn is the cross product of V1 and V3
//which means it is perpendicular.  It
//is the normal vect to the surface.
//It has been scaled to length 65536.

//Light source has been scaled to length 65536.
  i=(vn_x*dc->ls.x+vn_y*dc->ls.y+vn_z*dc->ls.z)>>16;

  i=(0x8000+i)>>1;//Tables are too dark ****TODO****

//The dot product of the light source
//vect and the surface normal
//gives an illumination number.
//65536*65536>>16=65536

//TempleOS will generate a random U16
//and compare to dither_probability and
//will pick from two colors.
//Probability dithering does not work with pen_width>1 at this time.
  if (color.c0.rop&ROPBF_TWO_SIDED) {
    color.c0.rop&=~ROPBF_TWO_SIDED;
    i=AbsI64(i)<<1;
  } else
    i+=65536;
  if (color.c0.rop&ROPBF_HALF_RANGE_COLOR) {
    color.c0.rop&=~ROPBF_HALF_RANGE_COLOR;
    i>>=1;
    if (color>=8) {
      color-=8;
      i+=65536;
    }
  }
  if (i<65536) {
    dc->color=ROPF_PROBABILITY_DITHER+color<<16+BLACK;
    dc->dither_probability=i;
  } else {
    dc->color=ROPF_PROBABILITY_DITHER+(color^8)<<16+color;
    dc->dither_probability=i-65536;
  }
}

#define LOS_SCALE	4

Bool LOSPlot(U8 *,I64 x,I64 y,I64)
{
  if (blueprint[(y/LOS_SCALE)*blueprint_width+(x/LOS_SCALE)]==WHITE)
    return FALSE;
  else
    return TRUE;
}

Bool LOS(I64 x1,I64 y1,I64 x2,I64 y2)
{ //Line of sight
  return LOSLine(NULL,x1*LOS_SCALE/SCREEN_SCALE,y1*LOS_SCALE/SCREEN_SCALE,0,
	       x2*LOS_SCALE/SCREEN_SCALE,y2*LOS_SCALE/SCREEN_SCALE,0,&LOSPlot);
}

I64 mp_not_done_flags;
I64 *r1,*r2,*r3,*s2w,cx,cy,xh,yh,zh,x1h,y1h;

U0 MPDrawWalls(CDC *_dc)
{
  CD3I32 poly[4];
  I64 i,j,n,cpu_num=Gs->num,xx,yy,x,y,x1,y1,z1,x2,y2,_x1h,_y1h,dx,dy,xx1,yy1,xx2,yy2,
	c,x1w,y1w,*_r3=MAllocIdentical(r3);
  Wall *tempw;
  CDC *dc=MAllocIdentical(_dc);
  dc->r=MAllocIdentical(_dc->r);
  _x1h=x1h-xh*cpu_num;
  _y1h=y1h-yh*cpu_num;
  for (j=cpu_num;j<PLOT_GRID_HEIGHT*2;j+=mp_cnt) {
    x1w=_x1h;
    y1w=_y1h;
    for (i=0;i<PLOT_GRID_WIDTH*4;i++) {
      xx=x1w/SCREEN_SCALE; yy=y1w/SCREEN_SCALE;
      x=xx*SCREEN_SCALE-man_xx; y=yy*SCREEN_SCALE-man_yy;
      if (1<=xx<blueprint_width-1 && 1<=yy<blueprint_height-1 &&
	    !LBts(panels_processed,yy*blueprint_width+xx)) {
	if ((c=blueprint[yy*blueprint_width+xx]) &&
	      LOS(xx*SCREEN_SCALE+SCREEN_SCALE/2,yy*SCREEN_SCALE+SCREEN_SCALE/2,
	      man_xx,man_yy)) {
	  if (c==WHITE||c==LTBLUE)
	    dc->color=BROWN;
	  else
	    dc->color=c;
	  poly[0].x=x;
	  poly[0].y=y;
	  poly[0].z=0;
	  poly[1].x=x+SCREEN_SCALE;
	  poly[1].y=y;
	  poly[1].z=0;
	  poly[2].x=x+SCREEN_SCALE;
	  poly[2].y=y+SCREEN_SCALE;
	  poly[2].z=0;
	  poly[3].x=x;
	  poly[3].y=y+SCREEN_SCALE;
	  poly[3].z=0;
	  GrFillPoly3(dc,4,poly);
	  if (!((yy*blueprint_width+xx)%3)) {
	    if (c==GREEN) {
	      x1=x+SCREEN_SCALE/2;
	      y1=y+SCREEN_SCALE/2;
	      z1=0;
	      GrTransform(dc,&x1,&y1,&z1);
	      if (z1>0)
		SpritePlotRotMat3B(dc,x+SCREEN_SCALE/2,y+SCREEN_SCALE/2,0,$IB,"<4>",4$,_r3);
	    } else if (c==BROWN) {
	      x1=x+SCREEN_SCALE/2;
	      y1=y+SCREEN_SCALE/2;
	      z1=0;
	      GrTransform(dc,&x1,&y1,&z1);
	      if (z1>0)
		SpritePlotRotMat3B(dc,x+SCREEN_SCALE/2,y+SCREEN_SCALE/2,0,$IB,"<5>",5$,_r3);
	    }
	  }
	}
      }
      x1w-=yh;
      y1w+=xh;
    }
    _x1h-=xh*mp_cnt;
    _y1h-=yh*mp_cnt;
  }

  tempw=walls_root.next;
  while (tempw!=&walls_root) {
    if (tempw->cpu_num==cpu_num) {
      dc->color=YELLOW;
      x1=tempw->p1.x*SCREEN_SCALE/BLUEPRINT_SCALE-man_xx;
      y1=tempw->p1.y*SCREEN_SCALE/BLUEPRINT_SCALE-man_yy;
      x2=tempw->p2.x*SCREEN_SCALE/BLUEPRINT_SCALE-man_xx;
      y2=tempw->p2.y*SCREEN_SCALE/BLUEPRINT_SCALE-man_yy;

      //The foreshortening distorts Z coordinates in a funny way,
      //so we do the wall in tiny pieces.
      n=2*(AbsI64(x2-x1)+AbsI64(y2-y1))/SCREEN_SCALE;
      if (n<1) n=1;
      dx=(x2-x1)<<16/n;
      dy=(y2-y1)<<16/n;
      xx1=x1<<16;
      yy1=y1<<16;
      for (i=0;i<n;i++) {
	xx2=xx1+dx;
	yy2=yy1+dy;
	poly[0].x=xx1>>16;
	poly[0].y=yy1>>16;
	poly[0].z=0;
	poly[1].x=xx2>>16;
	poly[1].y=yy2>>16;
	poly[1].z=0;
	poly[2].x=xx2>>16;
	poly[2].y=yy2>>16;
	poly[2].z=WALL_HEIGHT*SCREEN_SCALE;
	poly[3].x=xx1>>16;
	poly[3].y=yy1>>16;
	poly[3].z=WALL_HEIGHT*SCREEN_SCALE;
	GrFillPoly3(dc,4,poly);
	xx1=xx2; yy1=yy2;
      }
      dc->color=BLACK;
      dc->pen_width=1;
      GrLine3(dc,x1,y1,0,x1,y1,WALL_HEIGHT*SCREEN_SCALE);
      GrLine3(dc,x2,y2,0,x2,y2,WALL_HEIGHT*SCREEN_SCALE);
    }
    tempw=tempw->next;
  }
  Free(_r3);
  Free(dc->r);
  Free(dc);
  LBtr(&mp_not_done_flags,cpu_num);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,xx,yy,zz,x,y;
  U8 *temps;
  F64 tt;

  cx=task->win_pixel_width/2;
  cy=task->win_pixel_height/2;

  DCAllocDepthBuf(dc);
  MemSet(panels_processed,0,(blueprint_width*blueprint_height+7)>>3);

  //World to screen
  GrRotZEqu(dc->r,man_й+г/2);
  GrRotXEqu(dc->r,г/2);
  GrSetRotMat(dc,dc->r);

  xh=-man_xx/SCREEN_SCALE; yh=-man_yy/SCREEN_SCALE; zh=0;
  GrRot(dc->r,&xh,&yh,&zh);
  GrSetTranslation(dc->r,xh,yh,zh);

  //Screen to world
  s2w=GrRotX(-г/2,task);
  GrRotZEqu(s2w,-man_й-г/2);
  xh=0; yh=0; zh=SCREEN_SCALE;
  GrRot(s2w,&xh,&yh,&zh);

  //Rotate light source
  xx=dc->ls.x; yy=dc->ls.y; zz=-dc->ls.z;
  (*dc->transform)(dc,&xx,&yy,&zz);
  dc->ls.x=xx; dc->ls.y=yy; dc->ls.z=zz;

  dc->flags|=DCF_TRANSFORMATION;
  dc->transform=&DSTransform;
  dc->lighting=&DSLighting;
  dc->x=cx;
  dc->y=cy;
  r1=GrRotX(-г/2,task);
  GrRotZEqu(r1,tT);
  GrScaleMatEqu(r1,PERSON_SCALE);

  r2=GrIdent(task);
  GrScaleMatEqu(r2,PERSON_SCALE);

  r3=GrRotX(-г/2,task);
  GrScaleMatEqu(r3,PLANT_SCALE);

  x1h=man_xx+yh*PLOT_GRID_WIDTH/2+xh*(PLOT_GRID_HEIGHT-1);
  y1h=man_yy-xh*PLOT_GRID_WIDTH/2+yh*(PLOT_GRID_HEIGHT-1);
  xh>>=1; yh>>=1;

  mp_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    JobQue(&MPDrawWalls,dc,i);
  while (mp_not_done_flags)
    Yield;

    //Draw Persons
  for (i=0;i<NUM_PERSONS;i++) {
    x=persons[i].x;
    y=persons[i].y;
    if (LOS(x,y,man_xx,man_yy)) {
      x-=man_xx;
      y-=man_yy;
      xx=x;
      yy=y;
      zz=0;
      GrTransform(dc,&xx,&yy,&zz);
      if (zz>0) {
	tt=2*Tri(tT,1.0);
	if (tt<1.0)
	  temps=SpriteInterpolate($IB,"<2>",2$,$IB,"<1>",1$,tt);
	else
	  temps=SpriteInterpolate($IB,"<1>",1$,$IB,"<3>",3$,tt-1.0);
	SpritePlotRotMat3B(dc,x,y,0,temps,r1);
	Free(temps);
      }
    }
  }
  Free(r1);
  Free(r2);
  Free(r3);

  //Draw BluePrint heads-up display, scaled 2 pixels
  Free(dc->r);
  GrSetRotMat(dc,GrIdent(task));
  dc->x=task->win_pixel_width -2*blueprint_width;
  dc->y=task->win_pixel_height-2*blueprint_height;
  dc->z=0;
  dc->transform=&GrTransform;
  dc->pen_width=2;
  for (i=0;i<blueprint_height;i++)
    for (j=0;j<blueprint_width;j++) {
      dc->color=blueprint[(blueprint_height-1-i)*blueprint_width+j];
      GrPlot3(dc,2*j,2*i,0);
    }

    //Draw Things on heads-up BluePrint
  dc->color=LTPURPLE;
  for (i=0;i<NUM_PERSONS;i++)
    GrPlot3(dc,2*(persons[i].x/SCREEN_SCALE),2*(blueprint_height-1-persons[i].y/SCREEN_SCALE),0);
  dc->color=LTCYAN;
  GrPlot3(dc,2*(man_xx/SCREEN_SCALE),2*(blueprint_height-1-man_yy/SCREEN_SCALE),0);

  dc->color=LTGREEN;
  GrLine(dc,cx-5,cy,cx+5,cy);
  GrLine(dc,cx,cy-5,cx,cy+5);
  Free(s2w);
}

U0 BluePrintDrawIt(CTask *task,CDC *dc)
{
  task->text_attr=BLUE<<4+WHITE;
  GrBlot(dc,(task->win_pixel_width-blueprint_dc->width)>>1,
	    (task->win_pixel_height-blueprint_dc->height)>>1,blueprint_dc);
}

#define MAN_START_X	0
#define MAN_START_Y	1

U0 MakeBluePrint()
{
  I64 msg_code,i,x1,y1,x2,y2,p1,p2,color=WHITE,cpu_num=0;
  Wall *tempw;

  DCFill(blueprint_dc,LTBLUE);

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->win_inhibit=WIF_ALL-WIF_BORDER;
  DocClear;

"Draw Floor Plan:

Walls  in WHITE
Tables in BROWN
Plants in GREEN
";

  WinMax;
  WordStat;
  Fs->draw_it=&BluePrintDrawIt;

  do {
    msg_code=GetMsg(&p1,&p2,
	  1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+
	  1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_IP_R_UP:
	i=PopUpColor;
	if (i>=0) color=i;
	break;
      case MSG_IP_L_DOWN:
	if (color==WHITE) {
	  p1-=(Fs->win_pixel_width-blueprint_dc->width)>>1;
	  p2-=(Fs->win_pixel_height-blueprint_dc->height)>>1;
	  x1=p1; y1=p2;
	  x2=p1; y2=p2;
	  blueprint_dc->color=ROP_XOR+color^LTBLUE;
	  while (msg_code!=MSG_IP_L_UP) {
	    GrLine(blueprint_dc,x1,y1,x2,y2);
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    p1-=(Fs->win_pixel_width-blueprint_dc->width)>>1;
	    p2-=(Fs->win_pixel_height-blueprint_dc->height)>>1;
	    GrLine(blueprint_dc,x1,y1,x2,y2);
	    x2=p1; y2=p2;
	  }
	  blueprint_dc->color=color;
	  GrLine(blueprint_dc,x1,y1,x2,y2);

	  tempw=CAlloc(sizeof(Wall));
	  tempw->p1.x=x1;
	  tempw->p1.y=blueprint_dc->height-y1;
	  tempw->p2.x=x2;
	  tempw->p2.y=blueprint_dc->height-y2;
	  tempw->cpu_num=cpu_num++%mp_cnt;
	  QueIns(tempw,walls_root.last);
	} else {
	  p1-=(Fs->win_pixel_width-blueprint_dc->width)>>1;
	  p2-=(Fs->win_pixel_height-blueprint_dc->height)>>1;
	  x1=p1; y1=p2;
	  x2=p1; y2=p2;
	  blueprint_dc->color=ROP_XOR+color^LTBLUE;
	  while (msg_code!=MSG_IP_L_UP) {
	    GrRectB(blueprint_dc,x1,y1,x2,y2);
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    p1-=(Fs->win_pixel_width-blueprint_dc->width)>>1;
	    p2-=(Fs->win_pixel_height-blueprint_dc->height)>>1;
	    GrRectB(blueprint_dc,x1,y1,x2,y2);
	    x2=p1; y2=p2;
	  }
	  blueprint_dc->color=color;
	  GrRectB(blueprint_dc,x1,y1,x2,y2);
	}
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !p1);
  GetMsg(,,1<<MSG_KEY_UP);

  SettingsPop;
}

U0 Init()
{
  I64 i,j,x,y,c,c1;
  QueInit(&walls_root);

  blueprint_dc=DCNew(DCT_COLOR8,BLUEPRINT_SCALE*64,BLUEPRINT_SCALE*64);
  MakeBluePrint;

  DocClear;
  "$$BG,LTGRAY$$"; NewLine(TEXT_ROWS/2-1);

  blueprint_width =blueprint_dc->width/BLUEPRINT_SCALE;
  blueprint_height=blueprint_dc->height/BLUEPRINT_SCALE;
  Free(blueprint);
  Free(panels_processed);
  blueprint=MAlloc(blueprint_width*blueprint_height*sizeof(U8));
  MemSet(blueprint,WHITE,blueprint_width*blueprint_height);
  panels_processed=MAlloc((blueprint_width*blueprint_height+7)>>3);

  for (y=1;y<blueprint_height-1;y++)
    for (x=1;x<blueprint_width-1;x++) {
      c=LTBLUE;
      for (i=0;i<BLUEPRINT_SCALE;i++)
	for (j=0;j<BLUEPRINT_SCALE;j++)
	  if ((c1=GrPeek(blueprint_dc,x*BLUEPRINT_SCALE+i,y*BLUEPRINT_SCALE+j))!=LTBLUE)
	    c=c1;
      blueprint[(blueprint_height-1-y)*blueprint_width+x]=c;
    }
  DCDel(blueprint_dc);

  man_xx=(1+MAN_START_X)*SCREEN_SCALE;
  man_yy=(blueprint_height-1-MAN_START_Y)*SCREEN_SCALE;
  man_й=0;

  for (i=0;i<NUM_PERSONS;i++) {
    do {
      persons[i].x=RandU64%((blueprint_width-2)*SCREEN_SCALE)+SCREEN_SCALE;
      persons[i].y=RandU64%((blueprint_height-2)*SCREEN_SCALE)+SCREEN_SCALE;
    } while (blueprint[(persons[i].y/SCREEN_SCALE)*blueprint_width+persons[i].x/SCREEN_SCALE]==WHITE);
  }
  tf=0;
  t0=tT;
}

U0 AnimateTask(I64)
{
  I64 i,x,y,dd;
  while (TRUE) {
    dd=0.25*SCREEN_SCALE*Sin(tT/2);
    for (i=0;i<NUM_PERSONS;i++) {
      x=persons[i].x;
      y=persons[i].y;
      if (i&1)
	x+=dd;
      else
	y+=dd;
      if (0<=x<=blueprint_width*SCREEN_SCALE &&
	    0<=y<=blueprint_height*SCREEN_SCALE &&
	    blueprint[(y/SCREEN_SCALE)*blueprint_width+x/SCREEN_SCALE]!=WHITE) {
	if (blueprint[(y/SCREEN_SCALE)*blueprint_width+x/SCREEN_SCALE+1]==WHITE &&
	      x-RoundI64(x,SCREEN_SCALE)>SCREEN_SCALE/2 ||
	      blueprint[(y/SCREEN_SCALE)*blueprint_width+x/SCREEN_SCALE-1]==WHITE &&
	      x-RoundI64(x,SCREEN_SCALE)<SCREEN_SCALE/2)
	  x=RoundI64(x,SCREEN_SCALE)+SCREEN_SCALE/2;
	if (blueprint[(y/SCREEN_SCALE+1)*blueprint_width+x/SCREEN_SCALE]==WHITE &&
	      y-RoundI64(y,SCREEN_SCALE)>SCREEN_SCALE/2 ||
	      blueprint[(y/SCREEN_SCALE-1)*blueprint_width+x/SCREEN_SCALE]==WHITE &&
	      y-RoundI64(y,SCREEN_SCALE)<SCREEN_SCALE/2)
	  y=RoundI64(y,SCREEN_SCALE)+SCREEN_SCALE/2;
	persons[i].x=x;
	persons[i].y=y;
      }
    }
    Sleep(20);
  }
}

U0 CleanUp()
{
  Free(blueprint);
  blueprint=NULL;
  Free(panels_processed);
  panels_processed=NULL;
  QueDel(&walls_root);
}

U0 SongTask(I64)
{ //Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
	Play("2q.A#1eG2AeA#qAq.A#1eG2eAeA#qA");
	Play("q.A#1eG2A#A1qG2q.A#1eG2A#A1qG");
	Play("3eA#A2qG3eA#A2qG3eA#A2G3AA#A2qG");
  }
}

U0 MoveMan(F64 й)
{
  I64 x,y,color,step=SCREEN_SCALE/2;
  do {
    x=man_xx+step*Cos(й);
    y=man_yy-step*Sin(й);
    x=Limit(x,0,blueprint_width*SCREEN_SCALE);
    y=Limit(y,0,blueprint_height*SCREEN_SCALE);
    color=blueprint[y/SCREEN_SCALE*blueprint_width+x/SCREEN_SCALE];
    if (color==LTBLUE || color==GREEN) {
      man_xx=x;
      man_yy=y;
      break;
    } else
      step>>=1;
  } while (step);
}

#define MICRO_STEPS	4
U0 RotateMan(F64 d)
{
  I64 i;
  for (i=0;i<MICRO_STEPS;i++) {
    man_й+=d/MICRO_STEPS;
    Sleep(15);
  }
}

U0 DiningStars()
{
  I64 sc;
  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Fwd(,,SC_CURSOR_UP);"
  "  Backward(,,SC_CURSOR_DOWN);"
  "  Left(,,SC_CURSOR_LEFT);"
  "  Right(,,SC_CURSOR_RIGHT);"
      "}"
      );

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->text_attr=BROWN<<4+WHITE;
  Preempt(OFF);
  WinBorder;
  WinMax;
  WordStat;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  DocCursor;
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      switch (GetKey(&sc)) {
	case CH_ESC:
	case CH_SHIFT_ESC:
	  goto fs_done;
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_RIGHT:
	      Spawn(&RotateMan,(г/32)(I64));
	      break;
	    case SC_CURSOR_LEFT:
	      Spawn(&RotateMan,(-г/32)(I64));
	      break;
	    case SC_CURSOR_UP:
	      MoveMan(man_й);
	      break;
	    case SC_CURSOR_DOWN:
	      MoveMan(man_й+г);
	      break;
	  }
	  break;
      }
    }
fs_done:
  } catch
    CatchAll;
  DocClear;
  SettingsPop;
  CleanUp;
  MenuPop;
  AcctRegWriteBranch("TempleOS/DiningStars","F64 best_score=%5.4f;\n",best_score);
}

DiningStars;
                   ,      Ояяя      Ояяяыяяя   зяяяыяяя   зяяя      µяяя      Ќяяя       ѓяяя       Ќяяявяяя   µяяявяяяыяяяОяяя   ыяяяОяяяыяяяыяяязяяяыяяяыяяязяяя   ыяяяµяяя   ыяяяЌяяя   ыяяяЌяяявяяяыяяяµяяявяяя   Ќяяя      µяяя   вяяяµяяя   вяяяЌяяя      Ќяяяыяяя   µяяяыяяявяяяµяяяыяяявяяяЌяяяыяяя                             	               
   	      
            
            	                   	                                                                                                                                                      	            	             	   
               	                               
               
         	            
                                                                                                                                                                                        чяяя    чяяясяяя        чяяя    	                      фяяя	       чяяя	       	              шяяязяяя   мяяязяяя   мяяязяяяшяяяшяяязяяяряяя   зяяяряяя   зяяяшяяя   зяяя      зяяя                                                                         	      	            	            	            	            	   
               	                  	            	      
             
      
                                                                                                                                                           
     $   E   єяяя    ЭяяяИяяя    ЭяяяИяяя    ляяяєяяя    ляяяєяяяПяяяЭяяяИяяяПяяяЭяяяИяяяПяяяляяяєяяяПяяяляяя7       ЭяяяE       ЭяяяE       ляяя7       ляяя7   ПяяяЭяяяE   ПяяяЭяяяE   Пяяяляяя7   Пяяяляяя7          E          E       "   7       "   7   Пяяя   E   Пяяя   E   Пяяя"   7   Пяяя"   єяяя       Ияяя       Ияяя    "   єяяя    "   єяяяПяяя   ИяяяПяяя   ИяяяПяяя"   єяяяПяяя"   єяяяИяяяЭяяяE   ИяяяЭяяяE   Ияяя"   єяяяИяяя"                                                                                                                                                                    
   	        
                                       
        
           	                      
                      	   
     
                                                                                                                                                                                                                                                                                                                                                                                  #   "   !     #   !         #                   #     #   "              #           !     !            "   !              "        "                 К     p   Ё      ­яяяыяяяыяяя­яяяыяяя   ·яяяыяяяыяяя·яяяыяяя   ­яяя   ыяяя­яяя      ·яяя   ыяяя·яяя   
   ·яяяыяяяцяяя·яяяыяяя
   ЪяяяыяяяцяяяЪяяяыяяя
   ·яяя   цяяя·яяя   
   Ъяяя   цяяяЪяяя      єяяяьяяя   јяяяюяяя   Йяяя      Ляяя      ¶яяя       ёяяя      Еяяя	      Зяяя      Кяяя      Мяяя      Тяяя      Фяяя      Дяяя      Жяяя
      Мяяя      Ояяя      Сяяя      Уяяя      Хяяя      Чяяя      Няяя      Пяяя      Сяяя      Уяяя   цяяяёяяяяяяятяяяёяяя   уяяяЗяяяцяяяпяяяЗяяяшяяяцяяяєяяя   тяяяєяяя   уяяяЙяяяъяяяпяяяЙяяяьяяяуяяяЗяяяцяяяпяяяЗяяяшяяясяяяФяяяпяяяняяяФяяясяяяуяяяЙяяяъяяяпяяяЙяяяьяяясяяяЦяяяуяяяняяяЦяяяхяяясяяяФяяяряяяняяяФяяятяяяряяяЫяяяняяямяяяЫяяяпяяясяяяЦяяятяяяняяяЦяяяфяяяряяяЭяяяпяяямяяяЭяяясяяя	   Шяяяэяяя   Шяяяяяяя   дяяятяяя   дяяяфяяя	   Ьяяя      Ьяяя      ияяяцяяя   ияяяшяяя   дяяятяяя   дяяяфяяя   ряяязяяя   ряяяйяяя   ияяяцяяя   ияяяшяяя   фяяяляяя   фяяяняяя   хяяягяяя   хяяягяяя       ояяяяяяя    ояяя   пяяязяяя   пяяязяяя   ъяяятяяяяяяяъяяятяяяэяяяЬяяяэяяячяяяЮяяяяяяяяяяяияяя	   щяяякяяя   эяяяЦяяя   чяяяШяяя   яяяявяяя   щяяядяяя   яяяязяяя   щяяяйяяя
      чяяя   ыяяящяяя   яяяягяяя   щяяяеяяя      уяяя   ыяяяхяяя      эяяя   ыяяяэяяя      шяяя   эяяяшяяя       чяяя   ыяяячяяя      тяяя   эяяятяяя                                                                                                                                                         	      
      
      	                              	                  	         
                     
                  
         	                                                                                                                                                                                                                                                                                                                        !       "      "   #   !      %   $   &      &   '   %      !       $      $   %   !      #   "   &      &   '   #      "       $      $   &   "      #   !   %      %   '   #      )   (   *      *   +   )      -   ,   .      .   /   -      )   (   ,      ,   -   )      +   *   .      .   /   +      *   (   ,      ,   .   *      +   )   -      -   /   +      1   0   2      2   3   1      5   4   6      6   7   5      1   0   4      4   5   1      3   2   6      6   7   3      2   0   4      4   6   2      3   1   5      5   7   3      9   8   :      :   ;   9      =   <   >      >   ?   =      9   8   <      <   =   9      ;   :   >      >   ?   ;      :   8   <      <   >   :      ;   9   =      =   ?   ;      A   @   B      B   C   A      E   D   F      F   G   E      A   @   D      D   E   A      C   B   F      F   G   C      B   @   D      D   F   B      C   A   E      E   G   C      I   H   J      J   K   I      M   L   N      N   O   M      I   H   L      L   M   I      K   J   N      N   O   K      J   H   L      L   N   J      K   I   M      M   O   K      Q   P   R      R   S   Q      U   T   V      V   W   U      Q   P   T      T   U   Q      S   R   V      V   W   S      R   P   T      T   V   R      S   Q   U      U   W   S      Y   X   Z      Z   [   Y      ]   \   ^      ^   _   ]      Y   X   \      \   ]   Y      [   Z   ^      ^   _   [      Z   X   \      \   ^   Z      [   Y   ]      ]   _   [      a   `   b      b   c   a      e   d   f      f   g   e      a   `   d      d   e   a      c   b   f      f   g   c      b   `   d      d   f   b      c   a   e      e   g   c      i   h   j      j   k   i      m   l   n      n   o   m      i   h   l      l   m   i      k   j   n      n   o   k      j   h   l      l   n   j      k   i   m      m   o   k              К     p   Ё      ­яяяыяяяыяяя­яяяыяяя   ·яяяыяяяыяяя·яяяыяяя   ­яяя   ыяяя­яяя      ·яяя   ыяяя·яяя   
   ·яяяыяяяцяяя·яяяыяяя
   ЪяяяыяяяцяяяЪяяяыяяя
   ·яяя   цяяя·яяя   
   Ъяяя   цяяяЪяяя      ·яяя       №яяя       Жяяячяяя   Ияяячяяя   №яяя      »яяя      Ияяяыяяя   Кяяяыяяя   Жяяячяяя   Ияяячяяя   Уяяяряяя   Хяяяряяя   Ияяяыяяя   Кяяяыяяя   Хяяяфяяя   Чяяяфяяя   Уяяясяяя   Хяяясяяя   Ъяяяояяя   Ьяяяояяя   Хяяяуяяя   Чяяяуяяя   Ьяяяряяя   Юяяяряяяцяяя»яяяэяяятяяя»яяяэяяяуяяяКяяя   пяяяКяяя   цяяя·яяя   тяяя·яяя   уяяяЖяяя
   пяяяЖяяя
   уяяяЛяяя   пяяяЛяяя   сяяяУяяя   няяяУяяя   уяяяЕяяя	   пяяяЕяяя	   сяяяНяяя   няяяНяяя   сяяяТяяя   няяяТяяя   ряяяЦяяя   мяяяЦяяя   сяяяОяяя   няяяОяяя   ряяяТяяя   мяяяТяяя   	   Эяяяюяяя   Эяяяюяяя   йяяя
      йяяя
   	   Чяяя      Чяяя      гяяя      гяяя      ияяя	      ияяя	      шяяя      шяяя      дяяя      дяяя      фяяя      фяяя      ьяяя      юяяя      чяяя   яяяящяяя      цяяя      шяяя      сяяя   яяяяуяяя   эяяяЧяяяюяяячяяяЩяяяюяяяяяяягяяяуяяящяяяеяяяуяяяэяяяЫяяя   чяяяЭяяя   яяяязяяячяяящяяяйяяячяяяяяяягяяяуяяящяяяеяяяуяяя   пяяяияяяыяяясяяяияяяяяяязяяячяяящяяяйяяячяяя   уяяямяяяыяяяхяяямяяя   фяяявяяяыяяяцяяядяяя   яяяяняяяэяяя   пяяя   ояяяжяяяыяяяряяяияяя   щяяясяяяэяяяыяяяуяяя                                                                                                                                                      	      
      
      	                              	                  	         
                     
                  
         	                                                                                                                                                                                                                                                                                                                        !       "      "   #   !      %   $   &      &   '   %      !       $      $   %   !      #   "   &      &   '   #      "       $      $   &   "      #   !   %      %   '   #      )   (   *      *   +   )      -   ,   .      .   /   -      )   (   ,      ,   -   )      +   *   .      .   /   +      *   (   ,      ,   .   *      +   )   -      -   /   +      1   0   2      2   3   1      5   4   6      6   7   5      1   0   4      4   5   1      3   2   6      6   7   3      2   0   4      4   6   2      3   1   5      5   7   3      9   8   :      :   ;   9      =   <   >      >   ?   =      9   8   <      <   =   9      ;   :   >      >   ?   ;      :   8   <      <   >   :      ;   9   =      =   ?   ;      A   @   B      B   C   A      E   D   F      F   G   E      A   @   D      D   E   A      C   B   F      F   G   C      B   @   D      D   F   B      C   A   E      E   G   C      I   H   J      J   K   I      M   L   N      N   O   M      I   H   L      L   M   I      K   J   N      N   O   K      J   H   L      L   N   J      K   I   M      M   O   K      Q   P   R      R   S   Q      U   T   V      V   W   U      Q   P   T      T   U   Q      S   R   V      V   W   S      R   P   T      T   V   R      S   Q   U      U   W   S      Y   X   Z      Z   [   Y      ]   \   ^      ^   _   ]      Y   X   \      \   ]   Y      [   Z   ^      ^   _   [      Z   X   \      \   ^   Z      [   Y   ]      ]   _   [      a   `   b      b   c   a      e   d   f      f   g   e      a   `   d      d   e   a      c   b   f      f   g   c      b   `   d      d   f   b      c   a   e      e   g   c      i   h   j      j   k   i      m   l   n      n   o   m      i   h   l      l   m   i      k   j   n      n   o   k      j   h   l      l   n   j      k   i   m      m   o   k              К     p   Ё      ­яяяыяяяыяяя­яяяыяяя   ·яяяыяяяыяяя·яяяыяяя   ­яяя   ыяяя­яяя      ·яяя   ыяяя·яяя   
   ·яяяыяяяцяяя·яяяыяяя
   ЪяяяыяяяцяяяЪяяяыяяя
   ·яяя   цяяя·яяя   
   Ъяяя   цяяяЪяяя      ёяяяэяяя   єяяяэяяя   Йяяяэяяя   Ляяяэяяя   ёяяя      єяяя      Йяяя      Ляяя      Йяяяэяяя   Ляяяэяяя   Шяяяэяяя   Ъяяяэяяя   Йяяя      Ляяя      Шяяя      Ъяяя      Шяяяюяяя   Ъяяяюяяя   аяяяюяяя   вяяяюяяя   Шяяя      Ъяяя      аяяя      вяяя   цяяя№яяяэяяятяяя№яяяэяяяуяяяКяяяэяяяпяяяКяяяэяяяцяяя№яяя   тяяя№яяя   уяяяКяяя   пяяяКяяя   уяяяКяяяэяяяпяяяКяяяэяяясяяяЩяяяэяяяняяяЩяяяэяяяуяяяКяяя   пяяяКяяя   сяяяЩяяя   няяяЩяяя   сяяяЩяяяюяяяняяяЩяяяюяяяряяябяяяюяяямяяябяяяюяяясяяяЩяяя   няяяЩяяя   ряяябяяя   мяяябяяя   
   Ъяяяьяяя   Ъяяяьяяя
   кяяяьяяя   кяяяьяяя
   Ъяяя      Ъяяя   
   кяяя      кяяя   
   кяяяьяяя   кяяяьяяя
   ъяяяьяяя   ъяяяьяяя
   кяяя      кяяя   
   ъяяя      ъяяя   
       ьяяя       ьяяя
                    
   ъяяяьяяя   ъяяяьяяя
   ъяяя      ъяяя   юяяяЪяяяьяяяцяяяЪяяяьяяяюяяякяяяьяяяцяяякяяяьяяяюяяяЪяяя   цяяяЪяяя   юяяякяяя   цяяякяяя   юяяякяяяьяяяцяяякяяяьяяяюяяяъяяяьяяяцяяяъяяяьяяяюяяякяяя   цяяякяяя   юяяяъяяя   цяяяъяяя   юяяя    ьяяяцяяя    ьяяяюяяя       цяяя       юяяяъяяяьяяяцяяяъяяяьяяяюяяяъяяя   цяяяъяяя                                                                                                                                                         	      
      
      	                              	                  	         
                     
                  
         	                                                                                                                                                                                                                                                                                                                        !       "      "   #   !      %   $   &      &   '   %      !       $      $   %   !      #   "   &      &   '   #      "       $      $   &   "      #   !   %      %   '   #      )   (   *      *   +   )      -   ,   .      .   /   -      )   (   ,      ,   -   )      +   *   .      .   /   +      *   (   ,      ,   .   *      +   )   -      -   /   +      1   0   2      2   3   1      5   4   6      6   7   5      1   0   4      4   5   1      3   2   6      6   7   3      2   0   4      4   6   2      3   1   5      5   7   3      9   8   :      :   ;   9      =   <   >      >   ?   =      9   8   <      <   =   9      ;   :   >      >   ?   ;      :   8   <      <   >   :      ;   9   =      =   ?   ;      A   @   B      B   C   A      E   D   F      F   G   E      A   @   D      D   E   A      C   B   F      F   G   C      B   @   D      D   F   B      C   A   E      E   G   C      I   H   J      J   K   I      M   L   N      N   O   M      I   H   L      L   M   I      K   J   N      N   O   K      J   H   L      L   N   J      K   I   M      M   O   K      Q   P   R      R   S   Q      U   T   V      V   W   U      Q   P   T      T   U   Q      S   R   V      V   W   S      R   P   T      T   V   R      S   Q   U      U   W   S      Y   X   Z      Z   [   Y      ]   \   ^      ^   _   ]      Y   X   \      \   ]   Y      [   Z   ^      ^   _   [      Z   X   \      \   ^   Z      [   Y   ]      ]   _   [      a   `   b      b   c   a      e   d   f      f   g   e      a   `   d      d   e   a      c   b   f      f   g   c      b   `   d      d   f   b      c   a   e      e   g   c      i   h   j      j   k   i      m   l   n      n   o   m      i   h   l      l   m   i      k   j   n      n   o   k      j   h   l      l   n   j      k   i   m      m   o   k    