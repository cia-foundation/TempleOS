




    $SP,"<1>",BI=1$



    $SP,"<2>",BI=2$







    $SP,"<3>",BI=3$




















    $SP,"<4>",BI=4$









		 $SP,"<5>",BI=5$



























		 $SP,"<6>",BI=6$












RegDft("TempleOS/Varoom","F64 best_score=9999;\n");
RegExe("TempleOS/Varoom");

F64 distance,t0,tf;
Bool game_over;

#define BORDER		7500
#define RADIUS		10000
#define WIDTH		2000
#define SHOULDER	200
#define D_é		(2*ã/360)   //Curve track slice is one degree.
#define D_S		(2*ã*RADIUS/360) //Straight track is degree at 10000.
#define DIPS		5
#define DIP_DEPTH	50
class Track
{
  Track *next,*last;
  I32 num;
  CColorROPU16 c,pad;
  I64 x,z;
  F64 é,d;
  CD3I32 left[4],center[4],right[4];
} track_head,
  *track_start[MP_PROCESSORS_NUM],*track_end[MP_PROCESSORS_NUM];

CDC *track_map;
#define MAP_BITS	9
I64 t_minx,t_maxx,t_minz,t_maxz;

#define BUSHES_NUM	512
class Bush
{
  CD3I32 p;
  Bool sym,pad[3];
  U8 *img;
} b[BUSHES_NUM];

#define CARS_NUM	8
class Car
{
  CD3I32 p;
  F64 é,dé,speed;
  U8 *img;
  Track *t;
} c[CARS_NUM];

I64 DipY(I64 x,I64 z)
{
  F64 m,a;
  R2P(&m,&a,x,z);
  return DIP_DEPTH*m*Cos(DIPS*a)/RADIUS;
}

#define CAR_LENGTH	400

F64 Dipé(I64 x,I64 z,F64 é)
{
  F64 y_front,y_back;
  y_front=DipY(x-CAR_LENGTH/2*Cos(é),z-CAR_LENGTH/2*Sin(é));
  y_back =DipY(x+CAR_LENGTH/2*Cos(é),z+CAR_LENGTH/2*Sin(é));
  return ASin((y_front-y_back)/CAR_LENGTH);
}

Track *TrackFind(Track *_tmpt,I64 x,I64 z)
{
  Track *res=_tmpt,*tmpt;
  I64 dd,best=SqrI64(res->x-x)+SqrI64(res->z-z);

  tmpt=_tmpt;
  while (TRUE) {
    tmpt=tmpt->next;
    if (tmpt==&track_head)
      tmpt=tmpt->next;
    dd=SqrI64(tmpt->x-x)+SqrI64(tmpt->z-z);
    if (dd<best) {
      best=dd;
      res=tmpt;
    } else
      break;
  }
  tmpt=_tmpt;
  while (TRUE) {
    tmpt=tmpt->last;
    if (tmpt==&track_head)
      tmpt=tmpt->last;
    dd=SqrI64(tmpt->x-x)+SqrI64(tmpt->z-z);
    if (dd<best) {
      best=dd;
      res=tmpt;
    } else
      break;
  }
  return res;
}

