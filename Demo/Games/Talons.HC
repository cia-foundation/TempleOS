//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.

RegDft("TempleOS/Talons","F64 best_score=9999;\n");
RegExe("TempleOS/Talons");

//Keep these power of two so shift is used instead of multiply
//to index arrays.
#define MAP_WIDTH		1024
#define MAP_HEIGHT		1024

#define MAP_SCALE		150
#define DISPLAY_SCALE		100
#define CTRLS_SCALE		0.05

//I think I did these so the heads-up showed intelligable numbers.
//Scaling is a mess.
#define COORDINATE_SCALE	256
#define COORDINATE_BITS		8

#define WATER_ELEVATION		15
#define BIRD_ELEVATION		10
#define ROCK_ELEVATION		45
#define SNOW_ELEVATION		55

//Too big makes off-scrn draws take place.
#define PANEL_SIZE_MAX		16





			$SP,"<1>",BI=1$



















			$SP,"<2>",BI=2$

























			$SP,"<3>",BI=3$





















			$SP,"<4>",BI=4$















			$SP,"<5>",BI=5$















			$SP,"<6>",BI=6$








#define LS_TYPES	3
U8	*landscape_small_imgs[LS_TYPES]={$IB,"<4>",BI=4$,$IB,"<5>",BI=5$,$IB,"<6>",BI=6$},
	*landscape_large_imgs[LS_TYPES];

#define	B_NUM	256
class Bird
{
  Bird *next,*last;
  CD3I64 p;
  F64 й;
} b_head[mp_cnt];

class Obj
{
  Obj *next,*last;
  CD3I64 p;
  U8 *img;
  Bool fish;
};

class Panel
{//Polygon or Obj
  Panel *next;
  CD3I32 *pts;
  I64 cnt;
  U16 update,num_sides;
  CColorROPU32 color;
  Obj *next_obj,*last_obj;
} *panel_head,*panels[MAP_HEIGHT][MAP_WIDTH];

I64 critical_section_flag;
I16 elevations[MAP_HEIGHT][MAP_WIDTH];
CD3 normals[MAP_HEIGHT][MAP_WIDTH];

class MPCtrl {
  I64 init_not_done_flags,update_not_done_flags,app_not_done_flags;
  I64 strip_width[MP_PROCESSORS_NUM];
  Bool app_done;
} mp;

F64 game_t0,game_tf,pitch,roll,heading,и,speed;
Bool invert_pitch,rolled_over;
I64 strip_height,x,y,z,fish_left;
CD3 v;

U0 WrapAngles()
{
  I64 r[4][4],x,y,z;

  и=Wrap(и);
  pitch=Wrap(-и-г/2);
  if (Abs(pitch)>г/2) {
    invert_pitch=TRUE;
    pitch=Wrap(г-pitch);
  } else
    invert_pitch=FALSE;
  roll=Wrap(roll);
  if (invert_pitch ^^ -г/2<=roll<г/2)
    rolled_over=FALSE;
  else
    rolled_over=TRUE;
  heading=Wrap(heading,0);

  //World to scrn coordinates
  Mat4x4IdentEqu(r);
  Mat4x4RotZ(r,heading);
  Mat4x4RotX(r,и);
  Mat4x4RotZ(r,roll);

  //We use velocity vector for dog-fighting.
  x=0x100000000*speed; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  v.x=x/ToF64(0x100000000); v.y=y/ToF64(0x100000000); v.z=z/ToF64(0x100000000);
}

U0 EDTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
{
  I64 zz;
  Mat4x4MulXYZ(dc->r,x,y,z);
  *z=zz=-*z;
  if (zz>0) {
    *x = dc->x + *x * DISPLAY_SCALE/zz; //Foreshortening
    *y = dc->y - *y * DISPLAY_SCALE/zz;
  } else {
    *x = dc->x + *x;
    *y = dc->y - *y;
  }
}

U0 CalcNormals()
{/*Find the normal vect with a curl.

i,j and k are the axis unit vectors,
not to be confused with my local index variables.

i  j  k
0  1  dz2
1  0  dz1

Normal:  dz1*i + dz2*j - k
*/
  I64 i,j;
  for (j=0;j<MAP_HEIGHT-1;j++) {
    for (i=0;i<MAP_WIDTH-1;i++) {
      normals[j][i].x=elevations[j][i+1]-elevations[j][i];
      normals[j][i].y=elevations[j+1][i]-elevations[j][i];
      normals[j][i].z=-1;
      D3Unit(&normals[j][i]);
    }
    MemSet(&normals[j][i],0,sizeof(CD3));
  }
  for (i=0;i<MAP_WIDTH-1;i++)
    MemSet(&normals[j][i],0,sizeof(CD3));
}

Bool TestSameSlope(I64 x,I64 y,I64 w,I64 h)
{
  CD3 p,*s;
  I64 k1,k2;
  if (!(0<=x && x+w<MAP_WIDTH && 0<=y && y+h<MAP_HEIGHT))
    return FALSE;
  s=&normals[y][x];
  for (k2=0;k2<h;k2++)
    for (k1=0;k1<w;k1++)
      if (D3NormSqr(D3Sub(&p,&normals[y+k2][x+k1],s))>.10)
	return FALSE;
  return TRUE;
}

