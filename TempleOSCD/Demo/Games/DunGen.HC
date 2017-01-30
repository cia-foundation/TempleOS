RegDft("TempleOS/DunGen","F64 best_score=9999;\n");
RegExe("TempleOS/DunGen");

//Set snap to 4 and width to 4
//if you edit this map.

//Don't forget to change the
//starting pos.
$BG,0$

$SP,"<1>",BI=1$









$SP,"<2>",BI=2$



$SP,"<3>",BI=3$

 
$SP,"<4>",BI=4$

$SP,"<5>",BI=5$

$SP,"<6>",BI=6$

$SP,"<7>",BI=7$

$SP,"<8>",BI=8$

$SP,"<9>",BI=9$

$SP,"<10>",BI=10$


$BG$//These are indexed by color #.
//See $LK,"COLORS",A="MN:RED"$.

U8 *tiles1[16]={NULL,$IB,"<7>",BI=7$ ,$IB,"<5>",BI=5$ ,NULL,NULL,NULL,NULL,NULL,
  $IB,"<6>",BI=6$ ,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

U8 *tiles2[16]={NULL,$IB,"<8>",BI=8$ ,$IB,"<5>",BI=5$ ,NULL,NULL,NULL,NULL,NULL,
  $IB,"<6>",BI=6$ ,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

#define SCRN_SCALE	24
#define SCRN_WIDTH	24
#define SCRN_HEIGHT	24
I64 scrn_x,scrn_y;

#define MAP_SCALE	4
I64 map_width,map_height;
U8 *map=NULL;

I64 man_x,man_y,man_dx,man_dy;
Bool man_attack;
F64 man_attack_t0;

#define MONSTERS_NUM	10
I64 monsters_left;
class Monster
{
  I64 x,y,dx,dy;
  Bool dead,pad[7];
} monsters[MONSTERS_NUM];

F64 t0,tf;

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
  return Line(NULL,x1*LOS_SCALE+LOS_SCALE/2,y1*LOS_SCALE+LOS_SCALE/2,0,
	x2*LOS_SCALE+LOS_SCALE/2,y2*LOS_SCALE+LOS_SCALE/2,0,&LOSPlot);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  CDC *dc_t=DCAlias(gr.dc2,task);
  I64 i,x,y,xx,yy,x1,y1,z1,color,
	cx=task->pix_width/2,
	cy=task->pix_height/2;
  CD3I32 poly[4];
  U8 **_tiles;
  F64 tt;
  Monster *tmpm;

  if (Blink(5))
    _tiles=tiles1;
  else
    _tiles=tiles2;

  Mat4x4RotX(dc_t->r,60*2*ã/360);
  Mat4x4RotZ(dc_t->r,15*2*ã/360);
  DCMat4x4Set(dc_t,dc_t->r);
  dc_t->x=task->pix_width/2;
  dc_t->y=task->pix_height/2;
  dc_t->flags|=DCF_TRANSFORMATION;

  //You could make it much more efficient
  //if you did it like $LK,"::/Demo/Games/BigGuns.HC"$
  //with a $LK,"CDC",A="MN:CDC"$.

  for (y=-SCRN_HEIGHT/2;y<SCRN_HEIGHT/2;y++) {
    yy=y+scrn_y;
    if (0<=yy<map_height)
      for (x=-SCRN_WIDTH/2;x<SCRN_WIDTH/2;x++) {
	xx=x+scrn_x;
	if (0<=xx<map_width) {
	  if ((color=map[yy*map_width+xx]) &&
		LOS(xx,yy,man_x,man_y)) {
	    if (_tiles[color]) {
	      x1=x*SCRN_SCALE;
	      y1=y*SCRN_SCALE;
	      z1=0;
	      DCTransform(dc_t,&x1,&y1,&z1);
	      Sprite3(dc,x1,y1,z1,_tiles[color]);
	    } else {//If no tile defined, do solid color.
	      poly[0].x=x*SCRN_SCALE;
	      poly[0].y=y*SCRN_SCALE;
	      poly[0].z=0;
	      poly[1].x=(x+1)*SCRN_SCALE-1;
	      poly[1].y=y*SCRN_SCALE;
	      poly[1].z=0;
	      poly[2].x=(x+1)*SCRN_SCALE;
	      poly[2].y=(y+1)*SCRN_SCALE-1;
	      poly[2].z=0;
	      poly[3].x=x*SCRN_SCALE-1;
	      poly[3].y=(y+1)*SCRN_SCALE-1;
	      poly[3].z=0;
	      dc_t->color=color;
	      GrFillPoly3(dc_t,4,poly);
	    }
	  }
	}
      }
  }

  for (y=-SCRN_HEIGHT/2;y<SCRN_HEIGHT/2;y++) {
    yy=y+scrn_y;
    if (0<=yy<map_height)
      for (x=-SCRN_WIDTH/2;x<SCRN_WIDTH/2;x++) {
	xx=x+scrn_x;
	if (0<=xx<map_width) {
	  if (!map[yy*map_width+xx]) {
	    if (yy+1<map_height && LOS(xx,yy+1,man_x,man_y)) {
	      x1=x*SCRN_SCALE;
	      y1=y*SCRN_SCALE;
	      z1=0;
	      DCTransform(dc_t,&x1,&y1,&z1);
	      Sprite3(dc,x1,y1,z1,$IB,"<9>",BI=9$);
	    }
	    if (xx+1<map_width && LOS(xx+1,yy,man_x,man_y)) {
	      x1=x*SCRN_SCALE;
	      y1=y*SCRN_SCALE;
	      z1=0;
	      DCTransform(dc_t,&x1,&y1,&z1);
	      Sprite3(dc,x1,y1,z1,$IB,"<10>",BI=10$);
	    }
	  }
	}
      }
  }

  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++)
    if (!tmpm->dead && LOS(tmpm->x,tmpm->y,man_x,man_y)) {
      x1=(tmpm->x-scrn_x)*SCRN_SCALE+SCRN_SCALE/2;
      y1=(tmpm->y-scrn_y)*SCRN_SCALE+SCRN_SCALE/2;
      z1=0;
      DCTransform(dc_t,&x1,&y1,&z1);
      if (tmpm->dx<0) {
	dc->flags|=DCF_SYMMETRY|DCF_JUST_MIRROR;
	DCSymmetrySet(dc,x1,y1,x1,y1+1);
      } else
	dc->flags&=~(DCF_SYMMETRY|DCF_JUST_MIRROR);
      Sprite3(dc,x1,y1,z1,$IB,"<4>",BI=4$);
    }

  x1=(man_x-scrn_x)*SCRN_SCALE+SCRN_SCALE/2;
  y1=(man_y-scrn_y)*SCRN_SCALE+SCRN_SCALE/2;
  z1=0;
  if (tS-man_attack_t0<0.2) {
    x1+=Tri(tS-man_attack_t0,0.2)*SCRN_SCALE*man_dx;
    y1+=Tri(tS-man_attack_t0,0.2)*SCRN_SCALE*man_dy;
    if (man_dy!=1)
      y1-=Saw(tS-man_attack_t0,0.2)*SCRN_SCALE;
  }
  DCTransform(dc_t,&x1,&y1,&z1);
  if (man_dx<0) {
    dc->flags|=DCF_SYMMETRY|DCF_JUST_MIRROR;
    DCSymmetrySet(dc,x1,y1,x1,y1+1);
  } else
    dc->flags&=~(DCF_SYMMETRY|DCF_JUST_MIRROR);

  if (tS-man_attack_t0<0.2)
    Sprite3(dc,x1,y1,z1,$IB,"<3>",BI=3$);
  else
    Sprite3(dc,x1,y1,z1,$IB,"<2>",BI=2$);

  DCDel(dc_t);

  if (tf) {
    dc->color=LTRED;
    if (Blink)
      GrPrint(dc,cx-(FONT_WIDTH*14)/2,cy-FONT_HEIGHT/2,"Game Completed");
    tt=tf;
  } else {
    dc->color=LTGREEN;
    tt=tS;
  }
  GrPrint(dc,0,0,"Enemy:%d Time:%3.2f Best:%3.2f",
	monsters_left,tt-t0,best_score);
}