U0 TrackSlice(F64 *_x,F64 *_z,F64 é,F64 d)
{
  F64 x=*_x,z=*_z,c=Cos(é),s=Sin(é),dx=d*s,dz=-d*c;
  Track *tmpt,*last=track_head.last;
  if (last==&track_head)
    last=NULL;

  tmpt=CAlloc(sizeof(Track));
  if (last) {
    MemCpy(&tmpt->center[0],&last->center[3],sizeof(CD3I32));
    MemCpy(&tmpt->center[1],&last->center[2],sizeof(CD3I32));
  }
  tmpt->center[2].x=x+(WIDTH/2)*c+dx;
  tmpt->center[2].z=z+(WIDTH/2)*s+dz;
  tmpt->center[2].y=DipY(tmpt->center[2].x,tmpt->center[2].z);
  tmpt->center[3].x=x-(WIDTH/2)*c+dx;
  tmpt->center[3].z=z-(WIDTH/2)*s+dz;
  tmpt->center[3].y=DipY(tmpt->center[3].x,tmpt->center[3].z);

  if (last) {
    MemCpy(&tmpt->left[0],&last->left[3],sizeof(CD3I32));
    MemCpy(&tmpt->left[1],&last->left[2],sizeof(CD3I32));
  }
  tmpt->left[2].x=x-(WIDTH/2)*c+dx;
  tmpt->left[2].z=z-(WIDTH/2)*s+dz;
  tmpt->left[2].y=DipY(tmpt->left[2].x,tmpt->left[2].z);
  tmpt->left[3].x=x-(WIDTH/2+SHOULDER)*c+dx;
  tmpt->left[3].z=z-(WIDTH/2+SHOULDER)*s+dz;
  tmpt->left[3].y=DipY(tmpt->left[3].x,tmpt->left[3].z);

  if (last) {
    MemCpy(&tmpt->right[0],&last->right[3],sizeof(CD3I32));
    MemCpy(&tmpt->right[1],&last->right[2],sizeof(CD3I32));
  }
  tmpt->right[2].x=x+(WIDTH/2+SHOULDER)*c+dx;
  tmpt->right[2].z=z+(WIDTH/2+SHOULDER)*s+dz;
  tmpt->right[2].y=DipY(tmpt->right[2].x,tmpt->right[2].z);
  tmpt->right[3].x=x+(WIDTH/2)*c+dx;
  tmpt->right[3].z=z+(WIDTH/2)*s+dz;
  tmpt->right[3].y=DipY(tmpt->right[3].x,tmpt->right[3].z);

  tmpt->x=x;
  tmpt->z=z;
  tmpt->é=é;
  tmpt->num=track_head.last->num+1;
  tmpt->d  =track_head.last->d+d;

  QueIns(tmpt,track_head.last);

  if (tmpt->num&1)
    tmpt->c=RED;
  else
    tmpt->c=WHITE;

  if (x<t_minx) t_minx=x;
  if (x>t_maxx) t_maxx=x;
  if (z<t_minz) t_minz=z;
  if (z>t_maxz) t_maxz=z;

  x+=dx;
  *_x=x;
  z+=dz;
  *_z=z;

  if (x<t_minx) t_minx=x;
  if (x>t_maxx) t_maxx=x;
  if (z<t_minz) t_minz=z;
  if (z>t_maxz) t_maxz=z;
}

U0 CoupleEnds()
{
  Track *first=track_head.next,*last=track_head.last;
  MemCpy(&first->center[0],&last->center[3],sizeof(CD3I32));
  MemCpy(&first->center[1],&last->center[2],sizeof(CD3I32));
  MemCpy(&first->left[0]  ,&last->left[3]  ,sizeof(CD3I32));
  MemCpy(&first->left[1]  ,&last->left[2]  ,sizeof(CD3I32));
  MemCpy(&first->right[0] ,&last->right[3] ,sizeof(CD3I32));
  MemCpy(&first->right[1] ,&last->right[2] ,sizeof(CD3I32));
}

U0 InitTrack()
{
  I64 i,j;
  Track *tmpt;
  F64 x,z,é,d;

  MemSet(&track_head,0,sizeof(Track));
  QueInit(&track_head);

  t_minx=t_minz=I64_MAX;
  t_maxx=t_maxz=I64_MIN;

  x=0; z=0; é=0;
  for (d=0;d<6*RADIUS;d+=D_S)
    TrackSlice(&x,&z,é,D_S);
  for (i=0;i<180;i++,é+=D_é)
    TrackSlice(&x,&z,é,D_é*RADIUS);
  for (d=0;d<RADIUS;d+=D_S)
    TrackSlice(&x,&z,é,D_S);
  for (i=0;i<90;i++,é-=D_é)
    TrackSlice(&x,&z,é,D_é*RADIUS);
  for (i=0;i<180;i++,é+=D_é)
    TrackSlice(&x,&z,é,D_é*RADIUS);
  for (i=0;i<90;i++,é-=D_é)
    TrackSlice(&x,&z,é,D_é*RADIUS);
  for (d=0;d<RADIUS;d+=D_S)
    TrackSlice(&x,&z,é,D_S);
  for (i=0;i<180;i++,é+=D_é)
    TrackSlice(&x,&z,é,D_é*RADIUS);

  CoupleEnds;

  tmpt=track_head.next;
  for (i=0;i<mp_cnt;i++) {
    j=(i+1)*track_head.last->num/mp_cnt+1;
    track_start[i]=tmpt;
    while (tmpt!=&track_head && tmpt->num!=j)
      tmpt=tmpt->next;
    track_end[i]=tmpt;
  }

  t_minx-=BORDER; t_minz-=BORDER;
  t_maxx+=BORDER; t_maxz+=BORDER;
  track_map=DCNew((t_maxx-t_minx+1<<MAP_BITS-1)>>MAP_BITS,
	(t_maxz-t_minz+1<<MAP_BITS-1)>>MAP_BITS);

  track_map->color=LTGRAY;
  GrRect(track_map,0,0,track_map->width,track_map->height);
  tmpt=track_head.next;
  track_map->color=YELLOW;
  track_map->thick=3;
  while (tmpt!=&track_head) {
    GrPlot3(track_map,track_map->width-(tmpt->x-t_minx)>>MAP_BITS,
	  (tmpt->z-t_minz)>>MAP_BITS,0);
    tmpt=tmpt->next;
  }
}

