//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.

RegDft("TempleOS/CastleFrankenstein","F64 best_score=9999;\n");
RegExe("TempleOS/CastleFrankenstein");

//Set snap to 4 and width to 4
//if you edit this map.

//Don't forget to change the
//starting pos.
#define MAN_START_X	0
#define MAN_START_Y	4.5

$SP,"<1>",BI=1$














#define MONSTER_SCALE	2.0

		     $SP,"<2>",BI=2$
 









		     $SP,"<3>",BI=3$
 








		     $SP,"<4>",BI=4$
 






#define PLANT_SCALE	2.0



		     $SP,"<5>",BI=5$













		       $SP,"<6>",BI=6$







#define SCRN_SCALE		512
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

#define MONSTERS_NUM	10
I64 monsters_left;
class Monster
{
  I64 x,y;
  Bool dead,pad[7];
} monsters[MONSTERS_NUM];

U0 CFTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
{
  I64 zz;
  Mat4x4MulXYZ(dc->r,x,y,z);
  zz=SCRN_SCALE/3+*z;
  if (zz<1) zz=1;
  *x=SCRN_SCALE/2* *x/zz;
  *y=SCRN_SCALE/2* (*y+MAN_HEIGHT)/zz;
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
{//Line of sight
  return Line(NULL,x1*LOS_SCALE/SCRN_SCALE,y1*LOS_SCALE/SCRN_SCALE,0,
	x2*LOS_SCALE/SCRN_SCALE,y2*LOS_SCALE/SCRN_SCALE,0,&LOSPlot);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,*r1,*r2,*r3,*s2w,xx,yy,zz,x,y,x1,y1,z1,
	c,x1w,y1w,x1h,y1h,xh,yh,zh,
	cx=task->pix_width/2,
	cy=task->pix_height/2;
  U8 *tmps;
  F64 tt;
  CD3I32 poly[4];
  Monster *tmpm;

  DCDepthBufAlloc(dc);
  MemSet(panels_processed_bitmap,0,(map_width*map_height+7)>>3);

  //World to scrn
  Mat4x4RotZ(dc->r,man_é+ã/2);
  Mat4x4RotX(dc->r,ã/2);
  DCMat4x4Set(dc,dc->r);

  xh=-man_xx/SCRN_SCALE; yh=-man_yy/SCRN_SCALE; zh=0;
  Mat4x4MulXYZ(dc->r,&xh,&yh,&zh);
  Mat4x4TranslationEqu(dc->r,xh,yh,zh);

  //Scrn to world
  s2w=Mat4x4IdentNew(task);
  Mat4x4RotX(s2w,-ã/2);
  Mat4x4RotZ(s2w,-man_é-ã/2);
  xh=0; yh=0; zh=SCRN_SCALE;
  Mat4x4MulXYZ(s2w,&xh,&yh,&zh);

  //Rotate light source
  xx=dc->ls.x; yy=dc->ls.y; zz=-dc->ls.z;
  (*dc->transform)(dc,&xx,&yy,&zz);
  dc->ls.x=xx; dc->ls.y=yy; dc->ls.z=zz;

  dc->flags|=DCF_TRANSFORMATION;
  dc->transform=&CFTransform;
  dc->x=cx;
  dc->y=cy;
  r1=Mat4x4IdentNew(task);
  Mat4x4RotX(r1,-ã/2);
  Mat4x4RotZ(r1,tS);
  Mat4x4Scale(r1,MONSTER_SCALE);

  r2=Mat4x4IdentNew(task);
  Mat4x4Scale(r2,MONSTER_SCALE);

  r3=Mat4x4IdentNew(task);
  Mat4x4RotX(r3,-ã/2);
  Mat4x4Scale(r3,PLANT_SCALE);

  Seed(1);
  x1h=man_xx+yh*PLOT_GRID_WIDTH/2+xh*(PLOT_GRID_HEIGHT-1);
  y1h=man_yy-xh*PLOT_GRID_WIDTH/2+yh*(PLOT_GRID_HEIGHT-1);
  xh>>=1; yh>>=1;
  for (j=0;j<PLOT_GRID_HEIGHT*2;j++) {
    x1w=x1h;
    y1w=y1h;
    for (i=0;i<PLOT_GRID_WIDTH*4;i++) {
      xx=x1w/SCRN_SCALE; yy=y1w/SCRN_SCALE;
      x=xx*SCRN_SCALE-man_xx; y=yy*SCRN_SCALE-man_yy;
      if (1<=xx<map_width-1 && 1<=yy<map_height-1 &&
	    !LBts(panels_processed_bitmap,yy*map_width+xx)) {
	if ((c=map[yy*map_width+xx]) &&
	      LOS(xx*SCRN_SCALE+SCRN_SCALE/2,yy*SCRN_SCALE+SCRN_SCALE/2,
	      man_xx,man_yy)) {
	  if (c==YELLOW)
	    dc->color=DKGRAY;
	  else
	    dc->color=c;
	  poly[0].x=x;
	  poly[0].y=y;
	  poly[0].z=0;
	  poly[1].x=x+SCRN_SCALE;
	  poly[1].y=y;
	  poly[1].z=0;
	  poly[2].x=x+SCRN_SCALE;
	  poly[2].y=y+SCRN_SCALE;
	  poly[2].z=0;
	  poly[3].x=x;
	  poly[3].y=y+SCRN_SCALE;
	  poly[3].z=0;
	  GrFillPoly3(dc,4,poly);
	  if (c==GREEN) {
	    x1=x+SCRN_SCALE/2;
	    y1=y+SCRN_SCALE/2;
	    z1=0;
	    DCTransform(dc,&x1,&y1,&z1);
	    if (z1>0)
	      Sprite3Mat4x4B(dc,x+SCRN_SCALE/2,y+SCRN_SCALE/2,0,$IB,"<5>",BI=5$,r3);
	  } else if (c==YELLOW) {
	    x1=x+SCRN_SCALE/2;
	    y1=y+SCRN_SCALE/2;
	    z1=0;
	    DCTransform(dc,&x1,&y1,&z1);
	    if (z1>0)
	      Sprite3Mat4x4B(dc,x+SCRN_SCALE/2,y+SCRN_SCALE/2,0,$IB,"<6>",BI=6$,r3);
	  }

	  if (!map[(yy+1)*map_width+xx]) {
	    dc->color=LTGRAY;
	    poly[0].x=x;
	    poly[0].y=y+SCRN_SCALE;
	    poly[0].z=0;
	    poly[1].x=x+SCRN_SCALE;
	    poly[1].y=y+SCRN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x+SCRN_SCALE;
	    poly[2].y=y+SCRN_SCALE;
	    poly[2].z=SCRN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y+SCRN_SCALE;
	    poly[3].z=SCRN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[yy*map_width+xx+1]) {
	    dc->color=WHITE;
	    poly[0].x=x+SCRN_SCALE;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x+SCRN_SCALE;
	    poly[1].y=y+SCRN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x+SCRN_SCALE;
	    poly[2].y=y+SCRN_SCALE;
	    poly[2].z=SCRN_SCALE;
	    poly[3].x=x+SCRN_SCALE;
	    poly[3].y=y;
	    poly[3].z=SCRN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[(yy-1)*map_width+xx]) {
	    dc->color=LTGRAY;
	    poly[0].x=x;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x+SCRN_SCALE;
	    poly[1].y=y;
	    poly[1].z=0;
	    poly[2].x=x+SCRN_SCALE;
	    poly[2].y=y;
	    poly[2].z=SCRN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y;
	    poly[3].z=SCRN_SCALE;
	    GrFillPoly3(dc,4,poly);
	  }
	  if (!map[yy*map_width+xx-1]) {
	    dc->color=WHITE;
	    poly[0].x=x;
	    poly[0].y=y;
	    poly[0].z=0;
	    poly[1].x=x;
	    poly[1].y=y+SCRN_SCALE;
	    poly[1].z=0;
	    poly[2].x=x;
	    poly[2].y=y+SCRN_SCALE;
	    poly[2].z=SCRN_SCALE;
	    poly[3].x=x;
	    poly[3].y=y;
	    poly[3].z=SCRN_SCALE;
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
  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++) {
    x=tmpm->x;
    y=tmpm->y;
    if (LOS(x,y,man_xx,man_yy)) {
      x-=man_xx;
      y-=man_yy;
      xx=x;
      yy=y;
      zz=0;
      DCTransform(dc,&xx,&yy,&zz);
      if (zz>0) {
	if (tmpm->dead)
	  Sprite3Mat4x4B(dc,x,y,0,$IB,"<2>",BI=2$,r2);
	else {
	  tt=Tri(tS,1.0);
	  tmps=SpriteInterpolate(tt,$IB,"<3>",BI=3$,$IB,"<4>",BI=4$);
	  Sprite3Mat4x4B(dc,x,y,0,tmps,r1);
	  Free(tmps);
	}
      }
    }
  }
  Free(r1);
  Free(r2);
  Free(r3);

  //Draw Map heads-up display, scaled 2 pixs
  Free(dc->r);
  DCMat4x4Set(dc,Mat4x4IdentNew(task));
  dc->x=task->pix_width -2*map_width;
  dc->y=task->pix_height-2*map_height;
  dc->z=0;
  dc->transform=&DCTransform;
  dc->thick=2;
  for (i=0;i<map_height;i++)
    for (j=0;j<map_width;j++) {
      dc->color=map[(map_height-1-i)*map_width+j];
      GrPlot3(dc,2*j,2*i,0);
    }

    //Draw Things on heads-up Map
  dc->color=LTPURPLE;
  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++)
    if (!tmpm->dead)
      GrPlot3(dc,2*(tmpm->x/SCRN_SCALE),
	    2*(map_height-1-tmpm->y/SCRN_SCALE),0);
  dc->color=LTCYAN;
  GrPlot3(dc,2*(man_xx/SCRN_SCALE),2*(map_height-1-man_yy/SCRN_SCALE),0);

  if (tf) {
    dc->color=LTRED;
    if (Blink)
      GrPrint(dc,cx-(FONT_WIDTH*14)/2,cy-FONT_HEIGHT/2,"Game Completed");
    tt=tf;
  } else {
    dc->color=LTGREEN;
    GrLine(dc,cx-5,cy,cx+5,cy);
    GrLine(dc,cx,cy-5,cx,cy+5);
    tt=tS;
  }
  GrPrint(dc,0,0,"Enemy:%d Time:%3.2f Best:%3.2f",
	monsters_left,tt-t0,best_score);
  Free(s2w);
  Seed(0);
}

U0 Fire()
{
  I64 i,x,y;
  F64 d,dx,dy,xx=Cos(man_é),yy=Sin(man_é);
  Monster *tmpm;

  Noise(100,53,74);
  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++) {
    x=tmpm->x;
    y=tmpm->y;
    if (!tmpm->dead &&
	  LOS(x,y,man_xx,man_yy)) {
      dx=x-man_xx;
      dy=man_yy-y;
      if (d=Sqrt(dx*dx+dy*dy)) {
	dx/=d;
	dy/=d;
	if (dx*xx+dy*yy>0.995) {
	  tmpm->dead=TRUE;
	  if (!--monsters_left) {
	    tf=tS;
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
  CDC *dc;
  Monster *tmpm;

  DocClear;
  "$$BG,BLACK$$%h*c",TEXT_ROWS/2,'\n';

  dc=Sprite2DC($IB,"<1>",BI=1$);
  map_width =dc->width/MAP_SCALE+2;
  map_height=dc->height/MAP_SCALE+2;
  Free(map);
  Free(panels_processed_bitmap);
  map=CAlloc(map_width*map_height*sizeof(U8));
  panels_processed_bitmap=MAlloc((map_width*map_height+7)>>3);
  for (y=0;y<map_height-2;y++)
    for (x=0;x<map_width-2;x++)
      map[(map_height-2-y)*map_width+x+1]=GrPeek(dc,x*MAP_SCALE,y*MAP_SCALE);
  DCDel(dc);
  man_xx=(1+MAN_START_X)*SCRN_SCALE;
  man_yy=(map_height-1-MAN_START_Y)*SCRN_SCALE;
  man_é=0;

  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++) {
    tmpm->dead=FALSE;
    do {
      tmpm->x=RandU64%((map_width-2)*SCRN_SCALE)+SCRN_SCALE;
      tmpm->y=RandU64%((map_height-2)*SCRN_SCALE)+SCRN_SCALE;
    } while (!map[(tmpm->y/SCRN_SCALE)*map_width+tmpm->x/SCRN_SCALE]);
  }
  monsters_left=MONSTERS_NUM;
  tf=0;
  t0=tS;
}

U0 AnimateTask(I64)
{
  I64 i,x,y,dd;
  Monster *tmpm;

  while (TRUE) {
    dd=0.25*SCRN_SCALE*Sin(tS/2);
    for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++)
      if (!tmpm->dead) {
	x=tmpm->x;
	y=tmpm->y;
	if (i&1)
	  x+=dd;
	else
	  y+=dd;
	if (0<=x<=map_width*SCRN_SCALE &&
	      0<=y<=map_height*SCRN_SCALE &&
	      map[(y/SCRN_SCALE)*map_width+x/SCRN_SCALE]) {
	  if (!map[(y/SCRN_SCALE)*map_width+x/SCRN_SCALE+1] &&
		x-RoundI64(x,SCRN_SCALE)>SCRN_SCALE/2 ||
		!map[(y/SCRN_SCALE)*map_width+x/SCRN_SCALE-1] &&
		x-RoundI64(x,SCRN_SCALE)<SCRN_SCALE/2)
	    x=RoundI64(x,SCRN_SCALE)+SCRN_SCALE/2;
	  if (!map[(y/SCRN_SCALE+1)*map_width+x/SCRN_SCALE] &&
		y-RoundI64(y,SCRN_SCALE)>SCRN_SCALE/2 ||
		!map[(y/SCRN_SCALE-1)*map_width+x/SCRN_SCALE] &&
		y-RoundI64(y,SCRN_SCALE)<SCRN_SCALE/2)
	    y=RoundI64(y,SCRN_SCALE)+SCRN_SCALE/2;
	  tmpm->x=x;
	  tmpm->y=y;
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
{//Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("3q.A#eGAeA#qAq.A#eGeAeA#qA");
    Play("3q.A#eGA#AqGq.A#eGA#AqG");
    Play("4eA#AqGeA#AqGeA#AGAA#AqG");
  }
}

U0 MoveMan(F64 é)
{
  I64 x,y,color,step=SCRN_SCALE/2;
  do {
    x=man_xx+step*Cos(é);
    y=man_yy-step*Sin(é);
    x=Clamp(x,0,map_width*SCRN_SCALE);
    y=Clamp(y,0,map_height*SCRN_SCALE);
    color=map[y/SCRN_SCALE*map_width+x/SCRN_SCALE];
    if (color==DKGRAY || color==GREEN) {
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
	"  Bwd(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  Fire(,CH_SPACE);"
	"}"
	);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=DKGRAY<<4+WHITE;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
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
    PutExcept;
  DocClear;
  SettingsPop;
  CleanUp;
  MenuPop;
  RegWrite("TempleOS/CastleFrankenstein","F64 best_score=%5.4f;\n",
	best_score);
}

CastleFrankenstein;
        &        
             
              
       <      
<      <      
<             
,      ,   ,   
$      8      
8      8      
8      $      
$      $      
$      4      
4      4      
4      (      
(      (      
(      0      
,   0   T   0   
T   0   T   L   
T   L   ,   L   
,   L   ,   4   
0   4   P   4   
P   4   P   H   
P   H   0   H   
0   H   0   8   
0   8   L   8   
L   8   L   D   
L   D   4   D   
4   D   4   <   
4   <   H   <   
H   <   H   @   
H   @   8   @   
T   @   l   @   
l   4   l   L   
l   L   ˆ   L   
ˆ   L   ˆ   $   
ˆ   $   €   $   
€   $   €   0   
€   0   l   0   
p   4   „   4   
„   (   „   H   
€   H   p   H   
p   D   p   8   
p   8   €   8   
€   8   €   D   
€   D   t   H   
t   D   t   <   
t   <   |   <   
x   @   |   @   
|   D   x   D   
„       „      
„      t      
p      p      
p      \      
\      \      
\      l      
l      h      
l      l      
l      `      
`      `      
`      d      
d      d      
h      h      
X      T      
P      P      
P      L      
L      L      
(   @      @   
   8      H   
   H      H   
   H      8   
   8      8   
   8      D   
   D      D   
   D      <      Z   ,   u   
    @       Z   
D       D   <   
D   D   D   \   
D   \   D   d   T   0   T   8   T   H   L   0   L   L      d       d       h      h   8      8      x   8   |   8   |   4   x   4      t      t   $   t      @      8      H   L      L      ,   L   4   L   <   L           Z     <   H   ñÿÿÿ        ñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ                      ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
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
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                          Z     <   H   ñÿÿÿ    öÿÿÿñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ    öÿÿÿ             ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
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
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                          Z     <   H   ñÿÿÿ       ñÿÿÿØÿÿÿ    ìÿÿÿØÿÿÿ    ûÿÿÿ¿ÿÿÿ       ¿ÿÿÿ       Øÿÿÿ       Øÿÿÿ    ìÿÿÿ              öÿÿÿ   ƒÿÿÿ    çÿÿÿƒÿÿÿ    ûÿÿÿƒÿÿÿ       ƒÿÿÿ    
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
         0   /   .         1                     1            2   -   ,      7             7      6      5   3   4      3   !          3       4      5   8   3      3   !         8   3               9      ;   :   7      ;   7   6         6   ;         ;            1                                  ,      Îÿÿÿ      Îÿÿÿûÿÿÿ   çÿÿÿûÿÿÿ   çÿÿÿ      µÿÿÿ      ÿÿÿ       ƒÿÿÿ       ÿÿÿâÿÿÿ   µÿÿÿâÿÿÿûÿÿÿÎÿÿÿ   ûÿÿÿÎÿÿÿûÿÿÿûÿÿÿçÿÿÿûÿÿÿûÿÿÿçÿÿÿ   ûÿÿÿµÿÿÿ   ûÿÿÿÿÿÿ   ûÿÿÿÿÿÿâÿÿÿûÿÿÿµÿÿÿâÿÿÿ   ÿÿÿ      µÿÿÿ   âÿÿÿµÿÿÿ   âÿÿÿÿÿÿ      ÿÿÿûÿÿÿ   µÿÿÿûÿÿÿâÿÿÿµÿÿÿûÿÿÿâÿÿÿÿÿÿûÿÿÿ                             	               
   	      
            
            	                   	                                                                                                                                                      	            	             	   
               	                               
               
         	            
                                                                                                                                                                                        ÷ÿÿÿ    ÷ÿÿÿñÿÿÿ        ÷ÿÿÿ    	                      ôÿÿÿ	       ÷ÿÿÿ	       	              øÿÿÿçÿÿÿ   ìÿÿÿçÿÿÿ   ìÿÿÿçÿÿÿøÿÿÿøÿÿÿçÿÿÿðÿÿÿ   çÿÿÿðÿÿÿ   çÿÿÿøÿÿÿ   çÿÿÿ      çÿÿÿ                                                                         	      	            	            	            	            	   
               	                  	            	      
             
      
                                                                                                                                                                   (   Øÿÿÿ   (          Øÿÿÿ       ØÿÿÿØÿÿÿ   ìÿÿÿÎÿÿÿ       Îÿÿÿ       ìÿÿÿ‚          ‚   ìÿÿÿ    ‚   ìÿÿÿìÿÿÿ‚   öÿÿÿâÿÿÿ‚   
   âÿÿÿ‚              ìÿÿÿ                             
      
            
      	                        
         
   	         	            	                                                                                                                                                            