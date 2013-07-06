//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.


AcctRegSetDftEntry("TempleOS/CastleFrankenstein","F64 best_score=9999;\n");
AcctRegExeBranch("TempleOS/CastleFrankenstein");

//Set snap to 4 and width to 4
//if you edit this map.

//Don't forget to change the
//starting pos.
#define MAN_START_X	0
#define MAN_START_Y	4.5

$SP,"<1>",1$















#define MONSTER_SCALE	2.0


		     $SP,"<2>",2$
 









		     $SP,"<3>",3$
 








		     $SP,"<4>",4$
 






#define PLANT_SCALE	2.0



		     $SP,"<5>",5$














		       $SP,"<6>",6$









#define SCREEN_SCALE		512
#define PLOT_GRID_WIDTH		24
#define PLOT_GRID_HEIGHT	24
#define MAN_HEIGHT		125

#define MAP_SCALE	4
I64 map_width,map_height;
U8 *map=NULL,
*panels_processed_bitmap=NULL;

I64 man_xx,man_yy;
F64 man_é;

F64 t0,tf;

#define NUM_MONSTERS	16
I64 monsters_left;
class Monster
{
  I64 x,y;
  Bool dead,pad[7];
} monsters[NUM_MONSTERS];

U0 FPSTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
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

#define LOS_SCALE	4

Bool LOSPlot(U8 *,I64 x,I64 y,I64)
{
  if (!map[(y/LOS_SCALE)*map_width+(x/LOS_SCALE)])
    return FALSE;
  else
    return TRUE;
}