U0 MPDoPanels(CTask *task)
{
  I64 i,j,l,k1,k2,w,h,threshold,lo,hi;
  Bool cont;
  Panel *tmpp,*start_ptr=NULL,*end_ptr=NULL;
  CD3I32 *poly;
  Obj *tmpo;
  lo=Gs->num*(MAP_HEIGHT-1)/mp_cnt;
  hi=(Gs->num+1)*(MAP_HEIGHT-1)/mp_cnt;
  for (threshold=8;threshold>=1;threshold--)
    for (j=lo;j<hi;j++) {
      for (i=0;i<MAP_WIDTH-1;i++) {
	if (!panels[j][i]) {
	  w=1;
	  h=1;
	  do {
	    cont=FALSE;
	    if (w<PANEL_SIZE_MAX && TestSameSlope(i,j,w+1,h)) {
	      w++;
	      cont=TRUE;
	    }
	    if (h<PANEL_SIZE_MAX && TestSameSlope(i,j,w,h+1)) {
	      h++;
	      cont=TRUE;
	    }
	  } while (cont);
	  if (w>=threshold || h>=threshold) {
	    tmpp=CAlloc(sizeof(Panel),task);
	    QueInit(&tmpp->next_obj);
	    l=elevations[j][i];
	    if (l<=WATER_ELEVATION*MAP_SCALE &&
		  elevations[j][i+w-1]<=WATER_ELEVATION*MAP_SCALE &&
		  elevations[j+h-1][i]<=WATER_ELEVATION*MAP_SCALE &&
		  elevations[j+h-1][i+w-1]<=WATER_ELEVATION*MAP_SCALE) {
	      tmpp->color=BLUE;
	      if (Rand<0.05) {
		tmpo=MAlloc(sizeof(Obj),task);
		tmpo->p.x=(i+w/2)*MAP_SCALE;
		tmpo->p.y=(j+h/2)*MAP_SCALE;
		if (Rand<0.1) {
		  tmpo->fish=FALSE;
		  if (RandI16&1)
		    tmpo->img=landscape_large_imgs[0]; //Boat
		  else
		    tmpo->img=landscape_large_imgs[1]; //Boat
		  tmpo->p.z=(WATER_ELEVATION+2)*MAP_SCALE;
		} else {
		  tmpo->fish=TRUE;
		  tmpo->img=$IB,"<1>",BI=1$; //Fish
		  tmpo->p.z=WATER_ELEVATION*MAP_SCALE;
		}
		QueIns(tmpo,tmpp->last_obj);
	      }
	    } else {
	      if (l<ROCK_ELEVATION*MAP_SCALE) {
		if (RandI16&1)
		  tmpp->color=LTGREEN;
		else
		  tmpp->color=GREEN+LTGREEN<<16+ROPF_DITHER;
		if (Rand<0.03) {
		  tmpo=MAlloc(sizeof(Obj),task);
		  tmpo->p.x=(i+w/2)*MAP_SCALE;
		  tmpo->p.y=(j+h/2)*MAP_SCALE;
		  tmpo->p.z=l;
		  tmpo->img=landscape_large_imgs[2]; //Tree
		  tmpo->fish=FALSE;
		  QueIns(tmpo,tmpp->last_obj);
		}
	      } else if (l<SNOW_ELEVATION*MAP_SCALE) {
		if (!(RandU16&3)) {
		  if (RandI16&1)
		    tmpp->color=LTGRAY;
		  else
		    tmpp->color=DKGRAY+LTGRAY<<16+ROPF_DITHER;
		} else {
		  if (RandI16&1)
		    tmpp->color=LTGREEN;
		  else
		    tmpp->color=GREEN+LTGREEN<<16+ROPF_DITHER;
		}
	      } else {
		if (!(RandU16&3)) {
		  if (RandI16&1)
		    tmpp->color=WHITE;
		  else
		    tmpp->color=LTGRAY;
		} else {
		  if (RandI16&1)
		    tmpp->color=LTGRAY+WHITE<<16+ROPF_DITHER;
		  else
		    tmpp->color=DKGRAY+LTGRAY<<16+ROPF_DITHER;
		}
	      }
	    }
	    tmpp->num_sides=4;
	    poly=tmpp->pts=MAlloc(sizeof(CD3I32)*tmpp->num_sides,task);
	    poly[0].x=MAP_SCALE*i;
	    poly[0].y=MAP_SCALE*j;
	    poly[0].z=elevations[j][i];
	    poly[1].x=MAP_SCALE*(i+w);
	    poly[1].y=MAP_SCALE*j;
	    poly[1].z=elevations[j][i+w];
	    poly[2].x=MAP_SCALE*(i+w);
	    poly[2].y=MAP_SCALE*(j+h);
	    poly[2].z=elevations[j+h][i+w];
	    poly[3].x=MAP_SCALE*i;
	    poly[3].y=MAP_SCALE*(j+h);
	    poly[3].z=elevations[j+h][i];
	    tmpp->next=start_ptr;
	    start_ptr=tmpp;
	    if (!end_ptr)
	      end_ptr=tmpp;
	    for (k2=0;k2<h;k2++)
	      for (k1=0;k1<w;k1++)
		panels[j+k2][i+k1]=tmpp;
	  }
	}
      }
    }
  if (end_ptr) {
    while (LBts(&critical_section_flag,0))
      Yield;
    if (end_ptr)
      end_ptr->next=panel_head;
    panel_head=start_ptr;
    LBtr(&critical_section_flag,0);
  }
  LBtr(&mp.init_not_done_flags,Gs->num);
}

U0 InitElevations()
{
  I64 i,j,l,x,y,xx,yy,x1,y1,x2,y2;
  MemSet(elevations,0,sizeof(elevations));
  for (i=0;i<MAP_WIDTH*MAP_HEIGHT/128;i++) {
    x=RandU32%MAP_WIDTH;
    y=RandU32%MAP_HEIGHT;
    j=1<<(RandU32%6);
    l=0;
    while (j--) {
      if (!l && RandU16<U16_MAX/4)
	l=RandU16%(j+1);
      if (l) {
	x1=ClampI64(x-j,0,MAP_WIDTH-1);
	x2=ClampI64(x+j,0,MAP_WIDTH-1);
	y1=ClampI64(y-j,0,MAP_HEIGHT-1);
	y2=ClampI64(y+j,0,MAP_HEIGHT-1);
	for (yy=y1;yy<y2;yy++)
	  for (xx=x1;xx<x2;xx++)
	    elevations[yy][xx]+=MAP_SCALE/2;
	l--;
      }
    }
  }

  for (j=0;j<MAP_HEIGHT;j++)
    for (i=0;i<MAP_WIDTH;i++)
      if (elevations[j][i]<WATER_ELEVATION*MAP_SCALE)
	elevations[j][i]=WATER_ELEVATION*MAP_SCALE;
}

U0 InitMap()
{/*We make a topographic data structure "elevations[][]"
and convert it to panels. "panels[][]" holds the panels
for each spot.
*/
  I64 i;
  InitElevations;
  MemSet(panels,0,sizeof(panels));
  CalcNormals;
  panel_head=NULL;
  mp.init_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    Spawn(&MPDoPanels,Fs,"Do Panels",i);
  while (mp.init_not_done_flags)
    Sleep(1);
}

F64 claws_down;

#define HAND_X	(1.0-claws_down)*(0.3*w)+claws_down*(0.4*w)
#define HAND_Y	(1.0-claws_down)*(h-125)+claws_down*h




$SP,"<7>",BI=7$
$SP,"<8>",BI=8$



U0 ClawDraw(CDC *dc,I64 x1,I64 y1,I64 x2,I64 y2,I64 w,I64 segments,Bool talon)
{
  I64 i,j;
  for (i=0,j=segments;i<segments;i++,j--) {
    dc->thick=w;
    dc->color=BLACK;
    GrLine3(dc,
	j*x1/segments+i*x2/segments,
	j*y1/segments+i*y2/segments,0,
	(j-1)*x1/segments+(i+1)*x2/segments,
	(j-1)*y1/segments+(i+1)*y2/segments,0);
    dc->thick=w-2;
    dc->color=YELLOW;
    GrLine3(dc,
	j*x1/segments+i*x2/segments,
	j*y1/segments+i*y2/segments,0,
	(j-1)*x1/segments+(i+1)*x2/segments,
	(j-1)*y1/segments+(i+1)*y2/segments,0);
  }
  if (talon) {
    if (y1<y2)
      Sprite3B(dc,x1,y1,0,$IB,"<7>",BI=7$);
    else
      Sprite3B(dc,x1,y1,0,$IB,"<8>",BI=8$);
  }
}

