class Photon
{
  Photon *next,*last;
  CD3 p,v;
} p_root;

CDC *map;
F64 ts;

#define TAIL 20
U0 DrawIt(CTask *,CDC *dc)
{
  Photon *tmpp;
  GrBlot(dc,0,0,map);
  tmpp=p_root.next;
  while (tmpp!=&p_root) {
    dc->color=LTRED;
    GrLine(dc,tmpp->p.x-TAIL*tmpp->v.x,tmpp->p.y-TAIL*tmpp->v.y,
	  tmpp->p.x,tmpp->p.y);
    tmpp=tmpp->next;
  }
}

U0 RandomBurst()
{
  I64 i;
  F64 é;
  Photon *tmpp;
  for (i=0;i<1000;i++) {
    tmpp=CAlloc(sizeof(Photon));
    D3Equ(&tmpp->p,Fs->pix_width*Rand,Fs->pix_height*Rand);
    é=2*ã*Rand;
    D3Equ(&tmpp->v,Cos(é),Sin(é));
    QueIns(tmpp,p_root.last);
  }
}

#define HEIGHT	10
U0 FindNormal(CDC *dc,Photon *tmpp,I64 last,I64,CD3 *_normal)
{
  CD3 p1,p2,v,s;
  F64 é=Arg(tmpp->v.x,tmpp->v.y),è;

  for (è=0;è<ã/2;è+=ã/HEIGHT) {
    D3Equ(&s,Cos(é+ã-ã/4-è),Sin(é+ã-ã/4-è));
    D3Mul(&v,HEIGHT,&s);
    D3Add(&p1,&tmpp->p,&v);
    if (GrPeek(dc,p1.x,p1.y)!=last)
      break;
  }
  if (GrPeek(dc,p1.x,p1.y)==last)
    D3Copy(&p1,&tmpp->p);

  for (è=0;è<ã/2;è+=ã/HEIGHT) {
    D3Equ(&s,Cos(é+ã+ã/4+è),Sin(é+ã+ã/4+è));
    D3Mul(&v,HEIGHT,&s);
    D3Add(&p2,&tmpp->p,&v);
    if (GrPeek(dc,p2.x,p2.y)!=last)
      break;
  }
  if (GrPeek(dc,p2.x,p2.y)==last)
    D3Copy(&p2,&tmpp->p);
 
  D3Sub(&s,&p1,&p2);
  D3Equ(_normal,s.y,-s.x);
  if (D3Dot(_normal,&tmpp->v)<0)
    D3Equ(_normal,-s.y,s.x);

  D3Unit(_normal);
}

U0 SnellsLaw(CDC *dc,Photon *tmpp,I64 last,I64 next)
{
//n1 and n2 are refraction index.
//n1 Sin(é1) == n2 Sin(é2)
  F64 é=Arg(tmpp->v.x,tmpp->v.y),én,n1,n2,é1,é2,éa,éb;
  CD3 normal;
  if (last==WHITE)
    n1=1.5;
  else
    n1=1.0;
  if (next==WHITE)
    n2=1.5;
  else
    n2=1.0;
  FindNormal(dc,tmpp,last,next,&normal);
  én=Arg(normal.x,normal.y);
  //dot=m1m2Cos(é);
  é1=ACos(D3Dot(&normal,&tmpp->v));
  é2=ASin(n1*Sin(é1)/n2);
  //é --> (én+é1)
  //é <-- (én+é2)
  éa=én+é2;
  éb=én-é2;
  if (Abs(Wrap(éa-é))<Abs(Wrap(éb-é)))
    é=éa;
  else
    é=éb;
  D3Equ(&tmpp->v,Cos(é),Sin(é));
}