Bool LOS(I64 x1,I64 y1,I64 x2,I64 y2)
{ //Line of sight
  return LOSLine(NULL,x1*LOS_SCALE/SCREEN_SCALE,y1*LOS_SCALE/SCREEN_SCALE,0,
	       x2*LOS_SCALE/SCREEN_SCALE,y2*LOS_SCALE/SCREEN_SCALE,0,&LOSPlot);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,*r1,*r2,*r3,*s2w,xx,yy,zz,x,y,x1,y1,z1,
      c,x1w,y1w,x1h,y1h,xh,yh,zh,
      cx=task->win_pixel_width/2,
      cy=task->win_pixel_height/2,
      _man_xx=man_xx,_man_yy=man_yy;
  U8 *inter;
  F64 tt,_man_é=man_é;
  CD3I32 poly[4];

  DCAllocDepthBuf(dc);
  MemSet(panels_processed_bitmap,0,(map_width*map_height+7)>>3);

  //World to screen
  GrRotZEqu(dc->r,_man_é+ã/2);
  GrRotXEqu(dc->r,ã/2);
  GrSetRotMat(dc,dc->r);

  xh=-_man_xx/SCREEN_SCALE; yh=-_man_yy/SCREEN_SCALE; zh=0;
  GrRot(dc->r,&xh,&yh,&zh);
  GrSetTranslation(dc->r,xh,yh,zh);

  //Screen to world
  s2w=GrRotX(-ã/2,task);
  GrRotZEqu(s2w,-_man_é-ã/2);
  xh=0; yh=0; zh=SCREEN_SCALE;
  GrRot(s2w,&xh,&yh,&zh);

  //Rotate light source
  xx=dc->ls.x; yy=dc->ls.y; zz=-dc->ls.z;
  (*dc->transform)(dc,&xx,&yy,&zz);
  dc->ls.x=xx; dc->ls.y=yy; dc->ls.z=zz;

  dc->flags|=DCF_TRANSFORMATION;
  dc->transform=&FPSTransform;
  dc->x=cx;
  dc->y=cy;
  r1=GrRotX(-ã/2,task);
  GrRotZEqu(r1,tT);
  GrScaleMatEqu(r1,MONSTER_SCALE);

  r2=GrIdent(task);
  GrScaleMatEqu(r2,MONSTER_SCALE);

  r3=GrRotX(-ã/2,task);
  GrScaleMatEqu(r3,PLANT_SCALE);

  x1h=_man_xx+yh*PLOT_GRID_WIDTH/2+xh*(PLOT_GRID_HEIGHT-1);
  y1h=_man_yy-xh*PLOT_GRID_WIDTH/2+yh*(PLOT_GRID_HEIGHT-1);
  xh>>=1; yh>>=1;
  for (j=0;j<PLOT_GRID_HEIGHT*2;j++) {
    x1w=x1h;
    y1w=y1h;
    for (i=0;i<PLOT_GRID_WIDTH*4;i++) {
      xx=x1w/SCREEN_SCALE; yy=y1w/SCREEN_SCALE;
      x=xx*SCREEN_SCALE-_man_xx; y=yy*SCREEN_SCALE-_man_yy;
      if (1<=xx<map_width-1 && 1<=yy<map_height-1 &&
	  !LBts(panels_processed_bitmap,yy*map_width+xx)) {
	if ((c=map[yy*map_width+xx]) &&
	    LOS(xx*SCREEN_SCALE+SCREEN_SCALE/2,yy*SCREEN_SCALE+SCREEN_SCALE/2,
		_man_xx,_man_yy)) {
	  if (c==YELLOW)
	    dc->color=RED;
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
	  if (c==GREEN) {
	    x1=x+SCREEN_SCALE/2;
	    y1=y+SCREEN_SCALE/2;
	    z1=0;
	    GrTransform(dc,&x1,&y1,&z1);
	    if (z1>0)
	      SpritePlotRotMat3b(dc,x+SCREEN_SCALE/2,y+SCREEN_SCALE/2,0,$IB,"<5>",5$,r3);
	  } else if (c==YELLOW) {
	    x1=x+SCREEN_SCALE/2;
	    y1=y+SCREEN_SCALE/2;
	    z1=0;
	    GrTransform(dc,&x1,&y1,&z1);
	    if (z1>0)
	      SpritePlotRotMat3b(dc,x+SCREEN_SCALE/2,y+SCREEN_SCALE/2,0,$IB,"<6>",6$,r3);
	  }

	  if (!map[(yy+1)*map_width+xx]) {
	    dc->color=LTGRAY;
	    poly[0].x=x;
	    poly[0].y=y+SCREEN_SCALE;
	    poly[0].z=0;
	    poly[1].x=x+SCREEN_SCALE;
	    poly[1].y=y+SCREEN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x+SCREEN_SCALE;
	    poly[2].y=y+SCREEN_SCALE;
	    poly[2].z=SCREEN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y+SCREEN_SCALE;
	    poly[3].z=SCREEN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[yy*map_width+xx+1]) {
	    dc->color=DKGRAY;
	    poly[0].x=x+SCREEN_SCALE;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x+SCREEN_SCALE;
	    poly[1].y=y+SCREEN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x+SCREEN_SCALE;
	    poly[2].y=y+SCREEN_SCALE;
	    poly[2].z=SCREEN_SCALE;
	    poly[3].x=x+SCREEN_SCALE;
	    poly[3].y=y;
	    poly[3].z=SCREEN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[(yy-1)*map_width+xx]) {
	    dc->color=LTGRAY;
	    poly[0].x=x;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x+SCREEN_SCALE;
	    poly[1].y=y;
	    poly[1].z=0;
	    poly[2].x=x+SCREEN_SCALE;
	    poly[2].y=y;
	    poly[2].z=SCREEN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y;
	    poly[3].z=SCREEN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[yy*map_width+xx-1]) {
	    dc->color=DKGRAY;
	    poly[0].x=x;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x;
	    poly[1].y=y+SCREEN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x;
	    poly[2].y=y+SCREEN_SCALE;
	    poly[2].z=SCREEN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y;
	    poly[3].z=SCREEN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	}
      }
      x1w-=yh;
      y1w+=xh;
    }
    x1h-=xh;
    y1h-=yh;
  }

  //Draw Monsters
  for (i=0;i<NUM_MONSTERS;i++) {
    x=monsters[i].x;
    y=monsters[i].y;
    if (LOS(x,y,_man_xx,_man_yy)) {
      x-=_man_xx;
      y-=_man_yy;
      xx=x;
      yy=y;
      zz=0;
      GrTransform(dc,&xx,&yy,&zz);
      if (zz>0) {
	if (monsters[i].dead)
	  SpritePlotRotMat3b(dc,x,y,0,$IB,"<2>",2$,r2);
	else {
	  tt=Tri(tT,1.0);
	  inter=SpriteInterpolate($IB,"<3>",3$,$IB,"<4>",4$,tt);
	  SpritePlotRotMat3b(dc,x,y,0,inter,r1);
	  Free(inter);
	}
      }
    }
  }
  Free(r1);
  Free(r2);
  Free(r3);

  //Draw Map heads-up display, scaled 2 pixels
  Free(dc->r);
  GrSetRotMat(dc,GrIdent(task));
  dc->x=task->win_pixel_width -2*map_width;
  dc->y=task->win_pixel_height-2*map_height;
  dc->z=0;
  dc->transform=&GrTransform;
  dc->pen_width=2;
  for (i=0;i<map_height;i++)
    for (j=0;j<map_width;j++) {
      dc->color=map[(map_height-1-i)*map_width+j];
      GrPlot3(dc,2*j,2*i,0);
    }

    //Draw Things on heads-up Map
    dc->color=LTPURPLE;
  for (i=0;i<NUM_MONSTERS;i++)
    if (!monsters[i].dead)
      GrPlot3(dc,2*(monsters[i].x/SCREEN_SCALE),2*(map_height-1-monsters[i].y/SCREEN_SCALE),0);
  dc->color=LTCYAN;
  GrPlot3(dc,2*(_man_xx/SCREEN_SCALE),2*(map_height-1-_man_yy/SCREEN_SCALE),0);


  if (tf) {
    dc->color=LTRED;
    if (Blink)
      GrPutS(dc,cx-(FONT_WIDTH*14)/2,cy-FONT_HEIGHT/2,"Game Completed");
    tt=tf;
  } else {
    dc->color=LTGREEN;
    GrLine(dc,cx-5,cy,cx+5,cy);
    GrLine(dc,cx,cy-5,cx,cy+5);
    tt=tT;
  }
  GrPrintF(dc,0,0,"Enemy:%d Time:%3.2f Best:%3.2f",monsters_left,tt-t0,best_score);
  Free(s2w);
}

U0 Fire()
{
  I64 i,x,y;
  F64 d,dx,dy,xx=Cos(man_é),yy=Sin(man_é);
  Noise(100,300,1000);
  for (i=0;i<NUM_MONSTERS;i++) {
    x=monsters[i].x;
    y=monsters[i].y;
    if (!monsters[i].dead &&
	LOS(x,y,man_xx,man_yy)) {
      dx=x-man_xx;
      dy=man_yy-y;
      if (d=Sqrt(dx*dx+dy*dy)) {
	dx/=d;
	dy/=d;
	if (dx*xx+dy*yy>0.995) {
	  monsters[i].dead=TRUE;
	  if (!--monsters_left) {
	    tf=tT;
	    if (tf-t0<best_score)
	      best_score=tf-t0;
	  }
	}
      }
    }
  }
}


U0 Init()
{
  I64 i,x,y;
  I64 min_map_x,max_map_x,min_map_y,max_map_y;
  CDC *dc;
  DocClear;
  "$$BG,BLACK$$"; NewLine(TEXT_ROWS/2);

  SpriteExtents($IB,"<1>",1$,&min_map_x,&max_map_x,&min_map_y,&max_map_y);
  map_width =(max_map_x-min_map_x+1)/MAP_SCALE+2;
  map_height=(max_map_y-min_map_y+1)/MAP_SCALE+2;
  Free(map);
  Free(panels_processed_bitmap);
  map=CAlloc(map_width*map_height*sizeof(U8));
  panels_processed_bitmap=MAlloc((map_width*map_height+7)>>3);
  dc=DCNew(DCT_COLOR8,map_width*MAP_SCALE,map_height*MAP_SCALE);
  SpritePlot3(dc,-min_map_x+MAP_SCALE,-min_map_y+MAP_SCALE,0,$IB,"<1>",1$);
  for (y=1;y<map_height-1;y++)
    for (x=1;x<map_width-1;x++)
      map[(map_height-1-y)*map_width+x]=GrPeek(dc,x*MAP_SCALE,y*MAP_SCALE);
    DCDel(dc);
  man_xx=(1+MAN_START_X)*SCREEN_SCALE;
  man_yy=(map_height-1-MAN_START_Y)*SCREEN_SCALE;
  man_é=0;

  for (i=0;i<NUM_MONSTERS;i++) {
    monsters[i].dead=FALSE;
    do {
      monsters[i].x=RandU64%((map_width-2)*SCREEN_SCALE)+SCREEN_SCALE;
      monsters[i].y=RandU64%((map_height-2)*SCREEN_SCALE)+SCREEN_SCALE;
    } while (!map[(monsters[i].y/SCREEN_SCALE)*map_width+monsters[i].x/SCREEN_SCALE]);
  }
  monsters_left=NUM_MONSTERS;
  tf=0;
  t0=tT;
}

U0 AnimateTask(I64)
{
  I64 i,x,y,dd;
  while (TRUE) {
    dd=0.25*SCREEN_SCALE*Sin(tT/2);
    for (i=0;i<NUM_MONSTERS;i++)
      if (!monsters[i].dead) {
	x=monsters[i].x;
	y=monsters[i].y;
	if (i&1)
	  x+=dd;
	else
	  y+=dd;
	if (0<=x<=map_width*SCREEN_SCALE &&
	    0<=y<=map_height*SCREEN_SCALE &&
	    map[(y/SCREEN_SCALE)*map_width+x/SCREEN_SCALE]) {
	  if (!map[(y/SCREEN_SCALE)*map_width+x/SCREEN_SCALE+1] &&
	       x-RoundI64(x,SCREEN_SCALE)>SCREEN_SCALE/2 ||
	      !map[(y/SCREEN_SCALE)*map_width+x/SCREEN_SCALE-1] &&
	       x-RoundI64(x,SCREEN_SCALE)<SCREEN_SCALE/2)
	    x=RoundI64(x,SCREEN_SCALE)+SCREEN_SCALE/2;
	  if (!map[(y/SCREEN_SCALE+1)*map_width+x/SCREEN_SCALE] &&
	       y-RoundI64(y,SCREEN_SCALE)>SCREEN_SCALE/2 ||
	      !map[(y/SCREEN_SCALE-1)*map_width+x/SCREEN_SCALE] &&
	       y-RoundI64(y,SCREEN_SCALE)<SCREEN_SCALE/2)
	    y=RoundI64(y,SCREEN_SCALE)+SCREEN_SCALE/2;
	  monsters[i].x=x;
	  monsters[i].y=y;
	}
      }
    Sleep(20);
  }
}

U0 CleanUp()
{
  Free(map);
  Free(panels_processed_bitmap);
  map=NULL;
  panels_processed_bitmap=NULL;
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

U0 MoveMan(F64 é)
{
  I64 x,y,color,step=SCREEN_SCALE/2;
  do {
    x=man_xx+step*Cos(é);
    y=man_yy-step*Sin(é);
    x=Limit(x,0,map_width*SCREEN_SCALE);
    y=Limit(y,0,map_height*SCREEN_SCALE);
    color=map[y/SCREEN_SCALE*map_width+x/SCREEN_SCALE];
    if (color==RED || color==GREEN) {
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
    man_é+=d/MICRO_STEPS;
    Sleep(15);
  }
}

U0 CastleFrankenstein()
{
  I64 sc;
  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Restart(,'\n');"
  "  Fwd(,,SC_CURSOR_UP);"
  "  Backward(,,SC_CURSOR_DOWN);"
  "  Left(,,SC_CURSOR_LEFT);"
  "  Right(,,SC_CURSOR_RIGHT);"
  "  Fire(,CH_SPACE);"
      "}"
      );

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->text_attr=RED<<4+WHITE;
  Preempt(OFF);
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
	case CH_SPACE:
	  Fire;
	  break;
	case '\n':
	  Init;
	  break;
	case CH_ESC:
	case CH_SHIFT_ESC:
	  goto fs_done;
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_RIGHT:
	      Spawn(&RotateMan,(ã/32)(I64));
	      break;
	    case SC_CURSOR_LEFT:
	      Spawn(&RotateMan,(-ã/32)(I64));
	      break;
	    case SC_CURSOR_UP:
	      MoveMan(man_é);
	      break;
	    case SC_CURSOR_DOWN:
	      MoveMan(man_é+ã);
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
  AcctRegWriteBranch("TempleOS/CastleFrankenstein","F64 best_score=%5.4f;\n",best_score);
}

CastleFrankenstein;
           +                                          <      <      <      <             ,      ,   ,   $      8      8      8      8      $      $      $      $      4      4      4      4      (      (      (      (      0      ,   0   T   0   T   0   T   L   T   L   ,   L   ,   L   ,   4   0   4   P   4   P   4   P   H   P   H   0   H   0   H   0   8   0   8   L   8   L   8   L   D   L   D   4   D   4   D   4   <   4   <   H   <   H   <   H   @   H   @   8   @      T   @   l   @   l   4   l   L   l   L   ˆ   L   ˆ   L   ˆ   $   ˆ   $   €   $   €   $   €   0   €   0   l   0   p   4   „   4   „   (   „   H   €   H   p   H   p   D   p   8   p   8   €   8   €   8   €   D   €   D   t   H   t   D   t   <   t   <   |   <   x   @   |   @   |   D   x   D   „       „      „      t      p      p      p      \      \      \      \      l      l      h      l      l      l      `      `      `      `      d      d      d      h      h      X      T      P      P      P      L      L      L      (   @      @      8      H      H      H      H      8      8      8      8      D      D      D      D      <      Z   ,   u       @       Z   D       D   <   D   D   D   \   D   \   D   d   T   0   T   8   T   H   L   0   L   L      d       d       h      h   8      8      x   8   |   8   |   4   x   4      t      t   $   t      @      8      H   L      L      ,   L   4   L   <   L              Z     <   H   ñÿÿÿ        ñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ                      ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
   tÿÿÿ    öÿÿÿtÿÿÿ    
   ¿ÿÿÿ    öÿÿÿ¿ÿÿÿ       ƒÿÿÿ
   
   tÿÿÿ
      ƒÿÿÿ
   ûÿÿÿƒÿÿÿ
   çÿÿÿƒÿÿÿ
   
   ¿ÿÿÿ
      ¿ÿÿÿ
   ûÿÿÿ¿ÿÿÿ
   öÿÿÿ¿ÿÿÿ
   ìÿÿÿØÿÿÿ
   ñÿÿÿØÿÿÿ
      Øÿÿÿ
      Øÿÿÿ
      ûÿÿÿ
      ûÿÿÿ
   ñÿÿÿûÿÿÿ
   ìÿÿÿûÿÿÿ
      ƒÿÿÿ
   ìÿÿÿƒÿÿÿ
   çÿÿÿ«ÿÿÿ-   ìÿÿÿ«ÿÿÿ-      «ÿÿÿ-      «ÿÿÿ-      °ÿÿÿ-      °ÿÿÿ-   ìÿÿÿ°ÿÿÿ-   çÿÿÿ°ÿÿÿ-   øÿÿÿxÿÿÿ   ýÿÿÿ|ÿÿÿ      xÿÿÿ      xÿÿÿ      |ÿÿÿ   öÿÿÿtÿÿÿ
   ÿÿÿÿxÿÿÿ   ìÿÿÿûÿÿÿ   ñÿÿÿûÿÿÿ   ñÿÿÿ          ûÿÿÿ      ûÿÿÿ   ìÿÿÿ                                                                                                                                    	      	   
                                                                                         !            !                                                         &   '      '   "         #   %   $      $      #                                                      	                                          	                                                !                                 
                           
         
                     $   %   *      $   *   +      '   &   (      '   (   )      &   (            (   	      $   +         +   
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                             Z     <   H   ñÿÿÿ    öÿÿÿñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ    öÿÿÿ             ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
   tÿÿÿ    öÿÿÿtÿÿÿ    
   ¿ÿÿÿ    öÿÿÿ¿ÿÿÿ       ƒÿÿÿ
   
   tÿÿÿ
      ƒÿÿÿ
   ûÿÿÿƒÿÿÿ
   çÿÿÿƒÿÿÿ
   
   ¿ÿÿÿ
      ¿ÿÿÿ
   ûÿÿÿ¿ÿÿÿ
   öÿÿÿ¿ÿÿÿ
   ìÿÿÿØÿÿÿ
   ñÿÿÿØÿÿÿ
      Øÿÿÿ
      Øÿÿÿ
      ûÿÿÿ      ûÿÿÿ   ñÿÿÿûÿÿÿ    ìÿÿÿûÿÿÿ       ƒÿÿÿ
   ìÿÿÿƒÿÿÿ
   çÿÿÿ«ÿÿÿ-   ìÿÿÿ«ÿÿÿ-      «ÿÿÿÓÿÿÿ   «ÿÿÿÓÿÿÿ   °ÿÿÿÓÿÿÿ   °ÿÿÿÓÿÿÿìÿÿÿ°ÿÿÿ-   çÿÿÿ°ÿÿÿ-   øÿÿÿxÿÿÿ   ýÿÿÿ|ÿÿÿ      xÿÿÿ      xÿÿÿ      |ÿÿÿ   öÿÿÿtÿÿÿ
   ÿÿÿÿxÿÿÿ   ìÿÿÿûÿÿÿ   ñÿÿÿûÿÿÿ   ñÿÿÿ          ûÿÿÿ-      ûÿÿÿ-   ìÿÿÿ                        -          -                                                                                                 	      	   
                                                                                         !            !                                                         &   '      '   "         #   %   $      $      #                                                      	                                          	                                                !                                 
                           
         
                     $   %   *      $   *   +      '   &   (      '   (   )      &   (            (   	      $   +         +   
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                             Z     <   H   ñÿÿÿ       ñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ              öÿÿÿ   ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
   tÿÿÿ    öÿÿÿtÿÿÿ    
   ¿ÿÿÿ    öÿÿÿ¿ÿÿÿ       ƒÿÿÿ
   
   tÿÿÿ
      ƒÿÿÿ
   ûÿÿÿƒÿÿÿ
   çÿÿÿƒÿÿÿ
   
   ¿ÿÿÿ
      ¿ÿÿÿ
   ûÿÿÿ¿ÿÿÿ
   öÿÿÿ¿ÿÿÿ
   ìÿÿÿØÿÿÿ
   ñÿÿÿØÿÿÿ
      Øÿÿÿ
      Øÿÿÿ
      ûÿÿÿ       ûÿÿÿ    ñÿÿÿûÿÿÿ   ìÿÿÿûÿÿÿ      ƒÿÿÿ
   ìÿÿÿƒÿÿÿ
   çÿÿÿ«ÿÿÿÓÿÿÿìÿÿÿ«ÿÿÿÓÿÿÿ   «ÿÿÿ-      «ÿÿÿ-      °ÿÿÿ-      °ÿÿÿ-   ìÿÿÿ°ÿÿÿÓÿÿÿçÿÿÿ°ÿÿÿÓÿÿÿøÿÿÿxÿÿÿ   ýÿÿÿ|ÿÿÿ      xÿÿÿ      xÿÿÿ      |ÿÿÿ   öÿÿÿtÿÿÿ
   ÿÿÿÿxÿÿÿ   ìÿÿÿûÿÿÿ#   ñÿÿÿûÿÿÿ#   ñÿÿÿ    #      ûÿÿÿ      ûÿÿÿ   ìÿÿÿ    #          öÿÿÿ                                                                                                                  	      	   
                                                                                         !            !                                                         &   '      '   "         #   %   $      $      #                                                      	                                          	                                                !                                 
                           
         
                     $   %   *      $   *   +      '   &   (      '   (   )      &   (            (   	      $   +         +   
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                                     ,      Îÿÿÿ      Îÿÿÿûÿÿÿ   çÿÿÿûÿÿÿ   çÿÿÿ      µÿÿÿ      ÿÿÿ       ƒÿÿÿ       ÿÿÿâÿÿÿ   µÿÿÿâÿÿÿûÿÿÿÎÿÿÿ   ûÿÿÿÎÿÿÿûÿÿÿûÿÿÿçÿÿÿûÿÿÿûÿÿÿçÿÿÿ   ûÿÿÿµÿÿÿ   ûÿÿÿÿÿÿ   ûÿÿÿÿÿÿâÿÿÿûÿÿÿµÿÿÿâÿÿÿ   ÿÿÿ      µÿÿÿ   âÿÿÿµÿÿÿ   âÿÿÿÿÿÿ      ÿÿÿûÿÿÿ   µÿÿÿûÿÿÿâÿÿÿµÿÿÿûÿÿÿâÿÿÿÿÿÿûÿÿÿ                             	               
   	      
            
            	                   	                                                                                                                                                      	            	             	   
               	                               
               
         	            
                                                                                                                                                                                        ÷ÿÿÿ    ÷ÿÿÿñÿÿÿ        ÷ÿÿÿ    	                      ôÿÿÿ	       ÷ÿÿÿ	       	              øÿÿÿçÿÿÿ   ìÿÿÿçÿÿÿ   ìÿÿÿçÿÿÿøÿÿÿøÿÿÿçÿÿÿðÿÿÿ   çÿÿÿðÿÿÿ   çÿÿÿøÿÿÿ   çÿÿÿ      çÿÿÿ                                                                         	      	            	            	            	            	   
               	                  	            	      
             
      
                                                                                                                                                                      (   Øÿÿÿ   (          Øÿÿÿ       ØÿÿÿØÿÿÿ   ìÿÿÿÎÿÿÿ       Îÿÿÿ       ìÿÿÿ‚          ‚   ìÿÿÿ    ‚   ìÿÿÿìÿÿÿ‚   öÿÿÿâÿÿÿ‚   
   âÿÿÿ‚              ìÿÿÿ                             
      
            
      	                        
         
   	         	            	                                                                                                                                                            