U0 Attack()
{
  I64 i;
  Monster *tmpm;

  man_attack_t0=tS;
  Noise(100,53,74);
  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++) {
    if (!tmpm->dead &&
	  man_x+man_dx==tmpm->x && man_y+man_dy==tmpm->y) {
      tmpm->dead=TRUE;
      if (!--monsters_left) {
	tf=tS;
	if (tf-t0<best_score)
	  best_score=tf-t0;
      }
    }
  }
}

U0 Init()
{
  I64 i,x,y;
  CDC *dc;
  Monster *tmpm;

  dc=Sprite2DC($IB,"<1>",BI=1$);
  map_width =dc->width/MAP_SCALE;
  map_height=dc->height/MAP_SCALE;
  Free(map);
  map=MAlloc(map_width*map_height*sizeof(U8));
  for (y=0;y<map_height;y++)
    for (x=0;x<map_width;x++)
      map[y*map_width+x]=GrPeek(dc,x*MAP_SCALE,y*MAP_SCALE);
  DCDel(dc);

  man_attack_t0=0;
  man_attack=FALSE;
  man_x=0;
  man_y=4;
  man_dx=0;
  man_dy=0;
  scrn_x=0;
  scrn_y=0;
  for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++) {
    tmpm->dead=FALSE;
    tmpm->dx=0;
    tmpm->dy=0;
    do {
      tmpm->x=RandU64%(map_width-2)+1;
      tmpm->y=RandU64%(map_height-2)+1;
    } while (!map[(tmpm->y)*map_width+tmpm->x]);
  }
  monsters_left=MONSTERS_NUM;
  tf=0;
  t0=tS;
}