#define HORIZON_DIP	200

Bool PrepPoly(CD3I32 *p,I64 *r,I64 cx,I64 h,CD3I32 *poly)
{
  I64 x,y,z,i;
  F64 s;
  for (i=0;i<4;i++) {
    x=p[i].x-c[0].p.x;
    y=p[i].y-c[0].p.y;
    z=p[i].z-c[0].p.z;
    Mat4x4MulXYZ(r,&x,&y,&z);
    s=100.0/(AbsI64(z)+50);
    poly[i].y=s*y+h;
    if (z<-200 || !(-h<poly[i].y<2*h))
      return FALSE;
    poly[i].x=s*x+cx;
    poly[i].z=z+GR_Z_ALL;
  }
  return TRUE;
}

I64 mp_not_done_flags;

U0 MPUpdateWin(CDC *dc2)
{
  CTask *task=dc2->win_task;
  I64	i,x,y,z,
	w=task->pix_width,h=task->pix_height,r[16],cx=w>>1;
  F64	s,dip_é=Dipé(c[0].p.x,c[0].p.z,c[0].é);
  Car	*tmpc;
  CD3I32 poly[4];
  Track	*tmpt,*tmpt1;
  CDC	*dc=DCAlias(gr.dc2,task);

  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,ã-c[0].é);
  Mat4x4RotX(r,75*ã/180-dip_é);

  dc->depth_buf=dc2->depth_buf;

  //Track
  tmpt =track_start[Gs->num];
  tmpt1=track_end  [Gs->num];
  while (tmpt!=tmpt1) {
    dc->color=DKGRAY;
    if (PrepPoly(&tmpt->center,r,cx,h,poly)) {
      GrFillPoly3(dc,4,poly);
      dc->color=tmpt->c;
      if (PrepPoly(&tmpt->left,r,cx,h,poly))
	GrFillPoly3(dc,4,poly);
      if (PrepPoly(&tmpt->right,r,cx,h,poly))
	GrFillPoly3(dc,4,poly);
    }
    tmpt=tmpt->next;
  }

  dc->flags|=DCF_TRANSFORMATION;
  for (i=Gs->num;i<BUSHES_NUM;i+=mp_cnt) {
    x=b[i].p.x-c[0].p.x; y=b[i].p.y-c[0].p.y; z=b[i].p.z-c[0].p.z;
    Mat4x4MulXYZ(r,&x,&y,&z);
    if (z>0) {
      s=100.0/(AbsI64(z)+50);
      Mat4x4IdentEqu(dc->r);
      Mat4x4Scale(dc->r,s*2);
      DCMat4x4Set(dc,dc->r);
      if (b[i].sym) {
	dc->flags|=DCF_SYMMETRY|DCF_JUST_MIRROR;
	DCSymmetrySet(dc,s*x+cx,s*y+h,s*x+cx,s*y+h+10);
      }
      Sprite3B(dc,s*x+cx,s*y+h,z+GR_Z_ALL,b[i].img);
      dc->flags&=~(DCF_SYMMETRY|DCF_JUST_MIRROR);
    }
  }
  for (i=Gs->num+1;i<CARS_NUM;i+=mp_cnt) {
    tmpc=&c[i];
    x=tmpc->p.x-c[0].p.x; y=tmpc->p.y-c[0].p.y; z=tmpc->p.z-c[0].p.z;
    Mat4x4MulXYZ(r,&x,&y,&z);
    if (z>0) {
      s=100.0/(AbsI64(z)+50);
      Mat4x4IdentEqu(dc->r);
      Mat4x4Scale(dc->r,s*2);
      Mat4x4RotX(dc->r,Dipé(tmpc->p.x,tmpc->p.z,-tmpc->é));
      Mat4x4RotY(dc->r,tmpc->é-c[0].é);
      DCMat4x4Set(dc,dc->r);
      Sprite3B(dc,s*x+cx,s*y+h,z+GR_Z_ALL,tmpc->img);
    }
  }
  dc->depth_buf=NULL;
  DCDel(dc);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 VRTransform(CDC *dc,I64 *x,I64 *y,I64 *z)
{
  I64 zz;
  Mat4x4MulXYZ(dc->r,x,y,z);
  zz=400+*z;
  if (zz<1) zz=1;
  *x=400* *x/zz;
  *y=400* *y/zz;
  *x+=dc->x;
  *y+=dc->y;
  *z+=dc->z;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64	i,x,y,z,
	w=task->pix_width,
	h=task->pix_height,r[16],
	cx=w>>1;
  F64	s,dip_é=Dipé(c[0].p.x,c[0].p.z,c[0].é);
  Car	*tmpc=&c[0];

  dc->color=LTCYAN;
  GrRect(dc,0,0,w,HORIZON_DIP*Sin(dip_é)+FONT_HEIGHT*4.5);

  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,ã-c[0].é);
  Mat4x4RotX(r,75*ã/180-dip_é);

  DCDepthBufAlloc(dc);

  //Sun
  x=c[0].p.x; y=0; z=1000000-c[0].p.z;
  Mat4x4MulXYZ(r,&x,&y,&z);
  s=100.0/(AbsI64(z)+50);
  if (y<0) {
    dc->color=BROWN;
    GrCircle(dc,s*x+cx,15+HORIZON_DIP*Sin(dip_é),15);
    dc->color=YELLOW;
    GrFloodFill(dc,s*x+cx,15+HORIZON_DIP*Sin(dip_é));
  }

  mp_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    JobQue(&MPUpdateWin,dc,i);
  while (mp_not_done_flags)
    Yield;

  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,tmpc->dé);
  Mat4x4RotX(r,0.4-8*dip_é); //Made this up
  dc->transform=&VRTransform;
  dc->x=task->pix_width>>1;
  dc->y=task->pix_height-150;
  dc->z=GR_Z_ALL;
  Sprite3Mat4x4B(dc,0,0,-100,c[0].img,r);

  //Map
  GrBlot(dc,w-track_map->width,h-track_map->height,track_map);

  dc->thick=2;
  for (i=0;i<CARS_NUM;i++) {
    if (i)
      dc->color=LTPURPLE;
    else
      dc->color=LTCYAN;
    GrPlot3(dc,w-(c[i].p.x-t_minx)>>MAP_BITS,
	       h-track_map->height+(c[i].p.z-t_minz)>>MAP_BITS,0);
  }

  if (game_over) {
    dc->color=LTRED;
    if (tf) {
      s=tf-t0;
      if (Blink)
	GrPrint(dc,(w-FONT_WIDTH*14)/2,(h-FONT_HEIGHT)/2,"Game Completed");
    } else {
      s=99.9;
      if (Blink)
	GrPrint(dc,(w-FONT_WIDTH*9)/2,(h-FONT_HEIGHT)/2,"Game Over");
    }
  } else
    s=tS-t0;
  dc->color=BLACK;
  GrPrint(dc,0,0,"%0.1f%% Time:%0.2f Best:%0.2f",
	100.0*distance/track_head.last->d,s,best_score);
}

