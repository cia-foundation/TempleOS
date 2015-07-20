




    $SP,"<1>",BI=1$



    $SP,"<2>",BI=2$







    $SP,"<3>",BI=3$




















    $SP,"<4>",BI=4$









		 $SP,"<5>",BI=5$



























		 $SP,"<6>",BI=6$












RegSetDftEntry("TempleOS/Varoom","F64 best_score=9999;\n");
RegExeBranch("TempleOS/Varoom");

F64 distance,t0,tf;
Bool game_over;

#define NUM_BUSHES	512
class Bush
{
  CD3I32 p;
  Bool sym,pad[3];
  U8 *img;
} b[NUM_BUSHES];

#define NUM_CARS	8
class Car
{
  CD3I32 p;
  F64 é,speed;
  CSprite *img;
} c[NUM_CARS];

#define TRACK_INNER	16000
#define TRACK_OUTER	18000
#define SHOULDER	200
#define SLICES		500
#define D_é		(2*ã/SLICES)
#define TRACK_POLYS	(3*SLICES)
#define DIPS		5
#define DIP_DEPTH	100
class TrackPoly
{
  CColorROPU16 c,pad[3];
  CD3I32 p[4];
} track[TRACK_POLYS];

I64 DipY(I64 x,I64 z)
{
  F64 m,a;
  R2P(&m,&a,x,z);
  return DIP_DEPTH*m*Cos(DIPS*a)/((TRACK_INNER+TRACK_OUTER)/2);
}

#define CAR_LENGTH	400

F64 Dipé(I64 x,I64 z,F64 é)
{
  F64 y_front,y_back;
  y_front=DipY(x-CAR_LENGTH/2*Cos(é),z-CAR_LENGTH/2*Sin(é));
  y_back =DipY(x+CAR_LENGTH/2*Cos(é),z+CAR_LENGTH/2*Sin(é));
  return ASin((y_front-y_back)/CAR_LENGTH);
}

U0 InitTrackPoly(TrackPoly *tempt,CColorROPU16 c,
  I64 inner,I64 outer,F64 é1,F64 é2)
{
  tempt->c=c;
  tempt->p[0].x=inner*Cos(é1);
  tempt->p[0].z=inner*Sin(é1);
  tempt->p[0].y=DipY(tempt->p[0].x,tempt->p[0].z);
  tempt->p[1].x=outer*Cos(é1);
  tempt->p[1].z=outer*Sin(é1);
  tempt->p[1].y=DipY(tempt->p[1].x,tempt->p[1].z);
  tempt->p[2].x=outer*Cos(é2);
  tempt->p[2].z=outer*Sin(é2);
  tempt->p[2].y=DipY(tempt->p[2].x,tempt->p[2].z);
  tempt->p[3].x=inner*Cos(é2);
  tempt->p[3].z=inner*Sin(é2);
  tempt->p[3].y=DipY(tempt->p[3].x,tempt->p[3].z);
}

U0 InitTrack()
{
  TrackPoly *tempt=track;
  I64 i;
  for (i=0;i<SLICES;i++) {
    InitTrackPoly(tempt++,DKGRAY,TRACK_INNER,TRACK_OUTER,
	  i*2*ã/SLICES,(i+1)*2*ã/SLICES);
    if (i&1) {
      InitTrackPoly(tempt++,RED,TRACK_INNER-SHOULDER,TRACK_INNER,
	    i*2*ã/SLICES,(i+1)*2*ã/SLICES);
      InitTrackPoly(tempt++,WHITE,TRACK_OUTER,TRACK_OUTER+SHOULDER,
	    i*2*ã/SLICES,(i+1)*2*ã/SLICES);
    } else {
      InitTrackPoly(tempt++,WHITE,TRACK_INNER-SHOULDER,TRACK_INNER,
	    i*2*ã/SLICES,(i+1)*2*ã/SLICES);
      InitTrackPoly(tempt++,RED,TRACK_OUTER,TRACK_OUTER+SHOULDER,
	    i*2*ã/SLICES,(i+1)*2*ã/SLICES);
    }
  }
}

#define HORIZON_DIP	200

I64 mp_not_done_flags;