U0 ClawsDraw(CTask *task,CDC *dc)
{
  F64 claws_up=1.0-claws_down;
  I64 w=task->pix_width,h=task->pix_height;
  dc->flags|=DCF_SYMMETRY;
  DCSymmetrySet(dc,w>>1,0,w>>1,1);

  ClawDraw(dc,HAND_X-30,HAND_Y-50*claws_up,HAND_X-5,HAND_Y,	22,4,TRUE);
  ClawDraw(dc,HAND_X-10,HAND_Y-60*claws_up,HAND_X,HAND_Y,	22,4,TRUE);
  ClawDraw(dc,HAND_X+10,HAND_Y-60*claws_up,HAND_X,HAND_Y,	22,4,TRUE);
  ClawDraw(dc,HAND_X+30,HAND_Y-50*claws_up,HAND_X+5,HAND_Y,	22,4,TRUE);
  ClawDraw(dc,HAND_X+25,HAND_Y+40*claws_up,HAND_X+5,HAND_Y,	22,4,TRUE);

  ClawDraw(dc,HAND_X,HAND_Y,6*w/20,h,38,5,FALSE);
}

CDC *main_dc;
U0 DrawIt(CTask *task,CDC *dc)
{
  main_dc->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,0,0,main_dc);
  if (claws_down)
    ClawsDraw(task,dc);
}

/*
$SP,"<9>",BI=9$


















$SP,"<10>",BI=10$
































Cores render strips that +/- 15%. The cores check the panel map array
and render the panel for each square, marking-it done.

The depth buf is not locked in the graphic routines
so we get some glitches.
*/

I64 update_jiffy_limit;

U0 MPDrawIt(CTask *task,CDC *dc)
{
  I64 j,update=winmgr.updates&65535,strip_width,*s2w,x1,y1,z1,xx,yy,
	xh,yh,zh,yh2,xh2,x1h,y1h,x1wa,y1wa,x1wb,y1wb,x3,y3,z3,dd,dd_old,
	cx=task->pix_width>>1,cy=task->pix_height>>1,r[16],*elems1,*elems_hard;
  Panel reg *tmpp;
  Bird	*tmpb;
  Obj	*tmpo;
  Bool	w_on_map,h_on_map;

  xx=x/(MAP_SCALE*COORDINATE_SCALE);
  yy=y/(MAP_SCALE*COORDINATE_SCALE);

  //World to scrn coordinates
  Mat4x4IdentEqu(dc->r);
  Mat4x4RotZ(dc->r,heading);
  Mat4x4RotX(dc->r,и);
  Mat4x4RotZ(dc->r,roll);
  DCMat4x4Set(dc,dc->r);

  //Scrn to world coordinates

  //This gives us the vects for stepping through the grid in
  //the direction the plane is facing. we step horizontally and vertically
  //and use the reciprocal slope principle
  //y=mx+b and y=(-1/m)x+b are perpendicular.

  s2w=Mat4x4IdentNew;
  Mat4x4RotZ(s2w,-roll);
  Mat4x4RotX(s2w,-и);
  Mat4x4RotZ(s2w,-heading);

  xh=0;
  yh=0;
  zh=-256;
  Mat4x4MulXYZ(s2w,&xh,&yh,&zh);

  //The layer for core1 is not cleared automatically
  //it is persistent.  I have carefully syncronized to the update
  //cycle initiated by Core0 to prevent flicker.

  dc->flags|=DCF_TRANSFORMATION;
  dc->transform=&EDTransform;
  dc->x=cx;
  dc->y=cy;

  //dc->x and the translation part of dc->r are ident in effect,
  //but we use both to show-off.	We could add offsets together and
  //use one or the other.

  x1=-x>>COORDINATE_BITS;
  y1=-y>>COORDINATE_BITS;
  z1=-z>>COORDINATE_BITS;
  Mat4x4MulXYZ(dc->r,&x1,&y1,&z1);
  Mat4x4TranslationEqu(dc->r,x1,y1,z1);

  //This is a refinement.
  if (Abs(и*180/г)>90) {
    x3=0;
    y3=-cy;
    z3=0;
    Mat4x4MulXYZ(s2w,&x3,&y3,&z3);
    xx+=x3;
    yy+=y3;
  }

  if (Gs->num&1) {//alt left-right,right-left
    yh2=-yh;
    xh2=-xh;
  } else {
    yh2=yh;
    xh2=xh;
  }

  //Calc starting point.
  x1wa=xx<<8+xh*strip_height>>1/1.3*(Gs->num+1.15);
  y1wa=yy<<8+yh*strip_height>>1/1.3*(Gs->num+1.15);
  x1wb=0;
  y1wb=0;

  xh=-xh; //Back to front to help with depth.
  yh=-yh;

  //Take half steps to cover whole grid.
  xh>>=1;  yh>>=1;
  xh2>>=1; yh2>>=1;
  w_on_map=FALSE;
  dd_old=I64_MAX;
  for (strip_width=0;cnts.jiffies<update_jiffy_limit;strip_width++) {
    x1h=x1wa;
    y1h=y1wa;
    h_on_map=FALSE;
    for (j=0;j<strip_height && cnts.jiffies<update_jiffy_limit;j++) {
      x1=x1h>>8; y1=y1h>>8;
      if (0<=x1<MAP_WIDTH && 0<=y1<MAP_HEIGHT) {
	if ((tmpp=panels[y1][x1]) && tmpp->update!=update) {
	  tmpp->update=update;
	  if (tmpp->cnt>8*(1.1-Gs->idle_factor)) {
	    dc->color=tmpp->color;
	    tmpp->cnt=GrFillPoly3(dc,tmpp->num_sides,tmpp->pts);
	  } else
	    tmpp->cnt++;
	  tmpo=tmpp->next_obj;
	  while (tmpo!=&tmpp->next_obj) {
	    Sprite3(dc,tmpo->p.x,tmpo->p.y,tmpo->p.z,tmpo->img);
	    tmpo=tmpo->next;
	  }
	}
	h_on_map=TRUE;
      } else if (h_on_map)
	break;
      x1h+=xh;
      y1h+=yh;
    }
    if (h_on_map)
      w_on_map=TRUE;
    else if (w_on_map) {
      strip_width=I64_MAX;
      break;
    }
    x1wb-=yh2;
    y1wb+=xh2;
    if (strip_width&1) {
      x1wa-=x1wb;
      y1wa-=y1wb;
    } else {
      x1wa+=x1wb;
      y1wa+=y1wb;
    }
    if (!w_on_map) {
      dd=SqrI64(x1wa>>8-MAP_WIDTH>>1)+SqrI64(y1wa>>8-MAP_HEIGHT>>1);
      if (dd>dd_old)
	break;
      dd_old=dd;
    }
  }

  tmpb=b_head[Gs->num].next;
  while (tmpb!=&b_head[Gs->num]) {
    elems1=SpriteInterpolate(Tri(tS,0.2),$IB,"<2>",BI=2$,$IB,"<3>",BI=3$);

    Mat4x4IdentEqu(r);
    Mat4x4RotZ(r,tmpb->й);
    elems_hard=SpriteTransform(elems1,r);

    Sprite3(dc,tmpb->p.x,tmpb->p.y,tmpb->p.z,elems_hard);
    Free(elems_hard);
    Free(elems1);

    tmpb=tmpb->next;
  }

  Free(s2w);
  mp.strip_width[Gs->num]=strip_width;
  LBtr(&mp.update_not_done_flags,Gs->num);
}