U0 AnimateTask(I64)
{
  Car  *tmpc;
  I64 i,x,z;
  Bool on_track;
  Track *tmpt,*tmpt2;
  while (TRUE) {
    if (!game_over)
      Snd(12.0*Log2(c[0].speed/500+0.7));
    else
      Snd;
    for (i=0;i<CARS_NUM;i++) {
      tmpc=&c[i];
      tmpc->p.x-=0.01*tmpc->speed*Cos(tmpc->é-ã/2);
      tmpc->p.z+=0.01*tmpc->speed*Sin(tmpc->é-ã/2);
      tmpt=TrackFind(tmpc->t,tmpc->p.x,tmpc->p.z);
      if (i) {
	if (tmpt!=tmpc->t) {
	  tmpt2=tmpt->next;
	  if (tmpt2==&track_head)
	    tmpt2=tmpt2->next;
	  tmpc->é=Arg(-tmpt2->z+tmpc->p.z,-tmpt2->x+tmpc->p.x);
	}
      } else {
	tmpc->é+=0.01*tmpc->dé;
	x=track_map->width-(tmpc->p.x-t_minx)>>MAP_BITS;
	z=(tmpc->p.z-t_minz)>>MAP_BITS;
	if (GrPeek(track_map,x,z)!=YELLOW) {
	  on_track=FALSE;
	  tmpc->speed-=0.01*tmpc->speed;
	  if (tmpc->speed<0) tmpc->speed=0;
	} else
	  on_track=TRUE;
      }
      tmpc->t=tmpt;
      tmpc->p.y=DipY(tmpc->p.x,tmpc->p.z);
    }
    if (!game_over && on_track) {
      for (i=1;i<CARS_NUM;i++)
	if (D3I32DistSqr(&c[i].p,&c[0].p)<CAR_LENGTH>>1*CAR_LENGTH>>1) {
	  game_over=TRUE;
	  Noise(500,22,34);
	  Sleep(500);
	  break;
	}
      if (!game_over) {
	distance+=0.01*c[0].speed;
	if (distance>track_head.last->d&& c[0].t->num<track_head.last->num>>1) {
	  tf=tS;
	  game_over=TRUE;
	  Beep;
	  if (tf-t0<best_score) {
	    best_score=tf-t0;
	    Beep;
	  }
	}
      }
    }
    Sleep(10);
  }
}

