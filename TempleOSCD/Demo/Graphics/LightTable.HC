/*
There is a coarse and a fine-grained.
The coarse gets flood-filled but the
fine grained is only outlines.
*/

class Photon
{
  Photon *next,*last;
  CD3 p,v,n,p_snell_inhibit;
} p_root[mp_cnt];
I64 p_root_locks;

#define ANIMATE_JIFFIES	(JIFFY_FREQ*0.01)
I64 master_sleep_jiffy;
CTask *animate_tasks[mp_cnt];


CDC *map;
I64 photon_cnt,snell_cnt,snell_inhibit,zero_normal;
Bool full_speed,show_normals;

U8	*bmp;
F64	bmp_scale,find_normal_dist_sqr;
I64	bmp_mem,bmp_width,bmp_height,bmp_norm_radius;

I64 BmpPeek(I64 x,I64 y)
{
  if (0<=x<bmp_width && 0<=y<bmp_height) {
    if (Bt(bmp,y*bmp_width+x))
      return WHITE;
    else
      return BLACK;
  } else
    return -1;
}

U0 BmpPlot(U8 *bmp,I64 x,I64 y,I64)
{
  if (0<=x<bmp_width && 0<=y<bmp_height)
    Bts(bmp,y*bmp_width+x);
}

U0 DualLine(F64 x1,F64 y1,F64 x2,F64 y2)
{
  map->color=WHITE;
  GrLine(map,x1,y1,x2,y2);
  Line(bmp,x1*bmp_scale,y1*bmp_scale,0,x2*bmp_scale,y2*bmp_scale,0,&BmpPlot);
}

Photon *PhotonNew()
{
  I64 num=photon_cnt++%mp_cnt;
  Photon *res=CAlloc(sizeof(Photon));
  while (LBts(&p_root_locks,num))
    Yield;
  QueIns(res,p_root[num].last);
  LBtr(&p_root_locks,num);
  return res;
}

#define VECTOR 20
U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  Photon *tmpp;
  GrBlot(dc,0,0,map);
  dc->color=WHITE;
  GrPrint(dc,0,0,"Mem:0x%X %,dMeg Scale:%0.3f (%d,%d)-->(%d,%d)",
	bmp_mem,bmp_mem/1024/1024,bmp_scale,
	map->width,map->height,bmp_width,bmp_height);
  GrPrint(dc,0,FONT_HEIGHT,
	"PhotonCnt:%d SnellCnt:%d SnellInhibit:%d ZeroNormal:%d",
	photon_cnt,snell_cnt,snell_inhibit,zero_normal);
  for (i=0;i<mp_cnt;i++) {
    while (LBts(&p_root_locks,i))
      Yield;
    tmpp=p_root[i].next;
    while (tmpp!=&p_root[i]) {
      dc->color=LTRED;
      GrLine(dc,tmpp->p.x-VECTOR*tmpp->v.x,tmpp->p.y-VECTOR*tmpp->v.y,
	    tmpp->p.x,tmpp->p.y);
      if (show_normals) {
	dc->color=LTGREEN;
	GrLine(dc,tmpp->p.x,tmpp->p.y,
	      tmpp->p.x+VECTOR*tmpp->n.x,tmpp->p.y+VECTOR*tmpp->n.y);
      }
      tmpp=tmpp->next;
    }
    LBtr(&p_root_locks,i);
  }
}

U0 RayBurst(I64 x1,I64 y1,I64 x2,I64 y2)
{
  CD3 p,v,n,n2;
  I64 i;
  Photon *tmpp;
  if (x1!=x2 || y1!=y2) {
    D3Equ(&p,x2,y2);
    D3Equ(&v,x2-x1,y2-y1);
    D3Unit(&v);
    D3Equ(&n,v.y,-v.x);

    tmpp=PhotonNew;
    D3Copy(&tmpp->p,&p);
    D3Copy(&tmpp->v,&v);

    for (i=2;i<=9;i+=3) {
      D3Mul(&n2,i,&n);

      tmpp=PhotonNew;
      D3Add(&tmpp->p,&p,&n2);
      D3Copy(&tmpp->v,&v);

      tmpp=PhotonNew;
      D3Sub(&tmpp->p,&p,&n2);
      D3Copy(&tmpp->v,&v);
    }
  }
}