U0 CoreAPTalonsTask(CTask *master_task)
{
  CDC *dc=DCAlias(main_dc,master_task);
  while (!mp.app_done) {
    while (!Bt(&mp.update_not_done_flags,Gs->num) && !mp.app_done)
      Sleep(1);
    if (!mp.app_done)
      MPDrawIt(master_task,dc);
  }

  //We made an alias of this we don't want freed.
  dc->depth_buf=NULL;

  DCDel(dc);
  LBtr(&mp.app_not_done_flags,Gs->num);
}

U0 DrawHorizon(CDC *dc)
{
  I64 x1,y1,z1,x2,y2,z2,xh,yh,zh,*s2w=Mat4x4IdentNew,
	cx=Fs->pix_width>>1,
	cy=Fs->pix_height>>1;
  CD3I32 p[4];
  I32 *old_db=dc->depth_buf;
  dc->depth_buf=NULL;
  CColorROPU32 ground_color;

  if (game_tf && fish_left)
    DCFill(dc,BLACK);
  else if (-г/4<=Wrap(и-г)<г/4)
    DCFill(dc,LTCYAN);
  else {
    if (z/COORDINATE_SCALE<(WATER_ELEVATION+3)*MAP_SCALE)
      ground_color=BLUE;
    else
      ground_color=LTGREEN;

    Mat4x4IdentEqu(dc->r);
    Mat4x4RotZ(dc->r,heading);
    Mat4x4RotX(dc->r,и);
    Mat4x4RotZ(dc->r,roll);

    DCMat4x4Set(dc,dc->r);
    dc->flags&=~DCF_TRANSFORMATION;
    dc->transform=&EDTransform;
    dc->x=cx;
    dc->y=cy;

    Mat4x4RotZ(s2w,-roll);
    Mat4x4RotX(s2w,-и);
    Mat4x4RotZ(s2w,-heading);

    xh=0;
    yh=0;
    zh=-256;
    Mat4x4MulXYZ(s2w,&xh,&yh,&zh);
    Free(s2w);

    x1=xh+yh*32; y1=yh-xh*32; z1=0;
    (*dc->transform)(dc,&x1,&y1,&z1);
    x2=xh-yh*32; y2=yh+xh*32; z2=0;
    (*dc->transform)(dc,&x2,&y2,&z2);
    DCClipLine(dc,&x1,&y1,&x2,&y2);

    MemSet(p,0,sizeof(p));
    if (x2<x1) {
      SwapI64(&x1,&x2);
      SwapI64(&y1,&y2);
    }
    if (!x1 && x2==dc->width-1) {
      p[0].x=0;
      p[0].y=0;
      p[1].x=dc->width-1;
      p[1].y=0;
      p[2].x=dc->width-1;
      p[2].y=y2;
      p[3].x=0;
      p[3].y=y1;
      if (rolled_over)
	dc->color=ground_color;
      else
	dc->color=LTCYAN;
      GrFillPoly3(dc,4,p);
      p[0].y=dc->height-1;
      p[1].y=dc->height-1;
      if (rolled_over)
	dc->color=LTCYAN;
      else
	dc->color=ground_color;
      GrFillPoly3(dc,4,p);
    } else {
      if (y2<y1) {
	SwapI64(&x1,&x2);
	SwapI64(&y1,&y2);
      }
      if (!y1 && y2==dc->height-1) {
	p[0].x=0;
	p[0].y=0;
	p[1].x=0;
	p[1].y=dc->height-1;
	p[2].x=x2;
	p[2].y=dc->height-1;
	p[3].x=x1;
	p[3].y=0;
	if (x1<x2 ^^ rolled_over)
	  dc->color=ground_color;
	else
	  dc->color=LTCYAN;
	GrFillPoly3(dc,4,p);
	p[0].x=dc->width-1;
	p[1].x=dc->width-1;
	if (x1<x2 ^^ rolled_over)
	  dc->color=LTCYAN;
	else
	  dc->color=ground_color;
	GrFillPoly3(dc,4,p);
      } else
	DCFill(dc,LTCYAN); //Not correct.
    }
  }

  dc->depth_buf=old_db;
}

U0 Core0Talons()
{
  CDC *dc=DCAlias(main_dc,Fs);
  I64 i,xx,yy,elev,height,cx=Fs->pix_width>>1,cy=Fs->pix_height>>1;
  F64 min_strip_width,tt;

  update_jiffy_limit=cnts.jiffies+JIFFY_FREQ/40;

  xx=x/(MAP_SCALE*COORDINATE_SCALE);
  yy=y/(MAP_SCALE*COORDINATE_SCALE);
  if (0<=xx<MAP_WIDTH && 0<=yy<MAP_HEIGHT)
    elev=elevations[yy][xx];
  else
    elev=I64_MAX;

  height=z/COORDINATE_SCALE-elev;
  if (height<0 && elev>WATER_ELEVATION*MAP_SCALE && !game_tf) {
    music.mute=TRUE;
    Beep;
    game_tf=tS;
    music.mute=FALSE;
  }

  DrawHorizon(dc);

  if (game_tf) {
    tt=game_tf-game_t0;
    if (Blink) {
      dc->color=RED;
      GrPrint(dc,(Fs->pix_width-9*FONT_WIDTH)/2,
	    (Fs->pix_height-FONT_HEIGHT)/2,"Game Over");
    }
  } else {
    DCDepthBufRst(dc);
    mp.update_not_done_flags=1<<mp_cnt-1;
    MPDrawIt(Fs,dc);
    while (mp.update_not_done_flags)
      Sleep(1);

    min_strip_width=F64_MAX;
    for (i=0;i<mp_cnt;i++)
      min_strip_width=Min(min_strip_width,32.0*mp.strip_width[i]/(i+32.0));
    strip_height=ClampI64(
	  strip_height*Clamp(0.25*min_strip_width/strip_height,0.9,1.1),
	  64,1024);

    tt=tS-game_t0;
    if (tt<5.0 && Blink) {
      dc->color=WHITE;
      GrPrint(dc,(Fs->pix_width-13*FONT_WIDTH)/2,
	    (Fs->pix_height-FONT_HEIGHT)/2-4*FONT_HEIGHT,"Catch 10 Fish");
    }
  }

  dc->thick=2;
  if (game_tf && fish_left)
    dc->color=WHITE;
  else
    dc->color=BLACK;
  dc->flags&=~DCF_TRANSFORMATION;
  GrLine3(dc,cx+5,cy,0,cx-5,cy,0);
  GrLine3(dc,cx,cy+5,0,cx,cy-5,0);
  if (invert_pitch)
    GrPrint(dc,0,0,"Pitch:%5.1f Roll:%5.1f Heading:%5.1f "
	  "Height:%5d [Core Strip:%3d]",
	  pitch*180/г,Wrap(roll+г)*180/г,Wrap(heading+г,0)*180/г,
	  height,strip_height);
  else
    GrPrint(dc,0,0,"Pitch:%5.1f Roll:%5.1f Heading:%5.1f "
	  "Height:%5d [Core Strip:%3d]",
	  pitch*180/г,roll*180/г,heading*180/г,height,strip_height);
  GrPrint(dc,0,FONT_HEIGHT,"Fish Remaining:%d Time:%3.2f Best:%3.2f",
	fish_left,tt,best_score);

  //We made an alias of this we don't want freed.
  dc->depth_buf=NULL;
  DCDel(dc);
  Refresh;
}