U8 *imgs[8]={$IB,"<1>",BI=1$,$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<4>",BI=4$,$IB,"<4>",BI=4$,$IB,"<4>",BI=4$};


U0 InitBushes()
{
  Bush *tmpb;
  I64 i,j,x,z;
  track_map->color=LTGREEN;
  track_map->thick=1;
  for (i=0;i<BUSHES_NUM;i++) {
    tmpb=&b[i];

ib_restart:
    tmpb->p.x=Rand*(t_maxx-t_minx)+t_minx;
    tmpb->p.z=Rand*(t_maxz-t_minz)+t_minz;
    x=track_map->width-(tmpb->p.x-t_minx)>>MAP_BITS;
    z=(tmpb->p.z-t_minz)>>MAP_BITS;
    for (j=0;j<8;j++)
      if (GrPeek(track_map,x+gr_x_offsets[j],z+gr_y_offsets[j])!=LTGRAY)
	goto ib_restart;

    GrPlot(track_map,x,z);
    tmpb->p.y=DipY(tmpb->p.x,tmpb->p.z);
    tmpb->sym=RandU16&1;
    tmpb->img=imgs[i&7];
  }
}


U0 Init()
{
  Car	*tmpc;
  Track *tmpt;
  F64	d;
  I64	i;

  InitTrack;
  InitBushes;
  tmpt=track_head.next;
  for (i=0;i<CARS_NUM;i++) {
    tmpc=&c[i];
    tmpc->t=tmpt;
    tmpc->p.x=tmpt->x;
    tmpc->p.z=tmpt->z;
    tmpc->p.y=DipY(tmpc->p.x,tmpc->p.z);
    tmpc->é  =-tmpt->é;
    tmpc->dé=0;
    if (!i) {
      tmpc->img=$IB,"<5>",BI=5$;
      tmpc->speed=0;
    } else {
      tmpc->img=$IB,"<6>",BI=6$;
      tmpc->speed=2500.0;
    }
    d=(i+1)*track_head.last->d/CARS_NUM;
    while (tmpt->next!=&track_head && tmpt->d<d)
      tmpt=tmpt->next;
  }
  distance=0;
  tf=0;
  t0=tS;
  game_over=FALSE;
}

U0 CleanUp()
{
  while (mp_not_done_flags)
    Yield;
  QueDel(&track_head,TRUE);
  DCDel(track_map);
}

U0 Varoom()
{
  I64 sc;
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Accelerator(,,SC_CURSOR_UP);"
	"  Brake(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"}"
	);
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  try {
    Fs->text_attr=YELLOW<<4+BLUE;
    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	  -WIF_SELF_GRAB_SCROLL-WIF_FOCUS_TASK_MENU;
    AutoComplete;
    WinBorder;
    WinMax;
    DocCursor;
    DocClear;
    Init;
    Fs->draw_it=&DrawIt;
    Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
    while (TRUE)
      switch (GetKey(&sc)) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_LEFT:
	      c[0].dé-=ã/60;
	      break;
	    case SC_CURSOR_RIGHT:
	      c[0].dé+=ã/60;
	      break;
	    case SC_CURSOR_UP:
	      c[0].speed+=300;
	      break;
	    case SC_CURSOR_DOWN:
	      c[0].speed-=900;
	      if (c[0].speed<0) c[0].speed=0;
	      break;
	  }
	  break;
	case '\n':
	  CleanUp;
	  Init;
	  break;
	case CH_SHIFT_ESC:
	case CH_ESC:
	  goto vr_done;
      }
vr_done: //Don't goto out of try
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
  RegWrite("TempleOS/Varoom","F64 best_score=%5.4f;\n",best_score);
}

Varoom;
        ö         
   þÿÿÿ    èÿÿÿ
    ôÿÿÿ   êÿÿÿ
þÿÿÿîÿÿÿúÿÿÿèÿÿÿ
    øÿÿÿúÿÿÿôÿÿÿ
   ôÿÿÿ   ìÿÿÿ
   âÿÿÿúÿÿÿèÿÿÿ
    àÿÿÿ
   ìÿÿÿ
   îÿÿÿ   àÿÿÿ
       üÿÿÿîÿÿÿ
   øÿÿÿ   âÿÿÿ
   øÿÿÿ        
    öÿÿÿúÿÿÿþÿÿÿ
   úÿÿÿüÿÿÿÜÿÿÿ