U0 CleanUp()
{
  Free(map);
  map=NULL;
}

U0 AnimateTask(I64)
{
  I64 i,x,y,dx,dy;
  Monster *tmpm;

  while (TRUE) {
    for (i=0,tmpm=monsters;i<MONSTERS_NUM;i++,tmpm++)
      if (!tmpm->dead) {
	dx=RandU16%3-1;
	dy=RandU16%3-1;
	x=tmpm->x+dx;
	y=tmpm->y+dy;
	if (0<=x<map_width && 0<=y<map_height && map[y*map_width+x]) {
	  tmpm->x=x;
	  tmpm->y=y;
	  tmpm->dx=dx;
	  tmpm->dy=dy;
	}
      }
    Sleep(1000);
  }
}

U0 DunGen()
{
  I64 ch,sc;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Up(,,SC_CURSOR_UP);"
	"  Down(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  Attack(,CH_SPACE);"
	"}"
	);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      switch (GetMsg(&ch,&sc,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_KEY_DOWN:
	  switch (ch) {
	    case '\n':
	      Init;
	      break;
	    case CH_SPACE:
	      man_attack=TRUE;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto dg_done;
	    case 0:
	      switch (sc.u8[0]) {
		case SC_CURSOR_RIGHT:
		  if (man_attack) {
		    man_dx=1;
		    man_dy=0;
		    Attack;
		  } else
		    if (man_x+1<map_width &&
			  map[man_y*map_width+(man_x+1)]==DKGRAY) {
		      man_x++;
		      if (man_x-scrn_x>SCRN_WIDTH/2-3) {
			scrn_x+=SCRN_WIDTH/2;
			if (scrn_x+SCRN_WIDTH/2>map_width)
			  scrn_x=map_width-SCRN_WIDTH/2;
		      }
		    }
		  break;
		case SC_CURSOR_LEFT:
		  if (man_attack) {
		    man_dx=-1;
		    man_dy=0;
		    Attack;
		  } else
		    if (man_x-1>=0 &&
			  map[man_y*map_width+(man_x-1)]==DKGRAY) {
		      man_x--;
		      if (man_x-scrn_x<-SCRN_WIDTH/2+3) {
			scrn_x-=SCRN_WIDTH/2;
			if (scrn_x-SCRN_WIDTH/2<0)
			  scrn_x=SCRN_WIDTH/2;
		      }
		    }
		  break;
		case SC_CURSOR_UP:
		  if (man_attack) {
		    man_dx=0;
		    man_dy=-1;
		    Attack;
		  } else
		    if (man_y-1>=0 &&
			  map[(man_y-1)*map_width+man_x]==DKGRAY) {
		      man_y--;
		      if (man_y-scrn_y<-SCRN_HEIGHT/2+3) {
			scrn_y-=SCRN_HEIGHT/2;
			if (scrn_y-SCRN_HEIGHT/2<0)
			  scrn_y=SCRN_HEIGHT/2;
		      }
		    }
		  break;
		case SC_CURSOR_DOWN:
		  if (man_attack) {
		    man_dx=0;
		    man_dy=1;
		    Attack;
		  } else
		    if (man_y+1<map_height &&
			  map[(man_y+1)*map_width+man_x]==DKGRAY) {
		      man_y++;
		      if (man_y-scrn_y>SCRN_HEIGHT/2-3) {
			scrn_y+=SCRN_HEIGHT/2;
			if (scrn_y+SCRN_HEIGHT/2>map_height)
			  scrn_y=map_height-SCRN_HEIGHT/2;
		      }
		    }
		  break;
	      }
	  }
	  break;
	case MSG_KEY_UP:
	  if (ch==CH_SPACE)
	    man_attack=FALSE;
	  break;
      }
    }
dg_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
  RegWrite("TempleOS/DunGen","F64 best_score=%5.4f;\n",best_score);
}

DunGen;
        £        
      7      