Obj *FishFind(I64 x1,I64 y1,I64 *_dd)
{
  I64 dd,best_dd=I64_MAX;
  Obj *res=NULL,*tmpo;
  Panel *tmpp=panel_head;
  while (tmpp) {
    tmpo=tmpp->next_obj;
    while (tmpo!=&tmpp->next_obj) {
      if (tmpo->fish) {
	dd=SqrI64(tmpo->p.x-x1)+SqrI64(tmpo->p.y-y1);
	if (dd<best_dd) {
	  best_dd=dd;
	  res=tmpo;
	}
      }
      tmpo=tmpo->next;
    }
    tmpp=tmpp->next;
  }
  *_dd=best_dd;
  return res;
}

#define ANIMATE_MS	10

U0 AnimateTask(I64)
{//Steadily moves the airplane fwd.
  I64 i,*s2w,x1,y1,z1,dx,dy,dz,dd;
  F64 t0=tS,ms,й,d;
  Obj *tmpo;
  Bird	*tmpb;
  while (TRUE) {
    ms=1000*(tS-t0);
    t0=tS;
    if (!game_tf) {
//Scrn to world coordinates
      s2w=Mat4x4IdentNew;
      Mat4x4RotZ(s2w,-roll);
      Mat4x4RotX(s2w,-и);
      Mat4x4RotZ(s2w,-heading);

      dx=0;dy=0;dz=1<<16;
      Mat4x4MulXYZ(s2w,&dx,&dy,&dz);
      x-=speed*ms*COORDINATE_SCALE*dx/1<<16;
      y-=speed*ms*COORDINATE_SCALE*dy/1<<16;
      z-=speed*ms*COORDINATE_SCALE*dz/1<<16;
      Free(s2w);
      x1=x/COORDINATE_SCALE; y1=y/COORDINATE_SCALE; z1=z/COORDINATE_SCALE;
      if (z1<(WATER_ELEVATION+3)*MAP_SCALE) {
	if (z1<WATER_ELEVATION*MAP_SCALE) {
	  z=WATER_ELEVATION*MAP_SCALE*COORDINATE_SCALE;
	  if (invert_pitch) {
	    if (pitch<-г/8)
	      и+=ms/1000.0*Sign(3*г/8-и);
	  } else if (pitch<-г/8)
	    и+=ms/1000.0*Sign(-3*г/8-и);
	  WrapAngles;
	}
	speed=1.0;
	if (rolled_over || !(tmpo=FishFind(x1,y1,&dd)))
	  claws_down=0;
	else {
	  d=Sqrt(dd);
	  x1-=tmpo->p.x;
	  y1-=tmpo->p.y;
	  й=ACos((dx*x1+dy*y1)/(d*1<<16));
	  if (й>0 && d<MAP_SCALE*4)
	    claws_down=Saw(d,MAP_SCALE*4);
	  else
	    claws_down=0.01;
	  if (d<MAP_SCALE*2) {
	    if (!--fish_left) {
	      game_tf=tS;
	      if (game_tf-game_t0<best_score)
		best_score=game_tf-game_t0;
	    }
	    QueRem(tmpo);
	    Free(tmpo);
	    music.mute=TRUE;
	    Snd(74); Sleep(200); Snd;
	    music.mute=FALSE;
	  }
	}
      } else {
	claws_down=0;
	if (-г/4<=и<=г/4)
	  speed+=0.0005;
	else if (-3*г/4<=и<=3*г/4)
	  speed+=0.0001;
	else
	  speed-=0.0001;
	speed=Clamp(speed+(0.0005-0.0002*Abs(и)/(г/4)),0.1,5.0);
      }

      for (i=0;i<mp_cnt;i++) {
	tmpb=b_head[i].next;
	while (tmpb!=&b_head[i]) {
	  tmpb->p.x+=10*ms/1000*MAP_SCALE*Cos(tmpb->й);
	  tmpb->p.y+=10*ms/1000*MAP_SCALE*Sin(tmpb->й);
	  tmpb->p.z=BIRD_ELEVATION*MAP_SCALE+
		elevations[tmpb->p.y/MAP_SCALE][tmpb->p.x/MAP_SCALE];
	  tmpb->й+=2*г*ms/1000/10;
	  tmpb=tmpb->next;
	}
      }
    }
    Refresh;
  }
}

U0 MPEnd()
{
  update_jiffy_limit=0;
  mp.app_not_done_flags=1<<mp_cnt-1-1;
  mp.app_done=TRUE;
//Wait for all cores to exit
  while (mp.app_not_done_flags)
    Sleep(1);
}

U0 TaskEndCB()
{
  MPEnd;
  Exit;
}

U0 SongTask(I64)
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("5eCGFsD4A5e.C4sG5eGDqCDeGsGG4qG");
    Play("5eCGFsD4A5e.C4sG5eGDqCDeGsGG4qG");
    Play("5eGECGC4A5FCsC4B5C4B5e.GsG4qGB");
    Play("5eGECGC4A5FCsC4B5C4B5e.GsG4qGB");
  }
}

U0 PreInit()
{
  I64 i,*r=Mat4x4IdentNew;
  Mat4x4Scale(r,10.0);
  for (i=0;i<LS_TYPES;i++)
    landscape_large_imgs[i]=SpriteTransform(landscape_small_imgs[i],r);
  Free(r);
}

U0 PostCleanUp()
{
  I64 i;
  for (i=0;i<LS_TYPES;i++)
    Free(landscape_large_imgs[i]);
}