úÿÿÿðÿÿÿúÿÿÿäÿÿÿ        ã         
æÿÿÿöÿÿÿ   äÿÿÿ
ìÿÿÿ       ìÿÿÿæÿÿÿüÿÿÿ   
   äÿÿÿ   Öÿÿÿ
   Öÿÿÿ   æÿÿÿ
   æÿÿÿ"   àÿÿÿ
   êÿÿÿ$   äÿÿÿ
   ìÿÿÿ$   æÿÿÿ
äÿÿÿúÿÿÿäÿÿÿúÿÿÿ
æÿÿÿüÿÿÿæÿÿÿüÿÿÿ
îÿÿÿúÿÿÿ   êÿÿÿ
"   Üÿÿÿ&   æÿÿÿ
   äÿÿÿ   àÿÿÿ                

         þÿÿÿ
   üÿÿÿ      
   üÿÿÿ   ðÿÿÿ
   êÿÿÿ   øÿÿÿ
   øÿÿÿ   èÿÿÿ
   èÿÿÿ   þÿÿÿ
   þÿÿÿ   îÿÿÿ
   ìÿÿÿ   þÿÿÿ
   þÿÿÿ   îÿÿÿ

   öÿÿÿ    ìÿÿÿ
   êÿÿÿ   ðÿÿÿ
   ðÿÿÿ   äÿÿÿ
   âÿÿÿ   ðÿÿÿ
   ðÿÿÿ   êÿÿÿ
   êÿÿÿ   úÿÿÿ
           öÿÿÿ        ”        
   úÿÿÿ    Äÿÿÿ
úÿÿÿÄÿÿÿøÿÿÿúÿÿÿ
üÿÿÿÂÿÿÿ    üÿÿÿ
úÿÿÿÄÿÿÿæÿÿÿ¤ÿÿÿ
üÿÿÿÂÿÿÿäÿÿÿ¤ÿÿÿ
øÿÿÿ¼ÿÿÿúÿÿÿ†ÿÿÿ
   Äÿÿÿüÿÿÿ„ÿÿÿ
þÿÿÿºÿÿÿ
   Œÿÿÿ

   Œÿÿÿüÿÿÿ¸ÿÿÿ
   ¦ÿÿÿ   šÿÿÿ
   šÿÿÿ   zÿÿÿ
   zÿÿÿ   „ÿÿÿ
   †ÿÿÿ    žÿÿÿ
    žÿÿÿ   vÿÿÿ
   vÿÿÿþÿÿÿ`ÿÿÿ
þÿÿÿ`ÿÿÿìÿÿÿ„ÿÿÿ
ìÿÿÿ„ÿÿÿöÿÿÿ¬ÿÿÿ
öÿÿÿ¬ÿÿÿâÿÿÿžÿÿÿ
âÿÿÿžÿÿÿØÿÿÿ¨ÿÿÿ
Øÿÿÿ¨ÿÿÿîÿÿÿºÿÿÿ
îÿÿÿºÿÿÿòÿÿÿºÿÿÿ
ôÿÿÿ¸ÿÿÿúÿÿÿ¸ÿÿÿ

   –ÿÿÿ   ˆÿÿÿ
   „ÿÿÿ   Šÿÿÿ
þÿÿÿŽÿÿÿþÿÿÿjÿÿÿ
þÿÿÿjÿÿÿôÿÿÿ‚ÿÿÿ
ôÿÿÿ‚ÿÿÿøÿÿÿœÿÿÿ
ôÿÿÿ´ÿÿÿîÿÿÿ¦ÿÿÿ
èÿÿÿ²ÿÿÿÞÿÿÿ¦ÿÿÿ

   œÿÿÿ   –ÿÿÿ
øÿÿÿ€ÿÿÿ    hÿÿÿ
   ˆÿÿÿ   €ÿÿÿ
þÿÿÿ~ÿÿÿ   nÿÿÿ
    ~ÿÿÿ   ’ÿÿÿ

   œÿÿÿ   ¨ÿÿÿ