7   üÿÿÿ7      
7      S      
S      S   üÿÿÿ
S   üÿÿÿ7   üÿÿÿ
C      C   $   
;      O      
O      O       
O       ;       
;       ;      
;      K      
K      K      
K      ?      
?      ?      
?      G      
C   (   k   (   
k   (   k   D   
k   D   C   D   
C   D   C   ,   
G   ,   g   ,   
g   ,   g   @   
g   @   G   @   
G   @   G   0   
G   0   c   0   
c   0   c   <   
c   <   K   <   
K   <   K   4   
K   4   _   4   
_   4   _   8   
_   8   O   8      
k   8   ƒ   8   
ƒ   ,   ƒ   D   
ƒ   D   Ÿ   D   
Ÿ   D   Ÿ      
Ÿ      —      
—      —   (   
—   (   ƒ   (   
‡   ,   ›   ,   
›       ›   @   
—   @   ‡   @   
‡   <   ‡   0   
‡   0   —   0   
—   0   —   <   
—   <   ‹   @   
‹   <   ‹   4   
‹   4   “   4   
   8   “   8   
“   <      <   
›      ›      
›      ‹      
‡   üÿÿÿ‡      
‡      s      
s      s   üÿÿÿ
s   üÿÿÿƒ   üÿÿÿ
ƒ   üÿÿÿ      
ƒ      ƒ       
ƒ       w       
w       w      
w      {      
{      {      
            
o      k      
g       g      
g      c      
c      c       
?   8   /   8   
+   0   +   @   
+   @       @   
    @       0   
    0   '   0   
'   0   '   <   
'   <   #   <   
#   <   #   4   
7   8   7   P   
3   P   ;   P   
;   P   ;   \   
;   \   3   \   
3   \   3   T   
7   T   7   X   
[   øÿÿÿ[   4   
[   <   [   T   
[   T   [   \      O      c   0   c   <           Ò     ýÿÿÿëÿÿÿ   &   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ
ÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ
ÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ
ÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ
ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ

ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿ
ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ
ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ
ÿÿÿÿÿÿÿÿÿÿ     ÿÿ
ÿÿÿÿÿÿ     ÿÿ
ÿ     ÿÿ     ÿ     ÿ          ÿ     ÿ     ÿÿÿ     ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             ²     ýÿÿÿüÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿ     ÿÿÿ     ÿÿ     ÿÿ     ÿÿ     ÿÿ     ÿ     ÿ     ÿ     ÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             ²     ýÿÿÿüÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ	ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ		ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ			ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ			ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ					ÿÿÿÿÿÿÿ     ÿ			ÿÿÿ     ÿ				ÿ     ÿ							ÿ     ÿ					ÿ     				ÿ     			ÿ     								ÿ     			ÿÿ     ÿÿÿÿ			ÿÿ     ÿÿÿÿÿÿÿÿ					ÿÿ     ÿÿÿÿÿÿÿÿÿÿÿ		ÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             ²     ýÿÿÿüÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ	ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ			ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ			ÿÿÿÿÿÿÿÿÿÿÿ     ÿÿ			ÿÿÿÿÿÿÿ     ÿ						ÿÿÿ     ÿ				ÿÿ     ÿ					ÿÿÿ     ÿ				ÿÿÿ     				ÿÿÿ     			ÿÿÿ     								ÿÿÿ     ÿÿÿÿÿ     ÿÿÿÿÿ					ÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿ	ÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      	       ²     ýÿÿÿüÿÿÿ      ÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿ        ÿÿÿÿÿÿÿÿÿ     ÿ         ÿÿÿÿÿ     ÿ            ÿÿÿ     ÿ           ÿÿÿ     ÿ          ÿÿÿ     ÿ           ÿÿÿ     ÿ          ÿÿÿ     ÿ         ÿÿÿ     ÿ             ÿÿÿ     ÿ           ÿÿÿ     ÿ          ÿÿÿ     ÿ           ÿÿÿ     ÿÿÿ          ÿÿÿ     ÿÿÿÿÿÿ         ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿ         ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      
       ’                  ÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿ ÿ ÿÿ   ÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ   ÿÿÿ ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ        2     õÿÿÿèÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ      ÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ         ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ‚     øÿÿÿéÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿ      ÿ ÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿ       ÿÿÿÿ         ÿ  ÿÿ    ÿ   ÿÿÿÿ  ÿ  ÿÿÿ   ÿÿÿÿ  ÿ    ÿÿÿÿ   ÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           "     ùÿÿÿìÿÿÿ      ÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  
 ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿ  ÿÿÿÿÿÿÿ    ÿÿÿ   ÿÿÿÿÿÿ  
 ÿÿÿ  ÿÿÿÿÿ   ÿÿÿ  ÿÿÿÿÿ  
 ÿÿ  ÿÿÿÿÿ  
 ÿ  ÿÿÿÿÿÿ           ÿÿÿÿÿÿÿ         ÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿÿÿÿÿÿÿÿ        ÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿÿÿÿÿ   ÿ   ÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿ   ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ    ÿÿÿÿÿÿÿÿÿÿ 