U0 AnimateTask(I64)
{
  Photon *tmpp;
  I64 last,next;
  while (TRUE) {
    tmpp=p_root.next;
    while (tmpp!=&p_root) {
      last=GrPeek(map,tmpp->p.x,tmpp->p.y);
      D3AddEqu(&tmpp->p,&tmpp->v);
      if (tmpp->p.x<=0)		tmpp->v.x=-tmpp->v.x;
      if (tmpp->p.x>=map->width)	tmpp->v.x=-tmpp->v.x;
      if (tmpp->p.y<=0)		tmpp->v.y=-tmpp->v.y;
      if (tmpp->p.y>=map->height) tmpp->v.y=-tmpp->v.y;
      next=GrPeek(map,tmpp->p.x,tmpp->p.y);
      if (last!=next &&
	    (last==BLACK && next==WHITE) ||
	    (last==WHITE       && next==BLACK))
	SnellsLaw(map,tmpp,last,next);
      tmpp=tmpp->next;
    }
    Sleep(1000*ts);
  }
}

U0 Init()
{
  ts=0.01;
  map=DCNew(Fs->pix_width,Fs->pix_height);
  QueInit(&p_root);
}

U0 CleanUp()
{
  QueDel(&p_root,TRUE);
  DCDel(map);
}

#define PTS_NUM	1024
U0 LightTable()
{
  I64 msg_code,i,arg1,arg2,ray_x1,ray_y1,ray_x2,ray_y2;
  CD3I32 *c=MAlloc(PTS_NUM*sizeof(CD3I32));
  Photon *tmpp;

  MenuPush(
	"File {"
	"  Restart(,'\n');"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  Fill(,CH_SPACE);"
	"}"
	"Play {"
	"  RandomBurst(,'b');"
	"  ElapseTime(,'e');"
	"}"
	);
  PopUpOk(
	"Draw splines with $$GREEN$${Left-click}$$FG$$.\n\n"
	"Fill lens with $$GREEN$$<SPACE>$$FG$$.\n\n"
	"$$GREEN$${Right-drag}$$FG$$ to fire rays.\n"
	);
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
  try {
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+1<<MSG_MS_R_DOWN+1<<MSG_MS_R_UP);
lt_restart:
      switch (msg_code) {
	case MSG_MS_R_DOWN:
	  ray_x1=arg1; ray_y1=arg2;
	  break;
	case MSG_MS_R_UP:
	  ray_x2=arg1; ray_y2=arg2;
	  tmpp=CAlloc(sizeof(Photon));
	  tmpp->p.x=ray_x2; tmpp->p.y=ray_y2;
	  tmpp->v.x=ray_x2-ray_x1; tmpp->v.y=ray_y2-ray_y1;
	  D3Unit(&tmpp->v);
	  QueIns(tmpp,p_root.last);
	  break;
	case MSG_MS_L_DOWN:
	  i=0;
	  map->color=ROP_XOR+WHITE;
	  do {
	    c[i].x=arg1; c[i].y=arg2; c[i].z=0;
	    Gr2BSpline(map,c,i+1);
	    msg_code=GetMsg(&arg1,&arg2,
		  1<<MSG_KEY_DOWN+1<<MSG_MS_L_UP+1<<MSG_MS_MOVE+1<<MSG_MS_R_UP);
	    Gr2BSpline(map,c,i+1);
	    if (msg_code==MSG_KEY_DOWN)
	      goto lt_restart;
	    else if (msg_code==MSG_MS_L_UP) {
	      Noise(100,30,50);
	      i++;
	    }
	  } while (i<PTS_NUM-1 && msg_code!=MSG_MS_R_UP);
	  map->pen_width=2;
	  Gr2BSpline3(map,c,i);
	  map->pen_width=1;
	  break;
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case CH_SPACE:
	      GrFloodFill(map,ms.pos.x-Fs->pix_left,ms.pos.y-Fs->pix_top);
	      break;
	    case '\n':
	      DCFill;
	      CleanUp;
	      Init;
	      break;
	    case 'r':
	      RandomBurst;
	      break;
	    case 'e':
	      ts=0.00;
	      Sleep(2500);
	      ts=0.01;
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
  DCFill;
  Free(c);
  SettingsPop;
  CleanUp;
  MenuPop;
}

LightTable;