öÿÿÿ²ÿÿÿðÿÿÿ‚ÿÿÿ
ðÿÿÿ‚ÿÿÿúÿÿÿjÿÿÿ
êÿÿÿ´ÿÿÿÞÿÿÿ ÿÿÿ        J     `   Œ   F   çÿÿÿ}   -   çÿÿÿ}   -   çÿÿÿK   F   çÿÿÿK   7   ýÿÿÿ‡   7   ýÿÿÿ–       ýÿÿÿ–   (   ýÿÿÿ‡   (   ýÿÿÿA   7   ýÿÿÿA   7   ýÿÿÿ        ýÿÿÿ    F      }   -      }   -      K   F      K   7      ‡   7      –          –   (      ‡   (      A   7      A   7                     ºÿÿÿçÿÿÿ}   Óÿÿÿçÿÿÿ}   ÓÿÿÿçÿÿÿK   ºÿÿÿçÿÿÿK   Éÿÿÿýÿÿÿ‡   Éÿÿÿýÿÿÿ–       ýÿÿÿ–   Øÿÿÿýÿÿÿ‡   ØÿÿÿýÿÿÿA   ÉÿÿÿýÿÿÿA   Éÿÿÿýÿÿÿ        ýÿÿÿ    ºÿÿÿ   }   Óÿÿÿ   }   Óÿÿÿ   K   ºÿÿÿ   K   Éÿÿÿ   ‡   Éÿÿÿ   –          –   Øÿÿÿ   ‡   Øÿÿÿ   A   Éÿÿÿ   A   Éÿÿÿ                  F   çÿÿÿƒÿÿÿ-   çÿÿÿƒÿÿÿ-   çÿÿÿµÿÿÿF   çÿÿÿµÿÿÿ7   áÿÿÿyÿÿÿ7   áÿÿÿjÿÿÿ    áÿÿÿjÿÿÿ(   áÿÿÿyÿÿÿ(   áÿÿÿ¿ÿÿÿ7   áÿÿÿ¿ÿÿÿ7   ýÿÿÿ        ýÿÿÿ    F      ƒÿÿÿ-      ƒÿÿÿ-      µÿÿÿF      µÿÿÿ7      yÿÿÿ7      jÿÿÿ       jÿÿÿ(      yÿÿÿ(      ¿ÿÿÿ7      ¿ÿÿÿ7                     ºÿÿÿçÿÿÿƒÿÿÿÓÿÿÿçÿÿÿƒÿÿÿÓÿÿÿçÿÿÿµÿÿÿºÿÿÿçÿÿÿµÿÿÿÉÿÿÿáÿÿÿyÿÿÿÉÿÿÿáÿÿÿjÿÿÿ    áÿÿÿjÿÿÿØÿÿÿáÿÿÿyÿÿÿØÿÿÿáÿÿÿ¿ÿÿÿÉÿÿÿáÿÿÿ¿ÿÿÿÉÿÿÿýÿÿÿ        ýÿÿÿ    ºÿÿÿ   ƒÿÿÿÓÿÿÿ   ƒÿÿÿÓÿÿÿ   µÿÿÿºÿÿÿ   µÿÿÿÉÿÿÿ   yÿÿÿÉÿÿÿ   jÿÿÿ       jÿÿÿØÿÿÿ   yÿÿÿØÿÿÿ   ¿ÿÿÿÉÿÿÿ   ¿ÿÿÿÉÿÿÿ                                                                                                                                                                                       
   	      	                                                                                                                                                                                    	               
   	                                                       $   %   &       &   '   $                                    %   $       $             $   '                $          %          %   &          &                '   &      #   !   "      !   #          #                                                     /   -   .      -   /   ,      /   *   ,      ,   *   +      +   )   (      +   *   )         +   (      +                +          +      (      !   .   -      .   !   "          +          +       ,         *   )      *             <   =   >       >   ?   <       1   0   3       3   2   1       1   =   <       <   0   1       <   ?   3       3   0   <       1   =   2       =   >   2       >   2   3       3   ?   >      ;   9   :      9   ;   8      8   6   7      7   5   4      7   6   5      G   E   F      E   G   D      G   B   D      D   B   C      C   A   @      C   B   A      4   C   @      C   4   7      4   C   7      C   4   @      9   F   E      F   9   :      8   C   7      C   8   D      5   B   A      B   5   6       T   U   V       V   W   T       I   H   K       K   J   I       I   U   T       T   H   I       T   W   K       K   H   T       I   U   J       U   V   J       V   J   K       K   W   V      S   R   Q      Q   P   S      P   O   N      O   L   M      O   M   N      _   ^   ]      ]   \   _      _   \   Z      \   [   Z      [   X   Y      [   Y   Z      L   X   [      [   O   L      L   O   [      [   X   L      Q   ]   ^      ^   R   Q      P   O   [      [   \   P      M   Y   Z      Z   N   M         P   8      8   P   6      5   4   @      @   A   5      L   M   Y      Y   X   L           J     `   Œ   F   çÿÿÿ}   -   çÿÿÿ}   -   çÿÿÿK   F   çÿÿÿK   7   ýÿÿÿ‡   7   ýÿÿÿ–       ýÿÿÿ–   (   ýÿÿÿ‡   (   ýÿÿÿA   7   ýÿÿÿA   7   ýÿÿÿ        ýÿÿÿ    F      }   -      }   -      K   F      K   7      ‡   7      –          –   (      ‡   (      A   7      A   7                     ºÿÿÿçÿÿÿ}   Óÿÿÿçÿÿÿ}   ÓÿÿÿçÿÿÿK   ºÿÿÿçÿÿÿK   Éÿÿÿýÿÿÿ‡   Éÿÿÿýÿÿÿ–       ýÿÿÿ–   Øÿÿÿýÿÿÿ‡   ØÿÿÿýÿÿÿA   ÉÿÿÿýÿÿÿA   Éÿÿÿýÿÿÿ        ýÿÿÿ    ºÿÿÿ   }   Óÿÿÿ   }   Óÿÿÿ   K   ºÿÿÿ   K   Éÿÿÿ   ‡   Éÿÿÿ   –          –   Øÿÿÿ   ‡   Øÿÿÿ   A   Éÿÿÿ   A   Éÿÿÿ                  F   çÿÿÿƒÿÿÿ-   çÿÿÿƒÿÿÿ-   çÿÿÿµÿÿÿF   çÿÿÿµÿÿÿ7   áÿÿÿyÿÿÿ7   áÿÿÿjÿÿÿ    áÿÿÿjÿÿÿ(   áÿÿÿyÿÿÿ(   áÿÿÿ¿ÿÿÿ7   áÿÿÿ¿ÿÿÿ7   ýÿÿÿ        ýÿÿÿ    F      ƒÿÿÿ-      ƒÿÿÿ-      µÿÿÿF      µÿÿÿ7      yÿÿÿ7      jÿÿÿ       jÿÿÿ(      yÿÿÿ(      ¿ÿÿÿ7      ¿ÿÿÿ7                     ºÿÿÿçÿÿÿƒÿÿÿÓÿÿÿçÿÿÿƒÿÿÿÓÿÿÿçÿÿÿµÿÿÿºÿÿÿçÿÿÿµÿÿÿÉÿÿÿáÿÿÿyÿÿÿÉÿÿÿáÿÿÿjÿÿÿ    áÿÿÿjÿÿÿØÿÿÿáÿÿÿyÿÿÿØÿÿÿáÿÿÿ¿ÿÿÿÉÿÿÿáÿÿÿ¿ÿÿÿÉÿÿÿýÿÿÿ        ýÿÿÿ    ºÿÿÿ   ƒÿÿÿÓÿÿÿ   ƒÿÿÿÓÿÿÿ   µÿÿÿºÿÿÿ   µÿÿÿÉÿÿÿ   yÿÿÿÉÿÿÿ   jÿÿÿ       jÿÿÿØÿÿÿ   yÿÿÿØÿÿÿ   ¿ÿÿÿÉÿÿÿ   ¿ÿÿÿÉÿÿÿ                                                                                                                                                                                       
   	      	                                                                                                                                                                                    	               
   	                                                       $   %   &       &   '   $                                    %   $       $             $   '                $          %          %   &          &                '   &      #   !   "      !   #          #                                                     /   -   .      -   /   ,      /   *   ,      ,   *   +      +   )   (      +   *   )         +   (      +                +          +      (      !   .   -      .   !   "          +          +       ,         *   )      *             <   =   >       >   ?   <       1   0   3       3   2   1       1   =   <       <   0   1       <   ?   3       3   0   <       1   =   2       =   >   2       >   2   3       3   ?   >      ;   9   :      9   ;   8      8   6   7      7   5   4      7   6   5      G   E   F      E   G   D      G   B   D      D   B   C      C   A   @      C   B   A      4   C   @      C   4   7      4   C   7      C   4   @      9   F   E      F   9   :      8   C   7      C   8   D      5   B   A      B   5   6       T   U   V       V   W   T       I   H   K       K   J   I       I   U   T       T   H   I       T   W   K       K   H   T       I   U   J       U   V   J       V   J   K       K   W   V      S   R   Q      Q   P   S      P   O   N      O   L   M      O   M   N      _   ^   ]      ]   \   _      _   \   Z      \   [   Z      [   X   Y      [   Y   Z      L   X   [      [   O   L      L   O   [      [   X   L      Q   ]   ^      ^   R   Q      P   O   [      [   \   P      M   Y   Z      Z   N   M         P   8      8   P   6      5   4   @      @   A   5      L   M   Y      Y   X   L    