#define ZEROS_NUM	2
Complex zeros[ZEROS_NUM]={{10.0,0},{-30,0}};
#define POLES_NUM	2
Complex poles[POLES_NUM]={{-20.0,-15.0},{-20.0,15.0}};

F64 scale;

Complex *PoleZeroFind(I64 x,I64 y)
{
  I64 i;
  F64 dd,best_dd=F64_MAX;
  Complex *res=NULL;
  for (i=0;i<POLES_NUM;i++) {
    dd=Sqr(poles[i].x-x)+Sqr(poles[i].y-y);
    if (dd<best_dd) {
      best_dd=dd;
      res=&poles[i];
    }
  }
  for (i=0;i<ZEROS_NUM;i++) {
    dd=Sqr(zeros[i].x-x)+Sqr(zeros[i].y-y);
    if (dd<best_dd) {
      best_dd=dd;
      res=&zeros[i];
    }
  }
  return res;
}

F64 F(Complex *x)
{
  F64 m,a;
  Complex num,denom,n1;

  CPoly(&num,  ZEROS_NUM,zeros,x);
  CPoly(&denom,POLES_NUM,poles,x);
  CDiv(&n1,&num,&denom);

  R2P(&m,&a,n1.x,n1.y);
  if (ms.rb)
    return ã+a;
  else
    return m;
}

F64 MPDraw(CTask *task)
{
  Complex xx;
  I64 x,y,w=task->pix_width,h=task->pix_height,cx=w/2,cy=h/2,
	lo=Gs->num*h/mp_cnt,hi=(Gs->num+1)*h/mp_cnt;
  F64 yy,y_total=0;
  CDC *dc=DCAlias(,task);
  for (y=lo;y<hi;y++) {
    for (x=0;x<w;x++) {
      CEqu(&xx,x-cx,cy-y);
      yy=scale*F(&xx);
      dc->color=Clamp(yy,0,14);
      y_total+=Clamp(yy,-14,14);
      GrPlot(dc,x,y);
    }
  }
  DCDel(dc);
  return y_total;
}

U0 Draw()
{
  CJob *tmpm[MP_PROCESSORS_NUM];
  F64 y_total,old_y_total=F64_MAX;
  I64 i,w=Fs->pix_width,h=Fs->pix_height,cx=w/2,cy=h/2;
  CDC *dc=DCAlias;
  while (TRUE) {
    for (i=0;i<mp_cnt;i++)
      tmpm[i]=JobQue(&MPDraw,Fs,i,0);
    y_total=0;
    for (i=0;i<mp_cnt;i++)
      y_total+=JobResGet(tmpm[i])(F64);

    if (!y_total) break;
    scale*=7*GR_WIDTH*GR_HEIGHT/y_total;
    if (Abs(y_total-old_y_total)<1.0*GR_WIDTH*GR_HEIGHT)
      break;
    old_y_total=y_total;
  }
  dc->color=15;
  GrLine(dc,0,cy,w,cy);
  GrLine(dc,cx,0,cx,h);
  for (i=0;i<ZEROS_NUM;i++)
    GrPrint(dc,cx+zeros[i].x-FONT_WIDTH/2,cy-zeros[i].y-FONT_HEIGHT/2,"o");
  for (i=0;i<POLES_NUM;i++)
    GrPrint(dc,cx+poles[i].x-FONT_WIDTH/2,cy-poles[i].y-FONT_HEIGHT/2,"x");
  DCDel(dc);
}

U0 PoleZeros()
{
  I64 msg_code,arg1,arg2,p11,p22,cx,cy;
  Complex *tmpc=NULL;

  PopUpOk(
	"Drag the poles and zeros with left mouse.\n"
	"Hold right mouse for phase plot.\n"
	);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;
  GrPaletteSet(gr_palette_gray);
  GrPaletteColorSet(15,0xFFFF00000000); //White is red

  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  DCFill;
  scale=1.0;

  try {
    Draw;
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+
	    1<<MSG_MS_L_UP+1<<MSG_MS_R_DOWN+1<<MSG_MS_R_UP+1<<MSG_MS_MOVE);
pz_msg:
      cx=Fs->pix_width/2;
      cy=Fs->pix_height/2;
      switch (msg_code) {
	case MSG_MS_L_DOWN:
	  tmpc=PoleZeroFind(arg1-cx,cy-arg2);
	  break;
	case MSG_MS_MOVE:
	  if (tmpc) {
	    p11=arg1;p22=arg2;
//get to last mouse move
	    while (msg_code=ScanMsg(&arg1,&arg2,
		  1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+
		  1<<MSG_MS_L_UP+1<<MSG_MS_R_DOWN+
		  1<<MSG_MS_R_UP+1<<MSG_MS_MOVE))
	      if (msg_code==MSG_MS_MOVE) {
		p11=arg1;p22=arg2;
	      } else
		goto pz_msg;

	    tmpc->x=p11-cx;
	    tmpc->y=cy-p22;
	    Draw;
	  }
	  break;
	case MSG_MS_L_UP:
	  if (tmpc) {
	    tmpc->x=arg1-cx;
	    tmpc->y=cy-arg2;
	    tmpc=NULL;
	    Draw;
	  }
	  break;
	case MSG_MS_R_DOWN:
	case MSG_MS_R_UP:
	  Draw;
	  break;
	case MSG_KEY_DOWN:
	  if (arg1==CH_SHIFT_ESC||arg1==CH_ESC)
	    goto pz_done;
      }
      Refresh;
    }
pz_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  SettingsPop;
  DCFill;
}

PoleZeros;