U0 MPUpdateWin(CDC *dc2)
{
  CTask *task=dc2->win_task;
  I64 i,j,x,y,z,lo,hi,
	w=task->pix_width,
	h=task->pix_height,r[16],
	cx=w>>1;
  F64 s,dip_é=Dipé(c[0].p.x,c[0].p.z,c[0].é);
  Car  *tempc;
  Bool draw;
  CD3I32 poly[4];
  TrackPoly *tempt;
  CDC *dc=DCAlias(gr.dc2,task);

  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,ã-c[0].é);
  Mat4x4RotX(r,75*ã/180-dip_é);

  dc->depth_buf=dc2->depth_buf;

  //Track
  lo=Gs->num*TRACK_POLYS/mp_cnt;
  hi=(Gs->num+1)*TRACK_POLYS/mp_cnt;
  for (i=lo,tempt=&track[lo];i<hi;i++,tempt++) {
    dc->color=tempt->c;
    draw=TRUE;
    for (j=0;j<4;j++) {
      x=tempt->p[j].x-c[0].p.x;
      y=tempt->p[j].y-c[0].p.y;
      z=tempt->p[j].z-c[0].p.z;
      Mat4x4MulXYZ(r,&x,&y,&z);
      s=100.0/(AbsI64(z)+50);
      poly[j].y=s*y+h;
      if (z<-200 || !(-h<poly[j].y<2*h)) {
	draw=FALSE;
	break;
      }
      poly[j].x=s*x+cx;
      poly[j].z=z+GR_Z_ALL;
    }
    if (draw)
      GrFillPoly3(dc,4,poly);
  }

  dc->flags|=DCF_TRANSFORMATION;
  for (i=Gs->num;i<NUM_BUSHES;i+=mp_cnt) {
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
  for (i=Gs->num+1;i<NUM_CARS;i+=mp_cnt) {
    tempc=&c[i];
    x=tempc->p.x-c[0].p.x; y=tempc->p.y-c[0].p.y; z=tempc->p.z-c[0].p.z;
    Mat4x4MulXYZ(r,&x,&y,&z);
    if (z>0) {
      s=100.0/(AbsI64(z)+50);
      Mat4x4IdentEqu(dc->r);
      Mat4x4Scale(dc->r,s*2);
      Mat4x4RotX(dc->r,Dipé(tempc->p.x,tempc->p.z,-tempc->é));
      Mat4x4RotY(dc->r,tempc->é-c[0].é);
      DCMat4x4Set(dc,dc->r);
      Sprite3B(dc,s*x+cx,s*y+h,z+GR_Z_ALL,tempc->img);
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
  I64 i,x,y,z,
	w=task->pix_width,
	h=task->pix_height,r[16],
	cx=w>>1;
  F64 s,è,dip_é=Dipé(c[0].p.x,c[0].p.z,c[0].é);

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

  if (Bt(kbd.down_bitmap,SC_CURSOR_LEFT))
    è=-0.5;
  else if (Bt(kbd.down_bitmap,SC_CURSOR_RIGHT))
    è=0.5;
  else
    è=0;
  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,è);
  Mat4x4RotX(r,0.4-8*dip_é); //Made this up
  dc->transform=&VRTransform;
  dc->x=task->pix_width>>1;
  dc->y=task->pix_height-150;
  dc->z=GR_Z_ALL;
  SpriteMat3B(dc,0,0,-100,c[0].img,r);

  //Map
  dc->color=LTGRAY;
  GrRect(dc,w-64,h-64,64,64);
  dc->color=LTGREEN;
  for (i=0;i<NUM_BUSHES;i++)
    GrPlot(dc,w-64+64*(0x5000-b[i].p.x)/0xA000,
	  h-64*(-b[i].p.z+0x5000)/0xA000);

  dc->pen_width=2;
  for (i=0;i<NUM_CARS;i++) {
    if (i)
      dc->color=LTPURPLE;
    else
      dc->color=LTCYAN;
    GrPlot3(dc,w-64+64*(0x5000-c[i].p.x)/0xA000,
	  h-64*(-c[i].p.z+0x5000)/0xA000,0);
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
  GrPrint(dc,0,0,"é:%0.1f %0.1f%% Time:%0.2f Best:%0.2f",
	180+180/ã*Arg(c[0].p.x,c[0].p.z),
	100.0*distance/(2*ã*TRACK_INNER),s,best_score);
}

U0 AnimateTask(I64)
{
  Car  *tempc;
  I64 i;
  F64 m,a;
  while (TRUE) {
    if (!game_over)
      Snd(c[0].speed*3+10);
    else
      Snd(0);
    for (i=0;i<NUM_CARS;i++) {
      tempc=&c[i];
      tempc->p.x-=tempc->speed*Cos(tempc->é-ã/2);
      tempc->p.z+=tempc->speed*Sin(tempc->é-ã/2);
      if (i) {
	tempc->é=Arg(-tempc->p.x,tempc->p.z);
	tempc->p.x=-(TRACK_INNER+TRACK_OUTER)/2*Cos(tempc->é);
	tempc->p.z= (TRACK_INNER+TRACK_OUTER)/2*Sin(tempc->é);
      }
      tempc->p.y=DipY(tempc->p.x,tempc->p.z);
    }
    R2P(&m,&a,c[0].p.x,c[0].p.z);
    if (!game_over && TRACK_INNER<=m<=TRACK_OUTER) {
      for (i=1;i<NUM_CARS;i++)
	if (D3I32DistSqr(&c[i].p,&c[0].p)<CAR_LENGTH*CAR_LENGTH>>1) {
	  game_over=TRUE;
	  Noise(500,50,100);
	  Sleep(500);
	  break;
	}
      if (!game_over) {
	distance+=c[0].speed;
	if (distance>2*ã*TRACK_INNER && Wrap(a+ã,-ã)>0.0) {
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

CSprite *imgs[8]={$IB,"<1>",BI=1$,$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<4>",BI=4$,$IB,"<4>",BI=4$,$IB,"<4>",BI=4$};

U0 Init()
{
  Bush *tempb;
  Car  *tempc;
  I64 i;
  for (i=0;i<NUM_BUSHES;i++) {
    tempb=&b[i];
    do {
      tempb->p.x=RandU32&0x7FFF-0x4000;
      tempb->p.z=RandU32&0x7FFF-0x4000;
    } while (TRACK_INNER-SHOULDER<=
	  Sqrt(SqrI64(tempb->p.x)+SqrI64(tempb->p.z))<=TRACK_OUTER+SHOULDER);
    tempb->p.y=DipY(tempb->p.x,tempb->p.z);
    tempb->sym=RandU16&1;
    tempb->img=imgs[i&7];
  }
  InitTrack;
  for (i=0;i<NUM_CARS;i++) {
    tempc=&c[i];
    if (i==0) {
      tempc->img=$IB,"<5>",BI=5$;
      tempc->speed=0;
    } else {
      tempc->img=$IB,"<6>",BI=6$;
      tempc->speed=50.0;
    }
    tempc->é=i*2*ã/NUM_CARS;
    tempc->p.x=-(TRACK_INNER+TRACK_OUTER)/2*Cos(tempc->é);
    tempc->p.z= (TRACK_INNER+TRACK_OUTER)/2*Sin(tempc->é);
    tempc->p.y=DipY(tempc->p.x,tempc->p.z);
  }
  distance=0;
  tf=0;
  t0=tS;
  game_over=FALSE;
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
	"  Break(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"}"
	);
  SettingsPush(,TSF_SAME_SONG); //See $LK,"SettingsPush",A="MN:SettingsPush"$
  try {
    Fs->text_attr=YELLOW<<4+BLUE;
    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	  -WIF_SELF_GRAB_SCROLL-WIF_FOCUS_TASK_MENU;
    AutoComplete;
    WinMax;
    WinBorder;
    Preempt;
    Init;
    DocCursor;
    DocClear;
    Fs->draw_it=&DrawIt;
    Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
    while (TRUE)
      switch (GetKey(&sc)) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_LEFT:
	      c[0].é-=ã/100;
	      break;
	    case SC_CURSOR_RIGHT:
	      c[0].é+=ã/100;
	      break;
	    case SC_CURSOR_UP:
	      c[0].speed+=3;
	      break;
	    case SC_CURSOR_DOWN:
	      c[0].speed-=3;
	      break;
	  }
	  break;
	case '\n':
	  Init;
	  break;
	case CH_SHIFT_ESC:
	case CH_ESC:
	  goto vr_done;
      }
vr_done: //Don't goto out of try
  } catch
    Fs->catch_except=TRUE;
  SettingsPop(,TSF_SAME_SONG);
  MenuPop;
  RegWriteBranch("TempleOS/Varoom","F64 best_score=%5.4f;\n",best_score);
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