U0 RandomBurst()
{
  I64 i;
  F64 é;
  Photon *tmpp;
  for (i=0;i<256;i++) {
    tmpp=PhotonNew;
    D3Equ(&tmpp->p,Fs->pix_width*Rand,Fs->pix_height*Rand);
    é=2*ã*Rand;
    D3Equ(&tmpp->v,Cos(é),Sin(é));
  }
}

U0 FindNormal(Photon *tmpp)
{
  CD3 p,p1,p2;
  F64 step,x,y,é=Arg(tmpp->v.x,tmpp->v.y),è;
  I64 c;

  D3Copy(&tmpp->p_snell_inhibit,&tmpp->p);

  //Coarse grains has black and white filled-in BSplines.
  //Fine grained has only white outline without being filled-in.

  //Back-up a step and move fwd to get a fined-grained value
  //for the point of contact.
  D3SubEqu(&tmpp->p,&tmpp->v);
  D3Mul(&p,bmp_scale,&tmpp->p);
  D3Copy(&p1,&p);
  while (BmpPeek(p1.x,p1.y)==BLACK && D3DistSqr(&p,&p1)<find_normal_dist_sqr)
    D3AddEqu(&p1,&tmpp->v);
  D3Copy(&p,&p1);
  D3Div(&tmpp->p,&p,bmp_scale);

  //Draw an arc one direction, finding point of contact.
  for (step=1.0;step>=0.01;step/=4) {
    for (è=0;è<=ã/4;è+=step*ã/bmp_norm_radius) {
      x=p.x+bmp_norm_radius*Cos(é+ã-ã/4-è);
      y=p.y+bmp_norm_radius*Sin(é+ã-ã/4-è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p1;
      x=p.x+bmp_norm_radius*Cos(é+ã-ã/4+è);
      y=p.y+bmp_norm_radius*Sin(é+ã-ã/4+è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p1;
    }
    for (;è<=3*ã/4;è+=step*ã/bmp_norm_radius) {
      x=p.x+bmp_norm_radius*Cos(é+ã-ã/4-è);
      y=p.y+bmp_norm_radius*Sin(é+ã-ã/4-è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p1;
    }
  }
fn_p1:
  if (c==WHITE)
    D3Equ(&p1,x,y);
  else
    D3Copy(&p1,&tmpp->p);

    //Draw an arc other direction, finding point of contact.
  for (step=1.0;step>=0.01;step/=4) {
    for (è=0;è<=ã/4;è+=step*ã/bmp_norm_radius) {
      x=p.x+bmp_norm_radius*Cos(é+ã+ã/4+è);
      y=p.y+bmp_norm_radius*Sin(é+ã+ã/4+è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p2;
      x=p.x+bmp_norm_radius*Cos(é+ã+ã/4-è);
      y=p.y+bmp_norm_radius*Sin(é+ã+ã/4-è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p2;
    }
    for (;è<=3*ã/4;è+=step*ã/bmp_norm_radius) {
      x=p.x+bmp_norm_radius*Cos(é+ã+ã/4+è);
      y=p.y+bmp_norm_radius*Sin(é+ã+ã/4+è);
      c=BmpPeek(x,y);
      if (c!=BLACK)
	goto fn_p2;
    }
  }
fn_p2:
  if (c==WHITE)
    D3Equ(&p2,x,y);
  else
    D3Copy(&p2,&tmpp->p);

  D3Sub(&p,&p1,&p2);
  if (D3NormSqr(&p)<0.01) {
    D3Equ(&tmpp->n,Cos(é),Sin(é));
    lock {zero_normal++;}
  } else {
    D3Equ(&tmpp->n,p.y,-p.x);
    if (D3Dot(&tmpp->n,&tmpp->v)<0)
      D3Equ(&tmpp->n,-p.y,p.x);
    D3Unit(&tmpp->n);
  }
}

U0 SnellsLaw(Photon *tmpp,I64 last,I64 next)
{
//n1 and n2 are refraction index.
//n1 Sin(é1) == n2 Sin(é2)
  F64 é=Arg(tmpp->v.x,tmpp->v.y),én,n1,n2,é1,é2;
  if (last==WHITE)
    n1=1.5;
  else
    n1=1.0;
  if (next==WHITE)
    n2=1.5;
  else
    n2=1.0;
  FindNormal(tmpp);
  én=Arg(tmpp->n.x,tmpp->n.y);
  //Dot=m1m2Cos(é);
  é1=ACos(D3Dot(&tmpp->n,&tmpp->v));
  é2=ASin(n1*Sin(é1)/n2);
  if (Wrap(é-én)>=0)
    é=én+é2;
  else
    é=én-é2;
  D3Equ(&tmpp->v,Cos(é),Sin(é));
  lock {snell_cnt++;}
}

U0 AnimateTask(I64)
{
  while (TRUE) {
    master_sleep_jiffy+=ANIMATE_JIFFIES;
    if (cnts.jiffies>=master_sleep_jiffy)
      master_sleep_jiffy=cnts.jiffies+ANIMATE_JIFFIES;
    SleepUntil(master_sleep_jiffy);
  }
}

U0 MPAnimateTask(I64)
{
  I64	last_master_jiffy=0,
	timeout_jiffy=master_sleep_jiffy+ANIMATE_JIFFIES,
	last,next;
  Bool	inhibit;
  Photon *tmpp,*root=&p_root[Gs->num];
  while (TRUE) {
    while (LBts(&p_root_locks,Gs->num))
      Yield;
    tmpp=root->next;
    while (tmpp!=root) {
      last=GrPeek(map,tmpp->p.x,tmpp->p.y);
      D3AddEqu(&tmpp->p,&tmpp->v);
      if (tmpp->p.x<0) {
	tmpp->p.x=-tmpp->p.x;
	tmpp->v.x=-tmpp->v.x;
      }
      if (tmpp->p.x>=map->width) {
	tmpp->p.x-=tmpp->p.x-map->width;
	tmpp->v.x=-tmpp->v.x;
      }
      if (tmpp->p.y<0) {
	tmpp->p.y=-tmpp->p.y;
	tmpp->v.y=-tmpp->v.y;
      }
      if (tmpp->p.y>=map->height) {
	tmpp->p.y-=tmpp->p.y-map->height;
	tmpp->v.y=-tmpp->v.y;
      }
      next=GrPeek(map,tmpp->p.x,tmpp->p.y);

      if (D3DistSqr(&tmpp->p_snell_inhibit,&tmpp->p)<4.0)
	inhibit=TRUE;
      else
	inhibit=FALSE;

      if (last!=next && (last==BLACK && next==WHITE) ||
	    (last==WHITE && next==BLACK)) {
	if (inhibit)
	  lock {snell_inhibit++;}
	else
	  SnellsLaw(tmpp,last,next);
      } else if (!inhibit)
	D3Zero(&tmpp->p_snell_inhibit);

      tmpp=tmpp->next;
      if (cnts.jiffies>=timeout_jiffy)
	break;
    }
    LBtr(&p_root_locks,Gs->num);
    if (cnts.jiffies>=timeout_jiffy) {
      Sleep(1);
      timeout_jiffy=master_sleep_jiffy+ANIMATE_JIFFIES;
    }
    if (!full_speed) {
      while (master_sleep_jiffy==last_master_jiffy)
	Sleep(1);
      last_master_jiffy=master_sleep_jiffy;
      SleepUntil(master_sleep_jiffy);
      timeout_jiffy=master_sleep_jiffy+ANIMATE_JIFFIES;
    }
  }
}

U0 Init()
{
  I64 i;
  master_sleep_jiffy=cnts.jiffies;
  full_speed=show_normals=FALSE;
  photon_cnt=snell_cnt=snell_inhibit=zero_normal=0;
  map=DCNew(Fs->pix_width,Fs->pix_height);
  for (i=0;i<mp_cnt;i++) {
    while (LBts(&p_root_locks,i))
      Yield;
    QueInit(&p_root[i]);
    LBtr(&p_root_locks,i);
  }
//x*y=bmp_mem*8
  //x/y=640/480
  //x=640/480*y
  //640/480*y^2=bmp_mem*8
  //y=Sqrt(bmp_mem*8*480/640)
  //bmp_scale=Sqrt(bmp_mem*8*480/640)/480
  bmp_scale=Sqrt(bmp_mem*8*Fs->pix_height/Fs->pix_width)/Fs->pix_height;
  find_normal_dist_sqr=2*Sqr(bmp_scale);
  bmp_width =bmp_scale*Fs->pix_width;
  bmp_height=bmp_scale*Fs->pix_height;
  bmp=CAlloc(bmp_width*bmp_height/8);
  bmp_norm_radius=Min(10*bmp_scale,250);
}

U0 CleanUp()
{
  I64 i;
  for (i=0;i<mp_cnt;i++) {
    while (LBts(&p_root_locks,i))
      Yield;
    QueDel(&p_root[i],TRUE);
    LBtr(&p_root_locks,i);
  }
  DCDel(map);
  Free(bmp);
}

#define LTM_LINE	0
#define LTM_SPLINE	1
#define LTM_FLOOD_FILL	2
#define LTM_TEST_RAY	3

U0 LTMenuSet(I64 mode)
{
  CMenuEntry *entry=MenuEntryFind(Fs->cur_menu,"View/ToggleNormals");
  if (show_normals)
    entry->checked=TRUE;
  else
    entry->checked=FALSE;

  entry=MenuEntryFind(Fs->cur_menu,"Mode/LineMode");
  if (mode==LTM_LINE)
    entry->checked=TRUE;
  else
    entry->checked=FALSE;
  entry=MenuEntryFind(Fs->cur_menu,"Mode/SplineMode");
  if (mode==LTM_SPLINE)
    entry->checked=TRUE;
  else
    entry->checked=FALSE;
  entry=MenuEntryFind(Fs->cur_menu,"Mode/FloodFill");
  if (mode==LTM_FLOOD_FILL)
    entry->checked=TRUE;
  else
    entry->checked=FALSE;
  entry=MenuEntryFind(Fs->cur_menu,"Mode/TestRay");
  if (mode==LTM_TEST_RAY)
    entry->checked=TRUE;
  else
    entry->checked=FALSE;
}

#define PTS_NUM	1024
U0 LightTable()
{
  I64 msg_code,mode=LTM_LINE,i,cnt,arg1,arg2,x1,y1,x2,y2;
  CD3I32 *c=MAlloc(PTS_NUM*sizeof(CD3I32));

  p_root_locks=0;
  MenuPush(
	"File {"
	"  Restart(,'\n');"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Mode {"
	"  LineMode(,'0');"
	"  SplineMode(,'1');"
	"  FloodFill(,'2');"
	"  TestRay(,'3');"
	"}"
	"Play {"
	"  RandomBurst(,'r');"
	"  ElapseTime(,'e');"
	"}"
	"View {"
	"  ToggleNormals(,'n');"
	"}"
	);
  LTMenuSet(mode);

  MemBIOSRep;
  bmp_mem=GetI64("\n\n\nHow much memory for the high resolution\n"
	"shadow bitmap that helps improve the\n"
	"accuracy of the normal vector estimate?\n"
	"You can choose up to the largest\n"
	"contiguous chunk of physical memory.\n\n"
	"Mem (0x%0X):",1024*1024*16);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-
	WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU;
  Fs->text_attr=BLACK<<4+WHITE;	//Current $LK,"CTask",A="MN:CTask"$ is Fs segment register.
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  Init;
  Fs->draw_it=&DrawIt;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  for (i=0;i<mp_cnt;i++)
    animate_tasks[i]=Spawn(&MPAnimateTask,NULL,"MPAnimate",i);
  try {
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+1<<MSG_MS_L_UP+1<<MSG_MS_R_UP);
lt_restart:
      switch (msg_code) {
	case MSG_MS_L_UP:
	  Sweep(100,90,100);
	  x2=arg1; y2=arg2;
	  switch (mode) {
	    case LTM_FLOOD_FILL:
	      GrFloodFill(map,x2,y2);
	      mode=LTM_LINE;
	      LTMenuSet(mode);
	      break;
	    case LTM_TEST_RAY:
	      RayBurst(x1,y1,x2,y2);
	      break;
	  }
	  break;
	case MSG_MS_L_DOWN:
	  x1=arg1; y1=arg2;
	  switch (mode) {
	    case LTM_LINE:
	      map->color=ROP_XOR+WHITE;
	      while (TRUE) {
		x2=arg1; y2=arg2;
		GrLine(map,x1,y1,x2,y2);
		msg_code=GetMsg(&arg1,&arg2,
		      1<<MSG_KEY_DOWN+1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
		GrLine(map,x1,y1,x2,y2);
		if (msg_code==MSG_KEY_DOWN)
		  goto lt_restart;
		else if (msg_code==MSG_MS_L_UP) {
		  Sweep(100,90,100);
		  x2=arg1; y2=arg2;
		  break;
		}
	      }
	      DualLine(x1,y1,x2,y2);
	      break;
	    case LTM_SPLINE:
	      cnt=0;
	      map->color=ROP_XOR+WHITE;
	      do {
		c[cnt].x=arg1; c[cnt].y=arg2; c[cnt].z=0;
		Gr2BSpline(map,c,cnt+1);
		msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN+1<<MSG_MS_L_UP+
		      1<<MSG_MS_MOVE+1<<MSG_MS_R_UP);
		Gr2BSpline(map,c,cnt+1);
		if (msg_code==MSG_KEY_DOWN)
		  goto lt_restart;
		else if (msg_code==MSG_MS_L_UP) {
		  Sweep(100,90,100);
		  cnt++;
		}
	      } while (cnt<PTS_NUM-1 && msg_code!=MSG_MS_R_UP);
	      map->color=WHITE;
	      Gr2BSpline3(map,c,cnt);
	      for (i=0;i<cnt;i++) {
		c[i].x*=bmp_scale;
		c[i].y*=bmp_scale;
	      }
	      BSpline2(bmp,c,cnt,&BmpPlot);
	      mode=LTM_LINE;
	      LTMenuSet(mode);
	      break;
	  }
	  break;
	case MSG_MS_R_UP:
	  i=PopUpPickLst("Line\0Spline\0FloodFill\0TestRay\0");
	  if (i>=0) {
	    mode=i;
	    LTMenuSet(mode);
	  }
	  break;
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '\n':
	      CleanUp;
	      Init;
	      mode=LTM_LINE;
	      LTMenuSet(mode);
	      break;
	    case 'r':
	      RandomBurst;
	      break;
	    case 'e':
	      full_speed=TRUE;
	      Sleep(1500);
	      FlushMsgs;
	      full_speed=FALSE;
	      break;
	    case 'n':
	      show_normals=!show_normals;
	      LTMenuSet(mode);
	      break;
	    case '0'...'3':
	      mode=arg1-'0';
	      LTMenuSet(mode);
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto lt_done;
	  }
	  break;
      }
    }
lt_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  Free(c);
  SettingsPop;
  for (i=0;i<mp_cnt;i++)
    Kill(animate_tasks[i]);
  CleanUp;
  MenuPop;
}

LightTable;