U0 Init()
{
  I64 i,xx,yy;
  Bird *tmpb;

  main_dc=DCNew(GR_WIDTH,GR_HEIGHT);
  critical_section_flag=0;
  game_tf=0;
  fish_left=10;

  MemSet(&mp,0,sizeof(MPCtrl));
  InitMap;
  DCDepthBufAlloc(main_dc);

  strip_height=128;

  и	=-90.0*г/180.0;
  roll	=0;
  heading=0;
  speed =2.5;
  claws_down=0;
  WrapAngles;

  x=MAP_WIDTH>>1 *COORDINATE_SCALE*MAP_SCALE;
  y=MAP_HEIGHT>>1*COORDINATE_SCALE*MAP_SCALE;
  z=64		 *COORDINATE_SCALE*MAP_SCALE;

  xx=x/(MAP_SCALE*COORDINATE_SCALE);
  yy=y/(MAP_SCALE*COORDINATE_SCALE);
  z+=elevations[yy][xx]*COORDINATE_SCALE;

  for (i=0;i<mp_cnt;i++)
    QueInit(&b_head[i]);

  for (i=0;i<B_NUM;i++) {
    tmpb=MAlloc(sizeof(Bird));
    tmpb->p.x=Rand*MAP_WIDTH*MAP_SCALE;
    tmpb->p.y=Rand*MAP_HEIGHT*MAP_SCALE;
    tmpb->p.z=BIRD_ELEVATION*MAP_SCALE+
	  elevations[tmpb->p.y/MAP_SCALE][tmpb->p.x/MAP_SCALE];
    tmpb->й=2*г*Rand;
    QueIns(tmpb,b_head[i%mp_cnt].last);
  }

  for (i=1;i<mp_cnt;i++)
    Spawn(&CoreAPTalonsTask,Fs,"AP Talons",i);
  Fs->task_end_cb=&TaskEndCB;
  game_t0=tS;
}

U0 CleanUp()
{
  I64 i;
  Panel *tmpp=panel_head,*tmpp1;
  MPEnd;
  while (tmpp) {
    tmpp1=tmpp->next;
    QueDel(&tmpp->next_obj);
    Free(tmpp->pts);
    Free(tmpp);
    tmpp=tmpp1;
  }
  for (i=0;i<mp_cnt;i++) {
    QueDel(&b_head[i]);
    QueInit(&b_head[i]);
  }
  DCDel(main_dc);
}

U0 Talons()
{
  I64 ch,sc;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Down(,,SC_CURSOR_UP);"
	"  Up(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"}"
	);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  "Initializing...\n";
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  PreInit;
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;
  try //in case <CTRL-ALT-c> is pressed.
    do {
      if (ScanKey(&ch,&sc)) {
	switch (ch) {
	  case 0:
	    switch (sc.u8[0]) {
	      start:
		case SC_CURSOR_DOWN:
		  и-=CTRLS_SCALE*Cos(roll);
		  heading-=CTRLS_SCALE*Sin(roll)*Sin(и);
		  break;
		case SC_CURSOR_UP:
		  и+=CTRLS_SCALE*Cos(roll);
		  heading+=CTRLS_SCALE*Sin(roll)*Sin(и);
		  break;
		case SC_CURSOR_RIGHT:
		  roll+=CTRLS_SCALE;
		  break;
		case SC_CURSOR_LEFT:
		  roll-=CTRLS_SCALE;
		  break;
	      end:
		WrapAngles;
	    }
	    break;
	  case '\n':
	    Fs->draw_it=NULL;
	    CleanUp;
	    Refresh;
	    Init;
	    Fs->draw_it=&DrawIt;
	    break;
	}
      } else
	Core0Talons;
    } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
  catch
    PutExcept;
  SettingsPop;
  CleanUp;
  PostCleanUp;
  MenuPop;
  RegWrite("TempleOS/Talons","F64 best_score=%5.4f;\n",best_score);
}

Talons;
        ђ          
   аяяяэяяя   
эяяя      "   
   "   яяяя+   
яяяя+      &   
   &         
      	   ьяяя
	   ьяяя   бяяя
   бяяя   )    	       :_     йяяя   Ў   ‘   яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                                                                                                                                               яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                          яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяя         яяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяя          яяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяя  яяя    яяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяя     яяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяя   я  яяяя яяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяя   яя  яяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя  яяя  я яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя яяяяя   яяя  яяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя                                                                                                                                                                          яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяя   яяяя  яяя яяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяя   яяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя   яяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя я  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя я   яяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя   яяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя   яяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя  яяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя   яяяяяяяяяяяяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяя    яяяяяяяяяяя яяяяяяяяяяяяя яяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя   я яяяяяяяяяяяяя яяяяяяяяяяяяяяяя яяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя   яя яяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя  яяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя  яяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                           яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                                 яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                                 яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                                 яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя         яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя яяяяяяяяяяяяяяяяяяя                                                                                                                                                                                 
       Љ    ляяяэяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я                                                                                                                                                                                                                                             я       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                      яяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяя                                                            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                              яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя            яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                                                                                                                                                                                                                   я                                                                                                                                                                                                                                                   я                                                                                                                                                                                                                                                   я                                                                                                                                                                                                                                                   я                                                                                                                                                                                                                                                   я                                                                                                                                                                                                                                               я       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                        яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя                  яяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяя            яяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя    яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя     яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя      яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя                                                                                                                                                                                                                                                       я                                                                                                                                                                                                                                                       я                                                                                                                                                                                                                                                       я                                                                                                                                                                                                                                                       я                                                                                                                                                                                                                                                       я                                                                                                                                                                                                                                                       я       яяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяяя               b     юяяявяяя      яяяяя яяяяя     яяяя  яяяяя     яяяя  яяяяя     яяяя  яяяяя     яяя  яяяя     яя  яяяя     яя  яяяя     яя  яяяя     яя  яяя     я  яяя     я  яяя     я  яяя     я  яяя     я  яяя       яяя       яяя       яя       яя       я       я                             ‚     юяяя         яяяяяяяяяяяяя              яя     яяя     яяя     яяяя     яяяя     яяяяя     яяяяя   я  яяяяя   я  яяяяя   я  яяяяя   я  яяяяя   я  яяяяя   яя  яяяяя   яя  яяяяяя   яя  яяяяяя   яя  яяяяяя   яяя  яяяяяя   яяяя  яяяяяяя   яяяя  яяяяяяя   яяяя  яяяяяяя   яяяяя яяяяяяя   яяяяяяяяяяяяя           К           шяяямяяя       мяяя       шяяя                        шяяя       мяяя       мяяяшяяя    юяяяыяяя•      ыяяя•      юяяя•         •         •   юяяя   •   ыяяя   •   ыяяяюяяя•             	      	                   
      
   	                        
                                                                                                                                       P     
         Уяяязяяя   %   зяяя    ;   зяяяйяяя%   зяяяйяяяУяяязяяя!   јяяя    !   8           Y       Юяяя8       Юяяяјяяя                                                                                       	      	                            	                                                    	            	            	   	   	   яяяяяяяя        яяяя                яяяя        яяяяяяяя•       яяяя•           •   яяяя    •       Яяяя	   0            0            0           0           0           0           0            0           0           p   Ё      ьяяя3   ьяяяьяяя3      ьяяя-   ьяяяьяяя-         3   ьяяя   3         -   ьяяя   -      ьяяя-   щяяяьяяя-      ьяяя   щяяяьяяя         -   щяяя   -            щяяя      	   эяяя+      юяяя*         "         !   	       .         ,         $         #         !                      
               %         $             
                   
                                     
                           щяяяяяяя,   чяяя    ,   чяяящяяя#   хяяяыяяя#   щяяя   +   чяяя   +   чяяяьяяя"   хяяяэяяя"   чяяящяяя#   хяяяыяяя#   цяяяхяяя   фяяяцяяя   чяяяьяяя"   хяяяэяяя"   цяяячяяя   фяяящяяя   цяяяцяяя   фяяячяяя   цяяяфяяя   уяяяхяяя   цяяячяяя   фяяяшяяя   цяяяхяяя   уяяяцяяя      эяяя      юяяя      ьяяя      эяяя                                           ьяяя      эяяя      ыяяя       ьяяя                         юяяя                 ыяяя        ыяяя          юяяяяяяя   юяяя   ыяяя       ыяяя            яяяя      юяяяэяяя   ъяяяюяяя   яяяяяяяя   ыяяя       юяяя      ъяяя      яяяя      ыяяя      яяяяяяяя   ыяяяяяяя       яяяя   ьяяяяяяя   яяяя      ыяяя                ьяяя          яяяя    ьяяя                  юяяя	           яяяя   ьяяя                юяяя	      0            0           0           0           0            0           0           0           0            0           0           0           0  	      
   0  
      	   0           0           0  	         0        	   0     
      0           0  
         0        
   0     	      0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0            0           0           0           0            0           0           0           0            0  !       "   0  "   #   !   0  %   $   &   0  &   '   %   0  !       $   0  $   %   !   0  #   "   &   0  &   '   #   0  "       $   0  $   &   "   0  #   !   %   0  %   '   #   0  )   (   *   0  *   +   )   0  -   ,   .   0  .   /   -   0  )   (   ,   0  ,   -   )   0  +   *   .   0  .   /   +   0  *   (   ,   0  ,   .   *   0  +   )   -   0  -   /   +   0  1   0   2   0  2   3   1   0  5   4   6   0  6   7   5   0  1   0   4   0  4   5   1   0  3   2   6   0  6   7   3   0  2   0   4   0  4   6   2   0  3   1   5   0  5   7   3   0  9   8   :   0  :   ;   9   0  =   <   >   0  >   ?   =   0  9   8   <   0  <   =   9   0  ;   :   >   0  >   ?   ;   0  :   8   <   0  <   >   :   0  ;   9   =   0  =   ?   ;   0  A   @   B   0  B   C   A   0  E   D   F   0  F   G   E   0  A   @   D   0  D   E   A   0  C   B   F   0  F   G   C   0  B   @   D   0  D   F   B   0  C   A   E   0  E   G   C   0  I   H   J   0  J   K   I   0  M   L   N   0  N   O   M   0  I   H   L   0  L   M   I   0  K   J   N   0  N   O   K   0  J   H   L   0  L   N   J   0  K   I   M   0  M   O   K    0  Q   P   R    0  R   S   Q    0  U   T   V    0  V   W   U    0  Q   P   T    0  T   U   Q    0  S   R   V    0  V   W   S    0  R   P   T    0  T   V   R    0  S   Q   U    0  U   W   S   0  Y   X   Z   0  Z   [   Y   0  ]   \   ^   0  ^   _   ]   0  Y   X   \   0  \   ]   Y   0  [   Z   ^   0  ^   _   [   0  Z   X   \   0  \   ^   Z   0  [   Y   ]   0  ]   _   [   0  a   `   b   0  b   c   a   0  e   d   f   0  f   g   e   0  a   `   d   0  d   e   a   0  c   b   f   0  f   g   c   0  b   `   d   0  d   f   b   0  c   a   e   0  e   g   c    0  i   h   j    0  j   k   i    0  m   l   n    0  n   o   m    0  i   h   l    0  l   m   i    0  k   j   n    0  n   o   k    0  j   h   l    0  l   n   j    0  k   i   m    0  m   o   k           !     
      Уяяя   зяяя%      зяяя;       зяяя%   йяяязяяяУяяяйяяязяяяјяяя!       8   !       Y           8   Юяяя    јяяяЮяяя                                                                                       	      	                            	                                                    	            	            	   p   Ё   &      3   &   ьяяя3   &      -   &   ьяяя-          3       ьяяя3          -       ьяяя-   &      -   &   щяяя-   &         &   щяяя          -       щяяя-                 щяяя   %   	   +   $      *          "         !   "   	   .   !      ,         $         #          !                         
            %         $                
                   
                                     
                        #   щяяя,   "   чяяя,   )   чяяя#   '   хяяя#   !   щяяя+       чяяя+   &   чяяя"   %   хяяя"   )   чяяя#   '   хяяя#   -   цяяя   ,   фяяя   &   чяяя"   %   хяяя"   +   цяяя   )   фяяя   ,   цяяя   +   фяяя   .   цяяя   -   уяяя   +   цяяя   *   фяяя   -   цяяя   ,   уяяя   %         $         '         &         !                   $         "         '         &         *         )          $         "         &         %          *         *          !      юяяя!   яяяяюяяя*         *          !         !   яяяя   %   юяяя   $   ъяяя   "   яяяя   !   ыяяя   !   юяяя       ъяяя      яяяя      ыяяя   "   яяяя   "   ыяяя   "          "   ьяяя      яяяя      ыяяя                ьяяя   "           !   ьяяя                 юяяя    "          !   ьяяя               юяяя                                                                                                                                                         	      
      
      	                              	                  	         
                     
                  
         	                                                                                                                                                                                                                                                                                                                        !       "      "   #   !      %   $   &      &   '   %      !       $      $   %   !      #   "   &      &   '   #      "       $      $   &   "      #   !   %      %   '   #      )   (   *      *   +   )      -   ,   .      .   /   -      )   (   ,      ,   -   )      +   *   .      .   /   +      *   (   ,      ,   .   *      +   )   -      -   /   +      1   0   2      2   3   1      5   4   6      6   7   5      1   0   4      4   5   1      3   2   6      6   7   3      2   0   4      4   6   2      3   1   5      5   7   3      9   8   :      :   ;   9      =   <   >      >   ?   =      9   8   <      <   =   9      ;   :   >      >   ?   ;      :   8   <      <   >   :      ;   9   =      =   ?   ;      A   @   B      B   C   A      E   D   F      F   G   E      A   @   D      D   E   A      C   B   F      F   G   C      B   @   D      D   F   B      C   A   E      E   G   C      I   H   J      J   K   I      M   L   N      N   O   M      I   H   L      L   M   I      K   J   N      N   O   K      J   H   L      L   N   J      K   I   M      M   O   K       Q   P   R       R   S   Q       U   T   V       V   W   U       Q   P   T       T   U   Q       S   R   V       V   W   S       R   P   T       T   V   R       S   Q   U       U   W   S      Y   X   Z      Z   [   Y      ]   \   ^      ^   _   ]      Y   X   \      \   ]   Y      [   Z   ^      ^   _   [      Z   X   \      \   ^   Z      [   Y   ]      ]   _   [      a   `   b      b   c   a      e   d   f      f   g   e      a   `   d      d   e   a      c   b   f      f   g   c      b   `   d      d   f   b      c   a   e      e   g   c       i   h   j       j   k   i       m   l   n       n   o   m       i   h   l       l   m   i       k   j   n       n   o   k       j   h   l       l   n   j       k   i   m       m   o   k   p   Ё   аяяя       аяяяьяяя    аяяя      аяяяьяяя   Щяяя       Щяяяьяяя    Щяяя      Щяяяьяяя   аяяя      аяяящяяя   аяяя      аяяящяяя   Щяяя      Щяяящяяя   Щяяя      Щяяящяяя   Яяяя	      Юяяя      Щяяя      Шяяя      Эяяя	      Ыяяя      Чяяя      Цяяя      Щяяя      Шяяя      Сяяя      Пяяя
      Шяяя      Цяяя      Пяяя      Ояяя
      Ряяя   	   Пяяя
      Мяяя      Кяяя      Ряяя      Пяяя
   
   Мяяя   	   Кяяя      Эяяящяяя   Ьяяячяяя   гяяячяяя   вяяяхяяя   Ыяяящяяя   Щяяячяяя   аяяячяяя   Яяяяхяяя   гяяячяяя   вяяяхяяя   зяяяцяяя   жяяяфяяя   аяяячяяя   Яяяяхяяя   еяяяцяяя   гяяяфяяя   зяяяцяяя   еяяяфяяя   ияяяцяяя   зяяяуяяя   еяяяцяяя   дяяяфяяя   зяяяцяяя   жяяяуяяя   Юяяя      Ыяяя      Фяяя      Сяяя      Юяяя      Ыяяя      Фяяя      Сяяя      Фяяя      Сяяя      Кяяя      Зяяя      Фяяя      Сяяя      Кяяя      Зяяя      Еяяя       Еяяя      Еяяя   	   Еяяя
      Йяяя       Йяяя      Йяяя   	   Йяяя
      Ьяяяюяяя   Эяяяъяяя   Тяяяыяяя   Уяяячяяя   Ьяяяюяяя   Эяяяъяяя   Тяяяыяяя   Уяяячяяя   Тяяяыяяя   Уяяячяяя   Ияяящяяя   Йяяяхяяя   Тяяяыяяя   Уяяячяяя   Ияяящяяя   Йяяяхяяя   Еяяящяяя   Еяяяцяяя    Еяяяцяяя   Еяяятяяя	   Йяяящяяя   Йяяяцяяя    Йяяяцяяя   Йяяятяяя	   0            0           0           0           0            0           0           0           0            0           0           0           0  	      
   0  
      	   0           0           0  	         0        	   0     
      0           0  
         0        
   0     	      0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0           0            0           0           0           0            0           0           0           0            0  !       "   0  "   #   !   0  %   $   &   0  &   '   %   0  !       $   0  $   %   !   0  #   "   &   0  &   '   #   0  "       $   0  $   &   "   0  #   !   %   0  %   '   #   0  )   (   *   0  *   +   )   0  -   ,   .   0  .   /   -   0  )   (   ,   0  ,   -   )   0  +   *   .   0  .   /   +   0  *   (   ,   0  ,   .   *   0  +   )   -   0  -   /   +   0  1   0   2   0  2   3   1   0  5   4   6   0  6   7   5   0  1   0   4   0  4   5   1   0  3   2   6   0  6   7   3   0  2   0   4   0  4   6   2   0  3   1   5   0  5   7   3   0  9   8   :   0  :   ;   9   0  =   <   >   0  >   ?   =   0  9   8   <   0  <   =   9   0  ;   :   >   0  >   ?   ;   0  :   8   <   0  <   >   :   0  ;   9   =   0  =   ?   ;   
0  A   @   B   
0  B   C   A   
0  E   D   F   
0  F   G   E   
0  A   @   D   
0  D   E   A   
0  C   B   F   
0  F   G   C   
0  B   @   D   
0  D   F   B   
0  C   A   E   
0  E   G   C   0  I   H   J   0  J   K   I   0  M   L   N   0  N   O   M   0  I   H   L   0  L   M   I   0  K   J   N   0  N   O   K   0  J   H   L   0  L   N   J   0  K   I   M   0  M   O   K    0  Q   P   R    0  R   S   Q    0  U   T   V    0  V   W   U    0  Q   P   T    0  T   U   Q    0  S   R   V    0  V   W   S    0  R   P   T    0  T   V   R    0  S   Q   U    0  U   W   S   
0  Y   X   Z   
0  Z   [   Y   
0  ]   \   ^   
0  ^   _   ]   
0  Y   X   \   
0  \   ]   Y   
0  [   Z   ^   
0  ^   _   [   
0  Z   X   \   
0  \   ^   Z   
0  [   Y   ]   
0  ]   _   [   0  a   `   b   0  b   c   a   0  e   d   f   0  f   g   e   0  a   `   d   0  d   e   a   0  c   b   f   0  f   g   c   0  b   `   d   0  d   f   b   0  c   a   e   0  e   g   c    0  i   h   j    0  j   k   i    0  m   l   n    0  n   o   m    0  i   h   l    0  l   m   i    0  k   j   n    0  n   o   k    0  j   h   l    0  l   n   j    0  k   i   m    0  m   o   k           В        (   K   
       K   цяяя    -   зяяя    ыяяяЌяяяn   Ояяязяяя    Ояяя       ыяяяs   n   -          K   
      K   цяяя   -   зяяя   ыяяяЌяяя   Ояяязяяя   Ояяя      ыяяяs      -         Ояяя       Ояяя      Ояяя       Ояяязяяя    њяяяОяяя    њяяя2       Ояяя      Ояяязяяя   њяяяОяяя   њяяя2      0         	   0  	          0        
   0  
   	      0           0     
      0           0           0           0           0           0           0           0           0            0           0            0           0           0           0           0           0  	         0  
   	      0     
      0           0           0           0           0           0           0           0           0           0           0           0           0           0           0                   В        (   K   
       K   цяяя    -   зяяя    ыяяяЌяяя’яяяОяяязяяя    Ояяя       ыяяяs   ’яяя-          K   
      K   цяяя   -   зяяя   ыяяяЌяяяЈяяяОяяязяяя   Ояяя      ыяяяs   Јяяя-         Ояяя       Ояяя      Ояяя       Ояяязяяя    њяяяОяяя    њяяя2       Ояяя      Ояяязяяя   њяяяОяяя   њяяя2      0         	   0  	          0        
   0  
   	      0           0     
      0           0           0           0           0           0           0           0           0            0           0            0           0           0           0           0           0  	         0  
   	      0     
      0           0           0           0           0           0           0           0           0           0           0           0           0           0           0            